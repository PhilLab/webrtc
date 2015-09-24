#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpKernelDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_kernel_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpKernelDriverTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpKernelDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpKernelDriverTest() {};
    TEST_NAME_IMPL(CSrtpKernelDriverTest);
    TEST_PROJECT_IMPL(srtp_test_kernel_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpKernelDriverTest> SpSrtpKernelDriverTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_SRTPKERNELDRIVERTEST_H_
