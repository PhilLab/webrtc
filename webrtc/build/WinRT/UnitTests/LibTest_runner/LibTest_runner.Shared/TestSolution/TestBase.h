#pragma once

namespace LibTest_runner
{

  //=============================================================================
  //         class: CTestBase
  //   Description: Class provides basic test functionality
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CTestBase
  {
  private:
    //hide copy ctor
    CTestBase(const CTestBase&);
  public:
    CTestBase();
    virtual ~CTestBase();
    virtual void Execute() = 0;
  };

  typedef std::shared_ptr<CTestBase> SpTestBase_t;

}

