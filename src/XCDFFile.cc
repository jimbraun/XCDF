
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

#include <xcdf/XCDFFile.h>
#include <xcdf/XCDFFieldRoutines.h>
#include <xcdf/XCDFFieldDataAllocator.h>

#include <string>
#include <cstring>
#include <fstream>

void XCDFFile::Init() {

  blockSize_ = 1000;
  thresholdByteCount_ = 100000000; // Allow up to 100 MB in a block by default
  zeroAlign_ = true;

  eventCount_ = 0;
  blockCount_ = 0;
  blockEventCount_ = 0;

  isModifiable_ = true;
  blockTableComplete_ = false;
  headerWritten_ = false;
  isOpen_ = false;
  isAppend_ = false;
  recover_ = false;
  unusableGlobalsFromFile_ = false;
  haveV3Globals_ = false;

  currentFileStartOffset_ = 0;
  currentFrameStartOffset_ = 0;
  currentFrameEndOffset_ = 0;

  currentFileName_ = "";
}

/*
 *  Close the file.  Release any ifstream/ofstream resources allocated
 *  during Open().  If writing, do end checks to ensure all data is
 *  written out to file.
 */
void XCDFFile::Close() {

  if (IsWritable()) {

    // Check that the fields are empty
    FieldListForEach(CheckFieldContents);

    // Write out remaining data
    if (blockEventCount_ > 0) {
      WriteBlock();
    }

    // If header not written, write the header
    if (!headerWritten_) {
      fileHeader_.PackFrame(currentFrame_);
      WriteFrame();
      headerWritten_ = true;
    }

    std::ostream& ostream = streamHandler_.GetOutputStream();

    // Save file pointer to block table in header if available
    // (i.e. file is on disk).
    uint64_t currentPos = static_cast<uint64_t>(ostream.tellp());
    fileHeader_.SetFileTrailerPtr(currentPos);

    // Add the globals and event count to the trailer and write it
    fileTrailer_.ClearGlobals();
    FieldListForEach(CalculateGlobals);
    XCDFFieldGlobals globals;
    for (FieldList::iterator it = fieldList_.begin();
                             it != fieldList_.end(); ++it) {
      globals.globalsSet_ = (*it)->GlobalsSet();
      globals.rawGlobalMax_ = (*it)->GetRawGlobalMax();
      globals.rawGlobalMin_ = (*it)->GetRawGlobalMin();
      globals.totalBytes_ = (*it)->GetTotalBytes();
      fileTrailer_.AddGlobals(globals);
    }
    fileTrailer_.SetTotalEventCount(eventCount_);
    fileTrailer_.PackFrame(currentFrame_);
    WriteFrame();

    // Update header entry with block table pointer if possible.
    // Don't want to throw an exception here if operation is not allowed.
    // Skip header update if block table is disabled
    if (fileTrailer_.IsBlockTableEnabled()) {
      try {
        ostream.seekp(0);
        currentPos = static_cast<uint64_t>(ostream.tellp());
        if (!ostream.fail() && currentPos == 0) {
          fileHeader_.PackFrame(currentFrame_);
          WriteFrame();
        }
      } catch (std::ostream::failure& e) { }
    }

    ostream.flush();
  }

  streamHandler_.Close();

  fieldList_.clear();

  eventCount_ = 0;
  blockCount_ = 0;
  blockEventCount_ = 0;

  isModifiable_ = true;
  blockTableComplete_ = false;
  headerWritten_ = false;
  isOpen_ = false;

  currentFileStartOffset_ = 0;
  currentFrameStartOffset_ = 0;
  currentFrameEndOffset_ = 0;

  currentFileName_ = "";
}

/*
 *  Open the given file with the given name and mode
 */
