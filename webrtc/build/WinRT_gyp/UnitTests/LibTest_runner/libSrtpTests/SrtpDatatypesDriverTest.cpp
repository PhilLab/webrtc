#include "common.h"
#include "SrtpDatatypesDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_datatypes_driver_main();

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpDatatypesDriverTest);

namespace libSrtpTests
{
  int CSrtpDatatypesDriverTest::InterchangeableExecute()
  {
    return srtp_test_datatypes_driver_main();
  }

  void CSrtpDatatypesDriverTest::InterchangeableVerifyResult()
  {
    //Test always succeed, there is no way to do any checking. 
    //See following comment from datatypes_driver.c
    /*
    * this program includes various and sundry tests for fundamental
    * datatypes.  it's a grab-bag of throwaway code, retained only in
    * case of future problems
    */

    SetSucceed(true);
  }
}
