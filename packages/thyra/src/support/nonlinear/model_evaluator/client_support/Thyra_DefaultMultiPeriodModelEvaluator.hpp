// @HEADER
// ***********************************************************************
// 
//    Thyra: Interfaces and Support for Abstract Numerical Algorithms
//                 Copyright (2004) Sandia Corporation
// 
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
// 
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//  
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//  
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov) 
// 
// ***********************************************************************
// @HEADER

#ifndef THYRA_DEFAULT_MULTI_PERIOD_MODEL_EVALUATOR_HPP
#define THYRA_DEFAULT_MULTI_PERIOD_MODEL_EVALUATOR_HPP


#include "Thyra_ModelEvaluator.hpp"
#include "Thyra_DefaultProductVectorSpace.hpp"
#include "Thyra_PhysicallyBlockedLinearOpWithSolveBase.hpp" // Interface
#include "Thyra_DefaultBlockedTriangularLinearOpWithSolve.hpp" // Implementation
#include "Thyra_ModelEvaluatorDelegatorBase.hpp"
#include "Teuchos_implicit_cast.hpp"
#include "Teuchos_AbstractFactory.hpp" // Interface
#include "Teuchos_AbstractFactoryStd.hpp" // Implementation


namespace Thyra {


/** \brief Composite subclass that takes a single <tt>ModelEvaluator</tt>
 * object and represents it as a single aggregate multi-period
 * <tt>ModelEvalator</tt> object.
 *
 * Mathematically, this subclass is used to form a multi-period (or
 * multi-point) problem of the form:
 
 \verbatim

   (x_bar,{p_l}\p_period) -> f_bar

   (x_bar,{p_l}\p_period) -> g_bar

 \endverbatim

 * where

 \verbatim

   x_bar = [ x[0]; x[1]; ...; x[N-1]]

   {p_l}\p_period = { p_0, p_1; ..., p_(z_index-1), p_(z_index+1), ..., p_Np }

   f_bar(...) = [ f(x[0],{p_l},z[0]); f(x[1],{p_l},z[1]); ..., f(x[N-1],{p_l},z[N-1]) ]

   g_bar(...) = sum( g_weights[i]*g[g_index](x[0],{p_l},z[i]), i = 0,...,N-1 )

 \endverbatim

 * Above, the notation <tt>{p_l}\p_period</tt> is meant to represent the set
 * of all parameter subvectors in each of the constituent models minus the
 * parameter used to define period data.
 *
 * This gives the first derivative objects:

 \verbatim
                      
                      [ W[0], 0,     ...  0      ]
                      [ 0,    W[i],  ...  0      ]
   W_bar = DfDx_bar = [ .,    .,     ...  .      ]
                      [ 0,    0,     ...  W[N-1] ]

  
                 [ DfDp[0][l]   ]
                 [ DfDp[1][l]   ]
   DfDp_bar[l] = [ ...          ]
                 [ DfDp[N=1][l] ]


              [ g_wieght[0] * DgDx[0]     ]
              [ g_wieght[1] * DgDx[1]     ]
   DgDx_bar = [ ...                       ]
              [ g_wieght[N-1] * DgDx[N-1] ]

   DgDp_bar = sum( g_weights[i]*DgDp[i][g_index], i = 0,...,N-1 )

 \endverbatim
 
 * This class could be made much more general but for now ???
 */
template<class Scalar>
class DefaultMultiPeriodModelEvaluator : virtual public ModelEvaluator<Scalar> {
public:

  /** \name Constructors/Intializers/Accessors */
  //@{

  /** \brief . */
  DefaultMultiPeriodModelEvaluator();

  /** \brief Calls <tt>intialize(...)</tt>. */
  DefaultMultiPeriodModelEvaluator(
    const int N,
    const Teuchos::Array<Teuchos::RCP<ModelEvaluator<Scalar> > > &periodModels,
    const Teuchos::Array<int> &z_indexes,
    const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z,
    const int g_index,
    const Teuchos::Array<Scalar> g_weights,
    const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &x_bar_space = Teuchos::null,
    const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &f_bar_space = Teuchos::null,
    const Teuchos::RCP<const Teuchos::AbstractFactory<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > > &W_bar_factory = Teuchos::null
    );