bool XCDFFile::Open(const char* fileName,
                    const char* mode) {

  bool isRead = strchr(mode, 'r') || strchr(mode, 'R');
  recover_ = strchr(mode, 'c') || strchr(mode, 'C');
  isRead = isRead || recover_;
  bool isWrite = strchr(mode, 'w') || strchr(mode, 'W');
  bool isAppend = strchr(mode, 'a') || strchr(mode, 'A');

  bool incl = isRead || isWrite || isAppend;
  bool excl = (isRead && isWrite) ||
              (isRead && isAppend) ||
              (isWrite && isAppend);

  if (!incl || excl) {

    XCDFFatal("Unsupported file mode: \"" << mode <<
                "\".  Use \"r\" (read) or \"w\" (write) or \"a\" (append)");
  }

  if (isOpen_) {
    Close();
  }

  isOpen_ = false;


  if (isRead) {
    streamHandler_.OpenInputStream(fileName);
    if (streamHandler_.IsReadable()) {
      isModifiable_ = false;
      isOpen_ = true;
      ReadFileHeaders();
    } else {
      XCDFError("Unable to open " << fileName << " for reading");
    }
  }

  bool appendReadable = true;
  if (isAppend) {

    // Check if we're trying to append to a new file
    std::ifstream testOpen(fileName);
    appendReadable = testOpen.good();

    // If zero-length file, do write instead
    testOpen.seekg(0, std::ifstream::end);
    if (static_cast<uint64_t>(testOpen.tellg()) == 0) {
      appendReadable = false;
    }

    if (appendReadable) {
      streamHandler_.OpenInputStream(fileName);
      if (streamHandler_.IsReadable()) {
        isModifiable_ = false;
        isOpen_ = true;
        isAppend_ = true;
        ReadFileHeaders();
      } else {
        XCDFError("Unable to open " << fileName << " for appending");
      }

      if (!OpenAppend(fileName)) {
        XCDFError("Unable to open " << fileName << " for appending");
      }
    }
  }

  if (isWrite || !appendReadable) {
    streamHandler_.OpenOutputStream(fileName);
    if (streamHandler_.IsWritable()) {
      isOpen_ = true;
    } else {
      XCDFError("Unable to open " << fileName << " for writing");
    }
  }

  currentFileName_ = std::string(fileName);
  return isOpen_;
}

/*
 *  Prepare the file for append operation.  Read block table,
 *  read last incomplete block or start new one, transfer block data
 *  into write buffer, position output stream at proper point in
 *  file.
 */
bool XCDFFile::OpenAppend(const char* fileName) {

  // Save the position directly after header in case of zero-entry file
  uint64_t firstPos =
      static_cast<uint64_t>(streamHandler_.GetInputStream().tellg());

  // Get the block table
  if (!blockTableComplete_) {

    // Need to read the whole file to get the block table
    while (ReadNextBlock()) { }

    // Check that event counts match
    if (eventCount_ != fileTrailer_.GetTotalEventCount()) {
      return false;
    }
  }

  // For zero-entry files, just write after the header
  if (GetEventCount() == 0) {
    return PrepareAppend(fileName, firstPos, 0);
  }

  // If the file has events, does it have block entries?
  if (fileTrailer_.GetNBlockEntries() == 0) {
    return false;
  }

  // Is the last block full?
  const XCDFBlockEntry& lastEntry = fileTrailer_.GetLastBlockEntry();
  Seek(lastEntry.nextEventNumber_);
  bool lastBlockFull = blockEventCount_ + 1 >= blockSize_;

  // If last block is full, append after the end
  if (lastBlockFull) {
    uint64_t endPos =
          static_cast<uint64_t>(streamHandler_.GetInputStream().tellg());
    return PrepareAppend(fileName, endPos, 0);
  }

  // append at the start of the block
  uint64_t blockPos = static_cast<uint64_t>(lastEntry.filePtr_);

  // Remove the last block entry -- it will be rewritten
  fileTrailer_.PopBlockEntry();
  return PrepareAppend(fileName, blockPos, blockEventCount_ + 1);
}

bool XCDFFile::PrepareAppend(const char* fileName,
                             uint64_t position,
                             uint64_t cnt) {

  uint64_t finalEventCount = GetEventCount();

  headerWritten_ = true;
  streamHandler_.OpenOutputStream(fileName, true);
  if (!streamHandler_.IsWritable()) {
    return false;
  }

  // Seek to appropriate spot in file
  try {
    streamHandler_.GetOutputStream().seekp(position);
  } catch (std::ostream::failure& e) {
    return false;
  }
  if (static_cast<uint64_t>(streamHandler_.GetOutputStream().tellp()) !=
                                                                position) {
    return false;
  }

  // Copy data if necessary.  Need checked read to reset the active max.
  // First event is already read.
  if (cnt > 0) {
    Write();
    for (unsigned i = 1; i < cnt; ++i) {
      Read();
      Write();
    }
  } else {

    // Need to reset the fields because an event was potentially read, causing
    // active min/max to be stored
    FieldListForEach(ResetField);
  }

  eventCount_ = finalEventCount;
  blockEventCount_ = cnt;
  streamHandler_.CloseInputStream();
  return true;
}

