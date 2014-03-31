
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

#ifndef XCDF_BLOCK_HEADER_INCLUDED_H
#define XCDF_BLOCK_HEADER_INCLUDED_H

#include <xcdf/XCDFFieldHeader.h>
#include <xcdf/XCDFDefs.h>

#include <vector>
#include <cassert>
#include <stdint.h>

class XCDFBlockHeader {

  public:

    XCDFBlockHeader() : eventCount_(0) { }
    ~XCDFBlockHeader() { }

    void SetEventCount(const uint32_t eventCount) {eventCount_ = eventCount;}
    uint32_t GetEventCount() const {return eventCount_;}

    void Clear() {headers_.clear();}

    void AddFieldHeader(const XCDFFieldHeader& header) {
      headers_.push_back(header);
    }

    std::vector<XCDFFieldHeader>::const_iterator FieldHeadersBegin() const {
      return headers_.begin();
    }

    std::vector<XCDFFieldHeader>::const_iterator FieldHeadersEnd() const {
      return headers_.end();
    }

    unsigned GetNFieldHeaders() const {
      return headers_.size();
    }

    void UnpackFrame(XCDFFrame& frame) {

      Clear();

      assert(frame.GetType() == XCDF_BLOCK_HEADER);

      eventCount_ = frame.GetUnsigned32();

      unsigned nHeaders = frame.GetUnsigned32();
      headers_.reserve(nHeaders);
      XCDFFieldHeader header;
      for (unsigned i = 0; i < nHeaders; ++i) {
        header.rawActiveMin_ = frame.GetUnsigned64();
        header.activeSize_ = frame.GetChar();
        headers_.push_back(header);
      }
    }

    void PackFrame(XCDFFrame& frame) const {

      frame.Clear();
      frame.SetType(XCDF_BLOCK_HEADER);
      frame.PutUnsigned32(eventCount_);

      frame.PutUnsigned32(headers_.size());
      for (std::vector<XCDFFieldHeader>::const_iterator
                               it = headers_.begin();
                               it != headers_.end(); ++it) {

        frame.PutUnsigned64(it->rawActiveMin_);
        frame.PutChar(it->activeSize_);
      }
    }

  private:

    uint32_t eventCount_;
    std::vector<XCDFFieldHeader> headers_;
};

#endif // XCDF_BLOCK_HEADER_INCLUDED_H
