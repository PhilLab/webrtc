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
    public CLibSrtpTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRocDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRocDriverTest() {};
    TEST_NAME_IMPL(RocDriverTest);
    TEST_PROJECT_IMPL(roc_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CRocDriverTest> SpRocDriverTest_t;
}

