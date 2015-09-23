#include "common.h"


//test entry point declaration
extern "C" int opus_decode_main(int _argc, char **_argv);

namespace OpusTests
{

  AUTO_ADD_TEST_IMPL(COpusDecodeTest);

  int COpusDecodeTest::InterchangeableExecute()
  {
    char* argv[] = { "." };
    return opus_decode_main(1, argv);
  }
}

