
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

#ifndef XCDF_BLOCK_DATA_INCLUDED_H
#define XCDF_BLOCK_DATA_INCLUDED_H

#include <xcdf/XCDFDefs.h>
#include <xcdf/XCDFFrame.h>

#include <vector>
#include <cstring>
#include <stdint.h>

class XCDFBlockData {

  public:

    XCDFBlockData() {

      // Check machine endianness
      machineIsBigEndian_ = true;
      uint32_t test = 0x1;
      if (*reinterpret_cast<char*>(&test) == 0x1) {
        machineIsBigEndian_ = false;
      }
    }

    ~XCDFBlockData() { }

    void AddDatum(const uint64_t datum, const unsigned size) {

      if (machineIsBigEndian_) {
        AddDatumBigEndian(datum, size);
      } else {
        AddDatumLittleEndian(datum, size);
      }
    }

    uint64_t GetDatum(const unsigned size) {

     if (machineIsBigEndian_) {
        return GetDatumBigEndian(size);
      } else {
        return GetDatumLittleEndian(size);
      }
    }

    void SkipDatum(const unsigned size) {
      unsigned tot = size + buffer_.indexBits_;
      buffer_.index_    += tot >> 3;   // tot/8
      buffer_.indexBits_ = tot & 0x07; // tot%8
    }

    void Clear() {buffer_.Clear();}

    void Shrink() {buffer_.Shrink();}

    void UnpackFrame(XCDFFrame& frame) {

      buffer_.Clear();
      assert(frame.GetType() == XCDF_BLOCK_DATA);

      // Ensure a full 64-bit value will fit in the allocated space
      buffer_.Reserve(frame.GetDataSize() + 8);

      buffer_.Insert(frame.GetDataSize(), frame.GetData());
    }

    void PackFrame(XCDFFrame& frame) const {

      frame.Clear();
      frame.SetType(XCDF_BLOCK_DATA);

      if (buffer_.indexBits_ > 0) {
        frame.PutData(buffer_.index_ + 1, buffer_.data_);
      } else {
        frame.PutData(buffer_.index_, buffer_.data_);
      }
    }

    unsigned Capacity() const {return buffer_.capacity_;}

  private:

    class BitBuffer {

      public:

        BitBuffer() : capacity_(1300),
                      index_(0),
                      indexBits_(0) {

          data_ = static_cast<char*>(malloc(capacity_));
          if (!data_) {
            XCDFFatal("Failed to allocate internal data buffer");
          }
        }

        BitBuffer(const BitBuffer& buffer) :
                            capacity_(buffer.capacity_),
                            index_(buffer.index_),
                            indexBits_(buffer.indexBits_) {

          data_ = static_cast<char*>(malloc(capacity_));
          memmove(data_, buffer.data_, capacity_);
        }

        const BitBuffer& operator=(const BitBuffer& buffer) {

          if (this == &buffer) {
            return *this;
          }

          capacity_ = buffer.capacity_;
          index_ = buffer.index_;
          indexBits_ = buffer.indexBits_;
          memmove(data_, buffer.data_, capacity_);
          return *this;
        }

        ~BitBuffer() {free(data_);}

        void Reserve(const unsigned capacity) {
          if (capacity > capacity_) {
            unsigned newCapacity = capacity_ * 2;
            if (newCapacity < capacity) {
              newCapacity = capacity;
            }
            Reallocate(newCapacity);
          }
        }

        void Reallocate(unsigned capacity) {

          // Don't reallocate to a smaller buffer than we need
          if (capacity < index_ + 1) {
            capacity = index_ + 1;
          }

          // Don't reallocate to a ridiculously small buffer
          if (capacity < 1300) {
            capacity = 1300;
          }

          char* tempData = static_cast<char*>(malloc(capacity));
          if (!tempData) {
            XCDFFatal("Failed to allocate internal data buffer");
          }

          if (indexBits_ > 0) {
            memmove(tempData, data_, index_ + 1);
          } else {
            memmove(tempData, data_, index_);
          }

          free(data_);
          data_ = tempData;
          capacity_ = capacity;
        }

        void Insert(const unsigned size, const char* data) {
          Clear();
          if (size > capacity_) {
            Reserve(size);
          }
          memmove(data_, data, size);
        }

        void Clear() {
          index_ = 0;
          indexBits_ = 0;
        }

        void Shrink() {
          Reallocate(index_ + 1);
        }

        unsigned capacity_;

        unsigned index_;
        unsigned indexBits_;
        char* data_;
    };

