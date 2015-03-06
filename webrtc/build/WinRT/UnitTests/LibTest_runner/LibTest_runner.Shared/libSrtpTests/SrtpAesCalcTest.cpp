#include "pch.h"
#include "SrtpAesCalcTest.h"

//test entry point declaration
extern "C" int srtp_test_aes_calc_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpAesCalcTest);

int libSrtpTests::CSrtpAesCalcTest::InterchangeableExecute()
{
  //TODO: change proper parameters
  char* argv[] = { "." };

  return srtp_test_aes_calc_main(1, argv);
}