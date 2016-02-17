
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

#ifndef XCDF_FILE_TRAILER_INCLUDED_H
#define XCDF_FILE_TRAILER_INCLUDED_H

#include <xcdf/XCDFBlockEntry.h>
#include <xcdf/XCDFFieldGlobals.h>
#include <xcdf/XCDFDefs.h>

#include <vector>
#include <cassert>

#include <stdint.h>

class XCDFFileTrailer {

  public:

    XCDFFileTrailer() : totalEventCount_(0),
                        blockTableEnabled_(true) { }
    ~XCDFFileTrailer() { }

    void SetTotalEventCount(const uint64_t totalEventCount) {
      totalEventCount_ = totalEventCount;
    }

    uint64_t GetTotalEventCount() const {return totalEventCount_;}

    void Clear() {
      blockEntries_.clear();
      comments_.clear();
      globals_.clear();
    }

    bool IsBlockTableEnabled() {return blockTableEnabled_;}

    void DisableBlockTable() {
      blockEntries_.clear();
      blockTableEnabled_ = false;
    }

    void AddBlockEntry(const XCDFBlockEntry& entry) {
      if (blockTableEnabled_) {
        blockEntries_.push_back(entry);
      }
    }

    std::vector<XCDFBlockEntry>::const_iterator BlockEntriesBegin() const {
      return blockEntries_.begin();
    }

    std::vector<XCDFBlockEntry>::const_iterator BlockEntriesEnd() const {
      return blockEntries_.end();
    }

    XCDFBlockEntry& GetLastBlockEntry() {
      return blockEntries_.back();
    }

    const XCDFBlockEntry& GetLastBlockEntry() const {
      return blockEntries_.back();
    }

    unsigned GetNBlockEntries() const {return blockEntries_.size();}

    void PopBlockEntry() {blockEntries_.pop_back();}

    bool HasEntries() const {return GetNBlockEntries() > 0;}

    void AddComment(const std::string& comment) {
      comments_.push_back(comment);
    }

    std::vector<std::string>::const_iterator CommentsBegin() const {
      return comments_.begin();
    }

    std::vector<std::string>::const_iterator CommentsEnd() const {
      return comments_.end();
    }

    void AddGlobals(const XCDFFieldGlobals& globals) {
      if (blockTableEnabled_) {
        globals_.push_back(globals);
      }
    }

    unsigned GetNGlobals() const {return globals_.size();}
    const XCDFFieldGlobals& GlobalAt(int idx) const {return globals_[idx];}

    std::vector<XCDFFieldGlobals>::const_iterator GlobalsBegin() const {
      return globals_.begin();
    }

    std::vector<XCDFFieldGlobals>::const_iterator GlobalsEnd() const {
      return globals_.end();
    }

    void ClearGlobals() {globals_.clear();}

    uint32_t GetNComments() const {return comments_.size();}

    void UnpackFrame(XCDFFrame& frame, unsigned version) {

      Clear();

      assert(frame.GetType() == XCDF_FILE_TRAILER);

      totalEventCount_ = frame.GetUnsigned64();

      uint32_t nEntries = frame.GetUnsigned32();
      blockEntries_.reserve(nEntries);
      XCDFBlockEntry entry;
      for (uint32_t i = 0; i < nEntries; ++i) {
        entry.nextEventNumber_ = frame.GetUnsigned64();
        entry.filePtr_ = frame.GetUnsigned64();
        blockEntries_.push_back(entry);
      }

      uint32_t nComments = frame.GetUnsigned32();
      comments_.reserve(nComments);
      std::string comment;
      for (unsigned i = 0; i < nComments; ++i) {
        comment = frame.GetString();
        comments_.push_back(comment);
      }

      if (version > 2) {
        uint32_t nGlobals = frame.GetUnsigned32();
        XCDFFieldGlobals globals;
        for (unsigned i = 0; i < nGlobals; ++i) {
          globals.rawGlobalMax_ = frame.GetUnsigned64();
          globals.rawGlobalMin_ = frame.GetUnsigned64();
          globals.totalBytes_ = frame.GetUnsigned64();
          globals.globalsSet_ = frame.GetChar();
          globals_.push_back(globals);
        }
      }
    }

    void PackFrame(XCDFFrame& frame) const {

      frame.Clear();
      frame.SetType(XCDF_FILE_TRAILER);
      frame.PutUnsigned64(totalEventCount_);

      frame.PutUnsigned32(blockEntries_.size());
      for (std::vector<XCDFBlockEntry>::const_iterator
                          it = blockEntries_.begin();
                          it != blockEntries_.end(); ++it) {

        frame.PutUnsigned64(it->nextEventNumber_);
        frame.PutUnsigned64(it->filePtr_);
      }

      frame.PutUnsigned32(comments_.size());
      for (std::vector<std::string>::const_iterator
                                 it = comments_.begin();
                                 it != comments_.end(); ++it) {
        frame.PutString(*it);
      }
      frame.PutUnsigned32(globals_.size());
      for (std::vector<XCDFFieldGlobals>::const_iterator
                          it = globals_.begin();
                          it != globals_.end(); ++it) {

        frame.PutUnsigned64(it->rawGlobalMax_);
        frame.PutUnsigned64(it->rawGlobalMin_);
        frame.PutUnsigned64(it->totalBytes_);
        frame.PutChar(it->globalsSet_);
      }
    }

  private:

    uint64_t totalEventCount_;
    std::vector<XCDFBlockEntry> blockEntries_;
    std::vector<std::string> comments_;
    std::vector<XCDFFieldGlobals> globals_;

    bool blockTableEnabled_;
};

#endif // XCDF_FILE_TRAILER_INCLUDED_H
