
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

#ifndef XCDF_FILE_INCLUDED_H
#define XCDF_FILE_INCLUDED_H

#include <xcdf/alias/XCDFFieldAliasAllocator.h>
#include <xcdf/alias/XCDFFieldAlias.h>
#include <xcdf/alias/XCDFFieldAliasBase.h>
#include <xcdf/alias/XCDFAliasDescriptor.h>

#include <xcdf/XCDFFrame.h>
#include <xcdf/XCDFBlockData.h>
#include <xcdf/XCDFBlockHeader.h>
#include <xcdf/XCDFFileTrailer.h>
#include <xcdf/XCDFFileHeader.h>
#include <xcdf/XCDFField.h>
#include <xcdf/XCDFFieldHeader.h>
#include <xcdf/XCDFFieldDataAllocator.h>
#include <xcdf/XCDFBlockEntry.h>
#include <xcdf/XCDFFieldDescriptor.h>
#include <xcdf/XCDFStreamHandler.h>
#include <xcdf/XCDFDefs.h>
#include <xcdf/version.h>

#include <string>
#include <vector>
#include <map>
#include <ostream>
#include <istream>
#include <cassert>
#include <cctype>

/*!
 * @class XCDFFile
 * @author Jim Braun
 * @date 29 Nov 2011
 * @brief XCDF file handle with iterator access to stored records
 */
class XCDFFile {

  public:

    /// Open a disk file in the specified mode
    XCDFFile(const char* fileName,
             const char* mode) {

      Init();
      Open(fileName, mode);
    }

    /// Read data from the supplied istream
    XCDFFile(std::istream& istream) {

      Init();
      Open(istream);
    }

    /// Write data to the supplied ostream
    XCDFFile(std::ostream& ostream) {

      Init();
      Open(ostream);
    }

    XCDFFile() {

      Init();
    }

    ~XCDFFile() {

      // Close-on-destruction behavior prevents assignment/copy construction
      Close();
    }

    /*
     *  Open a file on-disk in the given mode
     *  @return: success or failure of the underlying open call
     */
    bool Open(const char* fileName, const char* mode);
    bool Open(const std::string& fileName, const std::string& mode) {
      return Open(fileName.c_str(), mode.c_str());
    }

    /// Open the file, reading from the provided istream
    void Open(std::istream& istream) {

      if (isOpen_) {
        Close();
      }
      isOpen_ = true;

      streamHandler_.SetInputStream(istream);
      isModifiable_ = false;
      currentFileName_ = "Unnamed input stream";
      ReadFileHeaders();
    }

    /// Open the file, writing to the provided istream
    void Open(std::ostream& ostream) {

      if (isOpen_) {
        Close();
      }
      isOpen_ = true;
      currentFileName_ = "Unnamed output stream";
      streamHandler_.SetOutputStream(ostream);
    }

    /* Close the file.  Underlying file/stream resources will be closed
     * and released unless the stream open/close constructors were invoked.
     * Fields will be deallocated.
     */
    void Close();

    bool IsWritable() const {return streamHandler_.IsWritable();}
    bool IsReadable() const {return streamHandler_.IsReadable();}
    bool IsOpen() const {return isOpen_;}
    const std::string& GetCurrentFileName() const {return currentFileName_;}

    /*
     *   Write the next event.  Return:
     *
     *   1 if event is written successfully
     *   0 if no further events can be written successfully
     */
    int Write();

    /*
     *   Read in the next event.  Return:
     *
     *   1 if another event is read successfully
     *   0 if no further events can be read successfully
     */
    int Read();

    /// Seek to the given event in the file by absolute position
    bool Seek(uint64_t absoluteEventPos);

    /// Return the total number of events in the file
    uint64_t GetEventCount();

    /// Return the number of the current event
    uint64_t GetCurrentEventNumber() const {

      // Should return the position of the next event
      if (IsWritable()) {
        return eventCount_;
      }

      // Return the position of the current event, or 2^64-1 if not yet read
      if (IsReadable()) {
        return eventCount_ - 1;
      }

      return -1;
    }

