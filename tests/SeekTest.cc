
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

  XCDFFile f("seektest.xcd", "w");
  XCDFUnsignedIntegerField field1 =
                      f.AllocateUnsignedIntegerField("field1", 1);

  for (int k = 0; k < 5001; k++) {
    field1 << k;
    f.Write();
  }

  f.AddComment("seek test file");
  f.AddVersionComment();

  std::cout << "Writing test file: " << f.GetEventCount()
                                    << " entries." << std::endl;

  f.Close();

  XCDFFile h("seektest.xcd", "r");

  field1 = h.GetUnsignedIntegerField("field1");

  // Count the entries
  unsigned count = h.GetEventCount();

  std::cout << "Reading file: " << count << " entries." << std::endl;

  h.Rewind();

  std::cout << "Seeking to entry 2003. Data should be 2003." << std::endl;
  std::cout << "  Seek: " << h.Seek(2003) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  std::cout << "Seeking to entry 0. Data should be 0." << std::endl;
  std::cout << "  Seek: " << h.Seek(0) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  std::cout << "Seeking to entry 3999. Data should be 3999." << std::endl;
  std::cout << "  Seek: " << h.Seek(3999) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  h.Close();
}
