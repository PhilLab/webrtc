#include "pch.h"
#include "SrtpAesCalcTest.h"

//test entry point declaration
extern "C" int srtp_test_aes_calc_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpAesCalcTest);

void libSrtpTests::CSrtpAesCalcTest::Execute()
{
  //TODO: change proper parameters
  char* argv[] = { "." };

  srtp_test_aes_calc_main(1, argv);
}