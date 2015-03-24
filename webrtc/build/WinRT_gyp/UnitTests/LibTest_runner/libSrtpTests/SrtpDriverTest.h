#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpDriverTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpDriverTest() {};
    TEST_NAME_IMPL(CSrtpDriverTest);
    TEST_PROJECT_IMPL(srtp_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpDriverTest> SpSrtpDriverTest_t;
}

