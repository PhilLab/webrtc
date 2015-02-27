#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CRocDriverTest
  //   Description: class executes roc_driver test project, 
  //                see chromium\src\third_party\libsrtp\roc_driver.vcxproj
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CRocDriverTest :
    public CTestBase
  {
  public:
    void Execute();
    virtual ~CRocDriverTest() {};
  };

  typedef std::shared_ptr<CRocDriverTest> SpRocDriverTest_t;
}

