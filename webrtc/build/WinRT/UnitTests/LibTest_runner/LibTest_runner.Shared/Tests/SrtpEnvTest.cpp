#include "pch.h"
#include "SrtpEnvTest.h"

//test entry point declaration
extern "C" int srtp_test_env_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpEnvTest);

void LibTest_runner::CSrtpEnvTest::Execute()
{
  srtp_test_env_main();
}

