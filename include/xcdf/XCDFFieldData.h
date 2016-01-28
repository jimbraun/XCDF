
/*
Copyright (c) 2014, James Braun
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

#ifndef XCDF_FIELD_DATA_INCLUDED_H
#define XCDF_FIELD_DATA_INCLUDED_H

#include <xcdf/XCDFPtr.h>
#include <xcdf/XCDFDefs.h>

#include <string>
#include <cmath>
#include <stdint.h>

namespace {
  unsigned SIZE_UNSET = 65;
}

/*!
 * @class XCDFFieldDataBase
 * @author Jim Braun
 * @brief Mostly virtual base class for XCDF data storage/manipulation classes
 */
class XCDFFieldDataBase {

  public:

    XCDFFieldDataBase(const XCDFFieldType type,
                      const std::string& name,
                      XCDFField<uint64_t>* parent) : type_(type),
                                                     name_(name),
                                                     parent_(parent) { }

    virtual ~XCDFFieldDataBase() { }

    virtual void AddIntegerRepresentation(uint64_t datum) = 0;
    virtual void AddIntegerRepresentationAppend(uint64_t datum) = 0;
    virtual void GetIntegerRepresentation(int index) const = 0;
    virtual void ZeroAlign() = 0;
    virtual void SetActiveSize(const uint32_t activeSize) = 0;
    virtual void AddUnchecked(uint64_t value) = 0;
    virtual void Clear() = 0;
    virtual void Shrink() = 0;
    virtual void Reset() = 0;
    virtual uint32_t GetActiveSize() const = 0;
    virtual uint64_t GetResolution() const = 0;
    virtual uint32_t GetSize() const = 0;
    virtual uint64_t GetActiveMin() const = 0;
    virtual void SetActiveMin(uint64_t activeMin) = 0;

    XCDFFieldType GetType() const {return type_;}

    const std::string& GetName() const {return name_;}

    /// Check if we have a parent
    bool HasParent() const {return parent_ != NULL;}

    /// Get the parent field
    const XCDFField<uint64_t>& GetParent() const {return *parent_;}

  protected:

    /// Data type stored in the field.
    XCDFFieldType type_;

    /// Name of the field
    std::string name_;

    /// Parent or NULL, depending on whether this is a vector field
    XCDFField<uint64_t>* parent_;
};

typedef XCDFPtr<XCDFFieldDataBase> XCDFFieldDataBasePtr;
typedef XCDFPtr<const XCDFFieldDataBase> XCDFFieldDataBaseConstPtr;

/*!
 * @class XCDFFieldData
 * @author Jim Braun
 * @brief XCDF field data container.  Manages data storage, type
 * conversions, and XCDF max/min/size calculations.
 */

template <typename T>
class XCDFFieldData : XCDFFieldDataBase {

  public:

      XCDFFieldData(const XCDFFieldType type,
                    const std::string& name,
                    XCDFField<uint64_t>* parent,
                    const T res) : XCDFFieldDataBase(type, name, parent),
                                   resolution_(res),
                                   activeMin_(0),
                                   activeMax_(0),
                                   isSet_(false),
                                   activeSize_(SIZE_UNSET) { }

    virtual ~XCDFFieldData() { }

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.
     */
    virtual void AddIntegerRepresentation(uint64_t datum) {

      AddDirect(CalculateTypeValue(datum));
    }

    /*
     *  Add a datum to the field as a #resolution units above the active min.
     *  Do the conversion into the appropriate type.  Need range check in the
     *  case of file append
     */
    virtual void AddIntegerRepresentationAppend(uint64_t datum) {

      AddAppend(CalculateTypeValue(datum));
    }

    /*
     *  Get the #resolution units between active min and datum
     *  at the given index.
     */
    virtual uint64_t GetIntegerRepresentation(int index) const {

      return CalculateIntegerValue(At(index));
    }

    /*
     * Align the field active min to be an integer number of resolution
     * units from zero.  This avoids field data that changes value as
     * the active min changes block-to-block.
     */
    virtual void ZeroAlign();

    /// Set the active size of the field, as when reading back a file.
    virtual void SetActiveSize(const uint32_t activeSize) {
      activeSize_ = activeSize;
    }

    /*
     * Enter in data of known limits.
     */
    virtual void AddUnchecked(uint64_t value) {
      AddDirect(XCDFSafeTypePun<uint64_t, T>(value));
    }

    virtual void Add(const T value);
    virtual void AddAppend(const T value);

    virtual void Reset() {
      Clear();
      activeMin_ = 0;
      activeMax_ = 0;
      isSet_ = false;
      activeSize_ = SIZE_UNSET;
    }

    /*
     * Get the compressed size of each datum in the field (in bytes)
     * for the current block
     */
    virtual uint32_t GetActiveSize() const {

      if (activeSize_ == SIZE_UNSET) {

        // Make activeSize_ mutable so this method is const
        activeSize_ = CalcActiveSize();
      }

      return activeSize_;
    }

    /*
     * Get the resolution of the field cast as a uint64_t
     */
    virtual uint64_t
    GetResolution() const {return XCDFSafeTypePun<T, uint64_t>(resolution_);}

    /*
     *  Get the number of entries in the field in the current event
     */
    virtual uint32_t GetSize() = 0;

    /*
     * Get the minimum value of the field in the current block cast
     * as a uint64_t
     */
    virtual uint64_t
    GetActiveMin() const {return XCDFSafeTypePun<T, uint64_t>(activeMin_);}

    /// Set the active min, as when reading back a file
    virtual void SetActiveMin(uint64_t activeMin) {
      activeMin_ = XCDFSafeTypePun<uint64_t, T>(activeMin);
    }

