#include "pch.h"
#include "SrtpSha1DriverTest.h"

//test entry point declaration
extern "C" int srtp_test_sha1_driver_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpSha1DriverTest);

void LibTest_runner::CSrtpSha1DriverTest::Execute()
{
  srtp_test_sha1_driver_main();
}

