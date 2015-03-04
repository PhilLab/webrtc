#include "pch.h"
#include "SrtpAesCalcTest.h"

//test entry point declaration
extern "C" int srtp_test_aes_calc_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpAesCalcTest);

void LibTest_runner::CSrtpAesCalcTest::Execute()
{
  //TODO: change proper parameters
  char* argv[] = { "FF", "AA" };

  //Commented out -> crashes
  //srtp_aes_calc_main(2, argv);
  printf("failed\n");
}