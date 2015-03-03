#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CSrtpEnvTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_env.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpEnvTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpEnvTest);
  public:
    void Execute();
    virtual ~CSrtpEnvTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpEnvTest);
  };

  typedef std::shared_ptr<CSrtpEnvTest> SpSrtpEnvTest_t;
}

