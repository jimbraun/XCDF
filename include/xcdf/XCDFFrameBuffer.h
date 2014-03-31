
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

#ifndef XCDF_FRAME_BUFFER_INCLUDED_H
#define XCDF_FRAME_BUFFER_INCLUDED_H

#include <xcdf/XCDFDefs.h>

#include <vector>
#include <stdint.h>

/*!
 * @class XCDFFrameBuffer
 * @author Jim Braun
 * @brief Data buffer based on STL vector.  Use vector to control memory
 * allocation and write pointer.  Track read pointer internally.
 */

class XCDFFrameBuffer {

  public:

    XCDFFrameBuffer() : readIndex_(0),
                        size_(0) {

      // Ensure vector has an allocated backing array
      data_.reserve(1);
    }

    ~XCDFFrameBuffer() { }

    char* Get() {return &data_[0];}

    const char* Get(uint32_t size) {
      uint32_t oldIndex = readIndex_;
      readIndex_ += size;
      return Get(size, oldIndex);
    }

    const char* Get(uint32_t size, uint32_t offset) const {
      if (offset + size > size_) {
        XCDFFatal("Frame buffer underflow: Asking for size " << size
                          << " position: " << offset << " size: " << size_);
      }
      return &data_[offset];
    }

    void Insert(const uint32_t size, const char* data) {
      data_.insert(data_.end(), data, data + size);
      size_ = data_.size();
    }

    void Clear() {
      data_.clear();
      readIndex_ = 0;
      size_ = 0;
    }

    void Reserve(uint32_t size) {
      data_.reserve(size);
    }

    uint32_t GetSize() const {return size_;}
    void SetSize(const uint32_t size) {size_ = size;}

  private:

    std::vector<char> data_;
    uint32_t readIndex_;
    uint32_t size_;
};

#endif // XCDF_FRAME_BUFFER_INCLUDED_H
