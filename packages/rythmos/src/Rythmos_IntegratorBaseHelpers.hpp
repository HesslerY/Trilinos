//@HEADER
// ***********************************************************************
//
//                     Rythmos Package
//                 Copyright (2006) Sandia Corporation
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
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301
// USA
// Questions? Contact Todd S. Coffey (tscoffe@sandia.gov)
//
// ***********************************************************************
//@HEADER

#ifndef Rythmos_INTEGRATOR_BASE_HELPERS_HPP
#define Rythmos_INTEGRATOR_BASE_HELPERS_HPP


#include "Rythmos_IntegratorBaseHelpers.hpp"


namespace Rythmos {


/** \brief Reset the initial conditions.
 *
 * \relates InterplatorBase
 */
template<class Scalar>
void resetInitialConditions(
  const Thyra::ModelEvaluatorBase::InArgs<double> &ic,
  const Ptr<IntegratorBase<Scalar> > &integrator
  )
{
  const RCP<StepperBase<Scalar> > stepper = integrator->getNonconstStepper();
  stepper->setInitialCondition(ic);
  integrator->setStepper(stepper, integrator->getFwdTimeRange().upper());
}


} // namespace Rythmos


#endif //Rythmos_INTEGRATOR_BASE_HELPERS_HPP
