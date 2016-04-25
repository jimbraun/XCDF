
/*
Copyright (c) 2014, University of Maryland
Jim Braun
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

#ifndef XCDF_UTILITY_UTILITY_INCLUDED_H
#define XCDF_UTILITY_UTILITY_INCLUDED_H

#include <xcdf/XCDF.h>

#include <string>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <utility>
#include <algorithm>
#include <set>

#include <unistd.h>

/*!
 * @file XCDFUtility
 * @author Jim Braun
 * @brief Utility routines for manipulating XCDF files
 */

class DumpFieldVisitor {
  public:

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      std::cout << field.GetName() << ": ";
      for (typename XCDFField<T>::ConstIterator
                                        it = field.Begin();
                                        it != field.End(); ++it) {

        std::cout << std::setprecision(10) << *it << " ";
      }
      std::cout << std::endl;
    }
};

class MatchFieldsVisitor {
  public:

    MatchFieldsVisitor(const std::set<std::string>& fieldSpecs) :
                                              fieldSpecs_(fieldSpecs) { }

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      const std::string& name = field.GetName();
      for (std::set<std::string>::const_iterator
                              it = fieldSpecs_.begin();
                              it != fieldSpecs_.end(); ++it) {

        if (Match(*it, name)) {
          fieldMatches_.insert(name);
          break;
        }
      }

    }

    const std::set<std::string>& GetMatches() const {return fieldMatches_;}

  private:

    std::set<std::string> fieldSpecs_;
    std::set<std::string> fieldMatches_;

    bool DoMatch(const std::string& spec, const std::string& name) {

      // Find the '*' character.  Match up to that point.
      size_t wpos = std::distance(spec.begin(),
                                  std::find(spec.begin(), spec.end(), '*'));
      return !(name.compare(0, wpos, spec, 0, wpos));
    }

    // Match spec against name.  Make copies.
    bool Match(std::string spec, std::string name) {

      // First check for a match
      if (spec == name) {
        return true;
      }

      // First count wildcards
      size_t wcnt = std::count(spec.begin(), spec.end(), '*');
      if (wcnt > 1) {
        XCDFFatal("Too many wildcards: " << spec);
      }
      if (wcnt == 1) {

        // Match forward and reverse, relative to the wildcard.
        if (!DoMatch(spec, name)) {
          return false;
        }
        std::reverse(name.begin(), name.end());
        std::reverse(spec.begin(), spec.end());
        return DoMatch(spec, name);
      }
      return false;
    }
};

class PrintFieldNameVisitor {
  public:

    PrintFieldNameVisitor(XCDFFile& file) : file_(file),
                                            firstCall_(true) { }

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      if (!firstCall_) {
        std::cout << ",";
      }
      firstCall_ = false;

      if (file_.IsVectorField(field.GetName())) {
        std::cout << field.GetName() << "[" <<
                   file_.GetFieldParentName(field.GetName()) << "]/";
      } else {
        std::cout << field.GetName() << "/";
      }

      if (file_.IsUnsignedIntegerField(field.GetName())) {
        std::cout << "U";
      } else if (file_.IsSignedIntegerField(field.GetName())) {
        std::cout << "I";
      } else {
        std::cout << "F";
      }

      std::cout << "/" << std::setprecision(16) << field.GetResolution();
    }

  private:

    XCDFFile& file_;
    bool firstCall_;
};

class PrintFieldDataVisitor {
  public:

    PrintFieldDataVisitor() : firstCall_(true) { }

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      if (!firstCall_) {
        std::cout << ",";
      }
      firstCall_ = false;

      for (typename XCDFField<T>::ConstIterator
                                        it = field.Begin();
                                        it != field.End(); ++it) {

        if (it != field.Begin()) {
          std::cout << ":";
        }

        std::cout << std::setprecision(15) << *it;
      }
    }

    void Reset() {firstCall_ = true;}

  private:

    bool firstCall_;
};

class GetFieldNamesVisitor {
  public:

    GetFieldNamesVisitor(std::set<std::string>& set) : set_(set) { }

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      set_.insert(field.GetName());
    }

  private:

    std::set<std::string>& set_;
};

class FileCompare {

  public:

    FileCompare() { }

    void operator()(const XCDFUnsignedIntegerField& field) {
      uiFields_.push_back(field);
    }

    void operator()(const XCDFSignedIntegerField& field) {
      siFields_.push_back(field);
    }

