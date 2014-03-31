
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

#ifndef XCDF_FIELD_DATA_INCLUDED_H
#define XCDF_FIELD_DATA_INCLUDED_H

/*!
 * @class XCDFFieldData
 * @author Jim Braun
 * @brief Helper class for XCDF field containing the template data type
 * vector, limits, and resolution.
 */

#include <vector>
#include <string>
#include <cmath>
#include <stdint.h>

namespace {
  unsigned SIZE_UNSET = 65;
}

template <typename T>
class XCDFFieldData {

  public:

    XCDFFieldData(const std::string& name,
                  T resolution) : name_(name),
                                  resolution_(resolution),
                                  activeMin_(0),
                                  activeMax_(0),
                                  isSet_(false),
                                  activeSize_(SIZE_UNSET),
                                  referenceCount_(0) { }

    ~XCDFFieldData() { }

    typedef typename std::vector<T>::iterator Iterator;
    typedef typename std::vector<T>::const_iterator ConstIterator;

    Iterator Begin() {return data_.begin();}
    Iterator End() {return data_.end();}

    ConstIterator Begin() const {return data_.begin();}
    ConstIterator End() const {return data_.end();}

    T GetResolution() const {return resolution_;}

    T GetActiveMin() const {return activeMin_;}
    void SetActiveMin(const T activeMin) {activeMin_ = activeMin;}

    T GetActiveMax() const {return activeMax_;}
    void SetActiveMax(const T activeMax) {activeMax_ = activeMax;}

    const T& At(unsigned index) const {return data_[index];}

    uint32_t GetSize() const {return data_.size();}

    void Clear() {data_.clear();}

    void Shrink() {std::vector<T>(data_).swap(data_);}

    void Reset() {
      Clear();
      activeMin_ = 0;
      activeMax_ = 0;
      isSet_ = false;
      activeSize_ = SIZE_UNSET;
    }

    void Add(const T value);
    void AddAppend(const T value);

    void AddUnchecked(const T value) {data_.push_back(value);}

    void SetActiveSize(uint32_t activeSize) {activeSize_ = activeSize;}
    uint32_t GetActiveSize() const {return activeSize_;}
    bool IsActiveSizeSet() const {return activeSize_ != SIZE_UNSET;}

    const std::string& GetName() const {return name_;}

    void IncrementReferenceCount() {++referenceCount_;}
    void DecrementReferenceCount() {--referenceCount_;}
    bool HasReferences() const {return referenceCount_ != 0;}

  private:

    std::string name_;
    std::vector<T> data_;

    T resolution_;
    T activeMin_;
    T activeMax_;

    bool isSet_;

    uint32_t activeSize_;
    uint32_t referenceCount_;

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
};

template <typename T>
inline void XCDFFieldData<T>::Add(const T value) {
  DoAdd(value);
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

template <typename T>
inline void XCDFFieldData<T>::AddAppend(const T value) {
  DoAddAppend(value);
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
