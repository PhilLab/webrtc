#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CRocDriverTest
  //   Description: class executes roc_driver test project, 
  //                see chromium\src\third_party\libsrtp\roc_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRocDriverTest :
    public CTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRocDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRocDriverTest() {};
    TEST_NAME_METHOD_IMPL(CRocDriverTest);
  };

  typedef std::shared_ptr<CRocDriverTest> SpRocDriverTest_t;
}

