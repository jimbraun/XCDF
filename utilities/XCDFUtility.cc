
#include <xcdf/utility/XCDFUtility.h>
#include <xcdf/utility/EventSelectExpression.h>
#include <xcdf/XCDFDefs.h>
#include <xcdf/config.h>

#include <set>

void Info(std::vector<std::string>& infiles) {

  XCDFFile f;
  if (infiles.size() == 0) {
    //read from stdin
    f.Open(std::cin);
  } else {
    f.Open(infiles[0], "r");
  }

  unsigned maxNameWidth = 0;
  unsigned maxParentWidth = 0;

  for (std::vector<XCDFFieldDescriptor>::const_iterator
                         it = f.FieldDescriptorsBegin();
                         it != f.FieldDescriptorsEnd(); ++it) {

    if ((it->name_).size() > maxNameWidth) {
      maxNameWidth = (it->name_).size();
    }

    if ((it->parentName_).size() > maxParentWidth) {
      maxParentWidth = (it->parentName_).size();
    }
  }

  if (maxNameWidth < 8) {
    maxNameWidth = 8;
  }

  maxParentWidth++;
  if (maxParentWidth < 8) {
    maxParentWidth = 8;
  }

  std::cout << std::endl;

  std::cout << std::setw(maxNameWidth) << "Field" << std::setw(17) <<
             "Type" << " " << std::setw(11) << "Resolution" <<
                 std::setw(maxParentWidth) << "Parent" << std::endl;

  std::cout << std::setw(maxNameWidth) << "-----" << std::setw(17) <<
               "----" << " " << std::setw(11) << "----------" <<
                   std::setw(maxParentWidth) << "------" << std::endl;

  for (std::vector<XCDFFieldDescriptor>::const_iterator
                         it = f.FieldDescriptorsBegin();
                         it != f.FieldDescriptorsEnd(); ++it) {

    std::cout << std::setw(maxNameWidth) << it->name_;

    switch (it->type_) {

      case XCDF_UNSIGNED_INTEGER:
        std::cout << std::setw(17) << "Unsigned Integer" << 
                            " " << std::setw(11) << it->rawResolution_;
        break;
      case XCDF_SIGNED_INTEGER:
        std::cout << std::setw(17) << "Signed Integer" << " " <<
                     std::setw(11) <<
                       XCDFSafeTypePun<uint64_t, int64_t>(it->rawResolution_);
        break;
      case XCDF_FLOATING_POINT:
        std::cout << std::setw(17) << "Floating Point" << " " << 
                     std::setw(11) <<
                       XCDFSafeTypePun<uint64_t, double>(it->rawResolution_);
        break;

    }

    std::cout << std::setw(maxParentWidth) << it->parentName_ << std::endl;
  }

  std::cout << std::endl << "Entries: " << f.GetEventCount() << std::endl;

  std::cout << std::endl << "Comments:" <<
               std::endl << "---------" << std::endl;

  for (std::vector<std::string>::const_iterator it = f.CommentsBegin();
                                                it != f.CommentsEnd(); ++it) {
    std::cout << *it << std::endl;
  }
}

void Dump(std::vector<std::string>& infiles) {

  uint64_t count = 0;
  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    while (f.Read()) {

      std::cout << "Event: " << count << std::endl;
      count++;
      std::cout << "------ " << std::endl;

      // Print out data from each field
      DumpFieldVisitor dumpFieldVisitor;
      f.ApplyFieldVisitor(dumpFieldVisitor);
      std::cout << std::endl;
    }

    std::cout << std::endl << "Comments:" <<
                 std::endl << "---------" << std::endl;

    for (std::vector<std::string>::const_iterator
                                  it = f.CommentsBegin();
                                  it != f.CommentsEnd(); ++it) {
      std::cout << *it << std::endl;
    }

    f.Close();
  }
}

void CSV(std::vector<std::string>& infiles) {

  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    if (i == 0) {
      PrintFieldNameVisitor printFieldNameVisitor(f);
      f.ApplyFieldVisitor(printFieldNameVisitor);
      std::cout << std::endl;
    }

    PrintFieldDataVisitor printFieldDataVisitor;
    while (f.Read()) {

      printFieldDataVisitor.Reset();
      f.ApplyFieldVisitor(printFieldDataVisitor);
      std::cout << std::endl;
    }

    f.Close();
  }
}

