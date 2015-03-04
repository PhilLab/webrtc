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
    virtual const std::string& Name() = 0;
  };

  typedef std::shared_ptr<CTestBase> SpTestBase_t;

  // This marco simplifies implementation of CTestBase::Name()
  #define TEST_NAME_METHOD_IMPL(CLAZZ) \
    virtual const std::string& LibTest_runner::CLAZZ::Name() \
    { \
      static std::string name = #CLAZZ; \
      return name; \
    }
}

