
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

#include <sys/time.h>

int main(int argc, char** argv) {

  XCDFFile f("speedtest.xcd", "w");
  XCDFUnsignedIntegerField field1 =
                  f.AllocateUnsignedIntegerField("field1", 1);
  XCDFUnsignedIntegerField field2 =
                  f.AllocateUnsignedIntegerField("field2", 4);
  XCDFSignedIntegerField field3 =
                  f.AllocateSignedIntegerField("field3", 2);
  XCDFFloatingPointField field4 =
                  f.AllocateFloatingPointField("field4", 0.1);
  XCDFFloatingPointField field5 =
                  f.AllocateFloatingPointField("field5", 0.1, "field1");
  XCDFUnsignedIntegerField field6 =
                  f.AllocateUnsignedIntegerField("field6", 1);


  for (int k = 0; k < 1000001; k++) {
    field1 << 4;
    field2 << k;
    field3 << -1;
    field4 << 101.3;
    field5 << 0.2 << 3.9 << 222.3 << 10840.4;
    field6 << k;
    f.Write();
  }

  f.AddComment("speed test file");

  std::cout << "Writing test file: " << f.GetEventCount()
                                    << " entries." << std::endl;

  f.Close();

  XCDFFile h("speedtest.xcd", "r");

  // Count the entries
  unsigned count = h.GetEventCount();

  std::cout << "Reading file: " << count << " entries." << std::endl;

  h.Rewind();

  h.Read();

  field1 = h.GetUnsignedIntegerField("field1");
  field2 = h.GetUnsignedIntegerField("field2");
  field3 = h.GetSignedIntegerField("field3");
  field4 = h.GetFloatingPointField("field4");
  field5 = h.GetFloatingPointField("field5");
  field6 = h.GetUnsignedIntegerField("field6");

  std::cout << "Field 1: " << *field1 << std::endl;
  std::cout << "Field 2: " << *field2 << std::endl;
  std::cout << "Field 3: " << *field3 << std::endl;
  std::cout << "Field 4: " << *field4 << std::endl;
  std::cout << "Field 5: ";
  for (unsigned k = 0; k < *field1; k++ ) {
    std::cout << field5[k] << " ";
  }
  std::cout << std::endl;
  std::cout << "Field 6: " << *field6 << std::endl;

  std::cout << std::endl << "Reading 1M events!" << std::endl;

  struct timeval tv;
  struct timezone tz;
  gettimeofday(&tv, &tz);
  int secs = tv.tv_sec;
  int usecs = tv.tv_usec;

  for (int k = 0; k < 1000000; k++ ) {
    h.Read();
  }

  gettimeofday(&tv, &tz);
  double io_time  = tv.tv_sec-secs + (tv.tv_usec-usecs)/1000000.;
  double io_speed = 9.*8./io_time;
  std::cout << "Time: " << io_time  << " seconds.  Speed: "
                 << io_speed << " MB/s" << std::endl << std::endl;

  std::cout << "Field 1: " << *field1 << std::endl;
  std::cout << "Field 2: " << *field2 << std::endl;
  std::cout << "Field 3: " << *field3 << std::endl;
  std::cout << "Field 4: " << *field4 << std::endl;
  std::cout << "Field 5: ";
  for (unsigned k = 0; k < *field1; k++ ) {
    std::cout << field5[k] << " ";
  }
  std::cout << std::endl;
  std::cout << "Field 6: " << *field6 << std::endl;

  h.Close();
}