    void operator()(const XCDFFloatingPointField& field) {
      flFields_.push_back(field);
    }


    // NOTE: Operation requires that fields of the same
    // type are in the same order (which they should be).
    // Returns true if not equal
    bool CompareFields(const FileCompare& compare) {
      bool ret = CompareFieldVector(uiFields_, compare.uiFields_) ||
                 CompareFieldVector(siFields_, compare.siFields_) ||
                 CompareFieldVector(flFields_, compare.flFields_);
      return ret;
    }

    // NOTE: Operation requires that fields of the same
    // type are in the same order (which they should be).
    // Returns true if not equal
    bool CompareData(const FileCompare& compare) {
      bool ret = CompareVectorData(uiFields_, compare.uiFields_) ||
                 CompareVectorData(siFields_, compare.siFields_) ||
                 CompareVectorData(flFields_, compare.flFields_);
      return ret;
    }

  private:

    std::vector<XCDFUnsignedIntegerField> uiFields_;
    std::vector<XCDFSignedIntegerField> siFields_;
    std::vector<XCDFFloatingPointField> flFields_;

    template <typename T>
    bool
    CompareFieldVector(const std::vector<XCDFField<T> >& v1,
                       const std::vector<XCDFField<T> >& v2) {
      if (v1.size() != v2.size()) {
        return true;
      }

      for (unsigned i = 0; i < v1.size(); ++i) {
        if (v1[i].GetName().compare(v2[i].GetName())) {
          return true;
        }
        if (v1[i].GetResolution() != v2[i].GetResolution()) {
          return true;
        }
      }

      return false;
    }

    // For integer operations
    template <typename T>
    bool
    CompareVectorData(const std::vector<XCDFField<T> >& v1,
                      const std::vector<XCDFField<T> >& v2) {

      if (v1.size() != v2.size()) {
        return true;
      }

      for (unsigned i = 0; i < v1.size(); ++i) {
        if (v1[i].GetSize() != v2[i].GetSize()) {
          return true;
        }

        for (unsigned j = 0; j < v1[i].GetSize(); ++j) {

          // For integers, the data must be within less than
          // the resolution unit, i.e. division yields zero.
          // Need to cast to signed integer.
          int64_t val = static_cast<int64_t>(v1[i][j] - v2[i][j]);
          if (val / v1[i].GetResolution() > 0) {
            return true;
          }
        }
      }

      return false;
    }

    // Specialization for double operations
    bool
    CompareVectorData(const std::vector<XCDFField<double> >& v1,
                      const std::vector<XCDFField<double> >& v2) {

      if (v1.size() != v2.size()) {
        return true;
      }

      for (unsigned i = 0; i < v1.size(); ++i) {
        if (v1[i].GetSize() != v2[i].GetSize()) {
          return true;
        }
        for (unsigned j = 0; j < v1[i].GetSize(); ++j) {

          // Check that nans and infs are the same
          bool nan1 = std::isnan(v1[i][j]);
          bool nan2 = std::isnan(v2[i][j]);
          bool inf1 = std::isinf(v1[i][j]);
          bool inf2 = std::isinf(v2[i][j]);
          if (nan1 != nan2 || inf1 != inf2) {
            return true;
          }

          // If full-resolution is specified, values must be identical
          if (v1[i].GetResolution() == 0.) {

            // Check that value is identical
            if (v1[i][j] != v2[i][j]) {
              return true;
            }
          } else {

            // Check whether within resolution limits
            if ((v1[i][j] - v2[i][j]) / v1[i].GetResolution() > 1.) {
              return true;
            }
          }
        }
      }

      return false;
    }
};

class FieldCopyBuffer {

  public:

    FieldCopyBuffer(XCDFFile& file) : file_(file) { }
    ~FieldCopyBuffer() { }

    bool HasFieldPair(const std::string& name) const {
      return (uiMap_.find(name) != uiMap_.end() ||
              siMap_.find(name) != siMap_.end() ||
              fpMap_.find(name) != fpMap_.end());
    }

    void SetField(XCDFUnsignedIntegerField field,
                  const std::string& parentName = "") {
      SetFieldImpl(uiMap_, field, parentName);
    }

    void SetField(XCDFSignedIntegerField field,
                  const std::string& parentName = "") {
      SetFieldImpl(siMap_, field, parentName);
    }

    void SetField(XCDFFloatingPointField field,
                  const std::string& parentName = "") {
      SetFieldImpl(fpMap_, field, parentName);
    }

