#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CSrtpKernelDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\srtp_test_kernel_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CSrtpKernelDriverTest :
    public CTestBase
  {
  private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpKernelDriverTest);
  public:
    void Execute();
    virtual ~CSrtpKernelDriverTest() {};
    TEST_NAME_METHOD_IMPL(CSrtpKernelDriverTest);
  };

  typedef std::shared_ptr<CSrtpKernelDriverTest> SpSrtpKernelDriverTest_t;
}

