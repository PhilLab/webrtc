#include "common.h"


//test entry point declaration
extern "C" int opus_encode_main(int _argc, char **_argv);

namespace OpusTests
{

  AUTO_ADD_TEST_IMPL(COpusEncodeTest);

  int COpusEncodeTest::InterchangeableExecute()
  {
    char* argv[] = { "." };
    return opus_encode_main(1, argv);
  }
}

