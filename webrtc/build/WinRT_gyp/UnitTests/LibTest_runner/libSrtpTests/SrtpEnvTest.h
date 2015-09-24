#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpEnvTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_env.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpEnvTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpEnvTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpEnvTest() {};
    TEST_NAME_IMPL(SrtpEnvTest);
    TEST_PROJECT_IMPL(srtp_test_env);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpEnvTest> SpSrtpEnvTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_SRTPENVTEST_H_
