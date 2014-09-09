
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

#ifndef XCDF_FIELD_INCLUDED_H
#define XCDF_FIELD_INCLUDED_H

#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFPtr.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFField
 * @author Jim Braun
 * @brief Wrapper class representing an XCDF data field.  Data is stored
 * in an instance of XCDFFieldData.  Provides reference counting and copy/
 * assign to manage memory.  Field access routines are provided here.
 */

template <typename T>
class XCDFField {

  public:

    XCDFField(const std::string& name,
              const T resolution) :
           fieldData_(xcdf_shared(new XCDFFieldData<T>(name, resolution))) { }

    XCDFField() : fieldData_(xcdf_shared(new XCDFFieldData<T>("", 1))) { }

    const std::string& GetName() const {return fieldData_->GetName();}

    T GetResolution() const {return fieldData_->GetResolution();}

    /// Get the number of entries in the field in the current event
    uint32_t GetSize() const {return fieldData_->GetSize();}


    /// Add a datum to the field
    void Add(const T value) {fieldData_->Add(value);}

    XCDFField<T>& operator<<(const T value) {
      fieldData_->Add(value);
      return *this;
    }


    /// Get a value from the field
    const T& At(const uint32_t index) const {return fieldData_->At(index);}
    const T& operator[](const uint32_t index) const {
      return fieldData_->At(index);
    }
    const T& operator*() const {return fieldData_->At(0);}


    /// Iterate over the field
    typedef typename XCDFFieldData<T>::Iterator Iterator;
    typedef typename XCDFFieldData<T>::ConstIterator ConstIterator;

    Iterator Begin() {return fieldData_->Begin();}
    Iterator End() {return fieldData_->End();}

    ConstIterator Begin() const {return fieldData_->Begin();}
    ConstIterator End() const {return fieldData_->End();}

  private:

    XCDFPtr<XCDFFieldData<T> > fieldData_;

  template <typename V> friend class XCDFDataManager;
};

typedef XCDFField<uint64_t> XCDFUnsignedIntegerField;
typedef XCDFField<int64_t> XCDFSignedIntegerField;
typedef XCDFField<double> XCDFFloatingPointField;

#endif // XCDF_FIELD_INCLUDED_H
