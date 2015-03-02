#include "pch.h"
#include "RocDriverTest.h"

//test entry point declaration
extern "C" int roc_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CRocDriverTest);

namespace LibTest_runner
{
  
  void CRocDriverTest::Execute()
  {
    roc_main();
  }

}