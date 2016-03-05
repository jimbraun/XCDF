
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
#include <xcdf/XCDFDeflate.h>

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

    XCDFFrameBuffer() : readIndex_(0) { }
    ~XCDFFrameBuffer() { }

    uint8_t* GetBuffer() {
      if (GetSize() == 0) {
        XCDFFatal("Getting data from an unallocated buffer");
      }
      return &(data_.front());
    }

    const uint8_t* Get(uint32_t size) {
      uint32_t oldIndex = readIndex_;
      readIndex_ += size;
      if (readIndex_ > GetSize()) {
        XCDFFatal("Frame buffer underflow");
      }
      return &data_[oldIndex];
    }

    void Insert(const uint32_t size, const uint8_t* data) {
      data_.insert(data_.end(), data, data + size);
    }

    void Clear() {
      data_.clear();
      readIndex_ = 0;
    }

    void Deflate() {
      std::vector<uint8_t> deflated;
      DeflateVector(data_, deflated);
      data_.swap(deflated);
      readIndex_ = 0;
    }

    void Inflate() {
      std::vector<uint8_t> inflated;
      InflateVector(data_, inflated);
      data_.swap(inflated);
      readIndex_ = 0;
    }

    uint32_t CalculateChecksum() {

      uint32_t value = adler32(0L, NULL, 0);
      if (GetSize() > 0) {
        value = adler32(value, GetBuffer(), GetSize());
      }

      return value;
    }

    void Reserve(uint32_t size) {data_.reserve(size);}
    void Resize(uint32_t size) {data_.resize(size);}
    uint32_t GetSize() const {return data_.size();}

  private:

    std::vector<uint8_t> data_;
    uint32_t readIndex_;
};

#endif // XCDF_FRAME_BUFFER_INCLUDED_H
