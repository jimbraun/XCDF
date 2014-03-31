
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

#ifndef XCDF_FRAME_INCLUDED_H
#define XCDF_FRAME_INCLUDED_H

#include <xcdf/XCDFFrameBuffer.h>
#include <xcdf/XCDFDefs.h>

#include <ostream>
#include <istream>

#include <zlib.h>
#include <stdint.h>

/*!
 * @class XCDFFrame
 * @author Jim Braun
 * @brief Outer container for data contained in the XCDF file consisting
 * of a size, a type, a checksum, and an inner data payload.  The checksum
 * is computed on write and verified on read. Data written to file is
 * guaranteed to be little-endian.  An endianness conversion is performed on
 * big-endian machines.
 */

class XCDFFrame {

  public:

    XCDFFrame() {

      // Check machine endianness
      machineIsBigEndian_ = true;
      uint32_t test = 0x1;
      if (*reinterpret_cast<char*>(&test) == 0x1) {
        machineIsBigEndian_ = false;
      }
    }

    ~XCDFFrame() { }

    bool IsBigEndian() const {return machineIsBigEndian_;}

    XCDFFrameType GetType() const {return type_;}
    void SetType(const XCDFFrameType type) {type_ = type;}

    void Write(std::ostream& o) {

      uint32_t type = type_;
      uint32_t size = buffer_.GetSize();
      uint32_t checksum = CalculateChecksum();

      if (IsBigEndian()) {
        ConvertEndian(size);
        ConvertEndian(type);
        ConvertEndian(checksum);
      }

      o.write(reinterpret_cast<char*>(&type), 4);
      o.write(reinterpret_cast<char*>(&size), 4);
      o.write(reinterpret_cast<char*>(&checksum), 4);

      if (buffer_.GetSize() > 0) {
        o.write(buffer_.Get(buffer_.GetSize(), 0), buffer_.GetSize());
      }
    }

    // Verify frame type before allocating and reading data
    void Read(std::istream& i) {

      uint32_t type, size, checksum;
      i.read(reinterpret_cast<char*>(&type), 4);
      i.read(reinterpret_cast<char*>(&size), 4);
      i.read(reinterpret_cast<char*>(&checksum), 4);

      if (i.fail()) {
        return;
      }

      if (IsBigEndian()) {
        ConvertEndian(size);
        ConvertEndian(type);
        ConvertEndian(checksum);
      }

      type_ = static_cast<XCDFFrameType>(type);

      // Ensure type is valid before allocating memory
      if (!XCDFFrameTypeValid(type_)) {
        return;
      }

      // If size field is corrupt, this could potentially allocate 4GB.
      // Ignore.  Checksum should fail and program should end.
      buffer_.Clear();
      buffer_.Reserve(size);
      if (size > 0) {
        i.read(buffer_.Get(), size);
      }

      if (i.fail()) {
        return;
      }

      buffer_.SetSize(size);

      if (checksum != CalculateChecksum()) {
        XCDFError("Frame data checksum failed");
        i.setstate(std::istream::failbit);
      }
    }

    void Clear() {buffer_.Clear();}

    void PutChar(char datum) {buffer_.Insert(1, &datum);}

    void PutUnsigned32(uint32_t datum) {
      if (IsBigEndian()) {
        ConvertEndian(datum);
      }
      buffer_.Insert(4, reinterpret_cast<char*>(&datum));
    }

    void PutUnsigned64(uint64_t datum) {
      if (IsBigEndian()) {
        ConvertEndian(datum);
      }
      buffer_.Insert(8, reinterpret_cast<char*>(&datum));
    }

    void PutString(const std::string& string) {
      // Include a NUL terminating character
      uint32_t size = string.size() + 1;
      PutUnsigned32(size);
      buffer_.Insert(size, string.c_str());
    }

    char GetChar() {return *(buffer_.Get(1));}

    uint32_t GetUnsigned32() {
      uint32_t datum = *reinterpret_cast<const uint32_t*>(buffer_.Get(4));
      if (IsBigEndian()) {
        ConvertEndian(datum);
      }
      return datum;
    }

    uint64_t GetUnsigned64() {
      uint64_t datum = *reinterpret_cast<const uint64_t*>(buffer_.Get(8));
      if (IsBigEndian()) {
        ConvertEndian(datum);
      }
      return datum;
    }

    const char* GetString() {
      uint32_t size = GetUnsigned32();
      return buffer_.Get(size);
    }

    const char* GetData() const {return buffer_.Get(buffer_.GetSize(), 0);}
    void PutData(uint32_t size, char* data) {buffer_.Insert(size, data);}
    uint32_t GetDataSize() const {return buffer_.GetSize();}

  private:

    XCDFFrameType type_;
    XCDFFrameBuffer buffer_;

    bool machineIsBigEndian_;

    uint32_t CalculateChecksum() {

      uint32_t value = adler32(0L, NULL, 0);

      if (buffer_.GetSize() > 0) {
        value = adler32(value,
          reinterpret_cast<unsigned char*>(buffer_.Get()), buffer_.GetSize());
      }

      return value;
    }

    void ConvertEndian(uint32_t& datum) const {
      datum = (datum>>24) |
              ((datum<<8) & 0x00FF0000) |
              ((datum>>8) & 0x0000FF00) |
              (datum<<24);
    }

    void ConvertEndian(uint64_t& datum) const {
        datum = (datum >> 56) |
          ((datum << 40) & 0x00FF000000000000ull) |
          ((datum << 24) & 0x0000FF0000000000ull) |
          ((datum << 8)  & 0x000000FF00000000ull) |
          ((datum >> 8)  & 0x00000000FF000000ull) |
          ((datum >> 24) & 0x0000000000FF0000ull) |
          ((datum >> 40) & 0x000000000000FF00ull) |
          (datum << 56);
    }
};


#endif // XCDF_FRAME_INCLUDED_H
