#include "pch.h"
#include "RdbxDriverTest.h"

//test entry point declaration
extern "C" int rdbx_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CRdbxDriverTest);

namespace libSrtpTests
{
  void CRdbxDriverTest::Execute()
  {
    //TODO: rdbx support two parameters -t and -v
    char* argv[] = { ".", "-v"};
    rdbx_driver_main(2, argv);
  }
}