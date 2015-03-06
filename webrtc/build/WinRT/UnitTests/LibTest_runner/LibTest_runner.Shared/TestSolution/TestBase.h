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
    void PrepareForExecution();
    void VerifyResult();
  protected:
    std::wstring   m_wsOutput; // test output
    //TODO: following members should be private and modified by methods !!!
    int            m_nExitStatus; //test exit code (from main method)
    bool           m_bSucceed; //true if test succeed
    bool           m_Executed; //true if test was executed
    virtual int    InterchangeableExecute() = 0;
    //=======================================================================
    //         Method: InterchangeablePrepareForExecution
    //    Description: Implement this method to do special test preparations
    //         return: void
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    virtual void InterchangeablePrepareForExecution() {};
    //=======================================================================
    //         Method: InterchangeableTestCleanup
    //    Description: Implement this method to do special test cleanup
    //         return: void
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    virtual void InterchangeableTestCleanup() {};

    //=======================================================================
    //         Method: InterchangeableVerifyResult
    //    Description: Implement this method to do special result verification
    //                 Don't forget to set m_bSucceed
    //         return: void
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    virtual void InterchangeableVerifyResult() {};

    //=======================================================================
    //         Method: OutputBufferSize
    //    Description: Returns Output buffer size
    //                 Overwrites this method if needed different value 
    //         return: size_t Output buffer size
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    virtual size_t OutputBufferSize() const { return 1024 * 1024; /*1MB*/ };
  public:
    CTestBase();
    virtual ~CTestBase() {};
    int Execute();
    //=======================================================================
    //         Method: Name
    //    Description: Returns test name
    //         return: const std::wstring& test name
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    virtual const std::wstring& Name() = 0;
    //=======================================================================
    //         Method: Output
    //    Description: Returns test output (from standard output)
    //         return: const std::wstring& console output
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    const std::wstring& Output() const { return m_wsOutput; }
    //=======================================================================
    //         Method: ExitStatus
    //    Description: Returns test exist status
    //         return: int exit status
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    int ExitStatus() const { return m_nExitStatus;  }
    //=======================================================================
    //         Method: Succeed
    //    Description: Returns "success" state
    //         return: bool true if success 
    //
    //       History:
    // 2015/03/06 TP: created
    //======================================================================
    bool Succeed() const { return m_bSucceed;  }
    void Reset();
  };

  typedef std::shared_ptr<CTestBase> SpTestBase_t;

  // This marco simplifies implementation of CTestBase::Name()
#define TEST_NAME_METHOD_IMPL(CLAZZ) \
    virtual const std::wstring& CLAZZ::Name() \
      { \
      static std::wstring name = L#CLAZZ; \
      return name; \
    }
}