    /// Return the number of the current block
    uint64_t GetCurrentBlockNumber() const {return blockCount_;}

    /// Return the file to a state where calling Read() gives the
    /// starting event, if possible.
    bool Rewind();

    /// Write out the current block and start a new one
    void StartNewBlock() {
      if (!IsWritable()) {
        XCDFFatal("Must be in write mode to start a new block");
      }
      WriteBlock();
    }

    /// Force write of the file header, if not already written
    void WriteFileHeader() {

      if (!IsWritable()) {
        XCDFFatal("Must be in write mode to force write of file header");
      }

      isModifiable_ = false;

      if (!headerWritten_) {
        fileHeader_.PackFrame(currentFrame_);
        WriteFrame();
        headerWritten_ = true;
      } else {
        XCDFError("File header already written.  Not writing.");
      }
    }

    /// Does the underlying file have a trailer pointer that we can seek to?
    bool IsSimple() {return isSimple_;}

    /// Get the version number of the current open file
    uint32_t GetVersion() const {return fileHeader_.GetVersion();}

    /// Get the total number of fields allocated in the file
    uint32_t GetNFields() const {return fieldList_.size();}

    /*
     *  Check if the file contains the given field
     */
    bool HasField(const std::string& name) const {

      return FindFieldByName(name) != fieldList_.end();
    }

    /*
     *  Check if a given field contains vector data
     */
    bool IsVectorField(const std::string& name) const {

      return (*FindFieldByName(name, true))->HasParent();
    }

    /*
     *  Get the name of the parent of the given field
     */
    std::string GetFieldParentName(const std::string& name) const {

      return (*FindFieldByName(name, true))->GetParentName();
    }

    /*
     *  Check if a given field contains unsigned integer data
     */
    bool IsUnsignedIntegerField(const std::string& name) const {
      return (*FindFieldByName(name, true))->IsUnsignedIntegerField();
    }

    /*
     *  Check if a given field contains signed integer data
     */
    bool IsSignedIntegerField(const std::string& name) const {
      return (*FindFieldByName(name, true))->IsSignedIntegerField();
    }

    /*
     *  Check if a given field contains floating point data
     */
    bool IsFloatingPointField(const std::string& name) const {
      return (*FindFieldByName(name, true))->IsFloatingPointField();
    }

    /*
     *  Provide explicit GetField routines rather than a templated
     */
    ConstXCDFUnsignedIntegerField
    GetUnsignedIntegerField(const std::string& name) const {
      return XCDFFieldDataAllocator::GetUnsignedIntegerField(
                                           **FindFieldByName(name, true));
    }
    ConstXCDFSignedIntegerField
    GetSignedIntegerField(const std::string& name) const {
      return XCDFFieldDataAllocator::GetSignedIntegerField(
                                           **FindFieldByName(name, true));
    }
    ConstXCDFFloatingPointField
    GetFloatingPointField(const std::string& name) const {
      return XCDFFieldDataAllocator::GetFloatingPointField(
                                           **FindFieldByName(name, true));
    }

    XCDFUnsignedIntegerField
    GetUnsignedIntegerField(const std::string& name) {
      return XCDFFieldDataAllocator::GetUnsignedIntegerField(
                                           **FindFieldByName(name, true));
    }
    XCDFSignedIntegerField
    GetSignedIntegerField(const std::string& name) {
      return XCDFFieldDataAllocator::GetSignedIntegerField(
                                           **FindFieldByName(name, true));
    }
    XCDFFloatingPointField
    GetFloatingPointField(const std::string& name) {
      return XCDFFieldDataAllocator::GetFloatingPointField(
                                           **FindFieldByName(name, true));
    }

    std::vector<XCDFFieldDescriptor>::const_iterator
    FieldDescriptorsBegin() const {return fileHeader_.FieldDescriptorsBegin();}

    std::vector<XCDFFieldDescriptor>::const_iterator
    FieldDescriptorsEnd() const {return fileHeader_.FieldDescriptorsEnd();}

    uint64_t GetFieldBytes(const std::string& name) {
      CheckGlobals();
      return (*FindFieldByName(name, true))->GetTotalBytes();
    }

