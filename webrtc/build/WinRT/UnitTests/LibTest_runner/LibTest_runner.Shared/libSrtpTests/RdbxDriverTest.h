#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CRdbxDriverValidTest
  //   Description: Rdbx_driver validation test. Parameter -v 
  //                see chromium\src\third_party\libsrtp\Rdbx_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRdbxDriverValidationTest :
    public CTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRdbxDriverValidationTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRdbxDriverValidationTest() {};
    TEST_NAME_IMPL(RdbxDriverValidationTest);
    TEST_PROJECT_IMPL(Rdbx_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  //=============================================================================
  //         class: CRdbxDriverTimingTest
  //   Description: Rdbx_driver timing test. Parameter -t 
  //                see chromium\src\third_party\libsrtp\Rdbx_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRdbxDriverTimingTest :
    public CTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRdbxDriverTimingTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRdbxDriverTimingTest() {};
    TEST_NAME_IMPL(RdbxDriverTimingTest);
    TEST_PROJECT_IMPL(Rdbx_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };
}

