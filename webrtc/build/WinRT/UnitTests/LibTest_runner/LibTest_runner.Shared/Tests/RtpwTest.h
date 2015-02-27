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
  public:
    virtual ~CRtpwTest() {};
    void Execute();
  };
}

