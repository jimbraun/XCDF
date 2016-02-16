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

#ifndef XCDF_FIELD_DATA_ALLOCATOR_INCLUDED_H
#define XCDF_FIELD_DATA_ALLOCATOR_INCLUDED_H

#include <xcdf/XCDFFieldDataBase.h>
#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFFieldDataScalar.h>
#include <xcdf/XCDFFieldDataVector.h>
#include <xcdf/XCDFFieldDataRecursive.h>
#include <xcdf/XCDFDefs.h>

namespace {


/*
 *  Get a typed XCDFField object from an XCDFFieldDataBase object.  Don't
 *  expose these functions.  Use only after appropriate type checks have
 *  been made.
 */
template <typename T>
XCDFField<T> GetField(XCDFFieldDataBase& base) {
  return XCDFField<T>(static_cast<XCDFFieldData<T>* >(&base));
}

template <typename T>
ConstXCDFField<T> GetField(const XCDFFieldDataBase& base) {
  return ConstXCDFField<T>(static_cast<const XCDFFieldData<T>* >(&base));
}

}

/*
 *  Routines for allocating/converting XCDFFieldDataBase objects.
 *  Encapsulates all XCDFFieldType --> object type conversions
 */

namespace XCDFFieldDataAllocator {

template <typename T>
XCDFFieldDataBasePtr
DoAllocateField(const std::string& name,
                const XCDFFieldType type,
                const T resolution,
                const XCDFFieldDataBase* parent = NULL) {

  if (parent) {
    if (parent->GetType() != XCDF_UNSIGNED_INTEGER) {
      XCDFFatal("Using a non-unsigned-integer field as a parent")
    }
    const XCDFFieldData<uint64_t>* p =
                static_cast<const XCDFFieldData<uint64_t>* >(parent);
    if (parent->HasParent()) {
      // A 2D+ vector.  Use XCDFFieldDataRecursive.
      return XCDFFieldDataBasePtr(
                 new XCDFFieldDataRecursive<T>(type, name, resolution, p));
    } else {
      // A 1D vector.  Use XCDFFieldDataVector for efficiency.
      return XCDFFieldDataBasePtr(
                 new XCDFFieldDataVector<T>(type, name, resolution, p));
    }
  } else {
    return XCDFFieldDataBasePtr(
                 new XCDFFieldDataScalar<T>(type, name, resolution));
  }
}

inline
XCDFFieldDataBasePtr
AllocateField(const std::string& name,
              const XCDFFieldType type,
              const uint64_t resolution,
              const XCDFFieldDataBase* parent = NULL) {

  // Here we use the field type and parentName to choose the
  // backing class for the XCDFFieldData object.
  switch(type) {
    case XCDF_UNSIGNED_INTEGER: {
      return DoAllocateField(name, type, resolution, parent);
    }
    case XCDF_SIGNED_INTEGER: {
      int64_t res = XCDFSafeTypePun<uint64_t, int64_t>(resolution);
      return DoAllocateField(name, type, res, parent);
    }
    case XCDF_FLOATING_POINT: {
      double res = XCDFSafeTypePun<uint64_t, double>(resolution);
      return DoAllocateField(name, type, res, parent);
    }
    default:
      XCDFFatal("Unknown field type: " << type);
      return XCDFFieldDataBasePtr();
  }
}

// Exploit that we know there are only three types of fields
// to allow visitor/variant data access pattern
template <typename V>
void Visit(XCDFFieldDataBase& base, V& v) {

  switch (base.GetType()) {
    case XCDF_UNSIGNED_INTEGER:
      v(GetField<uint64_t>(base));
      break;
    case XCDF_SIGNED_INTEGER:
      v(GetField<int64_t>(base));
      break;
    case XCDF_FLOATING_POINT:
      v(GetField<double>(base));
      break;
  }
}

template <typename V>
void Visit(const XCDFFieldDataBase& base, V& v) {

  switch (base.GetType()) {
    case XCDF_UNSIGNED_INTEGER:
      v(GetField<uint64_t>(base));
      break;
    case XCDF_SIGNED_INTEGER:
      v(GetField<int64_t>(base));
      break;
    case XCDF_FLOATING_POINT:
      v(GetField<double>(base));
      break;
  }
}

template <typename T>
void CheckConvertible(const XCDFFieldDataBase& base);

template<>
void CheckConvertible<uint64_t>(const XCDFFieldDataBase& base) {
  if (base.GetType() != XCDF_UNSIGNED_INTEGER) {
    XCDFFatal("Field " << base.GetName() << " is not unsigned integer type");
  }
}

template<>
void CheckConvertible<int64_t>(const XCDFFieldDataBase& base) {
  if (base.GetType() != XCDF_SIGNED_INTEGER) {
    XCDFFatal("Field " << base.GetName() << " is not signed integer type");
  }
}

template<>
void CheckConvertible<double>(const XCDFFieldDataBase& base) {
  if (base.GetType() != XCDF_FLOATING_POINT) {
    XCDFFatal("Field " << base.GetName() << " is not floating point type");
  }
}

template <typename T>
XCDFField<T> CheckedGetField(XCDFFieldDataBase& base) {
  CheckConvertible<T>(base);
  return GetField<T>(base);
}

template <typename T>
ConstXCDFField<T> CheckedGetField(const XCDFFieldDataBase& base) {
  CheckConvertible<T>(base);
  return GetField<T>(base);
}

/*
 * Get a field.  Need to convert to derived class.
 * Do the appropriate type checks.
 */
inline
XCDFUnsignedIntegerField
GetUnsignedIntegerField(XCDFFieldDataBase& base) {
  return CheckedGetField<uint64_t>(base);
}
inline
XCDFSignedIntegerField
GetSignedIntegerField(XCDFFieldDataBase& base) {
  return CheckedGetField<int64_t>(base);
}
inline
XCDFFloatingPointField
GetFloatingPointField(XCDFFieldDataBase& base) {
  return CheckedGetField<double>(base);
}

inline
ConstXCDFUnsignedIntegerField
GetUnsignedIntegerField(const XCDFFieldDataBase& base) {
  return CheckedGetField<uint64_t>(base);
}
inline
ConstXCDFSignedIntegerField
GetSignedIntegerField(const XCDFFieldDataBase& base) {
  return CheckedGetField<int64_t>(base);
}
inline
ConstXCDFFloatingPointField
GetFloatingPointField(const XCDFFieldDataBase& base) {
  return CheckedGetField<double>(base);
}

inline
std::pair<uint64_t, uint64_t>
GetUnsignedIntegerFieldRange(const XCDFFieldDataBase& base) {
  CheckConvertible<uint64_t>(base);
  return static_cast<const XCDFFieldData<uint64_t>* >(&base)->GetGlobalRange();
}

inline
std::pair<int64_t, int64_t>
GetSignedIntegerFieldRange(const XCDFFieldDataBase& base) {
  CheckConvertible<int64_t>(base);
  return static_cast<const XCDFFieldData<int64_t>* >(&base)->GetGlobalRange();
}

inline
std::pair<double, double>
GetFloatingPointFieldRange(const XCDFFieldDataBase& base) {
  CheckConvertible<double>(base);
  return static_cast<const XCDFFieldData<double>* >(&base)->GetGlobalRange();
}

}

#endif // XCDF_FIELD_DATA_ALLOCATOR_INCLUDED_H
