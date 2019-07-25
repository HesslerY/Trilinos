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
//                    Jonathan Hu       (jhu@sandia.gov)
//                    Andrey Prokopenko (aprokop@sandia.gov)
//                    Tobias Wiesner    (tawiesn@sandia.gov)
//                    Ray Tuminaro      (rstumin@sandia.gov)
//
// ***********************************************************************
//
// @HEADER
#ifndef XPETRA_BLOCKEDMULTIVECTOR_DECL_HPP
#define XPETRA_BLOCKEDMULTIVECTOR_DECL_HPP

/* this file is automatically generated - do not edit (see script/interfaces.py) */

#include "Xpetra_ConfigDefs.hpp"
#include "Xpetra_Map.hpp"
#include "Xpetra_MultiVector.hpp"

#include "Xpetra_BlockedMap.hpp"

#include "Xpetra_MultiVectorFactory.hpp"
#include "Xpetra_MapExtractor.hpp"
#include "Xpetra_BlockedVector.hpp"

namespace Xpetra {

#ifndef DOXYGEN_SHOULD_SKIP_THIS
    // forward declaration of Vector, needed to prevent circular inclusions
    template<class S, class LO, class GO, class N> class Vector;
    //template<class S, class LO, class GO, class N> class BlockedVector;
    //template<class S, class LO, class GO, class N> class MapExtractor;
    //template<class S, class LO, class GO, class N> class MultiVectorFactory;
#endif


