#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CRdbxDriverTest
  //   Description: class executes roc_driver test project, 
  //                see chromium\src\third_party\libsrtp\Rdbx_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRdbxDriverTest :
    public CTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRdbxDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRdbxDriverTest() {};
    TEST_NAME_IMPL(CRdbxDriverTest);
    TEST_PROJECT_IMPL(Rdbx_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CRdbxDriverTest> SpRdbxDriverTest_t;
}