void Count(std::vector<std::string>& infiles) {

  uint64_t count = 0;
  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    count += f.GetEventCount();
    f.Close();
  }

  std::cout << count << std::endl;
}

void Check(std::vector<std::string>& infiles) {

  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    // Allow internal checksum verification to detect errors
    while (f.Read()) { /* Do nothing */ }
    f.Close();
  }
}

std::set<std::string> ParseFields(std::string& exp) {

  std::set<std::string> fields;

  char* expPtr = const_cast<char*>(exp.c_str());
  for (char* tok = strtok(expPtr, ","); tok != NULL;
                                             tok = strtok(NULL, ",")) {

    std::string str(tok);

    // Trim leading and trailing whitespace
    size_t endPos = str.find_last_not_of(" \n\r\t");
    if(endPos != std::string::npos) {
      str = str.substr(0, endPos+1);
    }

    size_t startPos = str.find_first_not_of(" \n\r\t");
    if(startPos != std::string::npos) {
      str = str.substr(startPos);
    }

    fields.insert(str);
  }
  return fields;
}

void CopyComments(XCDFFile& destination,
                  XCDFFile& source) {

  for (std::vector<std::string>::const_iterator
                             it = source.CommentsBegin();
                             it != source.CommentsEnd(); ++it) {

    destination.AddComment(*it);
  }
}

void SelectFields(std::vector<std::string>& infiles,
                  std::ostream& out,
                  std::string& exp,
                  std::string& concatArgs) {

  XCDFFile outFile(out);
  outFile.AddComment(concatArgs);
  std::set<std::string> fields = ParseFields(exp);

  FieldCopyBuffer buf(outFile);

  // Spin through the files and copy the data
  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    // Check that the file contains all the fields
    for (std::set<std::string>::iterator it = fields.begin();
                                         it != fields.end(); ++it) {
      if (!f.HasField(*it)) {
        XCDFFatal("Unable to select field \"" <<
                               *it << "\": Field not present");
      }
    }

    SelectFieldVisitor selectFieldVisitor(f, fields, buf);
    f.ApplyFieldVisitor(selectFieldVisitor);

    while (f.Read()) {

      // Copy the data
      buf.CopyData();
      outFile.Write();
    }

    CopyComments(outFile, f);
    f.Close();
  }

  outFile.Close();
}

void Select(std::vector<std::string>& infiles,
            std::ostream& out,
            std::string& exp,
            std::string& concatArgs) {

  XCDFFile outFile(out);
  outFile.AddComment(concatArgs);

  FieldCopyBuffer buf(outFile);

  // Spin through the files and copy the data
  XCDFFile f;
  for (unsigned i = 0; i <= infiles.size(); ++i) {

    if (i == infiles.size()) {
      if (infiles.size() == 0) {
        //read from stdin
        f.Open(std::cin);
      } else {
        continue;
      }
    } else {
      f.Open(infiles[i], "r");
    }

    // Get the names of all the fields
    std::set<std::string> fields;
    GetFieldNamesVisitor getFieldNamesVisitor(fields);
    f.ApplyFieldVisitor(getFieldNamesVisitor);

    // Load the fields into the buffer for copying
    SelectFieldVisitor selectFieldVisitor(f, fields, buf);
    f.ApplyFieldVisitor(selectFieldVisitor);

    EventSelectExpression expression(exp, f);

    while (f.Read()) {

      // Check the expression; copy the data if true
      if (expression.SelectEvent()) {
        buf.CopyData();
        outFile.Write();
      }
    }

    CopyComments(outFile, f);
    f.Close();
  }

  outFile.Close();
}

