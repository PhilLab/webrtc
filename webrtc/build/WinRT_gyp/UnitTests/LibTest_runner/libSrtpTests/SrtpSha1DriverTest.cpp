#include "common.h"
#include "SrtpSha1DriverTest.h"

//test entry point declaration
extern "C" int srtp_test_sha1_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpSha1DriverTest);

int libSrtpTests::CSrtpSha1DriverTest::InterchangeableExecute()
{
  return srtp_test_sha1_driver_main();
}

