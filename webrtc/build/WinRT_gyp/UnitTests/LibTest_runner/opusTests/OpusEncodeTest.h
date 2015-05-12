#pragma once

namespace OpusTests
{
  using namespace LibTest_runner;
  //=============================================================================
  //         class: COpusEncodeTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\opus\test_opus_encode.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class COpusEncodeTest :
    public COpusTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, COpusEncodeTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~COpusEncodeTest() {};
    TEST_NAME_IMPL(OpusEncodeTest);
    TEST_PROJECT_IMPL(test_opus_encode);
    TEST_LIBRARY_IMPL(Opus);
  };

  typedef std::shared_ptr<COpusEncodeTest> SpOpusEncodeTest_t;
}

