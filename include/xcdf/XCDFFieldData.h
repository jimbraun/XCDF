
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
#include <xcdf/XCDFBlockData.h>
#include <xcdf/XCDFFieldDataBase.h>

#include <string>
#include <cmath>
#include <stdint.h>
#include <deque>
#include <functional>

/*!
 * @class XCDFFieldData
 * @author Jim Braun
 * @brief XCDF field data container.  Manages data storage, type
 * conversions, and XCDF max/min/size calculations.
 */

namespace {
  unsigned SIZE_UNSET = 65;

  /*
   *  Check a type against a target and a flag.  If the value compares
   *  true to the target according to the supplied operator or the flag
   *  is not set, set the target to the value and set the flag.
   */
  template <typename T, typename Operator>
  void DoCheck(const T value, T& target, bool& setFlag, Operator op) {
    if (op(value, target) || !setFlag) {
      target = value;
    }
    setFlag = true;
  }

  /*
   *  Partial specialization of DoCheck for type double.
   *  Need to check for NaNs explicitly, as they always compare false.
   */
  template <typename Operator>
  void DoCheck(const double value, double& target, bool& setFlag, Operator op) {
    if (op(value, target) || !setFlag) {
      target = value;
    }
    setFlag = true;

    if (std::isnan(value)) {
      target = value;
    }
  }
}

template <typename T>
class XCDFFieldData : public XCDFFieldDataBase {

  public:

      XCDFFieldData(const XCDFFieldType type,
                    const std::string& name,
                    const T res) : XCDFFieldDataBase(type, name),
                                   resolution_(res),
                                   activeMin_(0),
                                   activeMax_(0),
                                   globalMin_(0),
                                   globalMax_(0),
                                   minSet_(false),
                                   maxSet_(false),
                                   globalMinSet_(false),
                                   globalMaxSet_(false),
                                   activeSize_(SIZE_UNSET),
                                   totalBytes_(0),
                                   bitsProcessed_(0) { }

    virtual ~XCDFFieldData() { }

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

    void Add(const T value) {

      // Unset the active size when adding new data
      activeSize_ = SIZE_UNSET;

      // Check the current value against min/max
      CheckActiveMin(value);
      CheckActiveMax(value);
      AddDirect(value);
    }

    virtual void Reset() {
      Clear();
      if (minSet_) {
        CheckGlobalMin(activeMin_);
      }
      if (maxSet_) {
        CheckGlobalMax(activeMax_);
      }
      activeMin_ = 0;
      activeMax_ = 0;
      minSet_ = false;
      maxSet_ = false;
      activeSize_ = SIZE_UNSET;
    }

    virtual void CalculateGlobals() {
      if (minSet_) {
        CheckGlobalMin(activeMin_);
      }
      if (maxSet_) {
        CheckGlobalMax(activeMax_);
      }
      totalBytes_ = bitsProcessed_ >> 3;
    }

    virtual bool GlobalsSet() const {return globalMinSet_ && globalMaxSet_;}

    virtual void ClearBitsProcessed() {bitsProcessed_ = 0;}
    virtual uint64_t GetBitsProcessed() const {return bitsProcessed_;}

    virtual uint64_t GetStashSize() const {return stash_.size();}

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
    GetRawResolution() const {
      return XCDFSafeTypePun<T, uint64_t>(resolution_);
    }
    virtual T
    GetResolution() const {return resolution_;}

    /*
     * Get the minimum value of the field in the current block cast
     * as a uint64_t
     */
    virtual uint64_t
    GetRawActiveMin() const {return XCDFSafeTypePun<T, uint64_t>(activeMin_);}

    /// Set the active min, as when reading back a file
    virtual void SetRawActiveMin(uint64_t rawActiveMin) {
      activeMin_ = XCDFSafeTypePun<uint64_t, T>(rawActiveMin);
      minSet_ = true;
    }

    /*
     *  Get the global range of the field
     */
    std::pair<T, T> GetGlobalRange() const {
      return std::pair<T, T>(globalMin_, globalMax_);
    }

    /*
     * Get the minimum value seen by the field
     */
    virtual T
    GetGlobalMin() const {return globalMin_;}

