#pragma once

namespace libSrtpTests
{
  //=============================================================================
  //         class: CRtpwTest
  //   Description: class executes rtpw test project, 
  //                see chromium\src\third_party\libsrtp\rtpw.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRtpwTest :
    public CLibSrtpTestBase
  {
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CRtpwTest);
  protected:
    int InterchangeableExecute();
  public:
    virtual ~CRtpwTest() {};
    TEST_NAME_IMPL(CRtpwTest);
    TEST_PROJECT_IMPL(rtpw);
    TEST_LIBRARY_IMPL(libSrtp);
  };
}

