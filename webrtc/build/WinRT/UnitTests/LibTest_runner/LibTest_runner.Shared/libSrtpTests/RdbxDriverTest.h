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
    TEST_NAME_METHOD_IMPL(CRdbxDriverTest);
  };

  typedef std::shared_ptr<CRdbxDriverTest> SpRdbxDriverTest_t;
}