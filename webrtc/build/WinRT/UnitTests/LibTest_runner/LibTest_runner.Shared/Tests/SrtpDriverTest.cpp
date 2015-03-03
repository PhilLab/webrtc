#include "pch.h"
#include "SrtpDriverTest.h"

//test entry point declaration
extern "C" int srtp_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpDriverTest);

void LibTest_runner::CSrtpDriverTest::Execute()
{
  //TODO: configuration has to be handled
  char* argv[] = { " " };
  //crashes
  //srtp_driver_main(0, NULL);
  printf("failed\n");
}

