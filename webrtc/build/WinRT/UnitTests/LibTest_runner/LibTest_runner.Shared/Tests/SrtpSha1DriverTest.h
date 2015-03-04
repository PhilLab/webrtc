#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CSrtpSha1DriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_sha1_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpSha1DriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpSha1DriverTest);
  public:
    void Execute();
    virtual ~CSrtpSha1DriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpSha1DriverTest);
  };

  typedef std::shared_ptr<CSrtpSha1DriverTest> SpSrtpSha1DriverTest_t;
}

