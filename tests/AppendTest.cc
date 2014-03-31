
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
