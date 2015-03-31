#include "common.h"
#include "SrtpCipherDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_cipher_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpCipherDriverValidationTest);
AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpCipherDriverTimingTest);
AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpCipherDriverArrayTimingTest);

namespace libSrtpTests
{

  //=================================================================
  // CSrtpCipherDriverValidationTest

  int CSrtpCipherDriverValidationTest::InterchangeableExecute()
  {
    char* argv[] = { ".", "-v" };

    return srtp_test_cipher_driver_main(2, argv);
  }

  //=================================================================
  // CSrtpCipherDriverTimingTest
  int CSrtpCipherDriverTimingTest::InterchangeableExecute()
  {
    char* argv[] = { ".", "-t" };

    return srtp_test_cipher_driver_main(2, argv);
  }

  void CSrtpCipherDriverTimingTest::InterchangeableVerifyResult()
  {
    //Right now there is not "success" check. Timing test just measures
    //number of operation per second. Right now we have no requirements.
  }

  //=================================================================
  // CSrtpCipherDriverArrayTimingTest
  int CSrtpCipherDriverArrayTimingTest::InterchangeableExecute()
  {
    char* argv[] = { ".", "-a" };

    return srtp_test_cipher_driver_main(2, argv);
  }

  void CSrtpCipherDriverArrayTimingTest::InterchangeableVerifyResult()
  {
    //Right now there is not "success" check. Timing test just measures
    //number of operation per second. Right now we have no requirements.
  }
}