  /** \brief Initialize.
   *
   * \param  N
   *           [in] The number of periods.
   * \param  periodModels
   *           [in] Array (length <tt>N</tt>) of the period models.  For now,
   *           each of the period models at <tt>periodModels[i]</tt> must have
   *           an identical structure for every input and output.  The reason
   *           that different models are passed in is so that different
   *           behaviors of the output functions and different nominal values
   *           for each period's <tt>x</tt> and bounds can be specified.  The
   *           nominal values from <tt>periodModels[0]</tt> will be used to
   *           set the nominal values for the non-peroid parameters.
   * \param  z_indexes
   *           [in] Array of sorted zero-based indexes of the model's parameter
   *           subvector that represents <tt>z[i]</tt>.  Note, this array
   *           must be sorted from smallest to largets and can not have
   *           any duplicate entires!
   * \param  z
   *           [in] Array (length <tt>N</tt>) of the period defining auxiliary
   *           paramter subvectors.  Each entry <tt>z[i]</tt> is an array of
   *           subvectors where <tt>z[i][k]</tt> is the subvector
   *           <tt>p(z_indexes[k])</tt> in the underlying perild model.
   *           Therefore, the array <tt>z[i]</tt> must be ordered according to
   *           <tt>z_indexes</tt>.  Note that <tt>z[i][k]</tt> is allowed to
   *           be <tt>null</tt> in which case the underlying default value
   *           for this parameter will be used.
   * \param  g_index
   *           [in] The index of the response function that will be used for the
   *           period objective function.
   * \param  g_weights
   *           [in] Array (length <tt>N</tt>) of the g_weights for the auxiliary
   *           response functions in the summation for <tt>g_bar(...)</tt>
   *           shown above.
   * \param  x_bar_space
   *           [in] The product vector space that represents the space for
   *           <tt>x_bar</tt> as defined above.  If
   *           <tt>x_bar_space.get()==NULL</tt> then a default version of this
   *           product space will be created internally.
   * \param  f_bar_space
   *           [in] The product vector space that represents the space for
   *           <tt>f_bar</tt> as defined above.  If
   *           <tt>f_bar_space.get()==NULL</tt> then a default version of this
   *           product space will be created internally.
   * \param  W_bar_factory
   *           [in] Factory object that is used to create the block LOWS
   *           object that will be used to represent the block diagonal object
   *           <tt>W_bar</tt>.  If <tt>is_null(W_bar_factory)==true</tt> on
   *           input then a <tt>DefaultBlockedTriangularLinearOpWithSolve</tt>
   *           object will be create internally.
   *
   * <b>Preconditions:</b><ul>
   * <li><tt>N > 0</tt>
   * <li><tt>periodModels.size()==N</tt>.
   * <li><li>z_indexes.size() > 0</tt> and <tt>z_indexes[k] >= 0</tt> and <tt>z_indexes[k]</tt>
   *     are sorted low to high and are unique.
   * <li><tt>z.size()==N</tt> and <tt>z[i].size()==z_indexes.size()</tt>, for <tt>i=0...N-1</tt>
   * </ul>
   * 
   */
  void initialize(
    const int N,
    const Teuchos::Array<Teuchos::RCP<ModelEvaluator<Scalar> > > &periodModels,
    const Teuchos::Array<int> &z_indexes,
    const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z,
    const int g_index,
    const Teuchos::Array<Scalar> g_weights,
    const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &x_bar_space = Teuchos::null,
    const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &f_bar_space = Teuchos::null,
    const Teuchos::RCP<const Teuchos::AbstractFactory<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > > &W_bar_factory = Teuchos::null
    );

  /** \brief Reset z.
   *
   * \param  z
   *           [in] See <tt>initialize()</tt>.
   *
   * <b>Preconditions:</b><ul>
   * <li>See <tt>initialize()</tt>
   * </ul>
   * 
   */
  void reset_z(
    const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z
    );
  
  //@}

  /** \name Public functions overridden from ModelEvaulator. */
  //@{

  /** \brief . */
  int Np() const;
  /** \brief . */
  int Ng() const;
  /** \brief . */
  Teuchos::RCP<const VectorSpaceBase<Scalar> > get_x_space() const;
  /** \brief . */
  Teuchos::RCP<const VectorSpaceBase<Scalar> > get_f_space() const;
  /** \brief. */
  Teuchos::RCP<const VectorSpaceBase<Scalar> > get_p_space(int l) const;
  /** \brief . */
  Teuchos::RCP<const Teuchos::Array<std::string> > get_p_names(int l) const;
  /** \brief . */
  Teuchos::RCP<const VectorSpaceBase<Scalar> > get_g_space(int j) const;
  /** \brief . */
  ModelEvaluatorBase::InArgs<Scalar> getNominalValues() const;
  /** \brief . */
  ModelEvaluatorBase::InArgs<Scalar> getLowerBounds() const;
  /** \brief . */
  ModelEvaluatorBase::InArgs<Scalar> getUpperBounds() const;
  /** \brief . */
  Teuchos::RCP<LinearOpWithSolveBase<Scalar> > create_W() const;
  /** \breif . */
  Teuchos::RCP<LinearOpBase<Scalar> > create_W_op() const;
  /** \brief . */
  Teuchos::RCP<LinearOpBase<Scalar> > create_DfDp_op(int l) const;
  /** \brief . */
  Teuchos::RCP<LinearOpBase<Scalar> > create_DgDx_op(int j) const;
  /** \brief . */
  Teuchos::RCP<LinearOpBase<Scalar> > create_DgDp_op( int j, int l ) const;
  /** \brief . */
  ModelEvaluatorBase::InArgs<Scalar> createInArgs() const;
  /** \brief . */
  ModelEvaluatorBase::OutArgs<Scalar> createOutArgs() const;
  /** \brief . */
  void evalModel(
    const ModelEvaluatorBase::InArgs<Scalar> &inArgs,
    const ModelEvaluatorBase::OutArgs<Scalar> &outArgs
    ) const;
  /** \brief Ignores the final point. */
  void reportFinalPoint(
    const ModelEvaluatorBase::InArgs<Scalar> &finalPoint,
    const bool wasSolved
    );

