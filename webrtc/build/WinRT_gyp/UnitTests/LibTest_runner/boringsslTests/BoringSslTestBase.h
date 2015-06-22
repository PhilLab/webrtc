#pragma once

namespace BoringSSLTests
{
  using namespace LibTest_runner;
  //=============================================================================
  //         class: CBoringSSLTestBase
  //   Description: Test base class for BoringSSL
  
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CBoringSSLTestBase :
    public CTestBase
  {
    virtual void InterchangeableVerifyResult();
  };
}

