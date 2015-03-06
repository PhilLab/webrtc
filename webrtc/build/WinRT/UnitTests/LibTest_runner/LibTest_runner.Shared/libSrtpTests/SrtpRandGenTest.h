#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpRandGenTest :
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_rand_gen.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpRandGenTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpRandGenTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpRandGenTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpRandGenTest);
  };

  typedef std::shared_ptr<CSrtpRandGenTest> SpSrtpRandGenTest_t;
}

