#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CReplayDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\replay_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CReplayDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CReplayDriverTest);
  public:
    void Execute();
    virtual ~CReplayDriverTest() {};
    TEST_NAME_METHOD_IMPL(CReplayDriverTest);
  };

  typedef std::shared_ptr<CReplayDriverTest> SpReplayDriverTest_t;
}

