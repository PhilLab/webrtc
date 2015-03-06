#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpStatDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_stat_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpStatDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpStatDriverTest);
  public:
    void Execute();
    virtual ~CSrtpStatDriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpStatDriverTest);
  };

  typedef std::shared_ptr<CSrtpStatDriverTest> SpSrtpStatDriverTest_t;
}