  template <class Scalar = double,
            class LocalOrdinal = Map<>::local_ordinal_type,
            class GlobalOrdinal = typename Map<LocalOrdinal>::global_ordinal_type,
            class Node = typename Map<LocalOrdinal, GlobalOrdinal>::node_type>
  class BlockedMultiVector
    : public MultiVector< Scalar, LocalOrdinal, GlobalOrdinal, Node >
  {
  public:
    typedef Scalar scalar_type;
    typedef LocalOrdinal local_ordinal_type;
    typedef GlobalOrdinal global_ordinal_type;
    typedef Node node_type;

  private:

#undef XPETRA_BLOCKEDMULTIVECTOR_SHORT
#include "Xpetra_UseShortNames.hpp"

  public:

    //! @name Constructor/Destructor Methods
    //@{

    //! Constructor


    /*!
     * Const version of constructor which accepts a const version
     * of a blocked map
     *
     * \param map BlockedMap defining the block structure of the multi vector
     * \param NumVectors Number of vector columns in multi vector
     * \param zeroOut If true initialize multivector with zeros
     */
    BlockedMultiVector(const Teuchos::RCP<const BlockedMap>& map, size_t NumVectors, bool zeroOut = true);


    /*!
     * Const version of constructor which accepts a const version
     * of the multi-vector
     *
     * \note If you change the information in input vector v the data in the
     *       blocked multi-vector are not affected (and vice versa). Consider
     *       the blocked multivector to be a copy of the input multivector (not a view)
     *
     * \param bmap BlockedMap object containing information about the block splitting
     * \param v MultiVector that is to be splitted into a blocked multi vector
     */
    BlockedMultiVector(Teuchos::RCP<const Xpetra::BlockedMap<LocalOrdinal,GlobalOrdinal,Node> > bmap,
                       Teuchos::RCP<const MultiVector> v);


    /*!
     * Const version of constructor which accepts a const version
     * of the multi-vector
     *
     * \note If you change the information in input vector v the data in the
     *       blocked multi-vector are not affected (and vice versa). Consider
     *       the blocked multivector to be a copy of the input multivector (not a view)
     *
     * \param mapExtractor MapExtractor object containing information about the block splitting
     * \param v MultiVector that is to be splitted into a blocked multi vector
     */
    BlockedMultiVector(Teuchos::RCP<const Xpetra::MapExtractor<Scalar,LocalOrdinal,GlobalOrdinal,Node>> mapExtractor,
                       Teuchos::RCP<const MultiVector> v);


    /*!
     * Constructor to build a blocked multivector from a blocked map and component vectors
     *
     * \note We do *NOT* check map compatibility between the BlockedMap and the array of multivectors
     *
     * \param  map BlockedMap object containing information about the block splitting
     * \param  vin A std::vector of RCPs to component vectors
     */
    BlockedMultiVector(const Teuchos::RCP< const BlockedMap > &map, std::vector<Teuchos::RCP<MultiVector> > & vin);


    //! Destructor.
    virtual ~BlockedMultiVector();


    /// \brief Assignment operator: Does a deep copy.
    ///
    /// The assignment operator does a deep copy, just like
    /// subclasses' copy constructors.
    ///
    /// \note This currently only works if both <tt>*this</tt> and the
    ///   input argument are instances of the same subclass.
    BlockedMultiVector<Scalar, LocalOrdinal, GlobalOrdinal, Node>& operator=(const MultiVector& rhs);


    //@}
    //! @name Post-construction modification routines
    //@{

    //! Replace value, using global (row) index.
    virtual void replaceGlobalValue(GlobalOrdinal /* globalRow */, size_t /* vectorIndex */, const Scalar &/* value */);

    //! Add value to existing value, using global (row) index.
    virtual void sumIntoGlobalValue(GlobalOrdinal /* globalRow */, size_t /* vectorIndex */, const Scalar &/* value */);

    //! Replace value, using local (row) index.
    virtual void replaceLocalValue(LocalOrdinal /* myRow */, size_t /* vectorIndex */, const Scalar &/* value */);

    //! Add value to existing value, using local (row) index.
    virtual void sumIntoLocalValue(LocalOrdinal /* myRow */, size_t /* vectorIndex */, const Scalar &/* value */);

    //! Set all values in the multivector with the given value.
    virtual void putScalar(const Scalar& value);


    //@}
    //! @name Data Copy and View get methods
    //@{


    //! Return a Vector which is a const view of column j.
    virtual Teuchos::RCP<const Xpetra::Vector<Scalar, LocalOrdinal, GlobalOrdinal, Node>> getVector(size_t j) const;


    //! Return a Vector which is a nonconst view of column j.
    virtual Teuchos::RCP<Xpetra::Vector<Scalar, LocalOrdinal, GlobalOrdinal, Node>> getVectorNonConst(size_t j);


    //! Const view of the local values in a particular vector of this multivector.
    virtual Teuchos::ArrayRCP<const Scalar> getData(size_t j) const;


    //! View of the local values in a particular vector of this multivector.
    virtual Teuchos::ArrayRCP<Scalar> getDataNonConst(size_t j);


    //@}
    //! @name Mathematical methods
    //@{


    //! Compute dot product of each corresponding pair of vectors, dots[i] = this[i].dot(A[i]).
    virtual void dot(const MultiVector& /* A */, const Teuchos::ArrayView<Scalar>& /* dots */) const;


    //! Put element-wise absolute values of input Multi-vector in target: A = abs(this).
    virtual void abs(const MultiVector& /* A */);


    //! Put element-wise reciprocal values of input Multi-vector in target, this(i,j) = 1/A(i,j).
    virtual void reciprocal(const MultiVector& /* A */);


    //! Scale the current values of a multi-vector, this = alpha*this.
    virtual void scale(const Scalar& alpha);


    //! Scale the current values of a multi-vector, this[j] = alpha[j]*this[j].
    virtual void scale(Teuchos::ArrayView<const Scalar> alpha);


    //! Update multi-vector values with scaled values of A, this = beta*this + alpha*A.
    virtual void update(const Scalar& alpha, const MultiVector& A, const Scalar& beta);


    //! Update multi-vector with scaled values of A and B, this = gamma*this + alpha*A + beta*B.
    virtual void update(const Scalar &alpha, const MultiVector&A, const Scalar &beta, const MultiVector&B, const Scalar &gamma);


    //! Compute 1-norm of each vector in multi-vector.
    virtual void norm1(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const;


    //!
    virtual void norm2(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const;


    //! Compute Inf-norm of each vector in multi-vector.
    virtual void normInf(const Teuchos::ArrayView< typename Teuchos::ScalarTraits< Scalar >::magnitudeType > &norms) const;


    //! Compute mean (average) value of each vector in multi-vector. The outcome of this routine is undefined for non-floating point scalar types (e.g., int).
    virtual void meanValue(const Teuchos::ArrayView< Scalar > &/* means */) const;


    //! Matrix-matrix multiplication: this = beta*this + alpha*op(A)*op(B).
    virtual void multiply(Teuchos::ETransp /* transA */, Teuchos::ETransp /* transB */, const Scalar &/* alpha */, const MultiVector&/* A */, const MultiVector&/* B */, const Scalar &/* beta */);


    //! Element-wise multiply of a Vector A with a MultiVector B.
    virtual void elementWiseMultiply(Scalar scalarAB, const Xpetra::Vector<Scalar,LocalOrdinal,GlobalOrdinal,Node>&A, const MultiVector&B, Scalar scalarThis);


    //@}
    //! @name Attribute access functions
    //@{


    //! Number of columns in the multivector.
    virtual size_t getNumVectors() const;


    //! Local number of rows on the calling process.
    virtual size_t getLocalLength() const;


    //! Global number of rows in the multivector.
    virtual global_size_t getGlobalLength() const;


    //! Local number of rows on the calling process.
    virtual bool isSameSize(const Xpetra::MultiVector<Scalar,LocalOrdinal,GlobalOrdinal,Node> & vec) const;


    //@}
    //! @name Overridden from Teuchos::Describable
    //@{


    //! A simple one-line description of this object.
    virtual std::string description() const;


    //! Print the object with the given verbosity level to a FancyOStream.
    virtual void describe(Teuchos::FancyOStream &out, const Teuchos::EVerbosityLevel verbLevel=Teuchos::Describable::verbLevel_default) const;


    virtual void replaceMap(const RCP<const Map>& map);


    //! Import.
    virtual void doImport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &/* source */, const Import& /* importer */, CombineMode /* CM */);


    //! Export.
    virtual void doExport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &/* dest */, const Import& /* importer */, CombineMode /* CM */);


    //! Import (using an Exporter).
    virtual void doImport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &/* source */, const Export& /* exporter */, CombineMode /* CM */);


