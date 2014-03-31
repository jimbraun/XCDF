
#include <xcdf/XCDF.h>

#include <cstdio>

int main(int argc, char** argv) {

  XCDFFile f("seektest.xcd", "w");
  XCDFUnsignedIntegerField field1 =
                f.AllocateUnsignedIntegerField("field1", 1);

  for (int k = 0; k < 3001; k++) {
    field1 << k;
    f.Write();
  }

  f.AddComment("seek test file");

  std::cout << "Writing test file: " << f.GetEventCount()
                                    << " entries." << std::endl;

  f.Close();

  // Read the file and copy it;
  char data[20000];
  FILE* inf = fopen("seektest.xcd", "r");
  unsigned ndata = fread(data, 1, 20000, inf);
  fclose(inf);

  FILE* outf = fopen("seektest2x.xcd", "w");

  std::cout << "Read " << ndata << " bytes." << std::endl;

  fwrite(data, ndata, 1, outf);
  fwrite(data, ndata, 1, outf);
  fclose(outf);

  std::cout << "Finished copying" << std::endl;

  XCDFFile h("seektest2x.xcd", "r");

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

  std::cout << "Seeking to entry 3999. Data should be 998." << std::endl;
  std::cout << "  Seek: " << h.Seek(3999) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  std::cout << "Seeking to entry 6000. Data should be 2999." << std::endl;
  std::cout << "  Seek: " << h.Seek(6000) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  std::cout << "Seeking to entry 50. Data should be 50." << std::endl;
  std::cout << "  Seek: " << h.Seek(50) << std::endl;
  std::cout << "  Value: " << *field1 << std::endl;

  h.Close();
}