/*
 *  Write currentFrame_ to ostream_
 */
void XCDFFile::WriteFrame() {

  assert(IsWritable());

  std::ostream& ostream = streamHandler_.GetOutputStream();

  // Save start-of-frame file pointer
  currentFrameStartOffset_ = ostream.tellp();
  try {
    currentFrame_.Write(ostream);
  } catch (std::ostream::failure& e) {
    ostream.setstate(std::ostream::failbit);
  }

  // Save end-of-frame file pointer
  currentFrameEndOffset_ = ostream.tellp();

  if (ostream.fail()) {
    XCDFFatal("Write failed.  Byte offset: " << ostream.tellp());
  }
  currentFrame_.Clear();
}

/*
 *  Read a frame from istream_ into currentFrame_
 */
void XCDFFile::ReadFrame() {

  assert(IsReadable());

  std::istream& istream = streamHandler_.GetInputStream();

  currentFrame_.Clear();

  // Save start-of-frame file pointer
  currentFrameStartOffset_ = istream.tellg();
  try {
    currentFrame_.Read(istream);
  } catch (std::istream::failure& e) {
    istream.setstate(std::istream::failbit);
  }

  // Save end-of-frame file pointer
  currentFrameEndOffset_ = istream.tellg();

  if (istream.fail()) {
    XCDFFatal("Read failed.  Byte offset: " << currentFrameStartOffset_);
  }
}

/*
 *  Write one event to the uncompressed buffer.
 */
int XCDFFile::Write() {

  // Format is fixed after first write.  Prevent changes.
  isModifiable_ = false;

  // Check that stream is ready and opened for writing
  if (!IsWritable()) {
    XCDFFatal("XCDF Write Failed: File not opened for writing");
  }

  // Check that fields are filled and have the correct number of entries
  FieldListForEach(CheckFieldSize);

  // Stash the data and clear the fields
  FieldListForEach(StashField);

  eventCount_++;
  blockEventCount_++;

  uint64_t currentBlockSize = 0;
  for (FieldList::iterator it = fieldList_.begin();
                           it != fieldList_.end(); ++it) {
    currentBlockSize += (*it)->GetStashSize() * XCDF_DATUM_WIDTH_BYTES;
  }

  // Write out the block if we've reached specified block size or
  // buffer has reached the specified threshold size
  if (blockEventCount_ >= blockSize_ ||
                 currentBlockSize >= thresholdByteCount_) {
    WriteBlock();

    // If last block was larger than 150 MB, deallocate memory buffers.
    // Reallocation will require relatively zero CPU in this case.
    if (currentBlockSize > 150000000) {
      blockData_.Clear();
      blockData_.Shrink();
      FieldListForEach(ShrinkField);
    }
  }

  return 1;
}

/*
 *  Write a block of data to ostream_.  This involves:
 *
 *  1. Read data from the uncompressed buffer
 *  2. Calculate active size for each field
 *  3. Zero align the active mins
 *  4. Create the block header
 *  5. Serialize the data
 *  6. Write the block header and serialized data to file
 *  7. Reset counters
 */
void XCDFFile::WriteBlock() {

  assert(IsWritable());

  blockHeader_.Clear();
  blockData_.Clear();
  blockHeader_.SetEventCount(blockEventCount_);

  // Align the field bins with zero if possible
  if (zeroAlign_) {
    FieldListForEach(ZeroAlignField);
  }

  // Write the field headers
  XCDFFieldHeader header;
  for (FieldList::iterator it = fieldList_.begin();
                           it != fieldList_.end(); ++it) {
    header.rawActiveMin_ = (*it)->GetRawActiveMin();
    header.activeSize_ = (*it)->GetActiveSize();
    blockHeader_.AddFieldHeader(header);
  }

  // Write the data block
  for (unsigned i = 0; i < blockEventCount_; ++i) {
    WriteEvent();
  }

  // If header not written, write the header
  if (!headerWritten_) {
    fileHeader_.PackFrame(currentFrame_);
    WriteFrame();
    headerWritten_ = true;
  }

  // Mark the block starting point
  XCDFBlockEntry entry;
  entry.nextEventNumber_ = eventCount_ - blockEventCount_;
  entry.filePtr_ = streamHandler_.GetOutputStream().tellp();
  fileTrailer_.AddBlockEntry(entry);

  blockHeader_.PackFrame(currentFrame_);
  WriteFrame();
  blockData_.PackFrame(currentFrame_);
  WriteFrame();

  // Reset each field
  FieldListForEach(ResetField);

  blockCount_++;

  // Reset the uncompressed block
  blockEventCount_ = 0;
}