    //! Export (using an Importer).
    virtual void doExport(const DistObject<Scalar, LocalOrdinal, GlobalOrdinal, Node> &/* dest */, const Export& /* exporter */, CombineMode /* CM */);


    //@}
    //! @name Xpetra specific
    //@{


    //! Set seed for Random function.
    virtual void setSeed(unsigned int seed);


    virtual void randomize(bool bUseXpetraImplementation = false);


    //! Set multi-vector values to random numbers. XPetra implementation
    virtual void Xpetra_randomize();


#ifdef HAVE_XPETRA_KOKKOS_REFACTOR
    typedef typename Kokkos::Details::ArithTraits<Scalar>::val_type impl_scalar_type;
    typedef Kokkos::DualView<impl_scalar_type**, Kokkos::LayoutStride,
        typename node_type::execution_space,
        Kokkos::MemoryUnmanaged> dual_view_type;
    typedef typename dual_view_type::host_mirror_space host_execution_space;
    typedef typename dual_view_type::t_dev::execution_space dev_execution_space;

    /// \brief Return an unmanaged non-const view of the local data on a specific device.
    /// \tparam TargetDeviceType The Kokkos Device type whose data to return.
    ///
    /// \warning Be aware that the view on the multivector data is non-persisting, i.e.
    ///          only valid as long as the multivector does not run of scope!
    template<class TargetDeviceType>
    typename Kokkos::Impl::if_c<
      Kokkos::Impl::is_same<
        typename dev_execution_space::memory_space,
        typename TargetDeviceType::memory_space>::value,
        typename dual_view_type::t_dev_um,
        typename dual_view_type::t_host_um>::type
    getLocalView () const {
      if(Kokkos::Impl::is_same<
                   typename host_execution_space::memory_space,
                   typename TargetDeviceType::memory_space
         >::value) {
        return getHostLocalView();
      } else {
        return getDeviceLocalView();
      }
    }

    virtual typename dual_view_type::t_host_um getHostLocalView ()  const {
      typename dual_view_type::t_host_um test;
      return test;
    }

    virtual typename dual_view_type::t_dev_um  getDeviceLocalView() const {
      typename dual_view_type::t_dev_um test;
      return test;
    }
#endif


    //@}


    //! Access function for the underlying Map this DistObject was constructed with.
    Teuchos::RCP< const Map> getMap() const;


    //! Access function for the underlying Map this DistObject was constructed with.
    Teuchos::RCP< const Xpetra::BlockedMap<LocalOrdinal,GlobalOrdinal,Node> > getBlockedMap() const;


    /// return partial multivector associated with block row r
    Teuchos::RCP<MultiVector> getMultiVector(size_t r) const;


    /// return partial multivector associated with block row r
    Teuchos::RCP<MultiVector> getMultiVector(size_t r, bool bThyraMode) const;


    /// set partial multivector associated with block row r
    void setMultiVector(size_t r, Teuchos::RCP<const MultiVector> v, bool bThyraMode);


    /// merge BlockedMultiVector blocks to a single MultiVector
    Teuchos::RCP<MultiVector> Merge() const;


  protected:


    /// \brief Implementation of the assignment operator (operator=);
    ///   does a deep copy.
    ///
    /// Each subclass must implement this.  This includes
    /// Xpetra::EpetraMultiVector and Xpetra::TpetraMultiVector as well as
    /// Xpetra::BockedMultiVector
    virtual void assign (const MultiVector& rhs);

  private:


    // helper routines for interaction of MultiVector and BlockedMultiVectors

    void ExtractVector(RCP<const MultiVector>& full, size_t block, RCP<MultiVector>& partial) const;
    void ExtractVector(RCP<      MultiVector>& full, size_t block, RCP<MultiVector>& partial) const;

    RCP<MultiVector> ExtractVector(RCP<const MultiVector>& full, size_t block, bool bThyraMode = false) const;
    RCP<MultiVector> ExtractVector(RCP<      MultiVector>& full, size_t block, bool bThyraMode = false) const;

    void ExtractVector(const MultiVector& full, size_t block, MultiVector& partial) const;


    void InsertVector(const     MultiVector& partial, size_t block,     MultiVector& full, bool bThyraMode = false) const; // 1019
    void InsertVector(RCP<const MultiVector> partial, size_t block, RCP<MultiVector> full, bool bThyraMode = false) const; // 1059
    void InsertVector(RCP<      MultiVector> partial, size_t block, RCP<MultiVector> full, bool bThyraMode = false) const; // 1069


  private:


    Teuchos::RCP<const BlockedMap>          map_;         ///< blocked map containing the sub block maps (either thyra or xpetra mode)
    std::vector<Teuchos::RCP<MultiVector> > vv_;          ///< array containing RCPs of the partial vectors
    size_t                                  numVectors_;  ///< number of vectors (columns in multi vector)

  }; // BlockedMultiVector class


} // Xpetra namespace


#define XPETRA_BLOCKEDMULTIVECTOR_SHORT
#endif // XPETRA_BLOCKEDMULTIVECTOR_DECL_HPP


