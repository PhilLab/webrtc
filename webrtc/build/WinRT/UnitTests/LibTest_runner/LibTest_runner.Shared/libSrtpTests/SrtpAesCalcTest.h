#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpAesCalcTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_aes_calc.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpAesCalcTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpAesCalcTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpAesCalcTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpAesCalcTest);
  };

  typedef std::shared_ptr<CSrtpAesCalcTest> SpSrtpAesCalcTest_t;
}

