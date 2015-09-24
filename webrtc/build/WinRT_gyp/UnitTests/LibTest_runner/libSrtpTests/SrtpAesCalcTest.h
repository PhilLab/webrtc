#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpAesCalcTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_aes_calc.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpAesCalcTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpAesCalcTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpAesCalcTest() {};
    TEST_NAME_IMPL(SrtpAesCalcTest);
    TEST_PROJECT_IMPL(srtp_test_aes_calc);
    TEST_LIBRARY_IMPL(libSrtp)

      virtual void InterchangeableVerifyResult();

  };

  typedef std::shared_ptr<CSrtpAesCalcTest> SpSrtpAesCalcTest_t;
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_SRTPAESCALCTEST_H_
