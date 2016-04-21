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

#ifndef XCDF_FIELD_ALIAS_ALLOCATOR_INCLUDED_H
#define XCDF_FIELD_ALIAS_ALLOCATOR_INCLUDED_H

#include <xcdf/alias/XCDFFieldAlias.h>
#include <xcdf/alias/XCDFFieldAliasBase.h>
#include <xcdf/alias/XCDFAliasDescriptor.h>
#include <xcdf/utility/NumericalExpression.h>
#include <xcdf/utility/Expression.h>

class XCDFFile;

template <typename T>
XCDFFieldAliasBasePtr
DoAllocateFieldAlias(const std::string& name,
                     const std::string& expression,
                     const XCDFFile& f) {

  NumericalExpression<T> ne = NumericalExpression<T>(expression, f);
  return XCDFFieldAliasBasePtr(
               new XCDFFieldAlias<T>(name, expression, ne));
}

inline
XCDFFieldAliasBasePtr
AllocateFieldAlias(const std::string& name,
                   const std::string& expression,
                   const XCDFFile& f) {

  Expression fieldExp = Expression(expression, f);
  Symbol* start = fieldExp.GetHeadSymbol();
  switch (start->GetType()) {
    default:
    case FLOATING_POINT_NODE:
             return DoAllocateFieldAlias<double>(name, expression, f);
    case SIGNED_NODE:
             return DoAllocateFieldAlias<int64_t>(name, expression, f);
    case UNSIGNED_NODE:
             return DoAllocateFieldAlias<uint64_t>(name, expression, f);
  }
}

inline
XCDFAliasDescriptor
GetXCDFAliasDescriptor(const XCDFFieldAliasBase& base) {
  return XCDFAliasDescriptor(base.GetName(),
                             base.GetExpression(),
                             base.GetType());
}

template <typename T>
void CheckConvertible(const XCDFFieldAliasBase& base);

template<>
inline
void CheckConvertible<uint64_t>(const XCDFFieldAliasBase& base) {
  if (base.GetType() != XCDF_UNSIGNED_INTEGER) {
    XCDFFatal("Alias " << base.GetName() << " is not unsigned integer type");
  }
}

template<>
inline
void CheckConvertible<int64_t>(const XCDFFieldAliasBase& base) {
  if (base.GetType() != XCDF_SIGNED_INTEGER) {
    XCDFFatal("Alias " << base.GetName() << " is not signed integer type");
  }
}

template<>
inline
void CheckConvertible<double>(const XCDFFieldAliasBase& base) {
  if (base.GetType() != XCDF_FLOATING_POINT) {
    XCDFFatal("Alias " << base.GetName() << " is not floating point type");
  }
}

template <typename T>
const XCDFFieldAlias<T> CheckedGetAlias(const XCDFFieldAliasBase& base) {
  CheckConvertible<T>(base);
  return static_cast<const XCDFFieldAlias<T>&>(base);
}

#endif //XCDF_FIELD_ALIAS_ALLOCATOR_INCLUDED_H
