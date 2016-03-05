
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

#ifndef XCDF_DEFLATE_INCLUDED_H
#define XCDF_DEFLATE_INCLUDED_H

#include <zlib.h>
#include <vector>
#include <xcdf/XCDFDefs.h>

#define CHUNKSIZE 0x4000 // 16 k

inline
void DeflateVector(std::vector<uint8_t>& in, std::vector<uint8_t>& out) {

  out.clear();
  if (in.size() == 0) {
    return;
  }

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  if (deflateInit(&strm, Z_DEFAULT_COMPRESSION) != Z_OK) {
    XCDFFatal("Unable to initialize zlib deflate");
  }

  uint8_t output[CHUNKSIZE];
  uint8_t* input = &(in.front());
  uint8_t* end = input + in.size();
  for (int flush = Z_NO_FLUSH; flush != Z_FINISH;) {

    size_t remaining = end - input;
    if (remaining <= CHUNKSIZE) {
      strm.avail_in = remaining;
      flush = Z_FINISH;
    } else {
      strm.avail_in = CHUNKSIZE;
    }
    strm.next_in = input;
    input += strm.avail_in;

    for (;;) {
      strm.avail_out = CHUNKSIZE;
      strm.next_out = output;
      if (deflate(&strm, flush) == Z_STREAM_ERROR) {
        XCDFFatal("Error compressing output buffer");
      }
      size_t outputSize = CHUNKSIZE - strm.avail_out;
      out.insert(out.end(), output, output + outputSize);
      if (strm.avail_out > 0) {
        break;
      }
    }
  }
  deflateEnd(&strm);
}

inline
void InflateVector(std::vector<uint8_t>& in, std::vector<uint8_t>& out) {

  out.clear();
  if (in.size() == 0) {
    return;
  }

  z_stream strm;
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  if (inflateInit(&strm) != Z_OK) {
    XCDFFatal("Unable to initialize zlib inflate");
  }

  uint8_t output[CHUNKSIZE];
  uint8_t* input = &(in.front());
  uint8_t* end = input + in.size();
  for (int status = Z_OK; status == Z_OK;) {
    size_t remaining = end - input;
    if (remaining <= CHUNKSIZE) {
      strm.avail_in = remaining;
    } else {
      strm.avail_in = CHUNKSIZE;
    }
    strm.next_in = input;
    input += strm.avail_in;

    for (;;) {
      strm.avail_out = CHUNKSIZE;
      strm.next_out = output;
      status = inflate(&strm, Z_NO_FLUSH);
      if (!(status == Z_OK || status == Z_STREAM_END)) {
        XCDFFatal("Error decompressing input buffer: " << status);
      }
      size_t outputSize = CHUNKSIZE - strm.avail_out;
      out.insert(out.end(), output, output + outputSize);
      if (strm.avail_out > 0) {
        break;
      }
    }
  }
  inflateEnd(&strm);
}

#endif // XCDF_DEFLATE_INCLUDED_H
