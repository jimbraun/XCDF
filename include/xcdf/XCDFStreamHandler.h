
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

#ifndef XCDF_STREAM_HANDLER_INCLUDED_H
#define XCDF_STREAM_HANDLER_INCLUDED_H

#include <xcdf/XCDFPtr.h>

#include <ostream>
#include <istream>
#include <fstream>

/*!
 * @class XCDFStreamHandler
 * @author Jim Braun
 * @brief Provides memory-managed shared access to input/output streams.
 */

class XCDFStreamHandler {

  public:

    XCDFStreamHandler() : streams_(xcdf_shared(new StreamsContainer())) { }

    bool IsWritable() const {return streams_->ostream_ != NULL;}
    bool IsReadable() const {return streams_->istream_ != NULL;}

    std::ostream& GetOutputStream() const {return *(streams_->ostream_);}
    std::istream& GetInputStream() const {return *(streams_->istream_);}

    void SetOutputStream(std::ostream& ostream) {
      streams_->ostream_ = &ostream;
    }

    void SetInputStream(std::istream& istream) {
      streams_->istream_ = &istream;
    }

    void OpenOutputStream(const char* fileName, bool append = false) {
      streams_->OpenOutputFileStream(fileName, append);
    }

    void OpenInputStream(const char* fileName) {
      streams_->OpenInputFileStream(fileName);
    }

    void CloseOutputStream() {
      streams_->CloseOutputFileStream();
    }

    void CloseInputStream() {
      streams_->CloseInputFileStream();
    }

    void Close() {streams_->Close();}

  private:

    class StreamsContainer {

      public:

        StreamsContainer() : istream_(NULL),
                             ostream_(NULL),
                             referenceCount_(0) { }

        void OpenOutputFileStream(const char* fileName, bool append) {

          CloseOutputFileStream();
          if (append) {
            outputFileStream_.open(
                fileName, std::ofstream::in | std::ofstream::ate | std::ofstream::binary);
          } else {
            outputFileStream_.open(
                fileName, std::ofstream::out | std::ofstream::binary);
          }
          if (!(outputFileStream_.fail())) {
            ostream_ = &outputFileStream_;
          }
        }

        void CloseOutputFileStream() {

          if (outputFileStream_.is_open()) {
            outputFileStream_.close();
            outputFileStream_.clear();
          }
        }

        void OpenInputFileStream(const char* fileName) {

          CloseInputFileStream();
          inputFileStream_.open(
             fileName, std::ifstream::in | std::ifstream::binary);
          if (!(inputFileStream_.fail())) {
            istream_ = &inputFileStream_;
          }
        }

        void CloseInputFileStream() {

          if (inputFileStream_.is_open()) {
            inputFileStream_.close();
            inputFileStream_.clear();
          }
        }

        void Close() {

          CloseInputFileStream();
          CloseOutputFileStream();

          istream_ = NULL;
          ostream_ = NULL;
        }

        std::istream* istream_;
        std::ostream* ostream_;
        uint32_t referenceCount_;

        std::ifstream inputFileStream_;
        std::ofstream outputFileStream_;
    };

    XCDFPtr<StreamsContainer> streams_;
};

#endif // XCDF_STREAM_HANDLER_INCLUDED_H
