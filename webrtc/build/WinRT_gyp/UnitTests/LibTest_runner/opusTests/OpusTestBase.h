#pragma once

namespace OpusTests
{
  using namespace LibTest_runner;

  //=============================================================================
  //         class: COpusTestBase
  //   Description: Base class for Opus tests
  // History: 
  // 2015/03/10 TP: created
  //=============================================================================
  class COpusTestBase : public CTestBase
  {
  public:
    virtual ~COpusTestBase() {};
    virtual void InterchangeableVerifyResult();
  };
}

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_OPUSTESTS_OPUSTESTBASE_H_
