// @HEADER
// ****************************************************************************
//                Tempus: Copyright (2017) Sandia Corporation
//
// Distributed under BSD 3-clause license (See accompanying file Copyright.txt)
// ****************************************************************************
// @HEADER

#include "Teuchos_UnitTestHarness.hpp"
#include "Teuchos_XMLParameterListHelpers.hpp"
#include "Teuchos_TimeMonitor.hpp"
#include "Teuchos_DefaultComm.hpp"

#include "Thyra_VectorStdOps.hpp"
#include "Thyra_MultiVectorStdOps.hpp"

#include "Tempus_IntegratorBasic.hpp"
#include "Tempus_IntegratorForwardSensitivity.hpp"
#include "Tempus_IntegratorPseudoTransientForwardSensitivity.hpp"
#include "Tempus_IntegratorAdjointSensitivity.hpp"

#include "Thyra_DefaultMultiVectorProductVector.hpp"
#include "Thyra_DefaultProductVector.hpp"

#include "../TestModels/SinCosModel.hpp"
#include "../TestModels/SteadyQuadraticModel.hpp"
#include "../TestUtils/Tempus_ConvergenceTestUtils.hpp"

#include <fstream>
#include <vector>

namespace Tempus_Test {

using Teuchos::RCP;
using Teuchos::ParameterList;
using Teuchos::sublist;
using Teuchos::getParametersFromXmlFile;

using Tempus::IntegratorBasic;
using Tempus::SolutionHistory;
using Tempus::SolutionState;

// ************************************************************
// ************************************************************
void test_sincos_fsa(const bool use_combined_method,
                     const bool use_dfdp_as_tangent,
                     Teuchos::FancyOStream &out, bool &success)
{
  std::vector<std::string> RKMethods;
  RKMethods.push_back("RK Forward Euler");
  RKMethods.push_back("RK Explicit 4 Stage");
  RKMethods.push_back("RK Explicit 3/8 Rule");
  RKMethods.push_back("RK Explicit 4 Stage 3rd order by Runge");
  RKMethods.push_back("RK Explicit 5 Stage 3rd order by Kinnmark and Gray");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order TVD");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order by Heun");
  RKMethods.push_back("RK Explicit 2 Stage 2nd order by Runge");
  RKMethods.push_back("RK Explicit Trapezoidal");
  RKMethods.push_back("General ERK");
  std::vector<double> RKMethodErrors;
  if (use_combined_method) {
    RKMethodErrors.push_back(0.183799);
    RKMethodErrors.push_back(6.88637e-06);
    RKMethodErrors.push_back(6.88637e-06);
    RKMethodErrors.push_back(0.000264154);
    RKMethodErrors.push_back(5.22798e-05);
    RKMethodErrors.push_back(0.000261896);
    RKMethodErrors.push_back(0.000261896);
    RKMethodErrors.push_back(0.000261896);
    RKMethodErrors.push_back(0.00934377);
    RKMethodErrors.push_back(0.00934377);
    RKMethodErrors.push_back(6.88637e-06);
  }
  else {
    RKMethodErrors.push_back(0.183799);
    RKMethodErrors.push_back(2.1915e-05);
    RKMethodErrors.push_back(2.23367e-05);
    RKMethodErrors.push_back(0.000205051);
    RKMethodErrors.push_back(2.85141e-05);
    RKMethodErrors.push_back(0.000126478);
    RKMethodErrors.push_back(9.64964e-05);
    RKMethodErrors.push_back(0.000144616);
    RKMethodErrors.push_back(0.00826159);
    RKMethodErrors.push_back(0.00710492);
    RKMethodErrors.push_back(2.1915e-05);
  }
  Teuchos::RCP<const Teuchos::Comm<int> > comm =
    Teuchos::DefaultComm<int>::getComm();
  Teuchos::RCP<Teuchos::FancyOStream> my_out =
    Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));
  my_out->setProcRankAndSize(comm->getRank(), comm->getSize());
  my_out->setOutputToRootOnly(0);

  for(std::vector<std::string>::size_type m = 0; m != RKMethods.size(); m++) {

    std::string RKMethod_ = RKMethods[m];
    std::replace(RKMethod_.begin(), RKMethod_.end(), ' ', '_');
    std::replace(RKMethod_.begin(), RKMethod_.end(), '/', '.');
    std::vector<double> StepSize;
    std::vector<double> ErrorNorm;
    const int nTimeStepSizes = 7;
    double dt = 0.2;
    double order = 0.0;
    for (int n=0; n<nTimeStepSizes; n++) {

      // Read params from .xml file
      RCP<ParameterList> pList =
        getParametersFromXmlFile("Tempus_ExplicitRK_SinCos.xml");

      // Setup the SinCosModel
      RCP<ParameterList> scm_pl = sublist(pList, "SinCosModel", true);
      scm_pl->set("Use DfDp as Tangent", use_dfdp_as_tangent);
      RCP<SinCosModel<double> > model =
        Teuchos::rcp(new SinCosModel<double>(scm_pl));

      // Set the Stepper
      RCP<ParameterList> pl = sublist(pList, "Tempus", true);
      if (RKMethods[m] == "General ERK") {
        pl->sublist("Demo Integrator").set("Stepper Name", "Demo Stepper 2");
      } else {
        pl->sublist("Demo Stepper").set("Stepper Type", RKMethods[m]);
      }


      dt /= 2;

      // Setup sensitivities
      ParameterList& sens_pl = pl->sublist("Sensitivities");
      if (use_combined_method)
        sens_pl.set("Sensitivity Method", "Combined");
      else
        sens_pl.set("Sensitivity Method", "Staggered");
      sens_pl.set("Use DfDp as Tangent", use_dfdp_as_tangent);
      ParameterList& interp_pl =
        pl->sublist("Demo Integrator").sublist("Solution History").sublist("Interpolator");
      interp_pl.set("Interpolator Type", "Lagrange");
      interp_pl.set("Order", 3); // All RK methods here are at most 4th order

      // Setup the Integrator and reset initial time step
      pl->sublist("Demo Integrator")
        .sublist("Time Step Control").set("Initial Time Step", dt);
      RCP<Tempus::IntegratorForwardSensitivity<double> > integrator =
        Tempus::integratorForwardSensitivity<double>(pl, model);
      order = integrator->getStepper()->getOrder();

      // Initial Conditions
      double t0 = pl->sublist("Demo Integrator")
        .sublist("Time Step Control").get<double>("Initial Time");
      // RCP<const Thyra::VectorBase<double> > x0 =
      //   model->getExactSolution(t0).get_x()->clone_v();
      RCP<Thyra::VectorBase<double> > x0 =
        model->getNominalValues().get_x()->clone_v();
      const int num_param = model->get_p_space(0)->dim();
      RCP<Thyra::MultiVectorBase<double> > DxDp0 =
        Thyra::createMembers(model->get_x_space(), num_param);
      for (int i=0; i<num_param; ++i)
        Thyra::assign(DxDp0->col(i).ptr(),
                      *(model->getExactSensSolution(i, t0).get_x()));
      integrator->setInitialState(t0, x0, Teuchos::null, Teuchos::null,
                                  DxDp0, Teuchos::null, Teuchos::null);

      // Integrate to timeMax
      bool integratorStatus = integrator->advanceTime();
      TEST_ASSERT(integratorStatus)

      // Test if at 'Final Time'
      double time = integrator->getTime();
      double timeFinal = pl->sublist("Demo Integrator")
        .sublist("Time Step Control").get<double>("Final Time");
      TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

      // Time-integrated solution and the exact solution
      RCP<const Thyra::VectorBase<double> > x = integrator->getX();
      RCP<const Thyra::MultiVectorBase<double> > DxDp = integrator->getDxDp();
      RCP<const Thyra::VectorBase<double> > x_exact =
        model->getExactSolution(time).get_x();
      RCP<Thyra::MultiVectorBase<double> > DxDp_exact =
        Thyra::createMembers(model->get_x_space(), num_param);
      for (int i=0; i<num_param; ++i)
        Thyra::assign(DxDp_exact->col(i).ptr(),
                      *(model->getExactSensSolution(i, time).get_x()));

      // Plot sample solution and exact solution
      if (comm->getRank() == 0 && n == nTimeStepSizes-1) {
        typedef Thyra::DefaultMultiVectorProductVector<double> DMVPV;

        std::ofstream ftmp("Tempus_"+RKMethod_+"_SinCos_Sens.dat");
        RCP<const SolutionHistory<double> > solutionHistory =
          integrator->getSolutionHistory();
        RCP< Thyra::MultiVectorBase<double> > DxDp_exact_plot =
          Thyra::createMembers(model->get_x_space(), num_param);
        for (int i=0; i<solutionHistory->getNumStates(); i++) {
          RCP<const SolutionState<double> > solutionState =
            (*solutionHistory)[i];
          double time = solutionState->getTime();
          RCP<const DMVPV> x_prod_plot =
            Teuchos::rcp_dynamic_cast<const DMVPV>(solutionState->getX());
          RCP<const Thyra::VectorBase<double> > x_plot =
            x_prod_plot->getMultiVector()->col(0);
          RCP<const Thyra::MultiVectorBase<double> > DxDp_plot =
            x_prod_plot->getMultiVector()->subView(Teuchos::Range1D(1,num_param));
          RCP<const Thyra::VectorBase<double> > x_exact_plot =
            model->getExactSolution(time).get_x();
          for (int j=0; j<num_param; ++j)
            Thyra::assign(DxDp_exact_plot->col(j).ptr(),
                          *(model->getExactSensSolution(j, time).get_x()));
          ftmp << std::fixed << std::setprecision(7)
               << time
               << std::setw(11) << get_ele(*(x_plot), 0)
               << std::setw(11) << get_ele(*(x_plot), 1);
          for (int j=0; j<num_param; ++j)
            ftmp << std::setw(11) << get_ele(*(DxDp_plot->col(j)), 0)
                 << std::setw(11) << get_ele(*(DxDp_plot->col(j)), 1);
          ftmp << std::setw(11) << get_ele(*(x_exact_plot), 0)
               << std::setw(11) << get_ele(*(x_exact_plot), 1);
          for (int j=0; j<num_param; ++j)
            ftmp << std::setw(11) << get_ele(*(DxDp_exact_plot->col(j)), 0)
                 << std::setw(11) << get_ele(*(DxDp_exact_plot->col(j)), 1);
          ftmp << std::endl;
        }
        ftmp.close();
      }

      // Calculate the error
      RCP<Thyra::VectorBase<double> > xdiff = x->clone_v();
      RCP<Thyra::MultiVectorBase<double> > DxDpdiff = DxDp->clone_mv();
      Thyra::V_StVpStV(xdiff.ptr(), 1.0, *x_exact, -1.0, *(x));
      Thyra::V_VmV(DxDpdiff.ptr(), *DxDp_exact, *DxDp);
      StepSize.push_back(dt);
      double L2norm = Thyra::norm_2(*xdiff);
      L2norm *= L2norm;
      Teuchos::Array<double> L2norm_DxDp(num_param);
      Thyra::norms_2(*DxDpdiff, L2norm_DxDp());
      for (int i=0; i<num_param; ++i)
        L2norm += L2norm_DxDp[i]*L2norm_DxDp[i];
      L2norm = std::sqrt(L2norm);
      ErrorNorm.push_back(L2norm);

      *my_out << " n = " << n << " dt = " << dt << " error = " << L2norm
              << std::endl;
    }

    // Check the order and intercept
    double slope = computeLinearRegressionLogLog<double>(StepSize, ErrorNorm);
    *my_out << "  Stepper = " << RKMethods[m] << std::endl;
    *my_out << "  =========================" << std::endl;
    *my_out << "  Expected order: " << order << std::endl;
    *my_out << "  Observed order: " << slope << std::endl;
    *my_out << "  =========================" << std::endl;
    TEST_FLOATING_EQUALITY( slope, order, 0.04 );
    TEST_FLOATING_EQUALITY( ErrorNorm[0], RKMethodErrors[m], 1.0e-4 );

    if (comm->getRank() == 0) {
      std::ofstream ftmp("Tempus_"+RKMethod_+"_SinCos_Sens-Error.dat");
      double error0 = 0.8*ErrorNorm[0];
      for (int n=0; n<nTimeStepSizes; n++) {
        ftmp << StepSize[n]  << "   " << ErrorNorm[n] << "   "
             << error0*(pow(StepSize[n]/StepSize[0],order)) << std::endl;
      }
      ftmp.close();
    }
  }

  Teuchos::TimeMonitor::summarize();
}

