#include "pch.h"
#include "SrtpDatatypesDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_datatypes_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpDatatypesDriverTest);

void libSrtpTests::CSrtpDatatypesDriverTest::Execute()
{
  srtp_test_datatypes_driver_main();
}