/*
 * Read an event from the uncompressed buffer and then compress it to
 * the XCDFBlockData object.
 */
void XCDFFile::WriteEvent() {

  // Read in event from the uncompressed buffer
  FieldListForEach(UnstashField);

  // Compress event and clear field
  for (FieldList::iterator it = fieldList_.begin();
                           it != fieldList_.end(); ++it) {
    (*it)->Dump(blockData_);
  }
}

void XCDFFile::ReadEvent() {

  assert(blockEventCount_ > 0);

  // Read in event from the compressed buffer
  for (FieldList::iterator it = fieldList_.begin();
                           it != fieldList_.end(); ++it) {
    (*it)->Load(blockData_);
  }

  blockEventCount_--;
  eventCount_++;
}

bool XCDFFile::ReadNextBlock() {

  assert(IsReadable());

  if (streamHandler_.GetInputStream().fail()) {
    return false;
  }

  ReadFrame();

  if (currentFrame_.GetType() == XCDF_FILE_HEADER) {
    XCDFFatal("Corrupt file: Extraneous file header found at offset: " <<
                                  currentFrameStartOffset_ << ". Aborting.");

  } else if (currentFrame_.GetType() == XCDF_BLOCK_HEADER) {

    blockHeader_.UnpackFrame(currentFrame_);

    if (blockHeader_.GetNFieldHeaders() != GetNFields()) {

      XCDFFatal("File corrupt: Unexpected number of block headers");
    }

    // Reset each field
    FieldListForEach(ResetField);

    // Update field sizes for the block.
    uint32_t i = 0;
    for (std::vector<XCDFFieldHeader>::const_iterator
                      it = blockHeader_.FieldHeadersBegin();
                      it != blockHeader_.FieldHeadersEnd(); ++it) {

      fieldList_[i]->SetRawActiveMin(it->rawActiveMin_);
      fieldList_[i]->SetActiveSize(it->activeSize_);
      i++;
    }

    // Add any remaining events in previous block to the event count
    eventCount_ += blockEventCount_;

    // Get event count for next block
    blockEventCount_ = blockHeader_.GetEventCount();

    ReadFrame();

    if (currentFrame_.GetType() != XCDF_BLOCK_DATA) {
      XCDFFatal("Block header not followed by data block at file offset: " <<
                                   currentFrameStartOffset_ << ". Aborting.");
    }

    // Shrink internal buffers if previous block > 150 MB
    if (blockData_.Capacity() > 150000000) {
      blockData_.Clear();
      blockData_.Shrink();
      FieldListForEach(ShrinkField);
    }

    blockData_.UnpackFrame(currentFrame_);
    blockCount_++;
    return true;

  } else if (currentFrame_.GetType() == XCDF_FILE_TRAILER) {

    // Load the trailer if not already loaded
    if (!blockTableComplete_) {
      XCDFFileTrailer tempTrailer;
      tempTrailer.UnpackFrame(currentFrame_, fileHeader_.GetVersion());
      CopyTrailer(tempTrailer);
    }

    // Check if file is concatenated
    if (NextFrameExists()) {

      // Concatenated file. Read the header and get the next block
      currentFileStartOffset_ =  currentFrameEndOffset_;
      XCDFFileHeader tempHeader;
      LoadFileHeader(tempHeader);

      // Compare against original file header
      if (fileHeader_ != tempHeader) {
        XCDFFatal("Found mismatching header at file position "
                           << currentFrameStartOffset_ << ". Aborting");
      }

      // Go on to the next data block
      return ReadNextBlock();

    } else {

      // Reached EOF
      if (!blockTableComplete_) {

        // Set the event count.  This is a hack in the case of empty trailers
        fileTrailer_.SetTotalEventCount(eventCount_);
        blockTableComplete_ = true;

        // We've read all the trailers.  If we have complete globals, mark it.
        if (!unusableGlobalsFromFile_) {
          haveV3Globals_ = true;
        }
      }

      return false;
    }
  } else {

    XCDFFatal("Found unknown frame at file offset: " <<
                        currentFrameStartOffset_ << ". Aborting.");
  }

  // Unreachable
  return false;
}

