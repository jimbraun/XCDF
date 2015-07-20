
/*
Copyright (c) 2014, University of Maryland
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef XCDF_DATA_MANAGER_INCLUDED_H
#define XCDF_DATA_MANAGER_INCLUDED_H

#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFField.h>
#include <xcdf/XCDFDefs.h>
#include <xcdf/XCDFBlockData.h>
#include <xcdf/XCDFFrame.h>

#include <string>
#include <cmath>
#include <stdint.h>

/*!
 * @class XCDFDataManager
 * @author Jim Braun
 * @brief XCDF field data routines used internally by XCDFFile.
 */

template <typename T>
class XCDFDataManager {


  public:

    XCDFDataManager(XCDFFieldType type,
                    XCDFField<T> field,
                    XCDFField<uint64_t> parent,
                    bool hasParent,
                    XCDFPtr<XCDFBlockData> dataPtr) :
                            type_(type),
                            field_(field),
                            parent_(parent),
                            hasParent_(hasParent),
                            dataPtr_(dataPtr),
                            uncompressedBlock_(xcdf_shared(
                                          new XCDFUncompressedBlock())) { }

    XCDFFieldType GetType() const {return type_;}

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.
     */
    void AddIntegerRepresentation(uint64_t datum) {

      AddUnchecked(CalculateTypeValue(datum));
    }

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.  Need range check in the
     *  case of file append
     */
    void AddIntegerRepresentationAppend(uint64_t datum) {

      field_.fieldData_->AddAppend(CalculateTypeValue(datum));
    }

    /*
     *  Get the #resolution units between active min and datum
     *  at the given index.
     */
    uint64_t GetIntegerRepresentation(int index) const {

      return CalculateIntegerValue(At(index));
    }

    /*
     * Align the field active min to be an integer number of resolution
     * units from zero.  This avoids field data that changes value as
     * the active min changes block-to-block.
     */
    void ZeroAlign() { }

    /// Set the active size of the field, as when reading back a file.
    void SetActiveSize(const uint32_t activeSize) {
      field_.fieldData_->SetActiveSize(activeSize);
    }

    /*
     * Enter in data of known limits.
     */
    void AddUnchecked(T value) {field_.fieldData_->AddUnchecked(value);}

    /*
     * Clear the data from the XCDFFieldData vector.
     */
    void Clear() {field_.fieldData_->Clear();}

    /*
     * Shrink the underlying buffer in the XCDFFieldData vector.
     * This is a reallocation and invalidates iterators.
     */
    void Shrink() {
      field_.fieldData_->Shrink();
      dataPtr_->Clear();
      dataPtr_->Shrink();
      uncompressedBlock_->Shrink();
    }

    /*
     * Reset the data from the XCDFFieldData vector (e.g. a block had
     * just been completed).
     */
    void Reset() {field_.fieldData_->Reset();}

    /*
     * Get the compressed size of each datum in the field (in bytes)
     * for the current block
     */
    uint32_t GetActiveSize() const {

      if (!field_.fieldData_->IsActiveSizeSet()) {

        // Needs to appear as const from the outside, so cast appropriately
        const_cast<XCDFField<T>& >
                  (field_).fieldData_->SetActiveSize(CalcActiveSize());
      }

      return field_.fieldData_->GetActiveSize();
    }

    /*
     * Get the resolution of the field.
     */
    T GetResolution() const {return field_.GetResolution();}

    /// Get a value from the field
    const T& At(const uint32_t index) const {return field_.At(index);}

    /*
     *  Get the number of entries in the field in the current event
     */
    uint32_t GetSize() const {return field_.GetSize();}

    const std::string& GetName() const {return field_.GetName();}
    unsigned GetReferenceCount() const {return field_.GetReferenceCount();}

    /*
     * Get the minimum value of the field in the current block.
     */
    T GetActiveMin() const {return field_.fieldData_->GetActiveMin();}

    /*
     * Get the maximum value of the field in the current block.
     */
    T GetActiveMax() const {return field_.fieldData_->GetActiveMax();}

