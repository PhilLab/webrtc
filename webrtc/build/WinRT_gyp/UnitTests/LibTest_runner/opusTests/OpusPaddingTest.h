#pragma once

namespace OpusTests
{
  using namespace LibTest_runner;
  //=============================================================================
  //         class: COpusDecodeTest
  //   Description: class executes opus padding test project, 
  //                see chromium\src\third_party\opus\test_opus_padding.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class COpusPaddingTest :
    public COpusTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, COpusPaddingTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~COpusPaddingTest() {};
    TEST_NAME_IMPL(OpusPaddingTest);
    TEST_PROJECT_IMPL(test_opus_api);
    TEST_LIBRARY_IMPL(Opus);
  };

  typedef std::shared_ptr<COpusPaddingTest> SpOpusPaddingTest_t;
}

