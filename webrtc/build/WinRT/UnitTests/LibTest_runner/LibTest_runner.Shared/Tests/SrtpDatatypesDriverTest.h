#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CSrtpDatatypesDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_datatypes_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpDatatypesDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpDatatypesDriverTest);
  public:
    void Execute();
    virtual ~CSrtpDatatypesDriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpDatatypesDriverTest);
  };

  typedef std::shared_ptr<CSrtpDatatypesDriverTest> SpSrtpDatatypesDriverTest_t;
}

