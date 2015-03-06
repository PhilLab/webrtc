#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: CTestSolution
  //   Description: Class represents test solution
  //   !!! Class is not thread safe !!!
  // History: 
  // 2015/02/27 TP: created
  //=============================================================================
  class CTestSolution
  {
  private:
    // hide copy ctor
    CTestSolution(const CTestSolution&);

    //members
    typedef std::list<SpTestBase_t> TestCollection_t;
    typedef std::shared_ptr<TestCollection_t> SpTestCollection_t;

    SpTestCollection_t m_spTests;
    void InternalTestExecute(const SpTestBase_t& pTest);
  public:
    CTestSolution() {}
    void Execute() throw();
    void Execute(const char* testName);
    void AddTest(const SpTestBase_t& ptrTest);
    size_t GetTestCount() const;
    //=======================================================================
    //         Method: IsEmpty
    //    Description: Checks whether test suit is empty
    //         return: true if empty
    //
    //       History:
    // 2015/02/27 TP: created
    //======================================================================
    bool IsEmpty() const 
    {
      return (m_spTests == NULL) || m_spTests->empty(); 
    }
  };
};