    BitBuffer buffer_;
    bool machineIsBigEndian_;


    // Less efficient endian-independent bit-packing routine
    void AddDatumBigEndian(const uint64_t datum, const unsigned size) {

      if (size == 0) {
        return;
      }

      // Ensure a 64-bit value (with indexBits > 0) fits in allocated space
      buffer_.Reserve(buffer_.index_ + 9);

      unsigned bw = 0;
      while (bw < size) {

        if (buffer_.indexBits_) {
          buffer_.data_[buffer_.index_] |= datum << buffer_.indexBits_;
          bw += 8 - buffer_.indexBits_;
          buffer_.indexBits_ = 0;
        } else {
          if (bw) {
            buffer_.index_++;
          }
          buffer_.data_[buffer_.index_] = datum >> bw;
          bw += 8;
        }
      }

      buffer_.indexBits_ = size - bw + 8;
      if (buffer_.indexBits_ == 8) {
        buffer_.index_++;
        buffer_.indexBits_ = 0;
      }
    }

    // Less efficient endian-independent bit-unpacking routine
    uint64_t GetDatumBigEndian(const unsigned size) {

      uint64_t datum = 0;
      if (size == 0) {
        return datum;
      }

      unsigned br = 0;
      while (br < size) {

        if (buffer_.indexBits_) {
          datum |= buffer_.data_[buffer_.index_] >> buffer_.indexBits_;
          br += 8 - buffer_.indexBits_;
          buffer_.indexBits_ = 0;
        } else {
          if (br) {
            buffer_.index_++;
          }
          datum |= static_cast<uint64_t>(buffer_.data_[buffer_.index_]) << br;
          br += 8;
        }
      }

      buffer_.indexBits_ = size - br + 8;
      if (buffer_.indexBits_ == 8) {
        buffer_.index_++;
        buffer_.indexBits_ = 0;
      }

      // Clear out garbage-filled upper bits
      unsigned shift = XCDF_DATUM_WIDTH_BITS - size;
      datum <<= shift;
      datum >>= shift;

      return datum;
    }

    // Efficient little-endian bit-packing routine that can
    // pack in one or two iterations
    void AddDatumLittleEndian(const uint64_t datum, const unsigned size) {

      if (size == 0) {
        return;
      }

      // Ensure a 64-bit value (with indexBits > 0) fits in allocated space
      buffer_.Reserve(buffer_.index_ + 9);

      if (buffer_.indexBits_) {
        *reinterpret_cast<uint64_t*>(buffer_.data_ + buffer_.index_) =
                  datum << buffer_.indexBits_ | buffer_.data_[buffer_.index_];
      } else {
        *reinterpret_cast<uint64_t*>(buffer_.data_ + buffer_.index_) = datum;
      }

      unsigned tot = size + buffer_.indexBits_;
      if (tot > XCDF_DATUM_WIDTH_BITS) {

        // Field spread across 9 bytes  Pack the remaining bits.
        buffer_.data_[buffer_.index_+XCDF_DATUM_WIDTH_BYTES] =
           static_cast<uint8_t>(
               datum >> (XCDF_DATUM_WIDTH_BITS-buffer_.indexBits_));
      }

      buffer_.index_    += tot >> 3;   // tot/8
      buffer_.indexBits_ = tot & 0x07; // tot%8
    }

    // Efficient little-endian bit-packing routine that can
    // pack in one or two iterations
    uint64_t GetDatumLittleEndian(const unsigned size) {

      uint64_t datum = 0;
      if (size == 0) {
        return datum;
      }

      datum = *reinterpret_cast<uint64_t*>(
                  buffer_.data_ + buffer_.index_) >> buffer_.indexBits_;

      unsigned char tot = size + buffer_.indexBits_;
      if (tot > XCDF_DATUM_WIDTH_BITS) {

        // Field spread across 9 bytes  Unpack the remaining bits.
        datum |= static_cast<uint64_t>(
                    buffer_.data_[buffer_.index_+XCDF_DATUM_WIDTH_BYTES]) <<
                                 (XCDF_DATUM_WIDTH_BITS - buffer_.indexBits_);
      }

      // Clear out garbage-filled upper bits
      unsigned shift = XCDF_DATUM_WIDTH_BITS - size;

      datum <<= shift;
      datum >>= shift;

      buffer_.index_    += tot >> 3;   // tot/8
      buffer_.indexBits_ = tot & 0x07; // tot%8

      return datum;
    }
};

#endif // XCDF_BLOCK_DATA_INCLUDED_H
