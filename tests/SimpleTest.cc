
/*
Copyright (c) 2014, J. Braun
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

#include <xcdf/XCDF.h>

#include <cstdio>
#include <limits>

int main(int argc, char** argv) {

  XCDFFile f("test.xcd", "w");
  XCDFUnsignedIntegerField field1 =
                  f.AllocateUnsignedIntegerField("field1", 1);
  XCDFUnsignedIntegerField field2 =
                  f.AllocateUnsignedIntegerField("field2", 1, "field1");
  XCDFFloatingPointField field3 =
                  f.AllocateFloatingPointField("field3", 0.1);
  XCDFFloatingPointField field4 =
                  f.AllocateFloatingPointField("field4", 0.1);
  XCDFFloatingPointField field5 =
                  f.AllocateFloatingPointField("field5", 0.1);
  XCDFUnsignedIntegerField field6 =
                  f.AllocateUnsignedIntegerField("field6", 1);
  XCDFFloatingPointField field7 =
                  f.AllocateFloatingPointField("field7", 0.);

  field1 << 2;
  field2 << 1 << 1;
  field3 << 0.1;
  field4 << 5.;
  field5 << 5.;
  field6 << 0xDEADBEEFDEADBEEFULL;
  field7 << 0.12;

  std::cout << f.Write() << std::endl;

  field1 << 2;
  field2 << 1 << 3;
  field3 << 0.3;

  // Write a NaN into field4
  field4 << std::numeric_limits<double>::signaling_NaN();

  // Write an inf into field5
  field5 << std::numeric_limits<double>::infinity();

  field6 << 0xDEADBEEFDEADBEEFULL;

  field7 << 0.12;

  std::cout << f.Write() << std::endl;

  for (int k = 0; k < 1000; k++) {
    field1 << 2;
    field2 << 1 << 3;
    field3 << 0.3;
    field4 << std::numeric_limits<double>::signaling_NaN();
    field5 << std::numeric_limits<double>::infinity();
    field6 << 0xDEADBEEFDEADBEEFULL;
    field7 << 0.12;
    f.Write();
  }

  f.AddComment("test file");

  f.Close();

  f.Open("test.xcd", "r");

  for (std::vector<std::string>::const_iterator it = f.CommentsBegin();
                                                it != f.CommentsEnd(); ++it) {

    std::cout << *it << std::endl;
  }

  field1 = f.GetUnsignedIntegerField("field1");
  field2 = f.GetUnsignedIntegerField("field2");
  field3 = f.GetFloatingPointField("field3");
  field4 = f.GetFloatingPointField("field4");
  field5 = f.GetFloatingPointField("field5");
  field6 = f.GetUnsignedIntegerField("field6");
  field7 = f.GetFloatingPointField("field7");

  std::cout << f.Read() << std::endl;

  std::cout << *field1 << std::endl;

  for (XCDFUnsignedIntegerField::ConstIterator it = field2.Begin();
                                           it != field2.End(); ++it) {
    std::cout << *it << " ";
  }

  std::cout << std::endl;
  std::cout << *field3 << std::endl << std::endl;
  std::cout << *field4 << std::endl;
  std::cout << *field5 << std::endl;
  std::cout << std::hex << *field6 << std::endl;
  std::cout << *field7 << std::endl;

  std::cout << f.Read() << std::endl;

  std::cout << *field1 << std::endl;

  for (XCDFUnsignedIntegerField::ConstIterator it = field2.Begin();
                                           it != field2.End(); ++it) {
    std::cout << *it << " ";
  }

  std::cout << std::endl;
  std::cout << *field3 << std::endl << std::endl;
  std::cout << *field4 << std::endl;
  std::cout << *field5 << std::endl;
  std::cout << std::hex << *field6 << std::endl;
  std::cout << *field7 << std::endl;

  for (int k = 0; k < 1000; k++) {
    f.Read();
  }

  std::cout << f.Read() << std::endl;
  f.Close();

  return 0;
}