    /// Set the active min, as when reading back a file
    void SetActiveMin(T activeMin) {

      field_.fieldData_->SetActiveMin(activeMin);
    }

    /// Set the active max, as when reading back a file
    void SetActiveMax(T activeMax) {

      field_.fieldData_->SetActiveMax(activeMax);
    }

    /// Iterate over the field
    typedef typename XCDFField<T>::ConstIterator ConstIterator;
    ConstIterator Begin() const {return field_.Begin();}
    ConstIterator End() const {return field_.End();}

    XCDFField<T> GetField() const {return field_;}

    /// Check if we have a parent
    bool HasParent() const {return hasParent_;}

    /// Get the parent field
    const XCDFField<uint64_t> GetParent() const {return parent_;}

    /// Clear the underlying block data
    void ClearBlockData() {dataPtr_->Clear();}

    /// Pack block data into an XCDF frame
    void PackFrame(XCDFFrame& frame) const {
      dataPtr_->PackFrame(frame);
    }

    /// Load block data from an XCDF frame
    void UnpackFrame(XCDFFrame& frame) {
      dataPtr_->UnpackFrame(frame);
    }

    /*
     *  Write current data into the uncompressed block
     */
    uint64_t StoreUncompressed() {
      // Check that we have the correct number of entries
      unsigned expectedSize = 1;
      if (hasParent_) {
        expectedSize = parent_.At(0);
      }
      if (GetSize() != expectedSize) {
        XCDFFatal("Field \"" << GetName() << "\": Expected "
                     << expectedSize << " entries, got " << GetSize());
      }

      // Store the data
      for (ConstIterator it = Begin(); it != End(); ++it) {
        uncompressedBlock_->Add(*it);
      }

      // Clear out current data
      Clear();
      return uncompressedBlock_->GetByteCount();
    }

    /*
     *  Read data from the uncompressed block, compress it, and store in
     *  the compressed block
     */
    void StoreCompressed() {
      // Read the event back
      if (hasParent_) {
        unsigned size = GetParent().At(0);
        for (unsigned i = 0; i < size; ++i) {
          AddUnchecked(uncompressedBlock_->Get<T>());
          dataPtr_->AddDatum(GetIntegerRepresentation(i), GetActiveSize());
        }
      } else {
        AddUnchecked(uncompressedBlock_->Get<T>());
        dataPtr_->AddDatum(GetIntegerRepresentation(0), GetActiveSize());
      }

      // Clear out current data
      Clear();
    }

    /*
     *  Read data from the compressed block
     */
    void ReadCompressed() {
      if (hasParent_) {
        unsigned size = GetParent().At(0);
        for (unsigned i = 0; i < size; ++i) {
          AddIntegerRepresentation(dataPtr_->GetDatum(GetActiveSize()));
        }
      } else {
        AddIntegerRepresentation(dataPtr_->GetDatum(GetActiveSize()));
      }
    }

    /*
     *  Read data from the compressed block, checking ranges for append
     */
    void ReadCompressedChecked() {
      if (hasParent_) {
        unsigned size = GetParent().At(0);
        for (unsigned i = 0; i < size; ++i) {
          AddIntegerRepresentationAppend(dataPtr_->GetDatum(GetActiveSize()));
        }
      } else {
        AddIntegerRepresentationAppend(dataPtr_->GetDatum(GetActiveSize()));
      }
    }

  protected:

    /// Data type stored in the field.
    XCDFFieldType type_;

    /// The backing XCDFField object
    XCDFField<T> field_;

    /// Parent or dummy field, depending on hasParent_
    XCDFField<uint64_t> parent_;

    /// Does this field have a parent?
    bool hasParent_;

    /// Hold the black data and uncompressed data in shared memory to
    /// allow copying of the XCDFDataManager object
    XCDFPtr<XCDFBlockData> dataPtr_;
    XCDFPtr<XCDFUncompressedBlock> uncompressedBlock_;