    std::pair<uint64_t, uint64_t>
    GetUnsignedIntegerFieldRange(const std::string& name) {
      CheckGlobals();
      return XCDFFieldDataAllocator::GetUnsignedIntegerFieldRange(
                                               **FindFieldByName(name, true));
    }

    std::pair<int64_t, int64_t>
    GetSignedIntegerFieldRange(const std::string& name) {
      CheckGlobals();
      return XCDFFieldDataAllocator::GetSignedIntegerFieldRange(
                                               **FindFieldByName(name, true));
    }

    std::pair<double, double>
    GetFloatingPointFieldRange(const std::string& name) {
      CheckGlobals();
      return XCDFFieldDataAllocator::GetFloatingPointFieldRange(
                                               **FindFieldByName(name, true));
    }


    /// Set the maximum number of events contained in a block
    void SetBlockSize(const uint64_t blockSize) {blockSize_ = blockSize;}

    /// Get the maximum number of events contained in a block
    uint64_t GetBlockSize() const {return blockSize_;}

    /// Set the threshold above which a new block is started (default: 100 MB)
    void SetBlockThresholdByteCount(const uint64_t count) {
      thresholdByteCount_ = count;
    }

    /// Get the size (in bytes) of the largest event that can be written
    uint64_t GetBlockThresholdByteCount() const {
      return thresholdByteCount_;
    }

    /// Disable ability to do fast seek operations (usually never necessary)
    void DisableBlockTable() {fileTrailer_.DisableBlockTable();}

    /*
     * Set the zero alignment.  Zero alignment can only be set when writing.
     *
     *   true (default): When reading back, the difference of field values
     *                   and zero will always be an integer number of
     *                   resolution units.
     *
     *            false: When reading back, the difference of field values
     *                   and zero modulo the field resolution will differ
     *                   block-to-block, depending on the minimum field value
     *                   written to the block.
     */
    void SetZeroAlign(bool align = true) {zeroAlign_ = align;}

    /// Add a string comment to the file
    void AddComment(const std::string& comment) {
      fileTrailer_.AddComment(comment);
    }

    void AddVersionComment() {
      std::stringstream str;
      str << "XCDF version "
            << xcdf::get_version();
      AddComment(str.str());
    }

    /// Force load the comments at the end of a streaming
    /// file if we don't already have them
    void LoadComments() {
      if (!isModifiable_ && !blockTableComplete_) {
        // Get the event count, causing all trailers to be read
        GetEventCount();
      }
    }

    /// Get an iterator to the beginning of the comment list
    std::vector<std::string>::const_iterator
    CommentsBegin() {return fileTrailer_.CommentsBegin();}

    /// Get an iterator to the end of the comment list
    std::vector<std::string>::const_iterator
    CommentsEnd() {return fileTrailer_.CommentsEnd();}

    /// Get the number of comments
    /// Optional: Force load the comments at the end of a streaming
    /// file if we don't already have them
    unsigned GetNComments(bool forceLoad=false) {
      if (!isModifiable_ && !blockTableComplete_ && forceLoad) {
        // Get the event count, causing all trailers to be read
        GetEventCount();
      }
      return fileTrailer_.GetNComments();
    }

    /*
     *   Allocate a field with floating point data.
     *
     *   Parameters:
     *
     *           name: Name of the field
     *
     *     resolution: The floating point resolution needed in the field.
     *                 If resolution <= 0. is specified, the full 64-bit
     *                 double will be written to file.
     *
     *     parentName: If field is a vector, the name of the field that
     *                 contains the number of entries in the vector
     */
    XCDFFloatingPointField AllocateFloatingPointField(
                                  const std::string& name,
                                  double resolution,
                                  const std::string& parentName = NO_PARENT) {

      if (std::isnan(resolution) || std::isinf(resolution)) {
        XCDFFatal("Field " << name << ": Resolution "<<
                                     resolution << " not allowed.");
      }

      if (isAppend_) {
        CheckAppend(GetFloatingPointField(name), resolution, parentName);
      } else {
        CheckModifiable();
        uint64_t rawRes = XCDFSafeTypePun<double, uint64_t>(resolution);
        AllocateField(name, XCDF_FLOATING_POINT, rawRes, parentName, true);
      }
      return GetFloatingPointField(name);
    }