    void CopyData() {

      CopyDataImpl(uiMap_);
      CopyDataImpl(siMap_);
      CopyDataImpl(fpMap_);
    }

  private:

    XCDFFile& file_;

    typedef
    std::pair<XCDFUnsignedIntegerField, XCDFUnsignedIntegerField> UIPair;
    typedef std::pair<XCDFSignedIntegerField, XCDFSignedIntegerField> SIPair;
    typedef std::pair<XCDFFloatingPointField, XCDFFloatingPointField> FPPair;

    std::map<std::string, UIPair> uiMap_;
    std::map<std::string, SIPair> siMap_;
    std::map<std::string, FPPair> fpMap_;

    template <typename T>
    void CopyDataImpl(std::map<std::string,
                               std::pair<XCDFField<T>, XCDFField<T> > >& map) {

      for (typename std::map<std::string,
                      std::pair<XCDFField<T>, XCDFField<T> > >::iterator
                                   currentPair = map.begin();
                                   currentPair != map.end(); ++currentPair) {

        for (typename XCDFField<T>::ConstIterator
                               it = currentPair->second.first.Begin();
                               it != currentPair->second.first.End(); ++it) {
          currentPair->second.second << *it;
        }
      }
    }

    template <typename T>
    void SetFieldImpl(std::map<std::string,
                               std::pair<XCDFField<T>, XCDFField<T> > >& map,
                      XCDFField<T> field, const std::string& parentName) {


      const std::string& name = field.GetName();
      typename std::map<std::string,
                        std::pair<XCDFField<T>, XCDFField<T> > >::iterator
                                                          it = map.find(name);
      if (it == map.end()) {
        XCDFField<T> newField =
            AllocateField(name, field.GetResolution(), parentName);
        map.insert(std::make_pair(field.GetName(),
                                          std::make_pair(field, newField)));
      } else {
        it->second.first = field;
      }
    }

    XCDFUnsignedIntegerField AllocateField(const std::string& name,
                                           const uint64_t resolution,
                                           const std::string& parentName) {
      return file_.AllocateUnsignedIntegerField(name, resolution, parentName);
    }

    XCDFSignedIntegerField AllocateField(const std::string& name,
                                         const int64_t resolution,
                                         const std::string& parentName) {
      return file_.AllocateSignedIntegerField(name, resolution, parentName);
    }

    XCDFFloatingPointField AllocateField(const std::string& name,
                                         const double resolution,
                                         const std::string& parentName) {
      return file_.AllocateFloatingPointField(name, resolution, parentName);
    }
};

class SelectFieldVisitor {
  public:

    SelectFieldVisitor(XCDFFile& oldFile,
                       std::set<std::string>& fieldList,
                       FieldCopyBuffer& buf) : oldFile_(oldFile),
                                               fieldList_(fieldList),
                                               buf_(buf) { }

    template <typename T>
    void operator()(const XCDFField<T>& field) {

      // Skip fields not specified to be copied
      if (fieldList_.find(field.GetName()) == fieldList_.end()) {
        return;
      }

      std::string parentName = "";
      if (oldFile_.IsVectorField(field.GetName())) {
        parentName = oldFile_.GetFieldParentName(field.GetName());

        // Make sure parent field is allocated in new file
        if (fieldList_.find(parentName) == fieldList_.end()) {

          // Oops -- user error
          if (!buf_.HasFieldPair(parentName)) {
            XCDFWarn("Including parent field \"" << parentName <<
                          "\" for field \"" << field.GetName() << "\"");
          }
          buf_.SetField(oldFile_.GetUnsignedIntegerField(parentName));
        }
      }

      buf_.SetField(field, parentName);
    }

  private:

    XCDFFile& oldFile_;
    std::set<std::string>& fieldList_;
    FieldCopyBuffer& buf_;
};

class CSVInputHandler {

  public:

    CSVInputHandler(XCDFFile& f,
                    std::istream& in) : f_(f),
                                        in_(in),
                                        delim_(',') {
      ProcessFieldDefs();
    }

    CSVInputHandler(XCDFFile& f,
                    std::istream& in,
                    char& delim) : f_(f),
                                in_(in),
                                delim_(delim) {
      ProcessFieldDefs();
    }

