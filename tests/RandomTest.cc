
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
#include <vector>

int main(int argc, char** argv) {

  std::vector<uint64_t> field1Vector;
  std::vector<uint64_t> field2Vector;
  std::vector<int64_t> field3Vector;
  std::vector<double> field4Vector;
  std::vector<double> field5Vector;

  XCDFUnsignedIntegerField field1;
  XCDFUnsignedIntegerField field2;
  XCDFSignedIntegerField field3;
  XCDFFloatingPointField field4;
  XCDFFloatingPointField field5;

  for (int k = 0; k < 25000; ++k) {

    printf("Iteration %d\n", k);

    XCDFFile f;
    if (k == 0) {
      f.Open("randomtest.xcd", "w");
    } else {
      f.Open("randomtest.xcd", "a");
    }
    field1 = f.AllocateUnsignedIntegerField("field1", 1);
    field2 = f.AllocateUnsignedIntegerField("field2", 4);
    field3 = f.AllocateSignedIntegerField("field3", 2);
    field4 = f.AllocateFloatingPointField("field4", 0.01);
    field5 = f.AllocateFloatingPointField("field5", 0.1, "field1");

    field1Vector.push_back(rand() % 10);
    field1 << field1Vector.back();

    int64_t randNum = rand() % 10000;
    field2Vector.push_back((randNum / 4) * 4);
    field2 << randNum;

    randNum = rand() % 100000;
    field3Vector.push_back(-50000 + (randNum / 2) * 2);
    field3 << -50000 + randNum;

    double randDouble = 1000000.*rand()/(RAND_MAX + 1.);
    field4Vector.push_back(static_cast<int64_t>(
                            floor((randDouble+0.005)*100))/100.);
    field4 << randDouble;

    for (unsigned j = 0; j < field1Vector.back(); ++j) {
      randDouble = rand()/(RAND_MAX + 1.);
      field5Vector.push_back(
            static_cast<int64_t>(floor((randDouble+0.05)*10))/10.);
      field5 << randDouble;
    }

    f.Write();

    if (rand() % 1000 == 0) {
      f.StartNewBlock();
    }
    f.Close();
  }

  XCDFFile h("randomtest.xcd", "r");

  // Count the entries
  unsigned count = h.GetEventCount();

  std::cout << "Reading file: " << count << " entries." << std::endl;

  h.Rewind();

  field1 = h.GetUnsignedIntegerField("field1");
  field2 = h.GetUnsignedIntegerField("field2");
  field3 = h.GetSignedIntegerField("field3");
  field4 = h.GetFloatingPointField("field4");
  field5 = h.GetFloatingPointField("field5");

  unsigned vcnt = 0;
  for (int k = 0; k < 25000; ++k) {

    if (!h.Read()) {
      std::cerr << "Read failed.  Entries: " << k << std::endl;
      exit(1);
    }

    if (*field1 != field1Vector[k]) {
      std::cerr << "Field1: Expected: " << field1Vector[k] << " Got: "
                           << *field1 << ".  Entries: " << k << std::endl;
      exit(1);
    }

    if (*field2 != field2Vector[k]) {
      std::cerr << "Field2: Expected: " << field2Vector[k] << " Got: "
                           << *field2 << ".  Entries: " << k << std::endl;
      exit(1);
    }

    if (*field3 != field3Vector[k]) {
      std::cerr << "Field3: Expected: " << field3Vector[k] << " Got: "
                           << *field3 << ".  Entries: " << k << std::endl;
      exit(1);
    }

    if (fabs(*field4 - field4Vector[k]) > 0.0001) {
      std::cerr << "Field4: Expected: " << field4Vector[k] << " Got: "
                           << *field4 << ".  Entries: " << k << std::endl;
      exit(1);
    }

    for (unsigned j = 0; j < *field1; ++j) {
      if (fabs(field5[j] - field5Vector[vcnt++]) > 0.01) {
        std::cerr << "Field5: Expected: " << field5Vector[vcnt-1] << " Got: "
          << field5[j] << ".  Entries: " << k << " Index: " << j << std::endl;
        exit(1);
      }
    }
  }

  h.Close();

  std::cout << "Success!" << std::endl;
}