  //@}

private:

  // //////////////////////////
  // Private types

  typedef Teuchos::Array<Scalar> g_weights_t;
  typedef Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > z_t;

  // /////////////////////////
  // Private data members

  Teuchos::RCP<ModelEvaluator<Scalar> > periodModel_;
  Teuchos::Array<Teuchos::RCP<ModelEvaluator<Scalar> > > periodModels_;
  Teuchos::Array<int> z_indexes_;
  Teuchos::Array<int> period_l_map_;
  z_t z_; // size == N
  int g_index_;
  g_weights_t g_weights_; // size == N
  Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > x_bar_space_;
  Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > f_bar_space_;
  Teuchos::RCP<const Teuchos::AbstractFactory<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > > W_bar_factory_;
  int Np_;
  int Ng_;
  ModelEvaluatorBase::InArgs<Scalar> nominalValues_;
  ModelEvaluatorBase::InArgs<Scalar> lowerBounds_;
  ModelEvaluatorBase::InArgs<Scalar> upperBounds_;

  // /////////////////////////
  // Private member functions

  void set_z_indexes_and_create_period_l_map( const Teuchos::Array<int> &z_indexes );

  void wrapNominalValuesAndBounds();

  static
  Teuchos::RCP<ProductVectorBase<Scalar> >
  createProductVector(
    const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &prodVecSpc
    );

  // Return the index of a "free" parameter in the period model given its
  // index in this mulit-period model.
  int period_l(const int l) const
    {
      TEST_FOR_EXCEPT( !( 0 <= l && l < Np_) );
      return period_l_map_[l];
    }

  int numPeriodZs() const { return z_indexes_.size(); }

