#include "pch.h"
#include "SrtpDatatypesDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_datatypes_driver_main();

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpDatatypesDriverTest);

void LibTest_runner::CSrtpDatatypesDriverTest::Execute()
{
  srtp_test_datatypes_driver_main();
}