void Compare(const std::string& fileName1,
             const std::string& fileName2) {

  try {

    XCDFFile file1(fileName1.c_str(), "r");
    XCDFFile file2(fileName2.c_str(), "r");

    // load file comparison classes
    FileCompare compare1, compare2;
    file1.ApplyFieldVisitor(compare1);
    file2.ApplyFieldVisitor(compare2);

    // check field types, resolutions
    if (compare1.CompareFields(compare2)) {
      std::cout << "Files have fields with differing type or resolution\n";
      return;
    }

    // check that event counts are the same
    if (file1.GetEventCount() != file2.GetEventCount()) {
      std::cout << "Files have differing numbers of events\n";
      return;
    }

    // check event data
    uint64_t max = file1.GetEventCount();
    for (uint64_t i = 0; i < max; ++i) {
      file1.Read();
      file2.Read();
      if (compare1.CompareData(compare2)) {
        std::cout << "Event: " << i <<
                       ": Files have fields with differing data\n";
        return;
      }
    }
  } catch (XCDFException& e) {
    std::cout << "An error ocurred reading one of the files. Quitting"
                                                           << std::endl;
  }
}

void Recover(std::vector<std::string>& infiles,
             std::ostream& out) {

  XCDFFile f;
  if (infiles.size() == 0) {
    f.Open(std::cin);
  } else if (infiles.size() == 1) {
    f.Open(infiles[0], "r");
  } else {
    std::cerr << "Only one input file is allowed for recover. Quitting"
                                                            << std::endl;
    exit(1);
  }

  XCDFFile outFile(out);

  try {

    // Get the names of all the fields
    std::set<std::string> fields;
    GetFieldNamesVisitor getFieldNamesVisitor(fields);
    f.ApplyFieldVisitor(getFieldNamesVisitor);

    // Load the fields into the buffer for copying
    FieldCopyBuffer buf(outFile);
    SelectFieldVisitor selectFieldVisitor(f, fields, buf);
    f.ApplyFieldVisitor(selectFieldVisitor);

    CopyComments(outFile, f);
    while (f.Read()) {

      // Copy the data if true
      buf.CopyData();
      outFile.Write();
    }
  } catch (XCDFException& e) {
    std::cerr << "Corrupt file: Recovered " << outFile.GetEventCount()
                                               << " events." << std::endl;
  }

  try {
    CopyComments(outFile, f);
    f.Close();
  } catch (XCDFException& e) { }
  outFile.Close();
}

void RemoveComments(std::vector<std::string>& infiles,
                    std::ostream& out) {

  XCDFFile f;
  if (infiles.size() == 0) {
    f.Open(std::cin);
  } else if (infiles.size() == 1) {
    f.Open(infiles[0], "r");
  } else {
    std::cerr << "Only one input file is allowed for remove-comments."
                                              " Quitting" << std::endl;
    exit(1);
  }

  XCDFFile outFile(out);

  // Get the names of all the fields
  std::set<std::string> fields;
  GetFieldNamesVisitor getFieldNamesVisitor(fields);
  f.ApplyFieldVisitor(getFieldNamesVisitor);

  // Load the fields into the buffer for copying
  FieldCopyBuffer buf(outFile);
  SelectFieldVisitor selectFieldVisitor(f, fields, buf);
  f.ApplyFieldVisitor(selectFieldVisitor);

  while (f.Read()) {

    // Copy the data if true
    buf.CopyData();
    outFile.Write();
  }
  outFile.Close();
}

void AddComment(std::vector<std::string>& infiles,
                std::ostream& out,
                const std::string& comment) {

  XCDFFile f;
  if (infiles.size() == 0) {
    f.Open(std::cin);
  } else if (infiles.size() == 1) {
    f.Open(infiles[0], "r");
  } else {
    std::cerr << "Only one input file is allowed for add-comment."
                                              " Quitting" << std::endl;
    exit(1);
  }

  XCDFFile outFile(out);
  CopyComments(outFile, f);
  outFile.AddComment(comment);

  // Get the names of all the fields
  std::set<std::string> fields;
  GetFieldNamesVisitor getFieldNamesVisitor(fields);
  f.ApplyFieldVisitor(getFieldNamesVisitor);

  // Load the fields into the buffer for copying
  FieldCopyBuffer buf(outFile);
  SelectFieldVisitor selectFieldVisitor(f, fields, buf);
  f.ApplyFieldVisitor(selectFieldVisitor);

  while (f.Read()) {

    // Copy the data if true
    buf.CopyData();
    outFile.Write();
  }
  outFile.Close();
}

