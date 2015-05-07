#pragma once

namespace OpusTests
{
  using namespace LibTest_runner;
  //=============================================================================
  //         class: COpusApiTest
  //   Description: class executes opus api test project, 
  //                see chromium\src\third_party\opus\test_opus_api.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class COpusApiTest :
    public COpusTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, COpusApiTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~COpusApiTest() {};
    TEST_NAME_IMPL(OpusApiTest);
    TEST_PROJECT_IMPL(test_opus_api);
    TEST_LIBRARY_IMPL(Opus);
  };

  typedef std::shared_ptr<COpusApiTest> SpOpusApiTest_t;
}