/*
 * Read blocks until we find one with events or we reach EOF
 */
bool XCDFFile::GetNextBlockWithEvents() {

  for (;;) {
    if (!ReadNextBlock()) {
      return false;
    }
    if (blockEventCount_ > 0) {
      return true;
    }
  }

  // Unreachable, but silences compiler warnings...
  return false;
}

/*
 * Read the next event into the field buffers
 */
int XCDFFile::Read() {

  // Check that stream is ready and opened for reading
  if (!IsReadable()) {
    XCDFFatal("XCDF Read Failed: File not opened for reading");
  }

  if (blockEventCount_ == 0) {

    // No events in current block.  Get the next block with data
    if (!GetNextBlockWithEvents()) {

      // No more events
      return 0;
    }
  }

  ReadEvent();
  return 1;
}

/*
 *  Seek the istream to a new file position and check for failure.
 *  Return the status.
 */
bool XCDFFile::DoSeek(const std::streampos& pos) {

  assert(IsReadable());

  std::istream& istream = streamHandler_.GetInputStream();

  std::istream::iostate oldState = istream.rdstate();
  try {
    istream.seekg(pos);
  } catch (std::istream::failure& e) {
    istream.setstate(std::istream::failbit);
  }

  if (istream.fail() || istream.tellg() != pos) {

    // seek failed
    istream.clear();
    istream.setstate(oldState);
    return false;
  }

  // success
  return true;
}

/*
 * Read the first header, then read all subsequent headers in
 * the case of a concatenated file that we can seek
 */

void XCDFFile::ReadFileHeaders() {

  assert(IsReadable());

  // Read the file header
  LoadFileHeader(fileHeader_);

  // Store the position at the end of the header.
  std::streampos firstHeaderEndPos = currentFrameEndOffset_;

  // Load the field list
  for (std::vector<XCDFFieldDescriptor>::const_iterator
                        it = fileHeader_.FieldDescriptorsBegin();
                        it != fileHeader_.FieldDescriptorsEnd(); ++it) {

    XCDFFieldType type = static_cast<XCDFFieldType>(it->type_);
    AllocateField(it->name_, type, it->rawResolution_, it->parentName_);
  }

  // Read the trailer if we have a pointer
  if (fileHeader_.HasFileTrailerPtr() && !recover_) {
    if (DoSeek(fileHeader_.GetFileTrailerPtr())) {
      LoadFileTrailer(fileTrailer_);
      SetGlobals(fileTrailer_);
      blockTableComplete_ = true;
    } else {
      blockTableComplete_ = false;
    }
  }

  // If we have the block table, check if there are any more
  // file headers/trailers in the file (i.e. the file is
  // concatenated
  if (blockTableComplete_) {

    // At the end of file trailer.  Next position would be the next header.
    while (NextFrameExists()) {

      ReadFrame();

      // File is concatenated
      currentFileStartOffset_ =  currentFrameStartOffset_;
      XCDFFileHeader  tempHeader;
      XCDFFileTrailer tempTrailer;

      if (currentFrame_.GetType() != XCDF_FILE_HEADER) {
        XCDFFatal("Found extraneous data at end of file, position "
                          << currentFrameStartOffset_ << ". Aborting");
      }

      tempHeader.UnpackFrame(currentFrame_);

      if (fileHeader_ != tempHeader) {
        XCDFFatal("Found mismatching header at file position "
                           << currentFrameStartOffset_ << ". Aborting");
      }

      // Read in the new file trailer
      if (tempHeader.HasFileTrailerPtr()) {
        uint64_t fileStartPos = static_cast<uint64_t>(currentFileStartOffset_);
        if (DoSeek(fileStartPos + tempHeader.GetFileTrailerPtr())) {

          LoadFileTrailer(tempTrailer);
          blockTableComplete_ = true;
        }
      } else {
        blockTableComplete_ = false;
      }

      // Quit if we weren't able to load the block table
      if (!blockTableComplete_) {
        break;
      }

      CopyTrailer(tempTrailer);
    }
  }

  if (blockTableComplete_ && !unusableGlobalsFromFile_) {
    haveV3Globals_ = true;
  }

  // Return to the first block in the file --
  // should do nothing if we can't seek.  Frame offset pointers
  // may point to a different frame later in the file, but this
  // shouldn't be important.
  currentFileStartOffset_ = 0;
  DoSeek(firstHeaderEndPos);
}

