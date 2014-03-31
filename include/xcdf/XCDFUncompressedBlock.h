
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

#ifndef XCDF_UNCOMPRESSED_BLOCK_INCLUDED_H
#define XCDF_UNCOMPRESSED_BLOCK_INCLUDED_H

#include <xcdf/XCDFDefs.h>

#include <deque>
#include <cassert>

#include <stdint.h>

/*!
 * @class XCDFUncompressedBlock
 * @author Jim Braun
 * @brief Simple FIFO that stores block data until write as type uint64_t
 */

class XCDFUncompressedBlock {

  public:

    XCDFUncompressedBlock() { }
    ~XCDFUncompressedBlock() { }

    template <typename T>
    void Add(T datum) {
      data_.push_back(XCDFSafeTypePun<T, uint64_t>(datum));
    }

    void Clear() {data_.clear();}
    uint64_t GetByteCount() const {
      return data_.size() * XCDF_DATUM_WIDTH_BYTES;
    }

    template <typename T>
    T Get() {

      assert(data_.size() > 0);
      uint64_t temp = data_.front();
      data_.pop_front();
      return XCDFSafeTypePun<uint64_t, T>(temp);
    }

    void Shrink() {std::deque<uint64_t>(data_).swap(data_);}

    std::deque<uint64_t> data_;
};

#endif // XCDF_UNCOMPRESSED_BLOCK_INCLUDED_H
