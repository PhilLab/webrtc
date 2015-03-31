#include "common.h"
#include "SrtpEnvTest.h"

//test entry point declaration
extern "C" int srtp_test_env_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpEnvTest);

int libSrtpTests::CSrtpEnvTest::InterchangeableExecute()
{
  return srtp_test_env_main();
}