    /*
     * Get the maximum value seen by the field
     */
    virtual T
    GetGlobalMax() const {return globalMax_;}

    /*
     * Get the minimum value seen by the field in raw units
     */
    virtual uint64_t
    GetRawGlobalMin() const {return XCDFSafeTypePun<T, uint64_t>(globalMin_);}

    /*
     * Get the maximum value seen by the field in raw units
     */
    virtual uint64_t
    GetRawGlobalMax() const {return XCDFSafeTypePun<T, uint64_t>(globalMax_);}

    /*
     * Set the minimum value seen by the field in raw units.  Check the value
     * so if we can attempt to set it to a less-extreme value with no effect.
     * This is not elegant, but eliminates even less-elegant code when dealing
     * with concatenated files.
     */
    virtual void SetRawGlobalMin(uint64_t rawGlobalMin) {
      CheckGlobalMin(XCDFSafeTypePun<uint64_t, T>(rawGlobalMin));
    }

    /*
     * Set the maximum value seen by the field in raw units.  Check the value
     * so if we can attempt to set it to a less-extreme value with no effect.
     * This is not elegant, but eliminates even less-elegant code when dealing
     * with concatenated files.
     */
    virtual void SetRawGlobalMax(uint64_t rawGlobalMax) {
      CheckGlobalMax(XCDFSafeTypePun<uint64_t, T>(rawGlobalMax));
    }

    /*
     *  Get the number of bytes used by the field.  This is entirely
     *  storage.  We don't modify this internally ever.
     */
    virtual uint64_t
    GetTotalBytes() const {return totalBytes_;}
    virtual void
    SetTotalBytes(uint64_t totalBytes) {totalBytes_ = totalBytes;}

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

    /// Absolute min and max values
    T globalMin_;
    T globalMax_;

    /// Have we written data to the field?
    bool minSet_;
    bool maxSet_;

    /// Have we written the global max/min?
    bool globalMinSet_;
    bool globalMaxSet_;

    /// Number of bits needed for this field in the current block
    mutable uint32_t activeSize_;

    /// Storage for data held in write cache, awaiting max/min limits to be set
    std::deque<T> stash_;

    /// Total bytes used by the field.  We can't just use bitsProcessed
    /// because reading files back must necessarily alter bitsProcessed,
    /// but totalBytes_ should be static after we've calculated the value.
    uint64_t totalBytes_;

    /// Bits we've processed; used to determine total bytes
    uint64_t bitsProcessed_;

    void CheckActiveMin(const T value) {
      DoCheck(value, activeMin_, minSet_, std::less<T>());
    }

    void CheckActiveMax(const T value) {
      DoCheck(value, activeMax_, maxSet_, std::greater<T>());
    }

    void CheckGlobalMin(const T value) {
      DoCheck(value, globalMin_, globalMinSet_, std::less<T>());
    }

    void CheckGlobalMax(const T value) {
      DoCheck(value, globalMax_, globalMaxSet_, std::greater<T>());
    }

    /*
     *  Load a value from the XCDFBlockData
     */
    T LoadValue(XCDFBlockData& data) {
      T value = CalculateTypeValue(data.GetDatum(activeSize_));
      // We only have the active min.  We need to rediscover the max.
      CheckActiveMax(value);
      bitsProcessed_ += activeSize_;
      return value;
    }

    /*
     *  Dump a value to the XCDFBlockData
     */
    void DumpValue(XCDFBlockData& data, T datum) {
      data.AddDatum(CalculateIntegerValue(datum), GetActiveSize());
      bitsProcessed_ += activeSize_;
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

//////// Specializations for uint64_t type

template <>
inline void XCDFFieldData<uint64_t>::ZeroAlign() {

  uint64_t interval = activeMin_ / resolution_;
  activeMin_ = resolution_ * interval;
}

//////// Specializations for int64_t type

template <>
inline void XCDFFieldData<int64_t>::ZeroAlign() {

  int64_t interval = activeMin_ / resolution_;
  if (activeMin_ < 0 && (activeMin_ % resolution_)) {
    --interval;
  }
  activeMin_ = resolution_ * interval;
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
  activeMin_ = resolution_ * floor(interval);
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

#endif // XCDF_FIELD_DATA_INCLUDED_H
