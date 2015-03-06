#include "pch.h"
#include "SrtpDriverTest.h"

//test entry point declaration
extern "C" int srtp_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpDriverTest);

void libSrtpTests::CSrtpDriverTest::Execute()
{
  //TODO: configuration has to be handled
  char* argv[] = { ".", "-t" };
  srtp_driver_main(2, argv);
}

