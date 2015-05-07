#include "common.h"


//test entry point declaration
extern "C" int opus_api_main(int _argc, char **_argv);

namespace OpusTests
{

  AUTO_ADD_TEST_IMPL(COpusApiTest);

  int COpusApiTest::InterchangeableExecute()
  {
    char* argv[] = { "." };
    return opus_api_main(1, argv);
  }
}

