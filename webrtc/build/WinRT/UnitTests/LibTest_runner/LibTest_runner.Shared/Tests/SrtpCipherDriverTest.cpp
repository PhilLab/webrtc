#include "pch.h"
#include "SrtpCipherDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_cipher_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpCipherDriverTest);

void LibTest_runner::CSrtpCipherDriverTest::Execute()
{
  //TODO: change proper parameters
  char* argv[] = { "AA" };

  //Commented out -> crashes
  //cipher_driver_main(1, argv);
  printf("failed\n");
}

