
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

#ifndef XCDF_FIELD_DATA_VECTOR_INCLUDED_H
#define XCDF_FIELD_DATA_VECTOR_INCLUDED_H

#include <xcdf/XCDFFieldData.h>
#include <xcdf/XCDFDefs.h>

#include <string>
#include <stdint.h>
#include <algorithm>

/*!
 * @class XCDFFieldDataVector
 * @author Jim Braun
 * @brief XCDF field data container for vector data types
 */

template <typename T>
class XCDFFieldDataVector : XCDFFieldData<T> {


  public:

    XCDFFieldDataVector(const XCDFFieldType type,
                        const std::string& name,
                        XCDFField<uint64_t>* parent,
                        const T res) :
                          XCDFFieldData(type, name, parent, res) { }

    virtual ~XCDFFieldDataVector() { }

    virtual void Clear() {data_.Clear();}

    virtual void Shrink() {data_.Shrink();}

    virtual const T& At(unsigned index) const {return data_[index];}

    virtual uint32_t GetSize() const {return data_.Size();}

    // Would like to use std::vector as data storage container, but
    // we need T* as our iterator.  Use a custom class.
    virtual ConstIterator Begin() const {return data_.Begin();}
    virtual ConstIterator End() const {return data_.End();}

  protected:

    // Underlying data
    SSVector<T> data_;

    /*
     *  Add a datum to storage
     */
    virtual void AddDirect(const T datum) {data_.Push(value);}

    /*
     *  Stupidly-simple auto-growing array implementation using
     *  T* as an iterator.
     */
    template <typename T>
    class SSVector {

      public:

        SSVector() : begin_(NULL), next_(NULL), last_(NULL) {
          Reallocate(10);
        }
        ~SSVector() {Reallocate(0);}
        SSVector(const SSVector<T>& v) : begin_(NULL),
                                         next_(NULL),
                                         last_(NULL) {
          Reallocate(v.Size());
          std::copy(v.Begin(), v.End(), begin_);
          next_ = begin_ + v.Size();
        }

        SSVector<T>& operator=(SSVector<T> v) {
          Swap(v);
          return *this;
        }

        void Clear() {next_ = begin_;}
        void Shrink() {Reallocate(Size());}
        unsigned Size() const {return next_ - begin_;}
        void Swap(SSVector<T>& v) {
          std::swap(v.begin_, begin_);
          std::swap(v.next_, next_);
        }
        const T* Begin() const {return begin_;}
        const T* End() const {return next_;}
        void Push(const T& t) {
          if (next_ == last_) {
            // Double our space
            Reallocate((Size() * 2) + 1);
          }
          next_ = t;
          ++next_;
        }
        const T& operator[](unsigned i) const {
          return *(begin_ + i);
        }

      private:

        void Reallocate(unsigned size) {
          unsigned elementCount = std::min(size, Size());
          T* newBegin = NULL;
          if (size > 0) {
            newBegin = (T*)malloc(size * sizeof(T));
            if (!newBegin) {
              XCDFFatal("Unable to allocate field memory");
            }
            std::copy(begin_, begin_ + elementCount, newBegin);
          }
          if (begin_) {
            free(begin_);
          }
          begin_ = next_ = last_ = NULL;
          if (newBegin) {
            begin_ = newBegin;
            next_ = newBegin + elementCount;
            last_ = newBegin + size;
          }
        }

        T* begin_;
        T* next_;
        T* last_;
    };
};

#endif // XCDF_FIELD_DATA_VECTOR_INCLUDED_H
