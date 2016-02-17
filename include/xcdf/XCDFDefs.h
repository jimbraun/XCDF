
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

#ifndef XCDF_DEFS_INCLUDED_H
#define XCDF_DEFS_INCLUDED_H

#include <sstream>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <stdint.h>

#define XCDF_VERSION 3

#define XCDF_DATUM_WIDTH_BYTES 8
#define XCDF_DATUM_WIDTH_BITS  64

enum XCDFFrameType {

  XCDF_FILE_HEADER  = 0x436FC8A4,
  XCDF_BLOCK_HEADER = 0x160E17E4,
  XCDF_BLOCK_DATA   = 0x37DF239D,
  XCDF_FILE_TRAILER = 0xBD340AF6
};

inline bool XCDFFrameTypeValid(uint32_t type) {
  return (type == XCDF_FILE_HEADER ||
          type == XCDF_BLOCK_HEADER ||
          type == XCDF_BLOCK_DATA ||
          type == XCDF_FILE_TRAILER);
}

enum XCDFFieldType {
    XCDF_UNSIGNED_INTEGER    = 0,
    XCDF_SIGNED_INTEGER      = 1,
    XCDF_FLOATING_POINT      = 2
};

const std::string NO_PARENT = "";

class XCDFException {

  public:

    XCDFException(const std::string& message) : message_(message) { }

    const std::string& GetMessage() const { return message_; }

  private:

    std::string message_;
};

// Global logging routines
#define XCDFFatal(message) {                                          \
  std::stringstream s;                                                \
  s << __FILE__ << "," << __FUNCTION__ << ':' << __LINE__ << "]: ";   \
  s << message;                                                       \
  std::cerr << "XCDF FATAL ERROR: " << s.rdbuf()->str() << std::endl; \
  throw XCDFException(s.rdbuf()->str());                              \
}

#define XCDFError(message) {                                          \
  std::stringstream s;                                                \
  s << message;                                                       \
  std::cerr << "XCDF ERROR: " << s.rdbuf()->str() << std::endl;       \
}

#define XCDFWarn(message) {                                           \
  std::stringstream s;                                                \
  s << message;                                                       \
  std::cerr << "XCDF WARNING: " << s.rdbuf()->str() << std::endl;     \
}

template <typename T, typename U>
U XCDFSafeTypePun(const T data) {
  U out;
  std::memmove(&out, &data, sizeof(U));
  return out;
}

#endif // XCDF_DEFS_INCLUDED_H
