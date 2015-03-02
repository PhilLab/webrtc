#include "pch.h"
#include "ReplayDriverTest.h"

//test entry point declaration
extern "C" int rdb_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CReplayDriverTest);

void LibTest_runner::CReplayDriverTest::Execute()
{
  rdb_main();
}
