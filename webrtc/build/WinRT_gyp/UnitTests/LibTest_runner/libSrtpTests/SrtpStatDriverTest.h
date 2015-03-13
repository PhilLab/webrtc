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
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpStatDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpStatDriverTest() {};
    TEST_NAME_IMPL(SrtpStatDriverTest);
    TEST_PROJECT_IMPL(srtp_test_stat_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpStatDriverTest> SpSrtpStatDriverTest_t;
}

