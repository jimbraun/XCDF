
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

#ifndef XCDF_VISITOR_ROUTINES_INCLUDED_H
#define XCDF_VISITOR_ROUTINES_INCLUDED_H

#include <xcdf/XCDFDataManager.h>
#include <xcdf/XCDFBlockHeader.h>
#include <xcdf/XCDFBlockData.h>
#include <xcdf/XCDFUncompressedBlock.h>
#include <xcdf/XCDFFrame.h>
#include <xcdf/XCDFFile.h>

class StoreUncompressedFieldVisitor {
  public:

    StoreUncompressedFieldVisitor() : byteCount_(0) { }

    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      byteCount_ += manager.StoreUncompressed();
    }

    // The total number of uncompressed bytes we're holding
    uint64_t GetByteCount() const {return byteCount_;}

  private:

    uint64_t byteCount_;
};

class StoreCompressedFieldVisitor {
  public:

    StoreCompressedFieldVisitor() { }

    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.StoreCompressed();}
};

class UncompressedBufferWriteFieldVisitor {
  public:

    UncompressedBufferWriteFieldVisitor(XCDFUncompressedBlock& block) :
                                                          block_(block) { }
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      for (typename XCDFDataManager<T>::ConstIterator
                                       it = manager.Begin();
                                       it != manager.End(); ++it) {

        block_.Add(*it);
      }
    }

  private:

    XCDFUncompressedBlock& block_;
};

class ZeroAlignFieldVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.ZeroAlign();}
};

class BlockHeaderFieldVisitor {
  public:

    BlockHeaderFieldVisitor(XCDFBlockHeader& blockHeader) :
                                         blockHeader_(blockHeader) { }
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      header_.rawActiveMin_ = GetBytes(manager.GetActiveMin());
      header_.activeSize_ = manager.GetActiveSize();
      blockHeader_.AddFieldHeader(header_);
    }

  private:
    XCDFFieldHeader header_;
    XCDFBlockHeader& blockHeader_;

    /// Simple template method to get 8 raw bytes from a type
    template <typename T>
    uint64_t GetBytes(T datum) {
      return XCDFSafeTypePun<T, uint64_t>(datum);
    }
};

class UncompressedBufferReadFieldVisitor {
  public:

    UncompressedBufferReadFieldVisitor(XCDFUncompressedBlock& block) :
                                                         block_(block) { }
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      if (manager.HasParent()) {
        unsigned size = manager.GetParent().At(0);
        for (unsigned i = 0; i < size; ++i) {
          manager.AddUnchecked(block_.Get<T>());
        }
      } else {
        manager.AddUnchecked(block_.Get<T>());
      }
    }

  private:

    XCDFUncompressedBlock& block_;
};

class WriteFieldVisitor {
  public:

    WriteFieldVisitor(XCDFBlockData& blockData) : blockData_(blockData) { }
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      for (unsigned i = 0; i < manager.GetSize(); ++i) {
        blockData_.AddDatum(manager.GetIntegerRepresentation(i),
                                                manager.GetActiveSize());
      }
    }

  private:

    XCDFBlockData& blockData_;
};

class ReadFieldVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.ReadCompressed();}
};

class CheckedReadFieldVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      manager.ReadCompressedChecked();
    }
};

class ClearFieldVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.Clear();}
};

class ShrinkFieldVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.Shrink();}
};

class ResetFieldVisitor {
  public:

    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.Reset();}
};

class ClearBlockVisitor {
  public:

    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {manager.ClearBlockData();}
};

class FrameWriteFieldVisitor {
  public:

    FrameWriteFieldVisitor(XCDFFrame& frame,
                           XCDFFile& file) : frame_(frame), file_(file) { }

    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {

      manager.PackFrame(frame_);
      file_.WriteFrame();
      manager.Reset();
    }

  private:

    XCDFFrame& frame_;
    XCDFFile& file_;
};

class CheckFieldContentsVisitor {
  public:
    template <typename T>
    void operator()(XCDFDataManager<T>& manager) {
      if (manager.GetSize() > 0) {
        XCDFWarn("Field \"" << manager.GetName() <<
                   "\": Unwritten data added to field");
      }
    }
};

#endif // XCDF_VISITOR_ROUTINES_INCLUDED_H