void Paste(std::vector<std::string>& infiles,
           std::ostream& out,
           std::string& copyFile,
           std::string& concatArgs) {

  XCDFFile outFile(out);
  outFile.AddComment(concatArgs);

  FieldCopyBuffer buf(outFile);

  // Do we have an existing XCDF file to paste to?
  XCDFFile f;
  if (copyFile.compare("")) {

    // OK, prepare to copy it
    f.Open(copyFile, "r");

    // Get the names of all the fields
    std::set<std::string> fields;
    GetFieldNamesVisitor getFieldNamesVisitor(fields);
    f.ApplyFieldVisitor(getFieldNamesVisitor);

    // Load the fields into the buffer for copying
    SelectFieldVisitor selectFieldVisitor(f, fields, buf);
    f.ApplyFieldVisitor(selectFieldVisitor);
  }

  // Open the CSV input
  std::ifstream fileStream;
  std::istream* currentInputStream = &fileStream;

  if (infiles.size() == 0) {
    //read from stdin
    currentInputStream = &std::cin;
  } else {
    fileStream.open(infiles[0].c_str());
  }

  // Allocate the new fields
  CSVInputHandler csvIn(outFile, *currentInputStream);

  while(csvIn.CopyLine()) {
    if (f.IsOpen()) {
      if (!f.Read()) {
        XCDFWarn("Input file " << infiles[0] <<
                    " has fewer entries than text file.  Truncating.");
        break;
      }
      buf.CopyData();
    }

    outFile.Write();
  }

  if (fileStream.is_open()) {
    fileStream.close();
  }

  if (f.IsOpen()) {
    if (f.Read()) {
      XCDFWarn("Input text file has fewer entries than " <<
                                             copyFile << " Truncating.");
    }
    f.Close();
  }
  outFile.Close();
}

void PrintVersion() {

  std::cout << "\n XCDF version "
            << XCDF_MAJOR_VERSION << "."
            << XCDF_MINOR_VERSION << "."
            << XCDF_PATCH_VERSION << "\n"
            << std::endl;

}

void PrintUsage() {

  std::cout << "\n" <<

    "Usage: xcdf-utility [verb] {infiles}\n\n" <<

    "    verb:    Description\n" <<
    "    ----     -----------\n\n"

    "    version  Print XCDF version information and exit.\n\n" <<

    "    info     Print descriptions of each field in the file.\n\n" <<

    "    dump     Output data event-by-event in a human-readable format.\n\n" <<

    "    count    Count the number of events in the file.\n\n" <<

    "    csv      Output data into comma-separated-value format.\n\n" <<

    "    check    Check if input is a valid XCDF file and check internal\n" <<
    "             data checksums\n\n" <<

    "    select-fields \"field1, field2, ...\" {-o outfile} {infiles}:\n\n" <<

    "                    Copy the given fields and write the result to a\n" <<
    "                    new XCDF file at the path specified by\n" <<
    "                    {-o outfile}, or stdout if outfile is unspecified.\n\n" <<

    "    select \"boolean expression\" {-o outfile} {infiles}:\n\n" <<

    "                    Copy events satisfying the given boolean\n" <<
    "                    expression into a new XCDF file at the path\n" <<
    "                    specified by {-o outfile}, or stdout if outfile\n" <<
    "                    is unspecified. The expression is of the form\n" <<
    "                    e.g.: \"field1 == 0\" to select all events\n" <<
    "                    where the value of field1 is zero.  The variable\n" <<
    "                    \"currentEventNumber\" refers to the current\n" <<
    "                    event in the file.\n\n" <<

    "    paste {-c existingfile} {-o outfile} {infile}:\n\n" <<

    "                    Copy events in CSV format from infile (or stdin,\n" <<
    "                    if unspecified) into outfile (or stdout if unspecified).\n" <<
    "                    If an existing XCDF file is specified with -c, the\n" <<
    "                    fields are added to the existing file.\n\n" <<

    "    recover {-o outfile} {infiles} Recover a corrupt XCDF file.\n\n" <<

    "    add-comment \"comment\" {-o outfile} {infiles} Add comment to an XCDF file\n\n" <<

    "    remove-comments {-o outfile} {infiles} Remove all comments from an XCDF file\n\n" <<

    "    compare file1 file2 Compare the contents of file1 and file2\n\n";

  std::cout << "\n\n";
  std::cout <<
    "  Note: if input/output file(s) are not specified, they are\n" <<
    "  read/written from/to stdin/stdout.\n\n" <<
    "  Multiple input files are allowed.\n";
}

