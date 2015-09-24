#pragma once

namespace LibTest_runner
{
  //forward declaration
  class CTestSolution;
  class CTestBase;

#pragma warning ( push )
#pragma warning( disable : 4290 )
  
  //=============================================================================
  //         class: CTestsReporterBase
  //   Description: Test reporter base class
  // History: 
  // 2015/03/09 TP: created
  //=============================================================================
  class CTestsReporterBase
  {
  public:
    //=======================================================================
    //         Method: Begin
    //    Description: Called when generation of tests report begin
    //                 Implement this method to perform initialization
    //         return: void
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    virtual void Begin() throw(ReportGenerationException) {};
    //=======================================================================
    //         Method: AddTestSolutionHeader
    //    Description: Implement this method to add Test solution related header
    //       Argument: const CTestSolution & solution test solution
    //         return: void
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    virtual void AddTestSolutionHeader(const CTestSolution& solution) throw(ReportGenerationException) {};
    //=======================================================================
    //         Method: AddTestResult
    //    Description: Implement this method to add specified test result.
    //                 Called for all test in solution
    //       Argument: const CTestBase & test specified test
    //         return: void
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    virtual void AddTestResult(const CTestBase& test) throw(ReportGenerationException) = 0;
    //=======================================================================
    //         Method: AddTestSolutionFooter
    //    Description: Implement this method to add Test solution related footer
    //       Argument: const CTestSolution & solution test solution
    //         return: void
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    virtual void AddTestSolutionFooter(const CTestSolution& solution) throw(ReportGenerationException) {};

    //=======================================================================
    //         Method: End
    //    Description: Called when generation of tests report finishes
    //                 Implement this method to perform finalization
    //         return: void
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    virtual void End() throw(ReportGenerationException) {} ;
    virtual ~CTestsReporterBase() {};
  };

  typedef std::shared_ptr<CTestsReporterBase> SpTestReporter_t;
#pragma warning ( pop )
}
