
/*
Copyright (c) 2016, James Braun
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

#include <xcdf/XCDFDefs.h>
#include <xcdf/XCDFFieldData.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFField
 * @author Jim Braun
 * @brief Wrapper class representing an XCDF data field.  Data is stored
 * in an instance of XCDFFieldData.
 */

template <typename T>
class ConstXCDFField {

  public:

    typedef XCDFFieldData<T> XCDFFieldDataType;

    ConstXCDFField(const XCDFFieldDataType* fieldData) :
                                      fieldData_(fieldData) { }

    /// Allow default construction, but use of the default-constructed
    /// object is not allowed
    ConstXCDFField() : fieldData_(NULL) { }

    /// Check if we have a parent
    bool HasParent() const {return FieldData()->HasParent();}

    /// Get the parent field.
    ConstXCDFField<uint64_t> GetParent() const {
      return ConstXCDFField<uint64_t>(
          static_cast<const XCDFFieldData<uint64_t>* >(
                                     FieldData()->GetParent()));
    }

    const std::string& GetName() const {return FieldData()->GetName();}
    const std::string& GetParentName() const {
      return FieldData()->GetParentName();
    }

    T GetResolution() const {return FieldData()->GetResolution();}

    /// Get the number of entries in the field in the current event
    unsigned GetSize() const {return FieldData()->GetSize();}

    /// Get a value from the field
    const T& At(const uint32_t index) const {return FieldData()->At(index);}
    const T& operator[](const uint32_t index) const {
      return FieldData()->At(index);
    }
    const T& operator*() const {return FieldData()->At(0);}

    /// Iterate over the field
    typedef typename XCDFFieldDataType::ConstIterator ConstIterator;
    ConstIterator Begin() const {return FieldData()->Begin();}
    ConstIterator End() const {return FieldData()->End();}

  private:

    const XCDFFieldDataType* fieldData_;

    // Check if backing fieldData object exists.  If not, throw an exception.
    // This branch is not a performance penalty, as the CPU should predict
    // this one correctly ~100% of the time.
    const XCDFFieldDataType* FieldData() const {
      if (fieldData_) {
        return fieldData_;
      } else {
        XCDFFatal("Use of default-constructed XCDFField not supported.");
        return NULL;
      }
    }
};

template <typename T>
class XCDFField : public ConstXCDFField<T> {

  public:

    typedef XCDFFieldData<T> XCDFFieldDataType;

    XCDFField(XCDFFieldDataType* fieldData) : ConstXCDFField<T>(fieldData),
                                              fieldData_(fieldData) { }

    /// Allow default construction, but use of the default-constructed
    /// object is not allowed
    XCDFField() : fieldData_(NULL) { }

    /// @brief Add a datum to the field
    /// @param value Input value
    void Add(const T value) { FieldData()->Add(value); }

    /// @brief Add an array of values to the field
    /// @param value Input array of values
    void Add(const std::vector<T> value) {
      for (auto it = value.begin(); it != value.end(); ++it)
        FieldData()->Add(*it);
    }

    XCDFField<T>& operator<<(const T value) {
      FieldData()->Add(value);
      return *this;
    }

  private:

    XCDFFieldDataType* fieldData_;

    // Check if backing fieldData object exists.  If not, throw an exception.
    // This branch is not a performance penalty, as the CPU should predict
    // this one correctly ~100% of the time.
    XCDFFieldDataType* FieldData() const {
      if (fieldData_) {
        return fieldData_;
      } else {
        XCDFFatal("Use of default-constructed XCDFField not supported.");
        return NULL;
      }
    }
};

typedef XCDFField<uint64_t> XCDFUnsignedIntegerField;
typedef XCDFField<int64_t>  XCDFSignedIntegerField;
typedef XCDFField<double>   XCDFFloatingPointField;

typedef ConstXCDFField<uint64_t> ConstXCDFUnsignedIntegerField;
typedef ConstXCDFField<int64_t>  ConstXCDFSignedIntegerField;
typedef ConstXCDFField<double>   ConstXCDFFloatingPointField;

#endif // XCDF_FIELD_INCLUDED_H
