
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

#ifndef XCDF_FIELD_DATA_SCALAR_INCLUDED_H
#define XCDF_FIELD_DATA_SCALAR_INCLUDED_H

#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFDefs.h>

#include <string>
#include <stdint.h>

/*!
 * @class XCDFFieldDataScalar
 * @author Jim Braun
 * @brief XCDF field data container for scalar data types
 */

template <typename T>
class XCDFFieldDataScalar : public XCDFFieldData<T> {


  public:

    XCDFFieldDataScalar(const XCDFFieldType type,
                        const std::string& name,
                        const T res) : XCDFFieldData<T>(type, name, res),
                                       hasData_(0),
                                       datum_(0) { }

    typedef typename XCDFFieldData<T>::ConstIterator ConstIterator;

    virtual ~XCDFFieldDataScalar() { }

    virtual void Clear() {hasData_ = 0;}

    virtual void Shrink() { }

    virtual void Load(XCDFBlockData& data) {
      hasData_ = 1;
      datum_ = XCDFFieldData<T>::LoadValue(data);
    }
    virtual void Dump(XCDFBlockData& data) {
      XCDFFieldData<T>::DumpValue(data, datum_);
      hasData_ = 0;
    }

    virtual void Stash() {
      XCDFFieldData<T>::stash_.push_back(datum_);
      hasData_ = 0;
    }
    virtual void Unstash() {
      hasData_ = 1;
      datum_ = XCDFFieldData<T>::stash_.front();
      XCDFFieldData<T>::stash_.pop_front();
    }

    virtual unsigned GetSize() const {return hasData_;}
    virtual unsigned GetExpectedSize() const {return 1;}

    /// Iterate over the field
    virtual ConstIterator Begin() const {return &datum_;}
    virtual ConstIterator End() const {return &datum_ + hasData_;}

    /// Get a value from the field.  Preserve At() call for vectors
    /// At(x) is undefined for x > 0 (or possibly x == 0).  Just return datum_.
    virtual const T& At(const uint32_t index) const {return datum_;}

  protected:

    /// Use 0 and 1.  This allows avoidance of extra branches.
    unsigned hasData_;

    /// Datum
    T datum_;

    /*
     *  Add a datum to storage
     */
    virtual void AddDirect(const T datum) {
      hasData_ = 1;
      datum_ = datum;
    }
};

#endif // XCDF_FIELD_DATA_SCALAR_INCLUDED_H
