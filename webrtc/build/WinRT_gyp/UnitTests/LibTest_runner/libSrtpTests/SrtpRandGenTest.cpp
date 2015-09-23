#include "common.h"
#include "SrtpRandGenTest.h"

//test entry point declaration
extern "C" int srtp_test_rand_gen_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpRandGenTest);

int libSrtpTests::CSrtpRandGenTest::InterchangeableExecute()
{
  //TODO: configuration has to be handled
  char* argv[] = { ".", "10" };
  return srtp_test_rand_gen_main(2, argv);
}

