
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

#include <xcdf/XCDFUncompressedBlock.h>
#include <xcdf/XCDFFrame.h>
#include <xcdf/XCDFBlockData.h>
#include <xcdf/XCDFBlockHeader.h>
#include <xcdf/XCDFFileTrailer.h>
#include <xcdf/XCDFFileHeader.h>
#include <xcdf/XCDFField.h>
#include <xcdf/XCDFFieldHeader.h>
#include <xcdf/XCDFDataManager.h>
#include <xcdf/XCDFBlockEntry.h>
#include <xcdf/XCDFFieldDescriptor.h>
#include <xcdf/XCDFStreamHandler.h>
#include <xcdf/XCDFDefs.h>
#include <xcdf/config.h>

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
      ReadFileHeaders();
    }

    /// Open the file, writing to the provided istream
    void Open(std::ostream& ostream) {

      if (isOpen_) {
        Close();
      }
      isOpen_ = true;

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

    /// Get the total number of fields allocated in the file
    uint32_t GetNFields() const {

      return unsignedIntegerFieldList_.size() +
             signedIntegerFieldList_.size() +
             floatingPointFieldList_.size();
    }

    /*
     *  Check if the file contains the given field
     */
    bool HasField(const std::string& name) const {

      return (IsUnsignedIntegerField(name) ||
              IsSignedIntegerField(name) || IsFloatingPointField(name));
    }

    /*
     *  Check if a given field contains vector data
     */
    bool IsVectorField(const std::string& name) const {

      std::map<std::string, XCDFDataManager<uint64_t> >::const_iterator
        unsignedIntegerFieldMapIterator = unsignedIntegerFieldMap_.find(name);
      if (unsignedIntegerFieldMapIterator != unsignedIntegerFieldMap_.end()) {
        return (unsignedIntegerFieldMapIterator->second).HasParent();
      }

      std::map<std::string, XCDFDataManager<int64_t> >::const_iterator
           signedIntegerFieldMapIterator = signedIntegerFieldMap_.find(name);
      if (signedIntegerFieldMapIterator != signedIntegerFieldMap_.end()) {
        return (signedIntegerFieldMapIterator->second).HasParent();
      }

      std::map<std::string, XCDFDataManager<double> >::const_iterator
           floatingPointFieldMapIterator = floatingPointFieldMap_.find(name);
      if (floatingPointFieldMapIterator != floatingPointFieldMap_.end()) {
        return (floatingPointFieldMapIterator->second).HasParent();
      }

      return false;
    }

    /*
     *  Get the name of the parent of the given field
     */
    std::string GetFieldParentName(const std::string& name) const {

      std::map<std::string, XCDFDataManager<uint64_t> >::const_iterator
        unsignedIntegerFieldMapIterator = unsignedIntegerFieldMap_.find(name);
      if (unsignedIntegerFieldMapIterator != unsignedIntegerFieldMap_.end()) {
        return (unsignedIntegerFieldMapIterator->second).GetParent().GetName();
      }

      std::map<std::string, XCDFDataManager<int64_t> >::const_iterator
           signedIntegerFieldMapIterator = signedIntegerFieldMap_.find(name);
      if (signedIntegerFieldMapIterator != signedIntegerFieldMap_.end()) {
        return (signedIntegerFieldMapIterator->second).GetParent().GetName();
      }

      std::map<std::string, XCDFDataManager<double> >::const_iterator
           floatingPointFieldMapIterator = floatingPointFieldMap_.find(name);
      if (floatingPointFieldMapIterator != floatingPointFieldMap_.end()) {
        return (floatingPointFieldMapIterator->second).GetParent().GetName();
      }

      return "";
    }

    /*
     *  Check if a given field contains unsigned integer data
     */
    bool IsUnsignedIntegerField(const std::string& name) const {
      return unsignedIntegerFieldMap_.find(name) !=
                                          unsignedIntegerFieldMap_.end();
    }

    /*
     *  Check if a given field contains signed integer data
     */
    bool IsSignedIntegerField(const std::string& name) const {
      return
          signedIntegerFieldMap_.find(name) != signedIntegerFieldMap_.end();
    }

    /*
     *  Check if a given field contains floating point data
     */
    bool IsFloatingPointField(const std::string& name) const {
      return
          floatingPointFieldMap_.find(name) != floatingPointFieldMap_.end();
    }

    /*
     *  Provide explicit GetField routines and typedefs to avoid the
     *  certain errors due to incorrect template parameters (i.e.
     *  GetField<float>() ).
     */
    XCDFUnsignedIntegerField
    GetUnsignedIntegerField(const std::string& name) const {
      std::map<std::string, XCDFDataManager<uint64_t> >::const_iterator
                                   it = unsignedIntegerFieldMap_.find(name);
      if (it == unsignedIntegerFieldMap_.end()) {
        XCDFFatal("Field name: " << name << ": No such field");
      }
      return (it->second).GetField();
    }

    XCDFSignedIntegerField
    GetSignedIntegerField(const std::string& name) const {
      std::map<std::string, XCDFDataManager<int64_t> >::const_iterator
                                  it = signedIntegerFieldMap_.find(name);
      if (it == signedIntegerFieldMap_.end()) {
        XCDFFatal("Field name: " << name << ": No such field");
      }
      return (it->second).GetField();
    }

    XCDFFloatingPointField
    GetFloatingPointField(const std::string& name) const {
      std::map<std::string, XCDFDataManager<double> >::const_iterator
                                 it = floatingPointFieldMap_.find(name);
      if (it == floatingPointFieldMap_.end()) {
        XCDFFatal("Field name: " << name << ": No such field");
      }
      return (it->second).GetField();
    }

    std::vector<XCDFUnsignedIntegerField>::iterator
    UnsignedIntegerFieldsBegin() {
      return unsignedIntegerBareFieldList_.begin();
    }

    std::vector<XCDFUnsignedIntegerField>::iterator
    UnsignedIntegerFieldsEnd() {
      return unsignedIntegerBareFieldList_.end();
    }

    std::vector<XCDFSignedIntegerField>::iterator
    SignedIntegerFieldsBegin() {return signedIntegerBareFieldList_.begin();}

    std::vector<XCDFSignedIntegerField>::iterator
    SignedIntegerFieldsEnd() {return signedIntegerBareFieldList_.end();}

    std::vector<XCDFFloatingPointField>::iterator
    FloatingPointFieldsBegin() {return floatingPointBareFieldList_.begin();}

    std::vector<XCDFFloatingPointField>::iterator
    FloatingPointFieldsEnd() {return floatingPointBareFieldList_.end();}

    std::vector<XCDFUnsignedIntegerField>::const_iterator
    UnsignedIntegerFieldsBegin() const {
      return unsignedIntegerBareFieldList_.begin();
    }

    std::vector<XCDFUnsignedIntegerField>::const_iterator
    UnsignedIntegerFieldsEnd() const {
      return unsignedIntegerBareFieldList_.end();
    }

    std::vector<XCDFSignedIntegerField>::const_iterator
    SignedIntegerFieldsBegin() const {
      return signedIntegerBareFieldList_.begin();
    }

    std::vector<XCDFSignedIntegerField>::const_iterator
    SignedIntegerFieldsEnd() const {return signedIntegerBareFieldList_.end();}

    std::vector<XCDFFloatingPointField>::const_iterator
    FloatingPointFieldsBegin() const {
      return floatingPointBareFieldList_.begin();
    }

    std::vector<XCDFFloatingPointField>::const_iterator
    FloatingPointFieldsEnd() const {return floatingPointBareFieldList_.end();}

    std::vector<XCDFFieldDescriptor>::const_iterator
    FieldDescriptorsBegin() const {return fileHeader_.FieldDescriptorsBegin();}

    std::vector<XCDFFieldDescriptor>::const_iterator
    FieldDescriptorsEnd() const {return fileHeader_.FieldDescriptorsEnd();}


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
            << XCDF_MAJOR_VERSION << "."
            << XCDF_MINOR_VERSION << "."
            << XCDF_PATCH_VERSION;
      AddComment(str.str());
    }

    /// Get an iterator to the beginning of the comment list
    std::vector<std::string>::const_iterator CommentsBegin() const {
      return fileTrailer_.CommentsBegin();
    }

    /// Get an iterator to the end of the comment list
    std::vector<std::string>::const_iterator CommentsEnd() const {
      return fileTrailer_.CommentsEnd();
    }

    /// Get the number of comments
    unsigned GetNComments() const {
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
                                     const std::string& parentName = "") {

      if (isAppend_) {
        XCDFFloatingPointField exField = GetFloatingPointField(name);
        if (exField.GetResolution() != resolution ||
            GetFieldParentName(name) != parentName) {
          XCDFFatal("Unable to find matching field for " <<
                                          name << " in append");
        }
        return exField;
      }

      CheckModifiable();

      if (std::isnan(resolution) || std::isinf(resolution)) {
        XCDFFatal("Field " << name << ": Resolution "<<
                                     resolution << " not allowed.");
      }

      XCDFDataManager<double> manager =
             AllocateField(name, XCDF_FLOATING_POINT, resolution, parentName);
      floatingPointFieldList_.push_back(manager);
      floatingPointBareFieldList_.push_back(manager.GetField());
      floatingPointFieldMap_.insert(std::pair<std::string,
                  XCDFDataManager<double> >(name, manager));
      return manager.GetField();
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
                                    const std::string& parentName = "") {

      if (resolution == 0) {
        resolution = 1;
      }

      if (isAppend_) {
        XCDFUnsignedIntegerField exField = GetUnsignedIntegerField(name);
        if (exField.GetResolution() != resolution ||
            GetFieldParentName(name) != parentName) {
          XCDFFatal("Unable to find matching field for " <<
                                          name << " in append");
        }
        return exField;
      }

      CheckModifiable();

      XCDFDataManager<uint64_t> manager =
           AllocateField(name, XCDF_UNSIGNED_INTEGER, resolution, parentName);
      unsignedIntegerFieldList_.push_back(manager);
      unsignedIntegerBareFieldList_.push_back(manager.GetField());
      unsignedIntegerFieldMap_.insert(std::pair<std::string,
                   XCDFDataManager<uint64_t> >(name, manager));
      return manager.GetField();
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
                                       const std::string& parentName = "") {

      if (resolution <= 0) {
        resolution = 1;
      }

      if (isAppend_) {
        XCDFSignedIntegerField exField = GetSignedIntegerField(name);
        if (exField.GetResolution() != resolution ||
            GetFieldParentName(name) != parentName) {
          XCDFFatal("Unable to find matching field for " <<
                                          name << " in append");
        }
        return exField;
      }

      CheckModifiable();

      XCDFDataManager<int64_t> manager  =
             AllocateField(name, XCDF_SIGNED_INTEGER, resolution, parentName);
      signedIntegerFieldList_.push_back(manager);
      signedIntegerBareFieldList_.push_back(manager.GetField());
      signedIntegerFieldMap_.insert(std::pair<std::string,
                   XCDFDataManager<int64_t> >(name, manager));
      return manager.GetField();
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
     *  void operator()(const XCDFField<T>& field) { }
     */
    template <typename T>
    void ApplyFieldVisitor(T& visitor) {
      for (std::vector<XCDFField<uint64_t> >::iterator
                            it = unsignedIntegerBareFieldList_.begin();
                            it != unsignedIntegerBareFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFField<int64_t> >::iterator
                            it = signedIntegerBareFieldList_.begin();
                            it != signedIntegerBareFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFField<double> >::iterator
                            it = floatingPointBareFieldList_.begin();
                            it != floatingPointBareFieldList_.end(); ++it) {
        visitor(*it);
      }
    }

    template <typename T>
    void ApplyFieldVisitor(T& visitor) const {
      for (std::vector<XCDFField<uint64_t> >::const_iterator
                            it = unsignedIntegerBareFieldList_.begin();
                            it != unsignedIntegerBareFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFField<int64_t> >::const_iterator
                            it = signedIntegerBareFieldList_.begin();
                            it != signedIntegerBareFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFField<double> >::const_iterator
                            it = floatingPointBareFieldList_.begin();
                            it != floatingPointBareFieldList_.end(); ++it) {
        visitor(*it);
      }
    }

  private:

    // Allocated fields -- allocate for each data type
    std::map<std::string, XCDFDataManager<uint64_t> > unsignedIntegerFieldMap_;
    std::map<std::string, XCDFDataManager<int64_t> >  signedIntegerFieldMap_;
    std::map<std::string, XCDFDataManager<double> >   floatingPointFieldMap_;

    // Keep identical data in vectors to guarantee iteration order
    // is order is the order in which the fields were added
    std::vector<XCDFDataManager<uint64_t> > unsignedIntegerFieldList_;
    std::vector<XCDFDataManager<int64_t> >  signedIntegerFieldList_;
    std::vector<XCDFDataManager<double> >   floatingPointFieldList_;

    // Keep a vector of just the XCDFField to provide generic user access
    std::vector<XCDFField<uint64_t> > unsignedIntegerBareFieldList_;
    std::vector<XCDFField<int64_t> >  signedIntegerBareFieldList_;
    std::vector<XCDFField<double> >   floatingPointBareFieldList_;


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
    bool checkedReadForAppendFlag_;

    // Memory buffer to store data block as it is written.  Determine
    // max/min when block is complete.
    XCDFUncompressedBlock uncompressedBlock_;

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

    template <typename T>
    void ApplyFieldManagerVisitor(T& visitor) {
      for (std::vector<XCDFDataManager<uint64_t> >::iterator
                            it = unsignedIntegerFieldList_.begin();
                            it != unsignedIntegerFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFDataManager<int64_t> >::iterator
                            it = signedIntegerFieldList_.begin();
                            it != signedIntegerFieldList_.end(); ++it) {
        visitor(*it);
      }
      for (std::vector<XCDFDataManager<double> >::iterator
                            it = floatingPointFieldList_.begin();
                            it != floatingPointFieldList_.end(); ++it) {
        visitor(*it);
      }
    }

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
    bool NextFrameExists();
    void CheckParent(const std::string& parent) const;
    bool OpenAppend(const char* filename);
    bool PrepareAppend(const char* filename,
                       uint64_t position,
                       uint64_t cnt);


    // Close-on-destruction behavior prevents assignment/copy construction
    // Prevent these operations.
    const XCDFFile& operator=(const XCDFFile& file) {return *this;}
    XCDFFile(const XCDFFile& file) { }


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
    template <typename T>
    XCDFDataManager<T> AllocateField(const std::string& name,
                                     const XCDFFieldType type,
                                     const T resolution,
                                     const std::string& parentName = "") {

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

      // If writing, put the descriptor into the header
      if (IsWritable()) {
        XCDFFieldDescriptor descriptor;
        descriptor.name_ = name;
        descriptor.type_ = type;
        descriptor.rawResolution_ = XCDFSafeTypePun<T, uint64_t>(resolution);

        // Can just use name here if we write uint fields first.  They're
        // always in order, so no worries.
        descriptor.parentName_ = parentName;
        fileHeader_.AddFieldDescriptor(descriptor);
      }

      XCDFField<T> field(name, resolution);
      // Check parent if needed
      if (parentName.compare("")) {
        CheckParent(parentName);
        XCDFField<uint64_t> parent = GetUnsignedIntegerField(parentName);
        return XCDFDataManager<T>(type, field, parent, true);
      }

      // No parent
      return XCDFDataManager<T>(type, field, XCDFField<uint64_t>(), false);
    }
};

#endif // XCDF_FILE_INCLUDED_H
