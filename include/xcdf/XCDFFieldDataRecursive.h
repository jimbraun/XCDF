
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

#ifndef XCDF_FIELD_DATA_RECURSIVE_INCLUDED_H
#define XCDF_FIELD_DATA_RECURSIVE_INCLUDED_H

#include <xcdf/XCDFFieldDataVector.h>
#include <xcdf/XCDFDefs.h>

/*!
 * @class XCDFFieldDataRecursive
 * @author Jim Braun
 * @brief Recursive field data container suitable for 2D+ vectors.
 */

template <typename T>
class XCDFFieldDataRecursive : public XCDFFieldDataVector<T> {


  public:

    XCDFFieldDataRecursive(const XCDFFieldType type,
                           const std::string& name,
                           const T res,
                           const XCDFFieldData<uint64_t>* parent) :
                   XCDFFieldDataVector<T>(type, name, res, parent) { }

    virtual ~XCDFFieldDataRecursive() { }

    // Recursion into parent line to identify proper size
    virtual unsigned GetExpectedSize() const {
      unsigned expectedSize = 0;
      unsigned parentSize = XCDFFieldDataVector<T>::parent_->GetSize();
      for (int i = 0; i < parentSize; ++i) {
        expectedSize += XCDFFieldDataVector<T>::parent_->At(i);
      }
      return expectedSize;
    }
};

#endif // XCDF_FIELD_DATA_RECURSIVE_INCLUDED_H
