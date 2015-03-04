#include "pch.h"
#include "SrtpKernelDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_kernel_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(LibTest_runner::CSrtpKernelDriverTest);

void LibTest_runner::CSrtpKernelDriverTest::Execute()
{
  //TODO: configuration has to be handled
  char* argv[] = { ".", "-v" };
  srtp_test_kernel_driver_main(2, argv);
}

