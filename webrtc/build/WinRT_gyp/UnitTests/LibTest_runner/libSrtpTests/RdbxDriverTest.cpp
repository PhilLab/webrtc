#include "common.h"
#include "RdbxDriverTest.h"

//test entry point declaration
extern "C" int rdbx_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CRdbxDriverValidationTest);
AUTO_ADD_TEST_IMPL(libSrtpTests::CRdbxDriverTimingTest);

namespace libSrtpTests
{
  //==============================================================
  // CRdbxDriverValidationTest

  int CRdbxDriverValidationTest::InterchangeableExecute()
  {
    char* argv[] = { ".", "-v"};
    return rdbx_driver_main(2, argv);
  }

  //==============================================================
  // CRdbxDriverTimingTest

  int CRdbxDriverTimingTest::InterchangeableExecute()
  {
     char* argv[] = { ".", "-t" };
    return rdbx_driver_main(2, argv);
  }
}