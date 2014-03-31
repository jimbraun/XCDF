
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
