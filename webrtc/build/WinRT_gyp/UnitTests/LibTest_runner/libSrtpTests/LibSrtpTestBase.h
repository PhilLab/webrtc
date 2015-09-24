#pragma once

namespace libSrtpTests
{
  using namespace LibTest_runner;

  //=============================================================================
  //         class: CLibSrtpTestBase
  //   Description: Base class for LibSrtp test
  // History: 
  // 2015/03/10 TP: created
  //=============================================================================
  class CLibSrtpTestBase : public CTestBase
  {
  public:
    virtual ~CLibSrtpTestBase() {};
    virtual void InterchangeablePrepareForExecution();
  };
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_LIBSRTPTESTS_LIBSRTPTESTBASE_H_
