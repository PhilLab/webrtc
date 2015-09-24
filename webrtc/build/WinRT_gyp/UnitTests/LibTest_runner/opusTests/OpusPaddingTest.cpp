#include "common.h"


//test entry point declaration
extern "C" int opus_padding_main();

namespace OpusTests
{

  AUTO_ADD_TEST_IMPL(COpusPaddingTest);

  int COpusPaddingTest::InterchangeableExecute()
  {
    return opus_padding_main();
  }
}

