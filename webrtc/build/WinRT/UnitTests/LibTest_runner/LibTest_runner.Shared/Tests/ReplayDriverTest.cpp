#include "pch.h"
#include "ReplayDriverTest.h"

//test entry point declaration
extern "C" int rdb_main();

void LibTest_runner::CReplayDriverTest::Execute()
{
  rdb_main();
}
