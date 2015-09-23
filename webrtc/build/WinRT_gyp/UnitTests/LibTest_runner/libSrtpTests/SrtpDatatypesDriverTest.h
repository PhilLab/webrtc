#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpDatatypesDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_datatypes_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpDatatypesDriverTest :
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpDatatypesDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpDatatypesDriverTest() {};
    TEST_NAME_IMPL(SrtpDatatypesDriverTest);
    TEST_PROJECT_IMPL(srtp_test_datatypes_driver);
    TEST_LIBRARY_IMPL(libSrtp);

    virtual void InterchangeableVerifyResult();

  };

  typedef std::shared_ptr<CSrtpDatatypesDriverTest> SpSrtpDatatypesDriverTest_t;
}

