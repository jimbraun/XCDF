
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
class XCDFFieldDataVector : public XCDFFieldData<T> {


  public:

    XCDFFieldDataVector(const XCDFFieldType type,
                        const std::string& name,
                        const T res,
                        const XCDFFieldData<uint64_t>* parent) :
                                       XCDFFieldData<T>(type, name, res),
                                       parent_(parent) { }

    typedef typename XCDFFieldData<T>::ConstIterator ConstIterator;

    virtual ~XCDFFieldDataVector() { }

    virtual void Clear() {data_.Clear();}

    virtual void Shrink() {data_.Shrink();}

    virtual void Load(XCDFBlockData& data) {
      data_.Clear();
      unsigned cnt = GetExpectedSize();
      for (unsigned i = 0; i < cnt; ++i) {
        data_.Push(XCDFFieldData<T>::LoadValue(data));
      }
    }
    virtual void Dump(XCDFBlockData& data) {
      for (ConstIterator it = Begin(); it != End(); ++it) {
        XCDFFieldData<T>::DumpValue(data, *it);
      }
      data_.Clear();
    }

    virtual void Stash() {
      for (ConstIterator it = Begin(); it != End(); ++it) {
        XCDFFieldData<T>::stash_.push_back(*it);
      }
      data_.Clear();
    }
    virtual void Unstash() {
      data_.Clear();
      unsigned cnt = GetExpectedSize();
      for (unsigned i = 0; i < cnt; ++i) {
        data_.Push(XCDFFieldData<T>::stash_.front());
        XCDFFieldData<T>::stash_.pop_front();
      }
    }

    virtual const T& At(unsigned index) const {return data_[index];}

    virtual unsigned GetSize() const {return data_.Size();}
    virtual unsigned GetExpectedSize() const {return parent_->At(0);}

    // Would like to use std::vector as data storage container, but
    // we need T* as our iterator.  Use a custom class.
    virtual ConstIterator Begin() const {return data_.Begin();}
    virtual ConstIterator End() const {return data_.End();}

    /// Check if we have a parent
    virtual bool HasParent() const {return true;}

    /// Get the parent field
    virtual const XCDFFieldData<uint64_t>* GetParent() const {return parent_;}

    /// Get the parent field name.  Use the empty string to denote no parent.
    virtual const std::string& GetParentName() const {
      return parent_->GetName();
    }

  protected:

    /*
     *  Stupidly-simple auto-growing array implementation using
     *  T* as an iterator.
     */
    template <typename U>
    class SSVector {

      public:

        SSVector() : begin_(NULL), next_(NULL), last_(NULL) {
          Reallocate(10);
        }
        ~SSVector() {Reallocate(0);}
        SSVector(const SSVector<U>& v) : begin_(NULL),
                                         next_(NULL),
                                         last_(NULL) {
          Reallocate(v.Size());
          std::copy(v.Begin(), v.End(), begin_);
          next_ = begin_ + v.Size();
        }

        SSVector<U>& operator=(SSVector<U> v) {
          Swap(v);
          return *this;
        }

        void Clear() {next_ = begin_;}
        void Shrink() {Reallocate(Size());}
        unsigned Size() const {return next_ - begin_;}
        void Swap(SSVector<U>& v) {
          std::swap(v.begin_, begin_);
          std::swap(v.next_, next_);
        }
        const U* Begin() const {return begin_;}
        const U* End() const {return next_;}
        void Push(const U& t) {
          if (next_ == last_) {
            // Double our space
            Reallocate((Size() * 2) + 1);
          }
          *next_ = t;
          ++next_;
        }
        const U& operator[](unsigned i) const {
          return *(begin_ + i);
        }

      private:

        void Reallocate(unsigned size) {
          unsigned elementCount = std::min(size, Size());
          U* newBegin = NULL;
          if (size > 0) {
            newBegin = (U*)malloc(size * sizeof(T));
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

        U* begin_;
        U* next_;
        U* last_;
    };

    // Underlying data
    SSVector<T> data_;

    /// Parent field
    const XCDFFieldData<uint64_t>* parent_;

    /*
     *  Add a datum to storage
     */
    virtual void AddDirect(const T datum) {data_.Push(datum);}

};

#endif // XCDF_FIELD_DATA_VECTOR_INCLUDED_H