  int N() const { return x_bar_space_->numBlocks(); }
  
};


// /////////////////////////////////
// Implementations


// Constructors/Intializers/Accessors


template<class Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::DefaultMultiPeriodModelEvaluator()
  :g_index_(-1), Np_(-1), Ng_(-1)
{}


template<class Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::DefaultMultiPeriodModelEvaluator(
  const int N,
  const Teuchos::Array<Teuchos::RCP<ModelEvaluator<Scalar> > > &periodModels,
  const Teuchos::Array<int> &z_indexes,
  const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z,
  const int g_index,
  const Teuchos::Array<Scalar> g_weights,
  const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &x_bar_space,
  const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &f_bar_space,
  const Teuchos::RCP<const Teuchos::AbstractFactory<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > > &W_bar_factory
  )
  :g_index_(-1), Np_(-1), Ng_(-1)
{
  initialize(
    N, periodModels, z_indexes, z, g_index, g_weights,
    x_bar_space, f_bar_space, W_bar_factory
    );
}


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::initialize(
  const int N,
  const Teuchos::Array<Teuchos::RCP<ModelEvaluator<Scalar> > > &periodModels,
  const Teuchos::Array<int> &z_indexes,
  const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z,
  const int g_index,
  const Teuchos::Array<Scalar> g_weights,
  const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &x_bar_space,
  const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &f_bar_space,
  const Teuchos::RCP<const Teuchos::AbstractFactory<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > > &W_bar_factory
  )
{

  using Teuchos::RCP;
  using Teuchos::Array;
  using Teuchos::implicit_cast;
  typedef Teuchos::ScalarTraits<Scalar> ST;
  typedef ModelEvaluatorBase MEB;

  TEST_FOR_EXCEPT( N <= 0 );
  TEST_FOR_EXCEPT( implicit_cast<int>(periodModels.size()) != N );
  MEB::InArgs<Scalar> periodInArgs = periodModels[0]->createInArgs();
  MEB::OutArgs<Scalar> periodOutArgs = periodModels[0]->createOutArgs();
  for ( int k = 0; k < implicit_cast<int>(z_indexes.size()); ++k ) {
    TEST_FOR_EXCEPT( !( 0 <= z_indexes[k] && z_indexes[k] < periodInArgs.Np() ) );
  }
  TEST_FOR_EXCEPT( implicit_cast<int>(z.size())!=N );
  TEST_FOR_EXCEPT( !( 0 <= g_index && g_index < periodOutArgs.Ng() ) );
  TEST_FOR_EXCEPT( implicit_cast<int>(g_weights.size())!=N );
  // ToDo: Assert that the different period models are compatible!

  Np_ = periodInArgs.Np() - z_indexes.size();

  Ng_ = 1;

  set_z_indexes_and_create_period_l_map(z_indexes);
  
  periodModel_ = periodModels[0]; // Assume basic structure!
  
  periodModels_ = periodModels;
  
  z_.resize(N);
  reset_z(z);
  
  g_index_ = g_index;
  g_weights_ = g_weights;
  
  if ( periodInArgs.supports(MEB::IN_ARG_x) ) {
    if( !is_null(x_bar_space) ) {
      TEST_FOR_EXCEPT(!(x_bar_space->numBlocks()==N));
      // ToDo: Check the constituent spaces more carefully against models[]->get_x_space().
      x_bar_space_ = x_bar_space;
    }
    else {
      x_bar_space_ = productVectorSpace(periodModel_->get_x_space(),N);
      // ToDo: Update to build from different models for different x spaces!
    }
  }

  if ( periodOutArgs.supports(MEB::OUT_ARG_f) ) {
    if( !is_null(f_bar_space) ) {
      TEST_FOR_EXCEPT(!(f_bar_space->numBlocks()==N));
      // ToDo: Check the constituent spaces more carefully against models[]->get_f_space().
      f_bar_space_ = f_bar_space;
    }
    else {
      f_bar_space_ = productVectorSpace(periodModel_->get_f_space(),N);
      // ToDo: Update to build from different models for different f spaces!
    }
  }

  if ( periodOutArgs.supports(MEB::OUT_ARG_W) ) {
    if ( !is_null(W_bar_factory) ) {
      // ToDo: Check the compatability of the W_bar objects created using this object!
      W_bar_factory_ = W_bar_factory;
    }
    else {
      typedef PhysicallyBlockedLinearOpWithSolveBase<Scalar> PBLOWSB;
      typedef DefaultBlockedTriangularLinearOpWithSolve<Scalar> DBTLOWS;
      W_bar_factory_ = Teuchos::abstractFactoryStd<PBLOWSB,DBTLOWS>();
    }
  }

  wrapNominalValuesAndBounds();
  
}


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::reset_z(
  const Teuchos::Array<Teuchos::Array<Teuchos::RCP<const VectorBase<Scalar> > > > &z
  )
{

  using Teuchos::RCP;
  using Teuchos::Array;
  using Teuchos::implicit_cast;
  
  const int N = z_.size();

#ifdef TEUCHOS_DEBUG
  TEST_FOR_EXCEPT( N == 0 && "Error, must have called initialize() first!" );
  TEST_FOR_EXCEPT( implicit_cast<int>(z.size()) != N ); 
#endif 

  for( int i = 0; i < N; ++i ) {
    const Array<RCP<const VectorBase<Scalar> > >  &z_i = z[i];
#ifdef TEUCHOS_DEBUG
    TEST_FOR_EXCEPT( z_i.size() != z_indexes_.size() ); 
#endif 
    z_[i] = z_i;
  }
  
}


// Public functions overridden from ModelEvaulator


template<class Scalar>
int DefaultMultiPeriodModelEvaluator<Scalar>::Np() const
{
  return Np_;
}


template<class Scalar>
int DefaultMultiPeriodModelEvaluator<Scalar>::Ng() const
{
  return Ng_;
}


template<class Scalar>
Teuchos::RCP<const VectorSpaceBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::get_x_space() const
{
  return x_bar_space_;
}


template<class Scalar>
Teuchos::RCP<const VectorSpaceBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::get_f_space() const
{
  return f_bar_space_;
}


template<class Scalar>
Teuchos::RCP<const VectorSpaceBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::get_p_space(int l) const
{
  return  periodModel_->get_p_space(period_l(l));
}


template<class Scalar>
Teuchos::RCP<const Teuchos::Array<std::string> >
DefaultMultiPeriodModelEvaluator<Scalar>::get_p_names(int l) const
{
  return  periodModel_->get_p_names(period_l(l));
}


template<class Scalar>
Teuchos::RCP<const VectorSpaceBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::get_g_space(int j) const
{
  TEST_FOR_EXCEPT(j!=0);
  return periodModel_->get_g_space(g_index_);
}


template<class Scalar>
ModelEvaluatorBase::InArgs<Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::getNominalValues() const
{
  return nominalValues_;
}


template<class Scalar>
ModelEvaluatorBase::InArgs<Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::getLowerBounds() const
{
  return lowerBounds_;
}


template<class Scalar>
ModelEvaluatorBase::InArgs<Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::getUpperBounds() const
{
  return upperBounds_;
}


template<class Scalar>
Teuchos::RCP<LinearOpWithSolveBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::create_W() const
{
  return W_bar_factory_->create();
}


template<class Scalar>
Teuchos::RCP<LinearOpBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::create_W_op() const
{
  TEST_FOR_EXCEPT("This class does not support W as just a linear operator yet.");
  return Teuchos::null; // Should never be called!
}


template<class Scalar>
Teuchos::RCP<LinearOpBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::create_DfDp_op(int l) const
{
  TEST_FOR_EXCEPT("This class does not support DfDp(l) as a linear operator yet.");
  return Teuchos::null;
}


template<class Scalar>
Teuchos::RCP<LinearOpBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::create_DgDx_op(int j) const
{
  TEST_FOR_EXCEPT("This class does not support DgDx(j) as a linear operator yet.");
  return Teuchos::null;
}


template<class Scalar>
Teuchos::RCP<LinearOpBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::create_DgDp_op( int j, int l ) const
{
  TEST_FOR_EXCEPT("This class does not support DgDp(j,l) as a linear operator yet.");
  return Teuchos::null;
}


template<class Scalar>
ModelEvaluatorBase::InArgs<Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::createInArgs() const
{
  typedef ModelEvaluatorBase MEB;
  MEB::InArgs<Scalar> periodInArgs = periodModel_->createInArgs();
  MEB::InArgsSetup<Scalar> inArgs;
  inArgs.setModelEvalDescription(this->description());
  inArgs.set_Np(Np_);
  inArgs.setSupports( MEB::IN_ARG_x, periodInArgs.supports(MEB::IN_ARG_x) );
  return inArgs;
}


template<class Scalar>
ModelEvaluatorBase::OutArgs<Scalar>
DefaultMultiPeriodModelEvaluator<Scalar>::createOutArgs() const
{

  typedef ModelEvaluatorBase MEB;

  MEB::OutArgs<Scalar> periodOutArgs = periodModel_->createOutArgs();
  MEB::OutArgsSetup<Scalar> outArgs;

  outArgs.setModelEvalDescription(this->description());

  outArgs.set_Np_Ng(Np_,Ng_);

  if (periodOutArgs.supports(MEB::OUT_ARG_f) ) {
    outArgs.setSupports(MEB::OUT_ARG_f);
  }

  if (periodOutArgs.supports(MEB::OUT_ARG_W) ) {
    outArgs.setSupports(MEB::OUT_ARG_W);
    outArgs.set_W_properties(periodOutArgs.get_W_properties());
  }

  for ( int l = 0; l < Np_; ++l ) {
    const int period_l = this->period_l(l);
    const MEB::DerivativeSupport period_DfDp_l_support
      = periodOutArgs.supports(MEB::OUT_ARG_DfDp,period_l);
    if (!period_DfDp_l_support.none()) {
      outArgs.setSupports( MEB::OUT_ARG_DfDp, l, period_DfDp_l_support );
      outArgs.set_DfDp_properties(
        l, periodOutArgs.get_DfDp_properties(period_l) );
    }
  }

  const MEB::DerivativeSupport
    period_DgDx_support = periodOutArgs.supports(MEB::OUT_ARG_DgDx,g_index_);
  if (!period_DgDx_support.none()) {
    outArgs.setSupports( MEB::OUT_ARG_DgDx, 0, period_DgDx_support );
    outArgs.set_DgDx_properties(
      0, periodOutArgs.get_DgDx_properties(g_index_) );
  }

  for ( int l = 0; l < Np_; ++l ) {
    const int period_l = this->period_l(l);
    const MEB::DerivativeSupport period_DgDp_l_support
      = periodOutArgs.supports(MEB::OUT_ARG_DgDp, g_index_, period_l);
    if (!period_DgDp_l_support.none()) {
      outArgs.setSupports( MEB::OUT_ARG_DgDp, 0, l, period_DgDp_l_support );
      outArgs.set_DgDp_properties(
        0, l, periodOutArgs.get_DgDp_properties(g_index_,period_l) );
    }
  }
  
  return outArgs;

}


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::evalModel(
  const ModelEvaluatorBase::InArgs<Scalar>       &inArgs
  ,const ModelEvaluatorBase::OutArgs<Scalar>     &outArgs
  ) const
{

  using Teuchos::Array;
  using Teuchos::RCP;
  using Teuchos::rcp_dynamic_cast;
  typedef Teuchos::ScalarTraits<Scalar> ST;
  typedef ModelEvaluatorBase MEB;
  typedef Teuchos::VerboseObjectTempState<ModelEvaluatorBase> VOTSME;

  THYRA_MODEL_EVALUATOR_DECORATOR_EVAL_MODEL_GEN_BEGIN(
    "DefaultMultiPeriodModelEvaluator",inArgs,outArgs,periodModel_ );
  // ToDo: You will have to set the verbosity level for each of the
  // periodModels_[i] individually below!
  
  const int N = x_bar_space_->numBlocks();
  const int Np = this->Np_;
  //const int Ng = this->Ng_;

  //
  // Setup InArgs
  //

  // Get product vector x_bar if it is supported
  RCP<const ProductVectorBase<Scalar> > x_bar;
  if (inArgs.supports(MEB::IN_ARG_x)) {
    x_bar = rcp_dynamic_cast<const ProductVectorBase<Scalar> >(
      inArgs.get_x(), true );
    TEST_FOR_EXCEPTION(
      is_null(x_bar), std::logic_error,
      "Error, if x is supported, it must be set!"
      );
  }

  // OutArgs
  
  // Get product vector f_bar if it is supported and was set
  RCP<ProductVectorBase<Scalar> > f_bar;
  if (outArgs.supports(MEB::OUT_ARG_f)) {
    f_bar = rcp_dynamic_cast<ProductVectorBase<Scalar> >(
      outArgs.get_f(), true );
  }

  Array<MEB::Derivative<Scalar> > DfDp_bar(Np);
  Array<RCP<ProductMultiVectorBase<Scalar> > > DfDp_bar_mv(Np);
  for ( int l = 0; l < Np; ++l ) {
    if (!outArgs.supports(MEB::OUT_ARG_DfDp,l).none()) {
      MEB::Derivative<Scalar>
        DfDp_bar_l = outArgs.get_DfDp(l);
      DfDp_bar[l] = DfDp_bar_l;
      DfDp_bar_mv[l] = rcp_dynamic_cast<ProductMultiVectorBase<Scalar> >(
        DfDp_bar_l.getMultiVector(), true );
      TEST_FOR_EXCEPTION(
        (
          !DfDp_bar_l.isEmpty()
          &&
          (
            is_null(DfDp_bar_mv[l])
            ||
            DfDp_bar_l.getMultiVectorOrientation() != MEB::DERIV_MV_BY_COL
            )
          ),
        std::logic_error,
        "Error, we currently can only handle DfDp as an column-based multi-vector!"
        );
    }
  }

  RCP<PhysicallyBlockedLinearOpWithSolveBase<Scalar> > W_bar;
  if (outArgs.supports(MEB::OUT_ARG_W)) {
    W_bar = rcp_dynamic_cast<PhysicallyBlockedLinearOpWithSolveBase<Scalar> >(
      outArgs.get_W(), true
      );
  }

  RCP<VectorBase<Scalar> >
    g_bar = outArgs.get_g(0);

  MEB::Derivative<Scalar> DgDx_bar;
  RCP<ProductMultiVectorBase<Scalar> > DgDx_bar_mv;
  if (!outArgs.supports(MEB::OUT_ARG_DgDx,0).none()) {
    DgDx_bar = outArgs.get_DgDx(0);
    DgDx_bar_mv = rcp_dynamic_cast<ProductMultiVectorBase<Scalar> >(
      DgDx_bar.getMultiVector(), true );
    TEST_FOR_EXCEPTION(
      (
        !DgDx_bar.isEmpty()
        &&
        (
          is_null(DgDx_bar_mv)
          ||
          DgDx_bar.getMultiVectorOrientation() != MEB::DERIV_TRANS_MV_BY_ROW
          )
        ),
      std::logic_error,
      "Error, we currently can only handle DgDx as an row-based multi-vector!"
      );
  }

  Array<MEB::Derivative<Scalar> > DgDp_bar(Np);
  Array<RCP<MultiVectorBase<Scalar> > > DgDp_bar_mv(Np);
  for ( int l = 0; l < Np; ++l ) {
    if (!outArgs.supports(MEB::OUT_ARG_DgDp,0,l).none()) {
      MEB::Derivative<Scalar>
        DgDp_bar_l = outArgs.get_DgDp(0,l);
      DgDp_bar[l] = DgDp_bar_l;
      DgDp_bar_mv[l] = DgDp_bar_l.getMultiVector();
      TEST_FOR_EXCEPTION(
        !DgDp_bar_l.isEmpty() && is_null(DgDp_bar_mv[l]),
        std::logic_error,
        "Error, we currently can only handle DgDp as some type of multi-vector!"
        );
    }
  }

  //
  // Evaluate the model
  //

  MEB::InArgs<Scalar>
    periodInArgs = periodModel_->createInArgs();
  // ToDo: The above will have to change if you allow different structures for
  // each period model!
  
  // Set all of the parameters that will just be passed through 
  for ( int l = 0; l < Np; ++l )
    periodInArgs.set_p( period_l(l), inArgs.get_p(l) ); // Can be null just fine
  
  MEB::OutArgs<Scalar>
    periodOutArgs = periodModel_->createOutArgs();
  // ToDo: The above will have to change if you allow different structures for
  // each period model!
  
  // Create storage for period g's that will be summed into global g_bar
  periodOutArgs.set_g(
    g_index_, createMember<Scalar>( periodModel_->get_g_space(g_index_) ) );

  // Zero out global g_bar that will be summed into below
  if (!is_null(g_bar) )
    assign( &*g_bar, ST::zero() );

  // Determine if this is the first time this W_bar has been computed
  const bool reinit_W_bar = ( !is_null(W_bar) ? is_null(W_bar->productRange()) : false );
  if ( !is_null(W_bar) && reinit_W_bar ) {
    W_bar->beginBlockFill(f_bar_space_,x_bar_space_);
  }

  // Set up storage for peroid DgDp[l] objects that will be summed into global
  // DgDp_bar[l] and zero out DgDp_bar[l] that will be summed into.
  for ( int l = 0; l < Np; ++l ) {
    if ( !is_null(DgDp_bar_mv[l]) ) {
      assign(&*DgDp_bar_mv[l],ST::zero());
      periodOutArgs.set_DgDp(
        g_index_, period_l(l),
        create_DgDp_mv(
          *periodModel_, g_index_, period_l(l),
          DgDp_bar[l].getMultiVectorOrientation()
          )
        );
    }
  }
  
  // Loop over periods and assemble the model

  for ( int i = 0; i < N; ++i ) {

    VOTSME thyraModel_outputTempState(periodModels_[i],out,verbLevel);

    // Set period-speicific InArgs and OutArgs

    for ( int k = 0; k < numPeriodZs(); ++k )
      periodInArgs.set_p( z_indexes_[k], z_[i][k] );

    if (!is_null(x_bar))
      periodInArgs.set_x(x_bar->getVectorBlock(i));

    if (!is_null(f_bar))
      periodOutArgs.set_f(f_bar->getNonconstVectorBlock(i)); // Updated in place!

    if ( !is_null(W_bar) ) {
      if ( reinit_W_bar ) {
        periodOutArgs.set_W(periodModel_->create_W());
      }
      else {
        periodOutArgs.set_W(W_bar->getNonconstLOWSBlock(i,i));
      }
    }

    for ( int l = 0; l < Np; ++l ) {
      if ( !is_null(DfDp_bar_mv[l]) ) {
        periodOutArgs.set_DfDp(
          period_l(l),
          MEB::Derivative<Scalar>(
            DfDp_bar_mv[l]->getNonconstMultiVectorBlock(i),
            MEB::DERIV_MV_BY_COL
            )
          );
      }
    }
    
    if ( !is_null(DgDx_bar_mv) ) {
      periodOutArgs.set_DgDx(
        g_index_,
        MEB::Derivative<Scalar>(
          DgDx_bar_mv->getNonconstMultiVectorBlock(i),
          MEB::DERIV_TRANS_MV_BY_ROW
          )
        );
    }

    // Evaluate the period model

    periodModels_[i]->evalModel( periodInArgs, periodOutArgs );

    // Process output arguments that need processed

    // Set block of W_bar if first time through
    if ( !is_null(W_bar) && reinit_W_bar ) {
      W_bar->setNonconstLOWSBlock(i,i,periodOutArgs.get_W());
    }

    // Sum into global g_bar
    if (!is_null(g_bar)) {
      Vp_StV( &*g_bar, g_weights_[i], *periodOutArgs.get_g(g_index_) );
    }

    // Sum into global DgDp_bar
    for ( int l = 0; l < Np; ++l ) {
      if ( !is_null(DgDp_bar_mv[l]) ) {
        update(
          g_weights_[i],
          *periodOutArgs.get_DgDp(g_index_,period_l(l)).getMultiVector(),
          &*DgDp_bar_mv[l]
          );
      }
    }
    
    // Scale DgDx_bar_mv[i]
    if ( !is_null(DgDx_bar_mv) ) {
      scale( g_weights_[i], &*DgDx_bar_mv->getNonconstMultiVectorBlock(i) );
    }

  }

  // Finalize W_bar if first time through
  if ( !is_null(W_bar) && reinit_W_bar ) {
    W_bar->endBlockFill();
  }

  // ToDo: We need to do some type of global sum of g_bar and DgDp_bar to
  // account for other clusters of processes.

  // Once we get here, all of the quantities should be updated and we should
  // be all done!

  THYRA_MODEL_EVALUATOR_DECORATOR_EVAL_MODEL_END();

}


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::reportFinalPoint(
  const ModelEvaluatorBase::InArgs<Scalar>      &finalPoint
  ,const bool                                   wasSolved
  )
{
  // We are just going to ignore the final point here.  It is not clear how to
  // report a "final" point back to the underlying *periodModel_ object since
  // we have so many different "points" that we could return (i.e. one for
  // each period).  I guess we could report back the final parameter values
  // (other than the z parameter) but there are multiple states x[i] and
  // period parameters z[i] that we can report back.
}


// private


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::set_z_indexes_and_create_period_l_map(
  const Teuchos::Array<int> &z_indexes
  )
{
#ifdef TEUCHOS_DEBUG
  TEST_FOR_EXCEPT( Np_ <= 0 && "Error, Np must be set!" );
#endif
  z_indexes_ = z_indexes;
  period_l_map_.resize(0);
  const int numTotalParams = Np_ + z_indexes_.size();
  Teuchos::Array<int>::const_iterator
    z_indexes_itr = z_indexes_.begin(),
    z_indexes_end = z_indexes_.end();
  int last_z_index = -1;
  for ( int k = 0; k < numTotalParams; ++k ) {
    if ( z_indexes_itr == z_indexes_end || k < *z_indexes_itr ) {
      // This is a "free" parameter subvector
      period_l_map_.push_back(k);
    }
    else {
      // This is a "fixed" period parameter subvector so increment
      // z_indexes iterator.
#ifdef TEUCHOS_DEBUG
      TEST_FOR_EXCEPT( k != *z_indexes_itr && "This should never happen!" );
#endif
      const int tmp_last_z_index = *z_indexes_itr;
      ++z_indexes_itr;
      if ( z_indexes_itr != z_indexes_end ) {
#ifdef TEUCHOS_DEBUG
        if ( last_z_index >= 0 ) {
          TEST_FOR_EXCEPTION(
            *z_indexes_itr <= last_z_index, std::logic_error,
            "Error, the z_indexes array = " << toString(z_indexes_)
            << " is not sorted or contains duplicate entries!"
            );
        }
#endif
        last_z_index = tmp_last_z_index;
      }
    }
  }
}


template<class Scalar>
void DefaultMultiPeriodModelEvaluator<Scalar>::wrapNominalValuesAndBounds()
{

  using Teuchos::RCP;
  using Teuchos::rcp_dynamic_cast;
  typedef ModelEvaluatorBase MEB;

  nominalValues_ = this->createInArgs();
  lowerBounds_ = this->createInArgs();
  upperBounds_ = this->createInArgs();

  const MEB::InArgs<Scalar>
    periodNominalValues = periodModel_->getNominalValues(),
    periodLowerBounds = periodModel_->getLowerBounds(),
    periodUpperBounds = periodModel_->getUpperBounds();
  
  if (periodNominalValues.supports(MEB::IN_ARG_x)) {

    if( !is_null(periodNominalValues.get_x()) ) {
      // If the first peroid model has nominal values for x, then all of them
      // must also!
      Teuchos::RCP<Thyra::ProductVectorBase<Scalar> >
        x_bar_init = createProductVector(x_bar_space_);
      const int N = this->N();
      for ( int i = 0; i < N; ++i ) {
        assign(
          &*x_bar_init->getNonconstVectorBlock(i),
          *periodModels_[i]->getNominalValues().get_x()
          );
      }
      nominalValues_.set_x(x_bar_init);
    }
      
    if( !is_null(periodLowerBounds.get_x()) ) {
      // If the first peroid model has lower bounds for for x, then all of
      // them must also!
      Teuchos::RCP<Thyra::ProductVectorBase<Scalar> >
        x_bar_l = createProductVector(x_bar_space_);
      const int N = this->N();
      for ( int i = 0; i < N; ++i ) {
        assign(
          &*x_bar_l->getNonconstVectorBlock(i),
          *periodModels_[i]->getLowerBounds().get_x()
          );
      }
      lowerBounds_.set_x(x_bar_l);
    }
      
    if( !is_null(periodUpperBounds.get_x()) ) {
      // If the first peroid model has upper bounds for for x, then all of
      // them must also!
      Teuchos::RCP<Thyra::ProductVectorBase<Scalar> >
        x_bar_u = createProductVector(x_bar_space_);
      const int N = this->N();
      for ( int i = 0; i < N; ++i ) {
        assign(
          &*x_bar_u->getNonconstVectorBlock(i),
          *periodModels_[i]->getUpperBounds().get_x()
          );
      }
      upperBounds_.set_x(x_bar_u);
    }

  }

  // There can only be one set of nominal values for the non-period parameters
  // so just take them from the first period!
  for ( int l = 0; l < Np_; ++l ) {
    const int period_l = this->period_l(l);
    nominalValues_.set_p(l,periodNominalValues.get_p(period_l));
    lowerBounds_.set_p(l,periodLowerBounds.get_p(period_l));
    upperBounds_.set_p(l,periodUpperBounds.get_p(period_l));
  }
  
}


template<class Scalar>
Teuchos::RCP<ProductVectorBase<Scalar> >
DefaultMultiPeriodModelEvaluator<Scalar>::createProductVector(
  const Teuchos::RCP<const ProductVectorSpaceBase<Scalar> > &prodVecSpc
  )
{
  return Teuchos::rcp_dynamic_cast<ProductVectorBase<Scalar> >(
    Thyra::createMember<Scalar>(prodVecSpc), true );
}


} // namespace Thyra


#endif // THYRA_DEFAULT_MULTI_PERIOD_MODEL_EVALUATOR_HPP
