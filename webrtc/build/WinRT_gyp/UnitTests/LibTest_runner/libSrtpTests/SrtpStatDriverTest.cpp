#include "common.h"
#include "SrtpStatDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_stat_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpStatDriverTest);

int libSrtpTests::CSrtpStatDriverTest::InterchangeableExecute()
{
  return srtp_test_stat_driver_main();
}

