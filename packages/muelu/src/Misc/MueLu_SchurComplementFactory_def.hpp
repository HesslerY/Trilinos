// @HEADER
//
// ***********************************************************************
//
//        MueLu: A package for multigrid based preconditioning
//                  Copyright 2012 Sandia Corporation
//
// Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
// the U.S. Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the Corporation nor the names of the
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY SANDIA CORPORATION "AS IS" AND ANY
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SANDIA CORPORATION OR THE
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef MUELU_SCHURCOMPLEMENTFACTORY_DEF_HPP_
#define MUELU_SCHURCOMPLEMENTFACTORY_DEF_HPP_

#include <Xpetra_BlockedCrsMatrix.hpp>
#include <Xpetra_MultiVectorFactory.hpp>
#include <Xpetra_VectorFactory.hpp>
#include <Xpetra_MatrixFactory.hpp>
#include <Xpetra_Matrix.hpp>
#include <Xpetra_CrsMatrixWrap.hpp>
#include <Xpetra_BlockedCrsMatrix.hpp>
#include <Xpetra_CrsMatrix.hpp>
#include "MueLu_Level.hpp"
#include "MueLu_Monitor.hpp"
#include "MueLu_Utilities.hpp"
#include "MueLu_HierarchyHelpers.hpp"

#include "MueLu_SchurComplementFactory.hpp"

