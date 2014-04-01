
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

int main(int argc, char** argv) {

  std::cout << "Creating file 1" << std::endl;

  XCDFFile f1("appendtest.xcd", "a");
  XCDFUnsignedIntegerField field1 =
      f1.AllocateUnsignedIntegerField("field1", 1);
  field1 << 121212;
  f1.Write();
  f1.Close();

  std::cout << "Creating file 2" << std::endl;

  XCDFFile f2("appendtest.xcd", "a");
  field1 = f2.AllocateUnsignedIntegerField("field1", 1);

  for (int k = 0; k < 999; k++) {
    field1 << k;
    f2.Write();
  }
  f2.Close();

  std::cout << "Creating file 3" << std::endl;

  XCDFFile f3("appendtest.xcd", "a");
  field1 = f3.AllocateUnsignedIntegerField("field1", 1);

  for (int k = 999; k < 1000; k++) {
    field1 << k;
    f3.Write();
  }
  f3.Close();

  std::cout << "Creating file 4" << std::endl;

  XCDFFile f4("appendtest.xcd", "a");
  field1 = f4.AllocateUnsignedIntegerField("field1", 1);

  for (int k = 1000; k < 1010; k++) {
    field1 << k;
    f4.Write();
  }
  f4.Close();

  std::cout << "Creating file 5" << std::endl;

  XCDFFile f5("appendtest.xcd", "a");
  field1 = f5.AllocateUnsignedIntegerField("field1", 1);
  f5.Close();
}
