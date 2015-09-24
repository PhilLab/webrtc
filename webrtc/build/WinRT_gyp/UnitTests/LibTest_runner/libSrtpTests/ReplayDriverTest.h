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
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CReplayDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CReplayDriverTest() {};
    TEST_NAME_IMPL(ReplayDriverTest);
    TEST_PROJECT_IMPL(replay_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CReplayDriverTest> SpReplayDriverTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_REPLAYDRIVERTEST_H_
