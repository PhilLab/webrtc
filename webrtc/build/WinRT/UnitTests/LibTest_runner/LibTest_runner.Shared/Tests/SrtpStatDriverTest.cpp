#include "pch.h"
#include "SrtpStatDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_stat_driver_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpStatDriverTest);

void LibTest_runner::CSrtpStatDriverTest::Execute()
{
  srtp_test_stat_driver_main();
}

