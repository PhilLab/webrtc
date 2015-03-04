#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CSrtpDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpDriverTest);
  public:
    void Execute();
    virtual ~CSrtpDriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpDriverTest);
  };

  typedef std::shared_ptr<CSrtpDriverTest> SpSrtpDriverTest_t;
}

