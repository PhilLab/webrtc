#include "common.h"
#include "ReplayDriverTest.h"

//test entry point declaration
extern "C" int replay_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CReplayDriverTest);

int libSrtpTests::CReplayDriverTest::InterchangeableExecute()
{
  return replay_driver_main();
}

