#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CReplayDriverTest
  //   Description: class executes replay_driver test project, 
  //                see chromium\src\third_party\libsrtp\replay_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CReplayDriverTest :
    public CTestBase
  {
  public:
    void Execute();
    virtual ~CReplayDriverTest() {};
  };

  typedef std::shared_ptr<CReplayDriverTest> SpReplayDriverTest_t;
}

