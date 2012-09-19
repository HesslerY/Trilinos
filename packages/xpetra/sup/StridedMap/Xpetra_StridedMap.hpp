// @HEADER
//
// ***********************************************************************
//
//             Xpetra: A linear algebra interface package
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
//                    Jeremie Gaidamour (jngaida@sandia.gov)
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER

// WARNING: This code is experimental. Backwards compatibility should not be expected.

#ifndef XPETRA_STRIDEDMAP_HPP
#define XPETRA_STRIDEDMAP_HPP

/* this file is automatically generated - do not edit (see script/interfaces.py) */

#include <Kokkos_DefaultNode.hpp>
#include <Teuchos_Describable.hpp>
#include "Xpetra_Map.hpp"
#include "Xpetra_ConfigDefs.hpp"

#include "Xpetra_Exceptions.hpp"

// MPI helper
#define sumAll(rcpComm, in, out)                                        \
  Teuchos::reduceAll(*rcpComm, Teuchos::REDUCE_SUM, in, Teuchos::outArg(out));
#define minAll(rcpComm, in, out)                                        \
  Teuchos::reduceAll(*rcpComm, Teuchos::REDUCE_MIN, in, Teuchos::outArg(out));
#define maxAll(rcpComm, in, out)                                        \
  Teuchos::reduceAll(*rcpComm, Teuchos::REDUCE_MAX, in, Teuchos::outArg(out));

namespace Xpetra {

