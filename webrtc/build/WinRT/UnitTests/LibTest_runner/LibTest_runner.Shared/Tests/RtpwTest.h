#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CRtpwTest
  //   Description: class executes rtpw test project, 
  //                see chromium\src\third_party\libsrtp\rtpw.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRtpwTest :
    public CTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRtpwTest);
  public:
    virtual ~CRtpwTest() {};
    void Execute();
  };
}

