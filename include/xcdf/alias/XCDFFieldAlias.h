
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

#ifndef XCDF_FIELD_ALIAS_INCLUDED_H
#define XCDF_FIELD_ALIAS_INCLUDED_H

#include <xcdf/XCDFDefs.h>
#include <xcdf/alias/XCDFFieldAliasBase.h>
#include <xcdf/utility/NumericalExpression.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFFieldAlias
 * @author Jim Braun
 * @brief Wrapper class representing an expression derived from an
 * XCDFField object
 */

template <typename T>
class XCDFFieldAlias : XCDFFieldAliasBase {

  public:

    XCDFFieldAlias(const std::string& name,
                   const std::string& expression,
                   const NumericalExpression& ne) :
                              XCDFFieldAliasBase(name, expression),
                              expression_(ne) { }

    virtual XCDFFieldType GetType() const;
    const std::string& GetName() const {return name_;}
    Node<T>* GetHeadNode() {return expression_.GetHeadNode();}

    /// Get the number of entries in the expression for the current event
    unsigned GetSize() const {return expression_.GetSize();}

    /// Get a value from the field
    const T& At(const uint32_t index) const {return expression_.Evaluate(index);}
    const T& operator[](const uint32_t index) const {
      return At(index);
    }
    const T& operator*() const {return At(0);}

  private:

    std::string name_;
    std::string expString_;
    NumericalExpression expression_;
};

template<>
virtual XCDFFieldType XCDFFieldAlias<uint64_t>::GetType() const {
  return XCDF_UNSIGNED_INTEGER;
}

template<>
virtual XCDFFieldType XCDFFieldAlias<int64_t>::GetType() const {
  return XCDF_SIGNED_INTEGER;
}

template<>
virtual XCDFFieldType XCDFFieldAlias<double>::GetType() const {
  return XCDF_FLOATING_POINT;
}


typedef XCDFFieldAlias<uint64_t> XCDFUnsignedIntegerFieldAlias;
typedef XCDFFieldAlias<int64_t>  XCDFSignedIntegerFieldAlias;
typedef XCDFFieldAlias<double>   XCDFFloatingPointFieldAlias;

#endif // XCDF_FIELD_ALIAS_INCLUDED_H
