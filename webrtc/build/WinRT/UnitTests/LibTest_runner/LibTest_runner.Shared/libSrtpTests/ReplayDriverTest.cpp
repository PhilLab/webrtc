#include "pch.h"
#include "ReplayDriverTest.h"

//test entry point declaration
extern "C" int replay_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CReplayDriverTest);

void libSrtpTests::CReplayDriverTest::Execute()
{
  replay_driver_main();
}