namespace MueLu {

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
SchurComplementFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::SchurComplementFactory(/*Scalar omega*//**size_t row, size_t col, LocalOrdinal blksize*/)
//: omega_(omega)
  { }

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
SchurComplementFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::~SchurComplementFactory() {}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
RCP<const ParameterList> SchurComplementFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::GetValidParameterList(const ParameterList& paramList) const {
  RCP<ParameterList> validParamList = rcp(new ParameterList());

  validParamList->set< RCP<const FactoryBase> >("A", MueLu::NoFactory::getRCP()/*Teuchos::null*/, "Generating factory of the matrix A used for building SchurComplement (must be a 2x2 blocked operator, default = MueLu::NoFactory::getRCP())");
  validParamList->set< Scalar >          ("omega",   Teuchos::ScalarTraits<SC>::one(), "Scaling parameter in S = - 1/omega D diag{F}^{-1} G + Z");
  validParamList->set< bool >            ("lumping", false, "Use lumping, i.e. use the row sum of the absolute values on the diagonal as approximation of A00 (and A00^{-1}). default: false, just use diag(A00).");

  return validParamList;
}


template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void SchurComplementFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::DeclareInput(Level &currentLevel) const {
  Input(currentLevel, "A");
}

template <class Scalar, class LocalOrdinal, class GlobalOrdinal, class Node, class LocalMatOps>
void SchurComplementFactory<Scalar, LocalOrdinal, GlobalOrdinal, Node, LocalMatOps>::Build(Level & currentLevel) const
{
  FactoryMonitor  m(*this, "SchurComplementFactory", currentLevel);
  Teuchos::RCP<Matrix> A = Get<RCP<Matrix> >(currentLevel, "A");

  RCP<BlockedCrsMatrix> bA = Teuchos::rcp_dynamic_cast<BlockedCrsMatrix>(A);
  TEUCHOS_TEST_FOR_EXCEPTION(bA == Teuchos::null, Exceptions::BadCast, "MueLu::SchurComplementFactory::Build: input matrix A is not of type BlockedCrsMatrix! A generated by AFact_ must be a 2x2 block operator. error.");

  Teuchos::RCP<CrsMatrix> A00 = bA->getMatrix(0,0);
  Teuchos::RCP<CrsMatrix> A01 = bA->getMatrix(0,1);
  Teuchos::RCP<CrsMatrix> A10 = bA->getMatrix(1,0);
  Teuchos::RCP<CrsMatrix> A11 = bA->getMatrix(1,1);

  Teuchos::RCP<CrsMatrixWrap> Op00 = Teuchos::rcp(new CrsMatrixWrap(A00));
  Teuchos::RCP<CrsMatrixWrap> Op01 = Teuchos::rcp(new CrsMatrixWrap(A01));
  Teuchos::RCP<CrsMatrixWrap> Op10 = Teuchos::rcp(new CrsMatrixWrap(A10));
  Teuchos::RCP<CrsMatrixWrap> Op11 = Teuchos::rcp(new CrsMatrixWrap(A11));

  Teuchos::RCP<Matrix> F = Teuchos::rcp_dynamic_cast<Matrix>(Op00);
  Teuchos::RCP<Matrix> G = Teuchos::rcp_dynamic_cast<Matrix>(Op01);
  Teuchos::RCP<Matrix> D = Teuchos::rcp_dynamic_cast<Matrix>(Op10);
  Teuchos::RCP<Matrix> Z = Teuchos::rcp_dynamic_cast<Matrix>(Op11);

  // TODO move this to BlockedCrsMatrix->getMatrix routine...
  F->CreateView("stridedMaps", bA->getRangeMap(0), bA->getDomainMap(0));
  G->CreateView("stridedMaps", bA->getRangeMap(0), bA->getDomainMap(1));
  D->CreateView("stridedMaps", bA->getRangeMap(1), bA->getDomainMap(0));
  Z->CreateView("stridedMaps", bA->getRangeMap(1), bA->getDomainMap(1));

  const ParameterList& pL = GetParameterList();
  Scalar omega = pL.get<Scalar>("omega");

  // copy the value of G so we can do the left scale.
  RCP<Matrix> FhatinvG = MatrixFactory::Build(G->getRowMap(), G->getGlobalMaxNumRowEntries());
  RCP<Matrix> emptyMat = MatrixFactory::Build(G->getRowMap(), G->getGlobalMaxNumRowEntries());
  emptyMat->fillComplete(G->getDomainMap(),G->getRowMap());
  Utils2::TwoMatrixAdd(*G,false,1.0,*emptyMat,false,-1.0/omega,FhatinvG,GetOStream(Statistics2,0));
  FhatinvG->fillComplete(G->getDomainMap(),G->getRowMap()); // complete the matrix. left scaling does not change the pattern of the operator.

  bool lumping = pL.get<bool>("lumping");
  if(!lumping) {
    // extract diagonal of F. store it in ArrayRCP object
    Teuchos::ArrayRCP<SC> AdiagFinv = Utils::GetMatrixDiagonal(*F);
    for (size_t k = 0; k < Teuchos::as<size_t> (AdiagFinv.size ()); ++k) {
      if (Teuchos::ScalarTraits<SC>::magnitude(AdiagFinv[k]) < 1e-4)
        AdiagFinv[k] = Teuchos::ScalarTraits<SC>::one();
    }
    Utils::MyOldScaleMatrix(*FhatinvG,AdiagFinv,true,false,false);  // TODO check the MyOldScaleMatrix routine...
  } else {
    // use diagonal of lumped matrix as approximation
    Teuchos::ArrayRCP<SC> AdiagFinv = Utils::GetLumpedMatrixDiagonal(*F);
    Utils::MyOldScaleMatrix(*FhatinvG,AdiagFinv,true,false,false);  // TODO check the MyOldScaleMatrix routine...
  }

  // build D \hat{F}^{-1} G
  RCP<Matrix> DFhatinvG = Utils::Multiply(*D,false,*FhatinvG,false,GetOStream(Statistics2,0));

  // build full SchurComplement operator
  // S = - 1/omega D \hat{F}^{-1} G + Z
  RCP<Matrix> S;
  Utils2::TwoMatrixAdd(*Z,false,1.0,*DFhatinvG,false,-1.0/omega,S,GetOStream(Statistics2,0));
  S->fillComplete();

  {
    // note: variable "A" generated by this SchurComplement factory is in fact the SchurComplement matrix
    // we have to use the variable name "A" since the Smoother object expects the matrix to be called "A"
    Set(currentLevel, "A", S);
  }
}

} // namespace MueLu

#endif /* MUELU_SCHURCOMPLEMENTFACTORY_DEF_HPP_ */