void XCDFFile::LoadFileHeader(XCDFFileHeader& header) {

  ReadFrame();
  if (currentFrame_.GetType() != XCDF_FILE_HEADER) {
    XCDFFatal("Unable to read file: Not XCDF format.");
  }
  header.UnpackFrame(currentFrame_);
}

void XCDFFile::LoadFileTrailer(XCDFFileTrailer& trailer) {

  ReadFrame();

  if (currentFrame_.GetType() != XCDF_FILE_TRAILER) {
    XCDFFatal("File trailer not found.  File corrupt.");
  }

  trailer.UnpackFrame(currentFrame_, fileHeader_.GetVersion());
  blockTableComplete_ = true;
}

void XCDFFile::SetGlobals(const XCDFFileTrailer& trailer) {
  // Set field globals from the trailer data
  if (trailer.GetNGlobals() != fieldList_.size()) {
    // We can't use globals from file.  We must recalculate.
    unusableGlobalsFromFile_ = true;
    return;
  }

  uint32_t i = 0;
  for (std::vector<XCDFFieldGlobals>::const_iterator
                    it = trailer.GlobalsBegin();
                    it != trailer.GlobalsEnd(); ++it) {

    /// These will be checked so we don't accidentally set
    /// the globals to less-extreme values.
    if (it->globalsSet_) {
      fieldList_[i]->SetRawGlobalMin(it->rawGlobalMin_);
      fieldList_[i]->SetRawGlobalMax(it->rawGlobalMax_);
      fieldList_[i]->SetTotalBytes(
                      fieldList_[i]->GetTotalBytes() + it->totalBytes_);
    }
    i++;
  }
}

void XCDFFile::CopyTrailer(const XCDFFileTrailer& trailer) {

  // Add entries to current block table, modifying event number and offset
  XCDFBlockEntry entry;
  uint64_t oldEventCount = fileTrailer_.GetTotalEventCount();
  for (std::vector<XCDFBlockEntry>::const_iterator
                       it = trailer.BlockEntriesBegin();
                       it != trailer.BlockEntriesEnd(); ++it) {

    entry = *it;
    entry.filePtr_ += currentFileStartOffset_;
    entry.nextEventNumber_ += oldEventCount;
    fileTrailer_.AddBlockEntry(entry);
  }

  // Reset the total event count
  fileTrailer_.SetTotalEventCount(fileTrailer_.GetTotalEventCount() +
                                          trailer.GetTotalEventCount());

  // Copy the comments
  for (std::vector<std::string>::const_iterator
                       it = trailer.CommentsBegin();
                       it != trailer.CommentsEnd(); ++it) {

    fileTrailer_.AddComment(*it);
  }

  // Copy the global data, if present
  SetGlobals(trailer);
}

/*
 *  Check if there is another frame after the current one when reading.
 *  Don't disturb the state of the stream or throw an exception.
 */
bool XCDFFile::NextFrameExists() {

  assert(IsReadable());

  std::istream& istream = streamHandler_.GetInputStream();

  std::istream::iostate oldState = istream.rdstate();
  int test = 0;
  try {
    test = istream.peek();
  } catch (std::istream::failure& e) {
    istream.setstate(std::istream::failbit);
  }

  if (test == EOF) {
    istream.setstate(std::istream::failbit);
  }

  bool ret = !istream.fail();
  istream.clear();
  istream.setstate(oldState);
  return ret;
}

bool XCDFFile::Rewind() {

  // Check that stream is ready and opened for reading
  if (!IsReadable()) {
    XCDFFatal("XCDF Seek Failed: File not opened for reading");
  }

  // Try moving to the beginning
  if (!DoSeek(0)) {
    return false;
  }

  // Get ready to read the block
  eventCount_ = 0;
  blockEventCount_ = 0;
  blockCount_ = 0;

  // Get the header block out of the way
  ReadFrame();

  // Get the next data block
  if (!GetNextBlockWithEvents()) {
    return false;
  }

  return true;
}