    /*
     *   Allocate a field with unsigned integer data.
     *
     *   Parameters:
     *
     *           name: Name of the field
     *
     *     resolution: The (positive) integer resolution needed in the field
     *
     *     parentName: If field is a vector, the name of the field that
     *                 contains the number of entries in the vector
     */
    XCDFUnsignedIntegerField AllocateUnsignedIntegerField(
                                 const std::string& name,
                                 uint64_t resolution,
                                 const std::string& parentName = NO_PARENT) {

      if (resolution == 0) {
        resolution = 1;
      }

      if (isAppend_) {
        CheckAppend(GetUnsignedIntegerField(name), resolution, parentName);
      } else {
        CheckModifiable();
        AllocateField(name, XCDF_UNSIGNED_INTEGER,
                      resolution, parentName, true);
      }
      return GetUnsignedIntegerField(name);
    }

    /*
     *   Allocate a field with signed integer data.
     *
     *   Parameters:
     *
     *           name: Name of the field
     *
     *     resolution: The (positive) integer resolution needed in the field
     *
     *     parentName: If field is a vector, the name of the field that
     *                 contains the number of entries in the vector
     */
    XCDFSignedIntegerField AllocateSignedIntegerField(
                                 const std::string& name,
                                 int64_t resolution,
                                 const std::string& parentName = NO_PARENT) {

      if (resolution <= 0) {
        resolution = 1;
      }

      if (isAppend_) {
        CheckAppend(GetSignedIntegerField(name), resolution, parentName);
      } else {
        CheckModifiable();
        uint64_t rawRes = XCDFSafeTypePun<int64_t, uint64_t>(resolution);
        AllocateField(name, XCDF_SIGNED_INTEGER, rawRes, parentName, true);
      }
      return GetSignedIntegerField(name);
    }

    /*
     *  Apply an operator to all fields.
     *
     *  These operations must all be defined:
     *
     *  operator()(XCDFIntegerField&),
     *  operator()(XCDFSignedIntegerField&),
     *  operator()(XCDFFloatingPointField&)
     *
     *  It may be useful to use a catch-all template
     *  for routines that are not type-specific:
     *
     *  template <typename T>
     *  void operator()(XCDFField<T>& field) { }
     */
    template <typename T>
    void ApplyFieldVisitor(T& visitor) {
      for (FieldList::iterator it = fieldList_.begin();
                               it != fieldList_.end(); ++it) {
        XCDFFieldDataAllocator::Visit(**it, visitor);
      }
    }

    /*
     *  Apply an operator to all fields.
     *
     *  These operations must all be defined:
     *
     *  operator()(XCDFIntegerField&),
     *  operator()(XCDFSignedIntegerField&),
     *  operator()(XCDFFloatingPointField&)
     *
     *  It may be useful to use a catch-all template
     *  for routines that are not type-specific:
     *
     *  template <typename T>
     *  void operator()(ConstXCDFField<T>& field) { }
     */
    template <typename T>
    void ApplyFieldVisitor(T& visitor) const {
      for (FieldList::const_iterator it = fieldList_.begin();
                                     it != fieldList_.end(); ++it) {
        XCDFFieldDataAllocator::Visit(**it, visitor);
      }
    }

    void CreateAlias(const std::string& name, const std::string& expression) {
      CheckName(name);
      XCDFFieldAliasBasePtr ptr = AllocateFieldAlias(name, expression, *this);
      aliasList_.push_back(ptr);
      if (headerWritten_) {
        fileTrailer_.AddAliasDescriptor(GetXCDFAliasDescriptor(*ptr));
      } else {
        fileHeader_.AddAliasDescriptor(GetXCDFAliasDescriptor(*ptr));
      }
    }

    bool HasAlias(const std::string& name) const {
      return FindAliasByName(name) != aliasList_.end();
    }

