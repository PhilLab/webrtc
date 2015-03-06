#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_cipher_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpCipherDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpCipherDriverTest);
  public:
    void Execute();
    virtual ~CSrtpCipherDriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpCipherDriverTest);
  };

  typedef std::shared_ptr<CSrtpCipherDriverTest> SpSrtpCipherDriverTest_t;
}