bool XCDFFile::Seek(uint64_t absoluteEventPos) {

  // Check that stream is ready and opened for reading
  if (!IsReadable()) {
    XCDFFatal("XCDF Seek Failed: File not opened for reading");
  }

  // Check if current event is already loaded
  if (absoluteEventPos + 1 == eventCount_) {
    return true;
  }

  // Check if event is in unread portion of current block.
  if (!(absoluteEventPos < eventCount_ + blockEventCount_ &&
                                absoluteEventPos >= eventCount_)) {

    // If we have the block table, we can seek.
    // Go directly to the appropriate block
    bool blockSeekSuccess = false;
    if (blockTableComplete_) {

      // Check that event exists in the file
      if (absoluteEventPos >= fileTrailer_.GetTotalEventCount()) {
        XCDFError("Cannot seek to event " << absoluteEventPos <<
                    ". Total events: " << fileTrailer_.GetTotalEventCount());
        return false;
      }

      // Find the appropriate block
      uint64_t pos = 0;
      uint64_t nextEventNumber = 0;
      uint64_t blockNumber = 0;
      for (std::vector<XCDFBlockEntry>::const_iterator
                          it = fileTrailer_.BlockEntriesBegin();
                          it != fileTrailer_.BlockEntriesEnd(); ++it) {

        if (it->nextEventNumber_ > absoluteEventPos) {
          if (pos == 0) {
            pos = it->filePtr_;
            nextEventNumber = it->nextEventNumber_;
            blockNumber = it - fileTrailer_.BlockEntriesBegin();
          }
          break;
        }
        pos = it->filePtr_;
        nextEventNumber = it->nextEventNumber_;
        blockNumber = it - fileTrailer_.BlockEntriesBegin();
      }

      if (pos != 0) {
        // Load the block
        if (DoSeek(pos)) {
          ReadNextBlock();
          eventCount_ = nextEventNumber;
          blockCount_ = blockNumber + 1;
          blockSeekSuccess = true;
        } else {
          // XCDFError("Unable to seek to event " << absoluteEventPos);
          return false;
        }
      }
    }

    if (!blockTableComplete_ || blockSeekSuccess == false) {

      // No block table data.
      // Go through blocks one-by-one to find the correct block.

      // If event is backward, attempt to go to the beginning
      if (eventCount_ > absoluteEventPos + 1) {
        bool rewindSuccess = Rewind();

        if (!rewindSuccess) {
          // XCDFError("Unable to seek to event " << absoluteEventPos);
          return false;
        }
      }

      // Read through blocks until we get the right block
      while (!(absoluteEventPos - eventCount_ < blockEventCount_)) {

        if (!NextFrameExists()) {

          // Check EOF condition first in case of stream read
          return false;
        }

        if (!GetNextBlockWithEvents()) {

          // At end of file
          return false;
        }
      }
    }
  }

  // At the proper block.  Go to the proper event.
  assert(absoluteEventPos - eventCount_ < blockEventCount_);
  while (eventCount_ <= absoluteEventPos) {
    ReadEvent();
  }

  return true;
}


uint64_t XCDFFile::GetEventCount() {

  // If writing, event count is the total number of events written
  if (!IsReadable()) {
    return eventCount_;
  }

  uint64_t totalEventCount;
  uint64_t currentEventCount = eventCount_;

  // If we have the block table, we know the event count
  if (blockTableComplete_) {
    totalEventCount = fileTrailer_.GetTotalEventCount();

  } else {

    // Add events that have been already read
    totalEventCount = eventCount_;
    totalEventCount += blockEventCount_;

    // Add remaining events in the file
    while (GetNextBlockWithEvents()) {
      totalEventCount += blockEventCount_;
    }

    // Attempt to seek back to current event.  Note that
    // the current event count is 1 larger than the event
    // number that is loaded (e.g. if event 10 is loaded,
    // currentEventCount is 11).
    bool seekSuccess;
    if (currentEventCount == 0) {
      seekSuccess = Rewind();
    } else {
      seekSuccess = Seek(currentEventCount - 1);
    }

    if (!seekSuccess) {
      // XCDFError("Unable to reset file state");
      eventCount_ = totalEventCount + 1;
      blockEventCount_ = 0;
    }
  }

  return totalEventCount;
}

