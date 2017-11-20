// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#ifndef Tempus_AdjointSensitivityModelEvaluator_impl_hpp
#define Tempus_AdjointSensitivityModelEvaluator_impl_hpp

#include "Thyra_DefaultMultiVectorProductVectorSpace.hpp"
#include "Thyra_DefaultMultiVectorProductVector.hpp"
#include "Thyra_MultiVectorLinearOp.hpp"
#include "Thyra_MultiVectorLinearOpWithSolveFactory.hpp"
#include "Thyra_DefaultScaledAdjointLinearOp.hpp"
#include "Thyra_DefaultAdjointLinearOpWithSolve.hpp"
#include "Thyra_AdjointLinearOpWithSolveFactory.hpp"
#include "Thyra_VectorStdOps.hpp"
#include "Thyra_MultiVectorStdOps.hpp"

namespace Tempus {

template <typename Scalar>
AdjointSensitivityModelEvaluator<Scalar>::
AdjointSensitivityModelEvaluator(
  const Teuchos::RCP<const Thyra::ModelEvaluator<Scalar> > & model,
  const Scalar& t_final,
  const Teuchos::RCP<const Teuchos::ParameterList>& pList) :
  model_(model),
  t_final_(t_final),
  mass_matrix_is_computed_(false),
  t_interp_(Teuchos::ScalarTraits<Scalar>::rmax())
{
  typedef Thyra::ModelEvaluatorBase MEB;

  // Set parameters
  Teuchos::RCP<Teuchos::ParameterList> pl =
    Teuchos::rcp(new Teuchos::ParameterList);
  if (pList != Teuchos::null)
    *pl = *pList;
  pl->validateParametersAndSetDefaults(*this->getValidParameters());
  mass_matrix_is_constant_ = pl->get<bool>("Mass Matrix Is Constant");
  mass_matrix_is_identity_ = pl->get<bool>("Mass Matrix Is Identity");
  p_index_ = pl->get<int>("Sensitivity Parameter Index", 0);
  g_index_ = pl->get<int>("Response Function Index", 0);
  num_adjoint_ = model_->get_g_space(g_index_)->dim();

  // We currently do not support a non-constant mass matrix
  TEUCHOS_TEST_FOR_EXCEPTION(
    mass_matrix_is_constant_ == false, std::logic_error,
    "AdjointSensitivityModelEvaluator currently does not support " <<
    "non-constant mass matrix df/dx_dot!");

  adjoint_space_ =
    Thyra::multiVectorProductVectorSpace(model_->get_f_space(), num_adjoint_);
  residual_space_ =
    Thyra::multiVectorProductVectorSpace(model_->get_x_space(), num_adjoint_);
  response_space_=
    Thyra::multiVectorProductVectorSpace(model_->get_p_space(p_index_),
                                         num_adjoint_);

  MEB::InArgs<Scalar> me_inArgs = model_->createInArgs();
  MEB::InArgsSetup<Scalar> inArgs;
  inArgs.setModelEvalDescription(this->description());
  inArgs.setSupports(MEB::IN_ARG_x);
  inArgs.setSupports(MEB::IN_ARG_t);
  if (me_inArgs.supports(MEB::IN_ARG_x_dot))
    inArgs.setSupports(MEB::IN_ARG_x_dot);
  if (me_inArgs.supports(MEB::IN_ARG_alpha))
    inArgs.setSupports(MEB::IN_ARG_alpha);
  if (me_inArgs.supports(MEB::IN_ARG_beta))
    inArgs.setSupports(MEB::IN_ARG_beta);

  // Support additional parameters for x and xdot
  inArgs.set_Np(me_inArgs.Np());
  prototypeInArgs_ = inArgs;

  MEB::OutArgs<Scalar> me_outArgs = model_->createOutArgs();
  MEB::OutArgsSetup<Scalar> outArgs;
  outArgs.setModelEvalDescription(this->description());
  outArgs.set_Np_Ng(me_inArgs.Np(),1);
  outArgs.setSupports(MEB::OUT_ARG_f);
  outArgs.setSupports(MEB::OUT_ARG_W_op);
  prototypeOutArgs_ = outArgs;

  // ME must support W_op to define adjoint ODE/DAE.
  // Must support alpha, beta if it suports x_dot
  TEUCHOS_ASSERT(me_inArgs.supports(MEB::IN_ARG_x));
  TEUCHOS_ASSERT(me_outArgs.supports(MEB::OUT_ARG_W_op));
  if (me_inArgs.supports(MEB::IN_ARG_x_dot)) {
    TEUCHOS_ASSERT(me_inArgs.supports(MEB::IN_ARG_alpha));
    TEUCHOS_ASSERT(me_inArgs.supports(MEB::IN_ARG_beta));
  }
}

template <typename Scalar>
void
AdjointSensitivityModelEvaluator<Scalar>::
setForwardSolutionHistory(
  const Teuchos::RCP<const Tempus::SolutionHistory<Scalar> >& sh)
{
  sh_ = sh;
  t_interp_ = Teuchos::ScalarTraits<Scalar>::rmax();
  forward_state_ = Teuchos::null;
}

template <typename Scalar>
Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
get_p_space(int p) const
{
  TEUCHOS_ASSERT(p < model_->Np());
  return model_->get_p_space(p);
}

template <typename Scalar>
Teuchos::RCP<const Teuchos::Array<std::string> >
AdjointSensitivityModelEvaluator<Scalar>::
get_p_names(int p) const
{
  TEUCHOS_ASSERT(p < model_->Np());
  return model_->get_p_names(p);
}

template <typename Scalar>
Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
get_x_space() const
{
  return adjoint_space_;
}

template <typename Scalar>
Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
get_f_space() const
{
  return residual_space_;
}

template <typename Scalar>
Teuchos::RCP<const Thyra::VectorSpaceBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
get_g_space(int j) const
{
  TEUCHOS_ASSERT(j == 0);
  return response_space_;
}

template <typename Scalar>
Teuchos::RCP<Thyra::LinearOpBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
create_W_op() const
{
  Teuchos::RCP<Thyra::LinearOpBase<Scalar> > op = model_->create_W_op();
  return Thyra::nonconstMultiVectorLinearOp( Thyra::nonconstAdjoint(op),
                                             num_adjoint_);
}

template <typename Scalar>
Teuchos::RCP<const Thyra::LinearOpWithSolveFactoryBase<Scalar> >
AdjointSensitivityModelEvaluator<Scalar>::
get_W_factory() const
{
  using Teuchos::RCP;
  using Teuchos::rcp_dynamic_cast;

  RCP<const Thyra::LinearOpWithSolveFactoryBase<Scalar> > factory =
    model_->get_W_factory();
  RCP<const Thyra::LinearOpWithSolveFactoryBase<Scalar> > alowsfb =
    Thyra::adjointLinearOpWithSolveFactory(factory);
  RCP<Thyra::LinearOpBase<Scalar> > mv_op = this->create_W_op();
  RCP<const DMVPVS> mv_domain = rcp_dynamic_cast<const DMVPVS>(mv_op->domain());
  RCP<const DMVPVS> mv_range = rcp_dynamic_cast<const DMVPVS>(mv_op->range());
  return Thyra::multiVectorLinearOpWithSolveFactory(
    alowsfb, mv_range, mv_domain);
}

template <typename Scalar>
Thyra::ModelEvaluatorBase::InArgs<Scalar>
AdjointSensitivityModelEvaluator<Scalar>::
createInArgs() const
{
  return prototypeInArgs_;
}

template <typename Scalar>
Thyra::ModelEvaluatorBase::InArgs<Scalar>
AdjointSensitivityModelEvaluator<Scalar>::
getNominalValues() const
{
  typedef Thyra::ModelEvaluatorBase MEB;
  using Teuchos::RCP;
  using Teuchos::rcp_dynamic_cast;

  MEB::InArgs<Scalar> me_nominal = model_->getNominalValues();
  MEB::InArgs<Scalar> nominal = this->createInArgs();

  const Scalar zero = Teuchos::ScalarTraits<Scalar>::zero();

  // Set initial x, x_dot
  RCP< Thyra::VectorBase<Scalar> > x = Thyra::createMember(*adjoint_space_);
  Thyra::assign(x.ptr(), zero);
  nominal.set_x(x);

  if (me_nominal.supports(MEB::IN_ARG_x_dot)) {
    RCP< Thyra::VectorBase<Scalar> > x_dot =
      Thyra::createMember(*adjoint_space_);
    Thyra::assign(x_dot.ptr(), zero);
    nominal.set_x_dot(x_dot);
  }

  const int np = model_->Np();
  for (int i=0; i<np; ++i)
    nominal.set_p(i, me_nominal.get_p(i));

  return nominal;
}

template <typename Scalar>
Thyra::ModelEvaluatorBase::OutArgs<Scalar>
AdjointSensitivityModelEvaluator<Scalar>::
createOutArgsImpl() const
{
  return prototypeOutArgs_;
}

template <typename Scalar>
void
AdjointSensitivityModelEvaluator<Scalar>::
evalModelImpl(const Thyra::ModelEvaluatorBase::InArgs<Scalar> &inArgs,
              const Thyra::ModelEvaluatorBase::OutArgs<Scalar> &outArgs) const
{
  typedef Thyra::ModelEvaluatorBase MEB;
  using Teuchos::RCP;
  using Teuchos::rcp_dynamic_cast;

  // Interpolate forward solution at supplied time, reusing previous
  // interpolation if possible
  TEUCHOS_ASSERT(sh_ != Teuchos::null);
  const Scalar t = inArgs.get_t();
  const Scalar forward_t = t_final_ - t;
  if (forward_state_ == Teuchos::null || t_interp_ != t) {
    if (forward_state_ == Teuchos::null)
      forward_state_ = sh_->interpolateState(forward_t);
    else
      sh_->interpolateState(forward_t, forward_state_.get());
    t_interp_ = t;
  }

  // setup input arguments for model
  MEB::InArgs<Scalar> me_inArgs = model_->getNominalValues();
  me_inArgs.set_x(forward_state_->getX());
  if (me_inArgs.supports(MEB::IN_ARG_x_dot) &&
      inArgs.get_x_dot() != Teuchos::null)
    me_inArgs.set_x_dot(forward_state_->getXDot());
  if (me_inArgs.supports(MEB::IN_ARG_t))
    me_inArgs.set_t(forward_t);
  const int np = me_inArgs.Np();
  for (int i=0; i<np; ++i)
    me_inArgs.set_p(i, inArgs.get_p(i));

  // compute adjoint W == model W
  RCP<Thyra::LinearOpBase<Scalar> > op = outArgs.get_W_op();
  if (op != Teuchos::null) {
    if (me_inArgs.supports(MEB::IN_ARG_alpha))
      me_inArgs.set_alpha(inArgs.get_alpha());
    if (me_inArgs.supports(MEB::IN_ARG_beta))
      me_inArgs.set_beta(inArgs.get_beta());

    RCP<Thyra::MultiVectorLinearOp<Scalar> > mv_adjoint_op =
      rcp_dynamic_cast<Thyra::MultiVectorLinearOp<Scalar> >(op);
    RCP<Thyra::DefaultScaledAdjointLinearOp<Scalar> > adjoint_op =
      rcp_dynamic_cast<Thyra::DefaultScaledAdjointLinearOp<Scalar> >(
        mv_adjoint_op->getNonconstLinearOp());
    MEB::OutArgs<Scalar> me_outArgs = model_->createOutArgs();
    me_outArgs.set_W_op(adjoint_op->getNonconstOp());
    model_->evalModel(me_inArgs, me_outArgs);
  }

  RCP<Thyra::VectorBase<Scalar> > adjoint_f = outArgs.get_f();
  RCP<Thyra::VectorBase<Scalar> > adjoint_g = outArgs.get_g(0);
  RCP<const Thyra::MultiVectorBase<Scalar> > adjoint_x_mv;
  if (adjoint_f != Teuchos::null || adjoint_g != Teuchos::null) {
    RCP<const Thyra::VectorBase<Scalar> > adjoint_x =
      inArgs.get_x().assert_not_null();
    adjoint_x_mv = rcp_dynamic_cast<const DMVPV>(adjoint_x)->getMultiVector();
  }

  // Compute adjoint residual F(y):
  //   * For implicit form,  F(y) = d/dt( df/dx_dot^T*y ) + df/dx^T*y
  //   * For explict form, F(y) = -df/dx^T*y
  // For implicit form, we assume df/dx_dot is constant w.r.t. x, x_dot, and t,
  // so the residual becomes F(y) = df/dx_dot^T*y_dot + df/dx^T*y
  if (adjoint_f != Teuchos::null) {
    RCP<Thyra::MultiVectorBase<Scalar> > adjoint_f_mv =
      rcp_dynamic_cast<DMVPV>(adjoint_f)->getNonconstMultiVector();

    MEB::OutArgs<Scalar> me_outArgs = model_->createOutArgs();

    if (my_dfdx_ == Teuchos::null)
      my_dfdx_ = model_->create_W_op();
    me_outArgs.set_W_op(my_dfdx_);
    if (me_inArgs.supports(MEB::IN_ARG_alpha))
      me_inArgs.set_alpha(0.0);
    if (me_inArgs.supports(MEB::IN_ARG_beta))
      me_inArgs.set_beta(1.0);
    model_->evalModel(me_inArgs, me_outArgs);

    // Explicit form residual F(y) = -df/dx^T*y
    my_dfdx_->apply(Thyra::CONJTRANS, *adjoint_x_mv, adjoint_f_mv.ptr(),
                    Scalar(-1.0), Scalar(0.0));

    // Implicit form residual df/dx_dot^T*y_dot + df/dx^T*y using the second
    // scalar argument to apply() to change the explicit term above
    if (me_inArgs.supports(MEB::IN_ARG_x_dot)) {
      RCP<const Thyra::VectorBase<Scalar> > adjoint_x_dot = inArgs.get_x_dot();
      if (adjoint_x_dot != Teuchos::null) {
        RCP<const Thyra::MultiVectorBase<Scalar> > adjoint_x_dot_mv =
          rcp_dynamic_cast<const DMVPV>(adjoint_x_dot)->getMultiVector();
        if (mass_matrix_is_identity_) {
          // F = -F + y_dot
          Thyra::V_StVpV(adjoint_f_mv.ptr(), Scalar(-1.0), *adjoint_f_mv,
                         *adjoint_x_dot_mv);
        }
        else {
          if (my_dfdxdot_ == Teuchos::null)
            my_dfdxdot_ = model_->create_W_op();
          if (!mass_matrix_is_constant_ || !mass_matrix_is_computed_) {
            me_outArgs.set_W_op(my_dfdxdot_);
            me_inArgs.set_alpha(1.0);
            me_inArgs.set_beta(0.0);
            model_->evalModel(me_inArgs, me_outArgs);

            mass_matrix_is_computed_ = true;
          }
          my_dfdxdot_->apply(Thyra::CONJTRANS, *adjoint_x_dot_mv,
                             adjoint_f_mv.ptr(), Scalar(1.0), Scalar(-1.0));
        }
      }
    }
  }

  // Compute g = df/dp^T*y for computing the model parameter term in the
  // adjoint sensitivity formula
  if (adjoint_g != Teuchos::null) {
    RCP<Thyra::MultiVectorBase<Scalar> > adjoint_g_mv =
      rcp_dynamic_cast<DMVPV>(adjoint_g)->getNonconstMultiVector();

    MEB::OutArgs<Scalar> me_outArgs = model_->createOutArgs();
    MEB::DerivativeSupport dfdp_support =
      me_outArgs.supports(MEB::OUT_ARG_DfDp, p_index_);
    Thyra::EOpTransp trans = Thyra::CONJTRANS;
    if (dfdp_support.supports(MEB::DERIV_LINEAR_OP)) {
      if (my_dfdp_op_ == Teuchos::null)
        my_dfdp_op_ = model_->create_DfDp_op(p_index_);
      me_outArgs.set_DfDp(p_index_, MEB::Derivative<Scalar>(my_dfdp_op_));
      trans = Thyra::CONJTRANS;
    }
    else if (dfdp_support.supports(MEB::DERIV_MV_JACOBIAN_FORM)) {
      if (my_dfdp_mv_ == Teuchos::null)
        my_dfdp_mv_ = createMembers(model_->get_f_space(),
                                    model_->get_p_space(p_index_)->dim());
      me_outArgs.set_DfDp(p_index_,
                          MEB::Derivative<Scalar>(my_dfdp_mv_,
                                                  MEB::DERIV_MV_JACOBIAN_FORM));
      my_dfdp_op_ = my_dfdp_mv_;
      trans = Thyra::CONJTRANS;
    }
    else if (dfdp_support.supports(MEB::DERIV_MV_GRADIENT_FORM)) {
      if (my_dfdp_mv_ == Teuchos::null)
        my_dfdp_mv_ = createMembers(model_->get_p_space(p_index_),
                                    model_->get_f_space()->dim());
      me_outArgs.set_DfDp(p_index_,
                          MEB::Derivative<Scalar>(my_dfdp_mv_,
                                                  MEB::DERIV_MV_GRADIENT_FORM));
      my_dfdp_op_ = my_dfdp_mv_;
      trans = Thyra::CONJ;
    }
    else
      TEUCHOS_TEST_FOR_EXCEPTION(
        true, std::logic_error, "Invalid df/dp support");
    model_->evalModel(me_inArgs, me_outArgs);
    my_dfdp_op_->apply(trans, *adjoint_x_mv, adjoint_g_mv.ptr(),
                       Scalar(1.0), Scalar(0.0));
  }
}

template<class Scalar>
Teuchos::RCP<const Teuchos::ParameterList>
AdjointSensitivityModelEvaluator<Scalar>::
getValidParameters()
{
  Teuchos::RCP<Teuchos::ParameterList> pl = Teuchos::parameterList();
  pl->set<int>("Sensitivity Parameter Index", 0);
  pl->set<int>("Response Function Index", 0);
  pl->set<bool>("Mass Matrix Is Constant", true);
  pl->set<bool>("Mass Matrix Is Identity", false);
  return pl;
}

} // namespace Tempus

#endif
