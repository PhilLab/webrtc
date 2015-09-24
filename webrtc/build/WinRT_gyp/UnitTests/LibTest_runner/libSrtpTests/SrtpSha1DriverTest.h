#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpSha1DriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_sha1_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpSha1DriverTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpSha1DriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpSha1DriverTest() {};
    TEST_NAME_IMPL(SrtpSha1DriverTest);
    TEST_PROJECT_IMPL(srtp_test_sha1_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpSha1DriverTest> SpSrtpSha1DriverTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_SRTPSHA1DRIVERTEST_H_
