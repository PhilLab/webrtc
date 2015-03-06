#include "pch.h"
#include "SrtpRandGenTest.h"

//test entry point declaration
extern "C" int srtp_test_rand_gen_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpRandGenTest);

void libSrtpTests::CSrtpRandGenTest::Execute()
{
  //TODO: configuration has to be handled
  char* argv[] = { ".", "10" };
  srtp_test_rand_gen_main(1, argv);
}