    bool CopyLine() {

      ParseLine();
      if (!in_.good()) {
        return false;
      }

      if (currentParsedLine_.size() != fieldTypeList_.size()) {
        XCDFFatal("Expected " << fieldTypeList_.size() <<
                             " entries in line " << currentLine_);
      }

      std::vector<XCDFUnsignedIntegerField>::iterator
                   currentUnsignedField = unsignedFields_.begin();
      std::vector<XCDFSignedIntegerField>::iterator
                   currentSignedField = signedFields_.begin();
      std::vector<XCDFFloatingPointField>::iterator
                   currentFloatingPointField = floatingPointFields_.begin();

      unsigned long long uval;
      long long ival;
      double fval;

      unsigned count;
      std::vector<std::string> entries;

      for (unsigned i = 0; i < fieldTypeList_.size(); ++i) {

        count = 0;
        ParseString(currentParsedLine_[i], entries);

        switch(fieldTypeList_[i]) {

          case XCDF_UNSIGNED_INTEGER:

            for (std::vector<std::string>::iterator it = entries.begin();
                                                 it != entries.end(); ++it) {

              count += sscanf(it->c_str(), "%llu", &uval);
              *currentUnsignedField << uval;
            }
            currentUnsignedField++;
            break;

          case XCDF_SIGNED_INTEGER:

            for (std::vector<std::string>::iterator it = entries.begin();
                                                 it != entries.end(); ++it) {

              count += sscanf(it->c_str(), "%lld", &ival);
              *currentSignedField << ival;
            }
            currentSignedField++;
            break;

          case XCDF_FLOATING_POINT:

            for (std::vector<std::string>::iterator it = entries.begin();
                                                 it != entries.end(); ++it) {

              count += sscanf(it->c_str(), "%lg", &fval);
              *currentFloatingPointField << fval;
            }
            currentFloatingPointField++;
            break;

        }

        if (count != entries.size()) {
          XCDFFatal("Bad input string: " << currentParsedLine_[i]);
        }
      }

      return true;
    }

  private:

    XCDFFile& f_;
    std::istream& in_;
    char delim_;

    void ProcessFieldDefs() {

      ParseLine();
      for (std::vector<std::string>::iterator
                          it = currentParsedLine_.begin();
                          it != currentParsedLine_.end(); ++it) {
        AddField(*it);
      }
    }

    void ParseLine() {

      currentParsedLine_.clear();
      std::getline(in_, currentLine_);
      std::stringstream sstream(currentLine_);
      std::string currentString;

      while (std::getline(sstream, currentString, delim_)) {

        currentParsedLine_.push_back(currentString);
      }

      // Check for an empty delimiter
      if (currentLine_.size() > 0) {
        if (currentLine_[currentLine_.size() - 1] == delim_) {
          currentParsedLine_.push_back("");
        }
      }
    }

    void ParseString(const std::string& input,
                     std::vector<std::string>& vec) {

      vec.clear();
      std::stringstream sstream(input);
      std::string currentString;
      while (std::getline(sstream, currentString, ':')) {

        vec.push_back(currentString);
      }
    }

    void AddField(std::string& str) {

      // Trim leading and trailing whitespace
      size_t endPos = str.find_last_not_of(" \n\r\t");
      if(endPos != std::string::npos) {
        str = str.substr(0, endPos+1);
      }

      size_t startPos = str.find_first_not_of(" \n\r\t");
      if(startPos != std::string::npos) {
        str = str.substr(startPos);
      }

      // Parse the string
      size_t slPos = str.find_first_of("/");
      if (str.size() < 3 ||
          slPos == std::string::npos ||
          slPos == str.size() - 1) {
        XCDFFatal("Bad field specifier string: " << str);
      }

      std::string fieldSpecifier = str.substr(0, slPos);
      char fieldType = str[slPos + 1];

      // Check if resolution is specified
      std::string resStr = "";
      slPos = str.find_first_of("/", slPos + 1);
      if (slPos != std::string::npos) {
        resStr = str.substr(slPos + 1, str.size() - slPos);
      } else {
        XCDFFatal("No resolution specified in string: " << str);
      }
      std::stringstream strStream(resStr);

      // Check for parent;
     std::string fieldName, parentName;
     size_t bPos = fieldSpecifier.find_first_of("[");
     if (bPos != std::string::npos) {
       size_t bEnd = fieldSpecifier.find_last_of("]");
       if (bEnd == std::string::npos ||
           bEnd < bPos) {
         XCDFFatal("Bad field specifier string: " << str);
       }
       parentName = fieldSpecifier.substr(bPos + 1, bEnd - bPos - 1);
       fieldName = fieldSpecifier.substr(0, bPos);
     } else {
       fieldName = fieldSpecifier;
       parentName = "";
     }

      switch (fieldType) {
        case 'U': {
          fieldTypeList_.push_back(XCDF_UNSIGNED_INTEGER);
          uint64_t resolution;
          strStream >> resolution;
          unsignedFields_.push_back(f_.AllocateUnsignedIntegerField(
                                        fieldName, resolution, parentName));
          break;
        }

        case 'I': {
          fieldTypeList_.push_back(XCDF_SIGNED_INTEGER);
          int64_t resolution;
          strStream >> resolution;
          signedFields_.push_back(f_.AllocateSignedIntegerField(
                                        fieldName, resolution, parentName));
          break;
        }

        case 'F': {
          fieldTypeList_.push_back(XCDF_FLOATING_POINT);
          double resolution;
          strStream >> resolution;
          floatingPointFields_.push_back(f_.AllocateFloatingPointField(
                                        fieldName, resolution, parentName));
          break;
        }
      }
    }