int main(int argc, char** argv) {

  if (argc < 2) {
    PrintUsage();
    exit(1);
  }

  std::string concatArgs = "Arguments: ";
  for (int i = 0; i < argc; ++i) {
    concatArgs += argv[i];
    concatArgs += " ";
  }

  const std::string verb(argv[1]);

  std::string exp = "";
  std::ostream* outstream = &std::cout;
  std::ofstream fout;
  std::vector<std::string> infiles;
  std::string copyFile = "";
  int currentArg = 2;

  if (!verb.compare("recover")) {

    if (currentArg < argc) {

      std::string out(argv[currentArg]);
      if (!out.compare("-o")) {

        if (++currentArg == argc) {
          PrintUsage();
          exit(1);
        }

        fout.open(argv[currentArg++]);
        outstream = &fout;
      }
    }
  }

  if (!verb.compare("select") ||
      !verb.compare("select-fields") ||
      !verb.compare("add-comment")) {

    if (argc < 3) {
      PrintUsage();
      exit(1);
    }
    exp = std::string(argv[currentArg++]);

    if (currentArg < argc) {

      std::string out(argv[currentArg]);
      if (!out.compare("-o")) {

        if (++currentArg == argc) {
          PrintUsage();
          exit(1);
        }

        fout.open(argv[currentArg++]);
        outstream = &fout;
      }
    }
  }

  if (!verb.compare("paste")) {

    if (currentArg < argc) {

      std::string out(argv[currentArg]);
      if (!out.compare("-c")) {

        if (++currentArg == argc) {
          PrintUsage();
          exit(1);
        }

        copyFile = std::string(argv[currentArg++]);
        outstream = &fout;
      }
    }

    if (currentArg < argc) {

      std::string out(argv[currentArg]);
      if (!out.compare("-o")) {

        if (++currentArg == argc) {
          PrintUsage();
          exit(1);
        }

        fout.open(argv[currentArg++]);
        outstream = &fout;
      }
    }
  }

  while (currentArg < argc) {
    infiles.push_back(std::string(argv[currentArg++]));
  }

  if (!verb.compare("info")) {
    Info(infiles);
  }

  else if (!verb.compare("dump")) {
    Dump(infiles);
  }

  else if (!verb.compare("recover")) {
    Recover(infiles, *outstream);
  }

  else if (!verb.compare("count")) {
    Count(infiles);
  }

  else if (!verb.compare("csv")) {
    CSV(infiles);
  }

  else if (!verb.compare("check")) {
    Check(infiles);
  }

  else if (!verb.compare("remove-comments")) {
    RemoveComments(infiles, *outstream);
  }

  else if (!verb.compare("add-comment")) {
    AddComment(infiles, *outstream, exp);
  }

  else if (!verb.compare("select-fields")) {
    SelectFields(infiles, *outstream, exp, concatArgs);
  }

  else if (!verb.compare("select")) {
    Select(infiles, *outstream, exp, concatArgs);
  }

  else if (!verb.compare("paste")) {
    if (infiles.size() > 1) {
      PrintUsage();
      exit(0);
    }
    Paste(infiles, *outstream, copyFile, concatArgs);
  }

  else if (!verb.compare("version")) {
    PrintVersion();
  }

  else if (!verb.compare("compare")) {
    if (infiles.size() != 2) {
      PrintUsage();
      exit(1);
    }
    Compare(infiles[0], infiles[1]);
  }

  else {
    PrintUsage();
    exit(1);
  }

  return 0;
}
