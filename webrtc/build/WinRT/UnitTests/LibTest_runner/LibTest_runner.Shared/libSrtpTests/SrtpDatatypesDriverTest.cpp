#include "pch.h"
#include "SrtpDatatypesDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_datatypes_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpDatatypesDriverTest);

int libSrtpTests::CSrtpDatatypesDriverTest::InterchangeableExecute()
{
  return srtp_test_datatypes_driver_main();
}

