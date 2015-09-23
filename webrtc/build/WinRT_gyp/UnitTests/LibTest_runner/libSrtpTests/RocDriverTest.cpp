#include "common.h"
#include "RocDriverTest.h"

//test entry point declaration
extern "C" int roc_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CRocDriverTest);

namespace libSrtpTests
{
  int CRocDriverTest::InterchangeableExecute()
  {
    return roc_driver_main();
  }
}