  /*!
    @class StridedMap
    @brief Class that stores a strided map

    StridedMap extends the functionality of Xpetra::Map

    It derives from Xpetra::Map and adds a std::vector, which contains the striding information.
    E.g. for a strided map with 3dofs per node (2 velocity dofs, 1 pressure dof) the striding
    information looks like:
    std::vector<size_t> stridingInformation;
    stridingInformation.push_back(2); // 2 velocity dofs
    stridingInformation.push_back(1); // 1 pressure dof

    For this example the getFixedBlockSize() returns 3 (3 dofs per node).
    Providing a stridedBlockId parameter in the constructor the strided map only contains dofs of
    one strided block, e.g. with above stridingInformation the call

    StridingMap M(33,0,stridiningInformation,comm,0); // striding block 0 (velocity dofs)
    returns a map with the gids
    0, 1, 3, 4, 6, 7, ... (which contains only the velocity dofs)

    and
    StridingMap M(33,0,stridiningInformation,comm,1); // striding block 1 (pressure dofs)
    creates a map with only the pressure dofs
    2, 5, 8, ...

    @note: there's no support for global offset, yet.
  */
  template <class LocalOrdinal, class GlobalOrdinal = LocalOrdinal, class Node = Kokkos::DefaultNode::DefaultNodeType>
  class StridedMap
    : public virtual Map<LocalOrdinal, GlobalOrdinal, Node>
  {

  public:

    //! @name Constructor/Destructor Methods
    //@{ 

    StridedMap(global_size_t numGlobalElements, GlobalOrdinal indexBase, std::vector<size_t>& stridingInfo, const Teuchos::RCP< const Teuchos::Comm< int > > &comm, LocalOrdinal stridedBlockId, GlobalOrdinal offset = 0)
    : stridingInfo_(stridingInfo), stridedBlockId_(stridedBlockId), offset_(offset)
    { 
      TEUCHOS_TEST_FOR_EXCEPTION(stridingInfo.size() == 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: stridingInfo.size() = 0?");
      //TEUCHOS_TEST_FOR_EXCEPTION(numGlobalElements % getFixedBlockSize() != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: getFixedBlockSize is not an integer multiple of numGlobalElements.");

    }

    StridedMap(global_size_t numGlobalElements, size_t numLocalElements, GlobalOrdinal indexBase, std::vector<size_t>& stridingInfo, const Teuchos::RCP< const Teuchos::Comm< int > > &comm, LocalOrdinal stridedBlockId, GlobalOrdinal offset = 0)
    : stridingInfo_(stridingInfo), stridedBlockId_(stridedBlockId), offset_(offset)
    { 
      TEUCHOS_TEST_FOR_EXCEPTION(stridingInfo.size() == 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: stridingInfo.size() = 0?");
      //TEUCHOS_TEST_FOR_EXCEPTION(numGlobalElements % getFixedBlockSize() != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: getFixedBlockSize is not an integer multiple of numGlobalElements.");
      //TEUCHOS_TEST_FOR_EXCEPTION(numLocalElements % getFixedBlockSize() != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: getFixedBlockSize is not an integer multiple of numLocalElements.");
    
    }

    StridedMap(std::vector<size_t>& stridingInfo, LocalOrdinal stridedBlockId, GlobalOrdinal offset = 0)
    : stridingInfo_(stridingInfo), stridedBlockId_(stridedBlockId), offset_(offset)
    {
    }

    StridedMap(global_size_t numGlobalElements, const Teuchos::ArrayView< const GlobalOrdinal > &elementList, GlobalOrdinal indexBase, std::vector<size_t>& stridingInfo, const Teuchos::RCP< const Teuchos::Comm< int > > &comm, LocalOrdinal stridedBlockId=-1)
    : stridingInfo_(stridingInfo), stridedBlockId_(stridedBlockId)
    {
      TEUCHOS_TEST_FOR_EXCEPTION(stridingInfo.size() == 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: stridingInfo.size() = 0?");
      TEUCHOS_TEST_FOR_EXCEPTION(stridedBlockId < -1, Exceptions::RuntimeError, "StridedMap::StridedMap: stridedBlockId must not be smaller than -1.");

      // the following tests are not valid if stridedBlockId != 1
      if(stridedBlockId == -1) {
        TEUCHOS_TEST_FOR_EXCEPTION(numGlobalElements % getFixedBlockSize() != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: getFixedBlockSize is not an integer multiple of numGlobalElements.");
        TEUCHOS_TEST_FOR_EXCEPTION(elementList.size() % getFixedBlockSize() != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: getFixedBlockSize is not an integer multiple of elementList.size().");
      } else {
        // numGlobalElements can be -1! FIXME
        //TEUCHOS_TEST_FOR_EXCEPTION(numGlobalElements % stridingInfo[stridedBlockId] != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: stridingBlockInfo[stridedBlockId] is not an integer multiple of numGlobalElements.");
        TEUCHOS_TEST_FOR_EXCEPTION(elementList.size() % stridingInfo[stridedBlockId] != 0, Exceptions::RuntimeError, "StridedMap::StridedMap: stridingInfo not valid: stridingBlockInfo[stridedBlockId] is not an integer multiple of elementList.size().");
      }
       
      // calculate offset_

      // find minimum GID over all procs
      GlobalOrdinal minGidOnCurProc = 99999;  // TODO use scalar traits for max possible gid.
      for(Teuchos_Ordinal k=0; k<elementList.size(); ++k) { // TODO fix occurence of Teuchos_Ordinal
        if(elementList[k] < minGidOnCurProc) minGidOnCurProc = elementList[k];
      }
      Teuchos::reduceAll(*comm, Teuchos::REDUCE_MIN, minGidOnCurProc, Teuchos::outArg(offset_));

      // calculate striding index
      size_t nStridedOffset = 0;
      for(int j=0; j<stridedBlockId; j++) {
        nStridedOffset += stridingInfo[j];
      }
      const GlobalOrdinal goStridedOffset = Teuchos::as<GlobalOrdinal>(nStridedOffset);

      // adapt offset_
      offset_ -= goStridedOffset;
    }
     
    //! Destructor.
    virtual ~StridedMap() { }

   //@}

    //! @name Access functions for striding data
    //@{
    
    std::vector<size_t> getStridingData() const { return stridingInfo_; }

    void setStridingData(std::vector<size_t> stridingInfo) { stridingInfo_ = stridingInfo; }

    size_t getFixedBlockSize() const {
      // sum up size of all strided blocks (= number of dofs per node)
      size_t blkSize = 0;
      std::vector<size_t>::const_iterator it;
      for(it = stridingInfo_.begin(); it != stridingInfo_.end(); ++it) {
        blkSize += *it;
      }
      return blkSize;
    }

    /// returns strided block id of the dofs stored in this map
    /// or -1 if full strided map is stored in this map
    LocalOrdinal getStridedBlockId() const { return stridedBlockId_; }

    /// returns true, if this is a strided map (i.e. more than 1 strided blocks)
    bool isStrided() { return stridingInfo_.size() > 1 ? true : false; }

    /// returns true, if this is a blocked map (i.e. more than 1 dof per node)
    /// either strided or just 1 block per node
    bool isBlocked() { return getFixedBlockSize() > 1 ? true : false; }
    
    GlobalOrdinal getOffset() const { return offset_; } 
    
    void setOffset( GlobalOrdinal offset ) { offset_ = offset; }
    
    // returns number of strided block id which gid belongs to.
    size_t GID2StridingBlockId( GlobalOrdinal gid ) const {
      GlobalOrdinal tgid = gid - offset_;
      tgid = tgid % getFixedBlockSize();

      size_t nStridedOffset = 0;
      size_t stridedBlockId = 0;
      for(size_t j=0; j<stridingInfo_.size(); j++) {
        nStridedOffset += stridingInfo_[j];
        if(Teuchos::as<size_t>(tgid) < nStridedOffset) {
          stridedBlockId = j;
          break;
        }
      }
      return stridedBlockId;
    }

    /* // function currently not needed but maybe useful
    std::vector<GlobalOrdinal> NodeId2GlobalDofIds(GlobalOrdinal nodeId) const {
      TEUCHOS_TEST_FOR_EXCEPTION(stridingInfo_.size() == 0, Exceptions::RuntimeError, "StridedMap::NodeId2GlobalDofIds: stridingInfo not valid: stridingInfo.size() = 0?");
      std::vector<GlobalOrdinal> dofs;
      if(stridedBlockId_ > -1) {
          TEUCHOS_TEST_FOR_EXCEPTION(stridingInfo_[stridedBlockId_] == 0, Exceptions::RuntimeError, "StridedMap::NodeId2GlobalDofIds: stridingInfo not valid: stridingInfo[stridedBlockId] = 0?");

          // determine nStridedOffset
          size_t nStridedOffset = 0;
          for(int j=0; j<stridedBlockId_; j++) {
            nStridedOffset += stridingInfo_[j];
          }

          for(size_t i = 0; i<stridingInfo_[stridedBlockId_]; i++) {
            GlobalOrdinal gid =
                nodeId * Teuchos::as<GlobalOrdinal>(getFixedBlockSize()) +
                offset_ +
                Teuchos::as<GlobalOrdinal>(nStridedOffset) +
                Teuchos::as<GlobalOrdinal>(i);
            dofs.push_back(gid);
          }
      } else {
        for(size_t i = 0; i<getFixedBlockSize(); i++) {
          GlobalOrdinal gid =
              nodeId * Teuchos::as<GlobalOrdinal>(getFixedBlockSize()) +
              offset_ +
              Teuchos::as<GlobalOrdinal>(i);
          dofs.push_back(gid);
        }
      }
      return dofs;
    }*/
    //@}

  private:
    virtual bool CheckConsistency() = 0;

  protected:
    std::vector<size_t> stridingInfo_;   //!< vector with size of strided blocks (dofs)
    LocalOrdinal stridedBlockId_;        //!< member variable denoting which dofs are stored in map
                                         // stridedBlock == -1: the full map (with all strided block dofs)
                                         // stridedBlock >  -1: only dofs of strided block with index "stridedBlockId" are stored in this map
    GlobalOrdinal offset_;		 //!< offset for gids in map (default = 0).

  }; // StridedMap class

} // Xpetra namespace

#define XPETRA_STRIDEDMAP_SHORT
#endif // XPETRA_STRIDEDMAP_HPP
