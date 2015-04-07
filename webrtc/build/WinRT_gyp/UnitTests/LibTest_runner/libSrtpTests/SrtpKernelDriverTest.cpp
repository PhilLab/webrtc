#include "common.h"
#include "SrtpKernelDriverTest.h"

//test entry point declaration
extern "C" int srtp_test_kernel_driver_main(int argc, char *argv[]);

AUTO_ADD_TEST_IMPL(libSrtpTests::CSrtpKernelDriverTest);

int libSrtpTests::CSrtpKernelDriverTest::InterchangeableExecute()
{
  //TODO: configuration has to be handled
  char* argv[] = { ".", "-v" };
  return srtp_test_kernel_driver_main(2, argv);
}

