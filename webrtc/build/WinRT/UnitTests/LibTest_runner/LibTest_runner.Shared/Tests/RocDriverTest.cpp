#include "pch.h"
#include "RocDriverTest.h"

//test entry point declaration
extern "C" int roc_main();

namespace LibTest_runner
{
  
  void CRocDriverTest::Execute()
  {
    roc_main();
  }

}