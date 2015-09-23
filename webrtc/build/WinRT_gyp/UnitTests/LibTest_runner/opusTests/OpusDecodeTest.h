#pragma once

namespace OpusTests
{
  using namespace LibTest_runner;
  //=============================================================================
  //         class: COpusDecodeTest
  //   Description: class executes opus decode test project, 
  //                see chromium\src\third_party\opus\test_opus_decode.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class COpusDecodeTest :
    public COpusTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, COpusDecodeTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~COpusDecodeTest() {};
    TEST_NAME_IMPL(OpusDecodeTest);
    TEST_PROJECT_IMPL(test_opus_decode);
    TEST_LIBRARY_IMPL(Opus);
  };

  typedef std::shared_ptr<COpusDecodeTest> SpOpusDecodeTest_t;
}