TEUCHOS_UNIT_TEST(ExplicitRK, SinCos_Combined_FSA)
{
  test_sincos_fsa(true, false, out, success);
}

TEUCHOS_UNIT_TEST(ExplicitRK, SinCos_Combined_FSA_Tangent)
{
  test_sincos_fsa(true, true, out, success);
}

TEUCHOS_UNIT_TEST(ExplicitRK, SinCos_Staggered_FSA)
{
  test_sincos_fsa(false, false, out, success);
}

TEUCHOS_UNIT_TEST(ExplicitRK, SinCos_Staggered_FSA_Tangent)
{
  test_sincos_fsa(false, true, out, success);
}

// Note:  Staggered FSA approach not relevant for explicit methods

// ************************************************************
// ************************************************************
void test_pseudotransient_fsa(const bool use_dfdp_as_tangent,
                              Teuchos::FancyOStream &out, bool &success)
{
  // Read params from .xml file
  RCP<ParameterList> pList =
    getParametersFromXmlFile("Tempus_ExplicitRK_SteadyQuadratic.xml");

  // Setup the SteadyQuadraticModel
  RCP<ParameterList> scm_pl = sublist(pList, "SteadyQuadraticModel", true);
  scm_pl->set("Use DfDp as Tangent", use_dfdp_as_tangent);
  RCP<SteadyQuadraticModel<double> > model =
    Teuchos::rcp(new SteadyQuadraticModel<double>(scm_pl));

  // Setup sensitivities
  RCP<ParameterList> pl = sublist(pList, "Tempus", true);
  ParameterList& sens_pl = pl->sublist("Sensitivities");
  sens_pl.set("Use DfDp as Tangent", use_dfdp_as_tangent);

  // Setup the Integrator
  RCP<Tempus::IntegratorPseudoTransientForwardSensitivity<double> > integrator =
    Tempus::integratorPseudoTransientForwardSensitivity<double>(pl, model);

  // Integrate to timeMax
  bool integratorStatus = integrator->advanceTime();
  TEST_ASSERT(integratorStatus);

  // Test if at 'Final Time'
  double time = integrator->getTime();
  double timeFinal = pl->sublist("Demo Integrator")
    .sublist("Time Step Control").get<double>("Final Time");
  TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

  // Time-integrated solution and the exact solution
  RCP<const Thyra::VectorBase<double> > x_vec = integrator->getX();
  RCP<const Thyra::MultiVectorBase<double> > DxDp_vec = integrator->getDxDp();
  const double x = Thyra::get_ele(*x_vec, 0);
  const double dxdb = Thyra::get_ele(*(DxDp_vec->col(0)), 0);
  const double x_exact = model->getSteadyStateSolution();
  const double dxdb_exact = model->getSteadyStateSolutionSensitivity();

  TEST_FLOATING_EQUALITY( x,    x_exact,    1.0e-6 );
  TEST_FLOATING_EQUALITY( dxdb, dxdb_exact, 1.0e-6 );
}