    /// Iterate over the field
    typedef const T* ConstIterator;
    virtual ConstIterator Begin() const = 0;
    virtual ConstIterator End() const = 0;

    /// Get a value from the field.  Preserve At() call for vectors
    /// At(x) is undefined for x > 0 (or possibly x == 0).  Just return datum_.
    virtual const T& At(const uint32_t index) const = 0;

  protected:

    /// Resolution of the field
    T resolution_;

    /// Min and max values in the current block
    T activeMin_;
    T activeMax_;

    /// Have we written data to the field?
    bool isSet_;

    /// Number of bits needed for this field in the current block
    mutable uint32_t activeSize_;


    void DoAdd(const T value) {

      // Unset the active size when adding new data
      activeSize_ = SIZE_UNSET;

      if (value < activeMin_ || !isSet_) {
        activeMin_ = value;
      }

      if (value > activeMax_ || !isSet_) {
        activeMax_ = value;
      }

      isSet_ = true;
      AddUnchecked(value);
    }

    /*
     *  Cannot reset active min or active size during append operations
     *  while recovering the original data
     */
    void DoAddAppend(const T value) {

      if (value > activeMax_ || !isSet_) {
        activeMax_ = value;
      }

      isSet_ = true;
      AddUnchecked(value);
    }


    /*
     *  Add a datum without resetting min/max
     */
    virtual void AddDirect(const T datum) = 0;

    /*
     *  Convert #resolution units between active min and current datum back to
     *  a field value of the appropriate type.
     */
    T CalculateTypeValue(uint64_t datum) const {

      return activeMin_ + resolution_ * datum;
    }

    /*
     *  Get the #resolution units between active min and current datum.
     */
    uint64_t CalculateIntegerValue(T datum) const {

      return static_cast<uint64_t>((datum - activeMin_) / resolution_);
    }

    /*
     *  Calculate the number of bits needed to represent the field, considering
     *  only the max and min.
     */
    unsigned CalcActiveSize() const {

      uint64_t range = static_cast<uint64_t>(
                   (activeMax_ - activeMin_) / resolution_);

      unsigned bitCount = 0;
      while (range != 0) {
        bitCount++;
        range = range >> 1;
      }
      return bitCount;
    }
};

template <typename T>
inline void XCDFFieldData<T>::Add(const T value) {
  DoAdd(value);
}

template <typename T>
inline void XCDFFieldData<T>::AddAppend(const T value) {
  DoAddAppend(value);
}

//////// Specializations for uint64_t type

template <>
inline void XCDFFieldData<uint64_t>::ZeroAlign() {

  uint64_t interval = activeMin_ / resolution_;
  SetActiveMin(resolution_ * interval);
}

//////// Specializations for int64_t type

template <>
inline void XCDFFieldData<int64_t>::ZeroAlign() {

  int64_t interval = activeMin_ / resolution_;
  if (activeMin_ < 0 && (activeMin_ % resolution_)) {
    --interval;
  }
  SetActiveMin(resolution_ * interval);
}

//////// Specializations for double type

template <>
inline void XCDFFieldData<double>::ZeroAlign() {

  // Skip zero-align if we're required to write all 64 bits
  if (resolution_ <= 0.) {
    return;
  }

  double interval = activeMin_ / resolution_ + 0.5;

  // Zero align only if proximity to zero matters
  if (fabs(interval) > 1e10 ||
      std::isnan(activeMin_) || std::isinf(activeMin_)) {

    return;
  }
  SetActiveMin(resolution_ * floor(interval));
}

template <>
inline double
XCDFFieldData<double>::CalculateTypeValue(uint64_t datum) const {

  // Account for write with no compression (inf, NaN, etc.)
  if (activeSize_ == 64) {
    return XCDFSafeTypePun<uint64_t, double>(datum);
  }

  return activeMin_ + resolution_ * datum;
}

    /*
     *  Get the #resolution units between active min and current datum.
     */
template <>
inline uint64_t
XCDFFieldData<double>::CalculateIntegerValue(double datum) const {

  // Write out entire double if required by the data (e.g. inf, NaN)
  if (activeSize_ == 64) {
    return XCDFSafeTypePun<double, uint64_t>(datum);
  }

  /*
   *   Add half of resolution to interval to be sure
   *   values are rounded correctly.
   */
  double interval = (datum - activeMin_) / resolution_ + 0.5;
  return static_cast<const uint64_t>(interval);
}

    /*
     * Specialization to calculate the number of bits needed
     * to represent the field in the case of floating point
     */
template <>
inline unsigned XCDFFieldData<double>::CalcActiveSize() const {

  if (std::isnan(activeMax_) || std::isinf(activeMax_) ||
      std::isnan(activeMin_) || std::isinf(activeMin_) ||
      resolution_ <= 0.) {

    return 64;
  }

  double interval =
      (activeMax_ - activeMin_) / resolution_ + 0.5;
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

template <>
inline void XCDFFieldData<double>::Add(const double value) {

  // Need to care for NaNs, since comparison is always false
  if (std::isnan(static_cast<double>(value))) {
    activeMin_ = value;
    activeMax_ = value;
  }

  DoAdd(value);
}

template <>
inline void XCDFFieldData<double>::AddAppend(const double value) {

  // Need to care for NaNs, since comparison is always false
  if (std::isnan(static_cast<double>(value))) {
    activeMax_ = value;
  }

  DoAddAppend(value);
}

#endif // XCDF_FIELD_DATA_INCLUDED_H