    std::vector<XCDFUnsignedIntegerField> unsignedFields_;
    std::vector<XCDFSignedIntegerField> signedFields_;
    std::vector<XCDFFloatingPointField> floatingPointFields_;

    std::vector<XCDFFieldType> fieldTypeList_;
    std::vector<std::string> currentParsedLine_;
    std::string currentLine_;
};

class AliasAdder {

  public:

    AliasAdder(const std::string& name,
               const std::string& expression) : name_(name),
                                                expression_(expression) { }

    void Init(XCDFFile& f) {
      // This will fail if the alias already exists.  Since this
      // file is not written, we're not actually modifying the file yet.
      f.CreateAlias(name_, expression_);
      descriptor_ = f.GetAliasDescriptor(name_);
    }

    void Modify(XCDFFileTrailer& trailer) {
      trailer.AddAliasDescriptor(descriptor_);
    }

  private:

    std::string name_;
    std::string expression_;
    XCDFAliasDescriptor descriptor_;
};

class AliasRemover {

  public:

    AliasRemover(const std::string& name) : name_(name) { }

    void Init(XCDFFile& f) { }

    void Modify(XCDFFileTrailer& trailer) {
      try {
        trailer.RemoveAliasDescriptorByName(name_);
      } catch (XCDFException& e) {
        XCDFFatal("Alias " << name_ << " cannot be removed");
      }
    }

  private:

    std::string name_;
};

template <typename Modifier>
void ModifyTrailer(const std::string& infile,
                   Modifier modifier,
                   unsigned minVersion) {

  XCDFFile f;
  f.Open(infile, "r");

  // Don't modify trailer if version is not sufficient
  if (f.GetVersion() < minVersion) {
    XCDFFatal("Unable to modify file in-place: XCDF version < " << minVersion)
  }

  // Don't modify files that are concatenated or are written via streams
  if (!f.IsSimple()) {
    XCDFFatal("Unable to modify file in-place: " << infile <<
              " File is concatenated or written via stream/pipe")
  }

  // Initialize the modifier
  modifier.Init(f);
  f.Close();

  // Get the existing trailer
  XCDFFrame frame;
  XCDFFileHeader header;
  XCDFFileTrailer trailer;
  uint64_t filePos;
  try {
    std::ifstream in(infile.c_str());
    frame.Read(in);
    header.UnpackFrame(frame);
    filePos = header.GetFileTrailerPtr();
    in.seekg(filePos);
    frame.Read(in);
    trailer.UnpackFrame(frame, header.GetVersion());
    in.close();
  } catch (XCDFException& e) {
    XCDFFatal("Unable to open file " << infile);
  } catch (std::istream::failure& e) {
    XCDFFatal("Unable to seek to end of file " << infile);
  }

  // Modify the trailer
  modifier.Modify(trailer);

  // Overwrite the old trailer with the modified one
  try {
    std::fstream out(infile.c_str());
    out.seekp(filePos);
    frame.Clear();
    trailer.PackFrame(frame);
    frame.Write(out, true);
    filePos = out.tellp();
    out.close();
  } catch (std::fstream::failure& e) {
    XCDFFatal("Cannot write trailer to file " << infile);
  }

  // Now we need to truncate
  truncate(infile.c_str(), filePos);
}


#endif // XCDF_UTILITY_UTILITY_INCLUDED_H
