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
    public CLibSrtpTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpCipherDriverTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CSrtpCipherDriverTest() {};
    TEST_NAME_IMPL(CSrtpCipherDriverTest);
    TEST_PROJECT_IMPL(srtp_test_cipher_driver);
    TEST_LIBRARY_IMPL(libSrtp);
  };

  typedef std::shared_ptr<CSrtpCipherDriverTest> SpSrtpCipherDriverTest_t;
}

