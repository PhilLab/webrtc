#include "pch.h"
#include "SrtpCipherDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_cipher_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpCipherDriverTest);

int libSrtpTests::CSrtpCipherDriverTest::InterchangeableExecute()
{
  //TODO: change proper parameters
  char* argv[] = { ".", "-t" };

  return srtp_test_cipher_driver_main(2, argv);
}