void XCDFFile::CheckGlobals() {
  // Ensure we've read in global data.  If not, get it.
  // We have global data we've completely read the globals
  if (haveV3Globals_) {
    return;
  }

  // If we're writing, we have the data, but we need to calculate the globals
  if (IsWritable()) {
    FieldListForEach(CalculateGlobals);
    return;
  }

  // We're reading but don't have the globals yet.  The only way to get them
  // is to read every event.
  uint64_t currentEventCount = eventCount_;
  // Rewind if we can seek.  This possibly means reading the file twice
  // for version < 3.
  bool seekSuccess = Rewind();
  if (seekSuccess) {
    // we need to reset field byte counts
    FieldListForEach(ClearFieldBitsProcessed);
  }
  // Read all the remaining data
  while (Read());
  // Calculate the globals
  FieldListForEach(CalculateGlobals);
  haveV3Globals_ = true;
  // Return file to original position if possible
  if (currentEventCount == 0) {
    seekSuccess = Rewind();
  } else {
    seekSuccess = Seek(currentEventCount - 1);
  }

  if (!seekSuccess) {
    // Unable to return to previous state
    eventCount_ = GetEventCount() + 1;
    blockEventCount_ = 0;
  }
}

bool operator<(const XCDFFieldType& type, const XCDFFieldDataBasePtr& ptr) {
  return type < ptr->GetType();
}

/*
 *  Checks the field sanity, checks the parent (if applicable),
 *  then adds the field to the XCDFFileHeader object (if opened for
 *  writing) to be written to file.
 */
void XCDFFile::AllocateField(const std::string& name,
                             const XCDFFieldType type,
                             const uint64_t resolution,
                             const std::string& parentName,
                             bool writeHeader) {

  // Check if we already have a field with the given name
  if (HasField(name)) {
    XCDFFatal("Cannot create field " << name << ": already exists");
  }

  // Check for illegal characters
  if (name.find_first_of(",:+-/*%^)(\\\"=><&|!") != std::string::npos) {

    XCDFFatal("Field name " << name << " contains unsupported" <<
                                " characters: \",:+-/*%^)(\\\"=><&|!");
  }

  // Check for empty string
  if (name.size() == 0) {
    XCDFFatal("Field name cannot be an empty string");
  }

  // Check for leading/trailing whitespace
  if (name.find_first_not_of(" \t\r\n") != 0 ||
      name.find_last_not_of(" \t\r\n") != name.size() - 1) {
    XCDFFatal("Field name " << name << " contains unsupported" <<
                                    " leading or trailing white space");
  }

  char firstChar = name[0];
  if (!isalpha(firstChar)) {
    XCDFFatal("Field name " << name << " does not start with an" <<
                                                " alphabetic character");
  }

  if (!name.compare("currentEventNumber")) {
    XCDFFatal("Field name \"currentEventNumber\"" <<
                         " is reserved and cannot be used.");
  }

  const XCDFFieldDataBase* parent = NULL;
  if (parentName.compare(NO_PARENT)) {
    parent = CheckParent(parentName);
  }

  // If writing, put the descriptor into the header
  if (writeHeader) {
    XCDFFieldDescriptor descriptor;
    descriptor.name_ = name;
    descriptor.type_ = type;
    descriptor.rawResolution_ = resolution;

    // Can just use name here if we write uint fields first.  They're
    // always in order, so no worries.
    descriptor.parentName_ = parentName;
    fileHeader_.AddFieldDescriptor(descriptor);
  }

  XCDFFieldDataBasePtr ptr =
      XCDFFieldDataAllocator::AllocateField(name, type, resolution, parent);

  /* Order fields by:
   * 1. unsigned integers
   * 2. signed integers
   * 3. floating point
   * This preserves field order compatibility with previous versions
   */
  fieldList_.insert(
      std::upper_bound(fieldList_.begin(), fieldList_.end(), type), ptr);
}

const XCDFFieldDataBase*
XCDFFile::CheckParent(const std::string& parentName) const {

  if (!HasField(parentName)) {
    XCDFFatal("Parent field \"" << parentName << "\" has not been allocated");
  }
  if (!IsUnsignedIntegerField(parentName)) {
    XCDFFatal("Parent field \"" << parentName <<
                                     "\" is not unsigned integer type");
  }
  const XCDFFieldDataBase& parent = **findFieldByName(parentName, true);
  if (parent.GetRawResolution() != 1) {
    XCDFFatal("Parent field \"" << parentName << "\" must have resolution 1");
  }
  return &parent;
}