    bool IsUnsignedIntegerAlias(const std::string& name) const {
      return (*FindAliasByName(name, true))->IsUnsignedIntegerAlias();
    }

    bool IsSignedIntegerAlias(const std::string& name) const {
      return (*FindAliasByName(name, true))->IsSignedIntegerAlias();
    }

    bool IsFloatingPointAlias(const std::string& name) const {
      return (*FindAliasByName(name, true))->IsFloatingPointAlias();
    }

    ConstXCDFUnsignedIntegerFieldAlias
    GetUnsignedIntegerAlias(const std::string& name) const {
      return CheckedGetAlias<uint64_t>(**FindAliasByName(name, true));
    }

    ConstXCDFSignedIntegerFieldAlias
    GetSignedIntegerAlias(const std::string& name) const {
      return CheckedGetAlias<int64_t>(**FindAliasByName(name, true));
    }

    ConstXCDFFloatingPointFieldAlias
    GetFloatingPointAlias(const std::string& name) const {
      return CheckedGetAlias<double>(**FindAliasByName(name, true));
    }

    std::vector<XCDFAliasDescriptor>::const_iterator
    AliasDescriptorsBegin() const {return fileHeader_.AliasDescriptorsBegin();}

    std::vector<XCDFAliasDescriptor>::const_iterator
    AliasDescriptorsEnd() const {return fileHeader_.AliasDescriptorsEnd();}

    XCDFAliasDescriptor
    GetAliasDescriptor(const std::string& name) {
      return GetXCDFAliasDescriptor(**FindAliasByName(name, true));
    }

  private:

    // Keep a vector of XCDFFieldData objects.  List order is read/write
    // order, so it must be preserved.
    typedef std::vector<XCDFFieldDataBasePtr> FieldList;
    FieldList fieldList_;

    // Vector of XCDFFieldAliasBase objects.
    typedef std::vector<XCDFFieldAliasBasePtr> AliasList;
    AliasList aliasList_;

    // Configurable parameters
    uint64_t blockSize_;
    uint64_t thresholdByteCount_;
    bool zeroAlign_;

    // Counters
    uint64_t eventCount_;
    uint64_t blockCount_;
    uint32_t blockEventCount_;

    // Internal state controllers
    bool isModifiable_;
    bool blockTableComplete_;
    bool headerWritten_;
    bool isOpen_;
    bool isAppend_;
    bool recover_;
    std::string currentFileName_;
    bool isSimple_;

    // State of field global data
    bool unusableGlobalsFromFile_;
    bool haveV3Globals_;

    // I/O frame.  Allocate only one copy for efficiency.
    XCDFFrame currentFrame_;
    std::streampos currentFileStartOffset_;
    std::streampos currentFrameStartOffset_;
    std::streampos currentFrameEndOffset_;

    // Frame object pool.  Allocate only one copy for efficiency.
    XCDFFileHeader  fileHeader_;
    XCDFBlockHeader blockHeader_;
    XCDFBlockData   blockData_;
    XCDFFileTrailer fileTrailer_;

    // I/O streams
    XCDFStreamHandler streamHandler_;

    void Init();
    void WriteFrame();
    void ReadFrame();
    void WriteBlock();
    void WriteEvent();
    void ReadEvent();
    bool ReadNextBlock();
    bool GetNextBlockWithEvents();
    bool DoSeek(const std::streampos& pos);
    void ReadFileHeaders();
    void LoadFileHeader(XCDFFileHeader& header);
    void LoadFileTrailer(XCDFFileTrailer& trailer);
    void CopyTrailer(const XCDFFileTrailer& trailer);
    void SetGlobals(const XCDFFileTrailer& trailer);
    void CheckGlobals();
    bool NextFrameExists();
    bool OpenAppend(const char* filename);
    bool PrepareAppend(const char* filename,
                       uint64_t position,
                       uint64_t cnt);

    const XCDFFieldDataBase*
    CheckParent(const std::string& parentName) const;
    void CheckName(const std::string& name) const;

    /// Helper class to match an XCDF field/alias by name
    class XCDFNameMatch {

      public:

