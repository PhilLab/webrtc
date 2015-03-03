#include "pch.h"
#include "ReplayDriverTest.h"

//test entry point declaration
extern "C" int replay_driver_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CReplayDriverTest);

void LibTest_runner::CReplayDriverTest::Execute()
{
  replay_driver_main();
}

