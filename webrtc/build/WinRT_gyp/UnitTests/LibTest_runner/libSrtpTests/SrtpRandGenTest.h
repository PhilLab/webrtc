#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpRandGenTest :
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_rand_gen.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpRandGenTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpRandGenTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpRandGenTest() {};
    TEST_NAME_IMPL(SrtpRandGenTest);
    TEST_PROJECT_IMPL(srtp_test_rand_gen);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpRandGenTest> SpSrtpRandGenTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_SRTPRANDGENTEST_H_