        XCDFNameMatch(const std::string& name) : name_(name) { }
        bool operator()(const XCDFFieldDataBasePtr& ptr) {
          return ptr->GetName() == name_;
        }
        bool operator()(const XCDFFieldAliasBasePtr& base) {
          return base->GetName() == name_;
        }

      private:
        const std::string& name_;
    };

    AliasList::const_iterator
    FindAliasByName(const std::string& name,
                    bool requirePresent = false) const {
      AliasList::const_iterator it = std::find_if(aliasList_.begin(),
                                                  aliasList_.end(),
                                                  XCDFNameMatch(name));
      if (requirePresent) {
        if (it == aliasList_.end()) {
          XCDFFatal("No such alias: " << name);
        }
      }
      return it;
    }

    FieldList::const_iterator
    FindFieldByName(const std::string& name,
                    bool requirePresent = false) const {
      FieldList::const_iterator it = std::find_if(fieldList_.begin(),
                                                  fieldList_.end(),
                                                  XCDFNameMatch(name));
      if (requirePresent) {
        if (it == fieldList_.end()) {
          XCDFFatal("No such field: " << name);
        }
      }
      return it;
    }

    FieldList::iterator
    FindFieldByName(const std::string& name,
                    bool requirePresent = false) {
      FieldList::iterator it = std::find_if(fieldList_.begin(),
                                            fieldList_.end(),
                                            XCDFNameMatch(name));
      if (requirePresent) {
        if (it == fieldList_.end()) {
          XCDFFatal("No such field: " << name);
        }
      }
      return it;
    }

    template<typename T>
    void LoadNewAliases(const T& t) {
      for (std::vector<XCDFAliasDescriptor>::const_iterator
                       it = t.AliasDescriptorsBegin();
                       it != t.AliasDescriptorsEnd(); ++it) {
        if (!fileHeader_.HasAliasDescriptor(*it)) {
          fileHeader_.AddAliasDescriptor(*it);
          XCDFFieldAliasBasePtr ptr =
              AllocateFieldAlias(it->GetName(), it->GetExpression(), *this);
          aliasList_.push_back(ptr);
        }
      }
    }

    template<typename T>
    void LoadAliases(const T& t) {
      for (std::vector<XCDFAliasDescriptor>::const_iterator
                       it = t.AliasDescriptorsBegin();
                       it != t.AliasDescriptorsEnd(); ++it) {
        XCDFFieldAliasBasePtr ptr =
            AllocateFieldAlias(it->GetName(), it->GetExpression(), *this);
        aliasList_.push_back(ptr);
      }
    }

    // Close-on-destruction behavior prevents assignment/copy construction
    // Prevent these operations.
    const XCDFFile& operator=(const XCDFFile& file) {
      UNUSED(file);
      return *this;
    }
    XCDFFile(const XCDFFile& file) { UNUSED(file); }


    /* Check if calls have been made (e.g. calling Write(), opening the
     * file for reading, etc.) that prevent changes to certain aspects.
     */
    void CheckModifiable() const {
      if (!isModifiable_) {
        XCDFFatal("Unable to add fields to an existing file" <<
                                     " or after the first event is added.");
      }
    }

    /*
     *  Checks the field sanity, checks the parent (if applicable),
     *  then adds the field to the XCDFFileHeader object (if opened for
     *  writing) to be written to file.
     */
    void AllocateField(const std::string& name,
                       const XCDFFieldType type,
                       const uint64_t resolution,
                       const std::string& parentName = NO_PARENT,
                       bool writeHeader = false);

    template <typename T>
    void CheckAppend(XCDFField<T> field,
                     T resolution, const std::string& parentName) {

      if (field.GetResolution() != resolution ||
          field.GetParentName() != parentName) {

        XCDFFatal("Unable to find matching field for " <<
                                 field.GetName() << " in append");
      }
    }

    template <typename F>
    void FieldListForEach(F& fxn) {
      for (FieldList::iterator it = fieldList_.begin();
                               it != fieldList_.end(); ++it) {
        fxn(**it);
      }
    }
};

#endif // XCDF_FILE_INCLUDED_H