TEUCHOS_UNIT_TEST(ExplicitRK, SteadyQuadratic_PseudoTransient_FSA)
{
  test_pseudotransient_fsa(false, out, success);
}

TEUCHOS_UNIT_TEST(ExplicitRK, SteadyQuadratic_PseudoTransient_FSA_Tangent)
{
  test_pseudotransient_fsa(true, out, success);
}

// ************************************************************
// ************************************************************
TEUCHOS_UNIT_TEST(ExplicitRK, SinCos_ASA)
{
  std::vector<std::string> RKMethods;
  RKMethods.push_back("RK Forward Euler");
  RKMethods.push_back("RK Explicit 4 Stage");
  RKMethods.push_back("RK Explicit 3/8 Rule");
  RKMethods.push_back("RK Explicit 4 Stage 3rd order by Runge");
  RKMethods.push_back("RK Explicit 5 Stage 3rd order by Kinnmark and Gray");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order TVD");
  RKMethods.push_back("RK Explicit 3 Stage 3rd order by Heun");
  RKMethods.push_back("RK Explicit 2 Stage 2nd order by Runge");
  RKMethods.push_back("RK Explicit Trapezoidal");
  RKMethods.push_back("General ERK");
  std::vector<double> RKMethodErrors;
  RKMethodErrors.push_back(0.154904);
  RKMethodErrors.push_back(4.55982e-06);
  RKMethodErrors.push_back(4.79132e-06);
  RKMethodErrors.push_back(0.000113603);
  RKMethodErrors.push_back(4.98796e-05);
  RKMethodErrors.push_back(0.00014564);
  RKMethodErrors.push_back(0.000121968);
  RKMethodErrors.push_back(0.000109495);
  RKMethodErrors.push_back(0.00559871);
  RKMethodErrors.push_back(0.00710492);
  RKMethodErrors.push_back(4.55982e-06);

  Teuchos::RCP<const Teuchos::Comm<int> > comm =
    Teuchos::DefaultComm<int>::getComm();
  Teuchos::RCP<Teuchos::FancyOStream> my_out =
    Teuchos::fancyOStream(Teuchos::rcpFromRef(std::cout));
  my_out->setProcRankAndSize(comm->getRank(), comm->getSize());
  my_out->setOutputToRootOnly(0);

  for(std::vector<std::string>::size_type m = 0; m != RKMethods.size(); m++) {

    std::string RKMethod_ = RKMethods[m];
    std::replace(RKMethod_.begin(), RKMethod_.end(), ' ', '_');
    std::replace(RKMethod_.begin(), RKMethod_.end(), '/', '.');
    std::vector<double> StepSize;
    std::vector<double> ErrorNorm;
    const int nTimeStepSizes = 7;
    double dt = 0.2;
    double order = 0.0;
    for (int n=0; n<nTimeStepSizes; n++) {

      // Read params from .xml file
      RCP<ParameterList> pList =
        getParametersFromXmlFile("Tempus_ExplicitRK_SinCos.xml");

      // Setup the SinCosModel
      RCP<ParameterList> scm_pl = sublist(pList, "SinCosModel", true);
      RCP<SinCosModel<double> > model =
        Teuchos::rcp(new SinCosModel<double>(scm_pl));

      // Set the Stepper
      RCP<ParameterList> pl = sublist(pList, "Tempus", true);
      if (RKMethods[m] == "General ERK") {
        pl->sublist("Demo Integrator").set("Stepper Name", "Demo Stepper 2");
      } else {
        pl->sublist("Demo Stepper").set("Stepper Type", RKMethods[m]);
      }


      dt /= 2;

      // Setup sensitivities
      ParameterList& sens_pl = pl->sublist("Sensitivities");
      sens_pl.set("Mass Matrix Is Identity", true); // Necessary for explicit
      ParameterList& interp_pl =
        pl->sublist("Demo Integrator").sublist("Solution History").sublist("Interpolator");
      interp_pl.set("Interpolator Type", "Lagrange");
      interp_pl.set("Order", 3); // All RK methods here are at most 4th order

      // Setup the Integrator and reset initial time step
      pl->sublist("Demo Integrator")
        .sublist("Time Step Control").set("Initial Time Step", dt);
      RCP<Tempus::IntegratorAdjointSensitivity<double> > integrator =
        Tempus::integratorAdjointSensitivity<double>(pl, model);
      order = integrator->getStepper()->getOrder();

      // Initial Conditions
      double t0 = pl->sublist("Demo Integrator")
        .sublist("Time Step Control").get<double>("Initial Time");
      // RCP<const Thyra::VectorBase<double> > x0 =
      //   model->getExactSolution(t0).get_x()->clone_v();
      RCP<Thyra::VectorBase<double> > x0 =
        model->getNominalValues().get_x()->clone_v();
      const int num_param = model->get_p_space(0)->dim();
      RCP<Thyra::MultiVectorBase<double> > DxDp0 =
        Thyra::createMembers(model->get_x_space(), num_param);
      for (int i=0; i<num_param; ++i)
        Thyra::assign(DxDp0->col(i).ptr(),
                      *(model->getExactSensSolution(i, t0).get_x()));
      integrator->setInitialState(t0, x0, Teuchos::null, Teuchos::null,
                                  DxDp0, Teuchos::null, Teuchos::null);

      // Integrate to timeMax
      bool integratorStatus = integrator->advanceTime();
      TEST_ASSERT(integratorStatus)

      // Test if at 'Final Time'
      double time = integrator->getTime();
      double timeFinal = pl->sublist("Demo Integrator")
        .sublist("Time Step Control").get<double>("Final Time");
      TEST_FLOATING_EQUALITY(time, timeFinal, 1.0e-14);

      // Time-integrated solution and the exact solution along with
      // sensitivities (relying on response g(x) = x).  Note we must transpose
      // dg/dp since the integrator returns it in gradient form.
      RCP<const Thyra::VectorBase<double> > x = integrator->getX();
      RCP<const Thyra::MultiVectorBase<double> > DgDp = integrator->getDgDp();
      RCP<Thyra::MultiVectorBase<double> > DxDp =
        Thyra::createMembers(model->get_x_space(), num_param);
      {
        Thyra::ConstDetachedMultiVectorView<double> dgdp_view(*DgDp);
        Thyra::DetachedMultiVectorView<double> dxdp_view(*DxDp);
        const int num_g = DgDp->domain()->dim();
        for (int i=0; i<num_g; ++i)
          for (int j=0; j<num_param; ++j)
            dxdp_view(i,j) = dgdp_view(j,i);
      }
      RCP<const Thyra::VectorBase<double> > x_exact =
        model->getExactSolution(time).get_x();
      RCP<Thyra::MultiVectorBase<double> > DxDp_exact =
        Thyra::createMembers(model->get_x_space(), num_param);
      for (int i=0; i<num_param; ++i)
        Thyra::assign(DxDp_exact->col(i).ptr(),
                      *(model->getExactSensSolution(i, time).get_x()));

      // Plot sample solution, exact solution, and adjoint solution
      if (comm->getRank() == 0 && n == nTimeStepSizes-1) {
        typedef Thyra::DefaultProductVector<double> DPV;
        typedef Thyra::DefaultMultiVectorProductVector<double> DMVPV;

        std::ofstream ftmp("Tempus_"+RKMethod_+"_SinCos_AdjSens.dat");
        RCP<const SolutionHistory<double> > solutionHistory =
          integrator->getSolutionHistory();
        for (int i=0; i<solutionHistory->getNumStates(); i++) {
          RCP<const SolutionState<double> > solutionState =
            (*solutionHistory)[i];
          const double time = solutionState->getTime();
          RCP<const DPV> x_prod_plot =
            Teuchos::rcp_dynamic_cast<const DPV>(solutionState->getX());
          RCP<const Thyra::VectorBase<double> > x_plot =
            x_prod_plot->getVectorBlock(0);
          RCP<const DMVPV > adjoint_prod_plot =
            Teuchos::rcp_dynamic_cast<const DMVPV>(x_prod_plot->getVectorBlock(1));
          RCP<const Thyra::MultiVectorBase<double> > adjoint_plot =
            adjoint_prod_plot->getMultiVector();
          RCP<const Thyra::VectorBase<double> > x_exact_plot =
            model->getExactSolution(time).get_x();
          ftmp << std::fixed << std::setprecision(7)
               << time
               << std::setw(11) << get_ele(*(x_plot), 0)
               << std::setw(11) << get_ele(*(x_plot), 1)
               << std::setw(11) << get_ele(*(adjoint_plot->col(0)), 0)
               << std::setw(11) << get_ele(*(adjoint_plot->col(0)), 1)
               << std::setw(11) << get_ele(*(adjoint_plot->col(1)), 0)
               << std::setw(11) << get_ele(*(adjoint_plot->col(1)), 1)
               << std::setw(11) << get_ele(*(x_exact_plot), 0)
               << std::setw(11) << get_ele(*(x_exact_plot), 1)
               << std::endl;
        }
        ftmp.close();
      }

      // Calculate the error
      RCP<Thyra::VectorBase<double> > xdiff = x->clone_v();
      RCP<Thyra::MultiVectorBase<double> > DxDpdiff = DxDp->clone_mv();
      Thyra::V_StVpStV(xdiff.ptr(), 1.0, *x_exact, -1.0, *(x));
      Thyra::V_VmV(DxDpdiff.ptr(), *DxDp_exact, *DxDp);
      StepSize.push_back(dt);
      double L2norm = Thyra::norm_2(*xdiff);
      L2norm *= L2norm;
      Teuchos::Array<double> L2norm_DxDp(num_param);
      Thyra::norms_2(*DxDpdiff, L2norm_DxDp());
      for (int i=0; i<num_param; ++i)
        L2norm += L2norm_DxDp[i]*L2norm_DxDp[i];
      L2norm = std::sqrt(L2norm);
      ErrorNorm.push_back(L2norm);

      *my_out << " n = " << n << " dt = " << dt << " error = " << L2norm
              << std::endl;
    }

    // Check the order and intercept
    double slope = computeLinearRegressionLogLog<double>(StepSize, ErrorNorm);
    *my_out << "  Stepper = " << RKMethods[m] << std::endl;
    *my_out << "  =========================" << std::endl;
    *my_out << "  Expected order: " << order << std::endl;
    *my_out << "  Observed order: " << slope << std::endl;
    *my_out << "  =========================" << std::endl;
    TEST_FLOATING_EQUALITY( slope, order, 0.015 );
    TEST_FLOATING_EQUALITY( ErrorNorm[0], RKMethodErrors[m], 1.0e-4 );

    if (comm->getRank() == 0) {
      std::ofstream ftmp("Tempus_"+RKMethod_+"_SinCos_AdjSens-Error.dat");
      double error0 = 0.8*ErrorNorm[0];
      for (int n=0; n<nTimeStepSizes; n++) {
        ftmp << StepSize[n]  << "   " << ErrorNorm[n] << "   "
             << error0*(pow(StepSize[n]/StepSize[0],order)) << std::endl;
      }
      ftmp.close();
    }
  }

  Teuchos::TimeMonitor::summarize();
}

} // namespace Tempus_Test