    /*
     *  Convert #resolution units between active min and current datum back to
     *  a field value of the appropriate type.
     */
    T CalculateTypeValue(uint64_t datum) const {

      return GetActiveMin() + GetResolution() * datum;
    }

    /*
     *  Get the #resolution units between active min and current datum.
     */
    uint64_t CalculateIntegerValue(T datum) const {

      return static_cast<uint64_t>
                ((datum - GetActiveMin()) / GetResolution());
    }

    /*
     *  Calculate the number of bits needed to represent the field, considering
     *  only the max and min.
     */
    unsigned CalcActiveSize() const {

      uint64_t range = static_cast<uint64_t>(
                   (GetActiveMax() - GetActiveMin()) / GetResolution());

      unsigned bitCount = 0;
      while (range != 0) {
        bitCount++;
        range = range >> 1;
      }
      return bitCount;
    }
};

//////// Specializations for uint64_t type

template <>
inline void XCDFDataManager<uint64_t>::ZeroAlign() {

  uint64_t interval = GetActiveMin() / GetResolution();
  SetActiveMin(GetResolution() * interval);
}

//////// Specializations for int64_t type

template <>
inline void XCDFDataManager<int64_t>::ZeroAlign() {

  int64_t interval = GetActiveMin() / GetResolution();
  if (GetActiveMin() < 0 && (GetActiveMin() % GetResolution())) {
    --interval;
  }
  SetActiveMin(GetResolution() * interval);
}

//////// Specializations for double type

template <>
inline void XCDFDataManager<double>::ZeroAlign() {

  // Skip zero-align if we're required to write all 64 bits
  if (GetResolution() <= 0.) {
    return;
  }

  double interval = GetActiveMin() / GetResolution() + 0.5;

  // Zero align only if proximity to zero matters
  if (fabs(interval) > 1e10 ||
      std::isnan(GetActiveMin()) || std::isinf(GetActiveMin())) {

    return;
  }
  SetActiveMin(GetResolution() * floor(interval));
}

template <>
inline double
XCDFDataManager<double>::CalculateTypeValue(uint64_t datum) const {

  // Account for write with no compression (inf, NaN, etc.)
  if (GetActiveSize() == 64) {
    return XCDFSafeTypePun<uint64_t, double>(datum);
  }

  return GetActiveMin() + GetResolution() * datum;
}

    /*
     *  Get the #resolution units between active min and current datum.
     */
template <>
inline uint64_t
XCDFDataManager<double>::CalculateIntegerValue(double datum) const {

  // Write out entire double if required by the data (e.g. inf, NaN)
  if (GetActiveSize() == 64) {
    return XCDFSafeTypePun<double, uint64_t>(datum);
  }

  /*
   *   Add half of resolution to interval to be sure
   *   values are rounded correctly.
   */
  double interval = (datum - GetActiveMin()) / GetResolution() + 0.5;
  return static_cast<const uint64_t>(interval);
}

    /*
     * Specialization to calculate the number of bits needed
     * to represent the field in the case of floating point
     */
template <>
inline unsigned XCDFDataManager<double>::CalcActiveSize() const {

  if (std::isnan(GetActiveMax()) || std::isinf(GetActiveMax()) ||
      std::isnan(GetActiveMin()) || std::isinf(GetActiveMin()) ||
      GetResolution() <= 0.) {

    return 64;
  }

  double interval =
      (GetActiveMax() - GetActiveMin()) / GetResolution() + 0.5;
  uint64_t range = static_cast<uint64_t>(interval);

  // Catch interval values that cannot be represented as uint64_t and
  // must get 64-bit treatment.
  // Note: static_cast<uint64_t> fails for interval values > ~1e19
  if (interval > 1e16) {
    return 64;
  }

  unsigned bitCount = 0;
  while (range != 0) {
    bitCount++;
    range = range >> 1;
  }

  // Double-precision significand is 52 bits.  If larger than this,
  // use full 64-bit double
  if (bitCount > 52) {
    return 64;
  }

  return bitCount;
}

#endif // XCDF_DATA_MANAGER_INCLUDED_H
