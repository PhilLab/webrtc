#include "pch.h"
#include "SrtpSha1DriverTest.h"

//test entry point declaration
extern "C" int srtp_test_sha1_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpSha1DriverTest);

void libSrtpTests::CSrtpSha1DriverTest::Execute()
{
  srtp_test_sha1_driver_main();
}

