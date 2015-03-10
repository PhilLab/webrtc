#pragma once

namespace LibTest_runner
{
#pragma warning ( push )
#pragma warning( disable : 4290 )

  //=============================================================================
  //         class: CWStringReporter
  //   Description: Simple string reporter
  // History: 
  // 2015/03/09 TP: created
  //=============================================================================
  class CWStringReporter : public CTestsReporterBase
  {
  public:
    static const int kAllTests = 0x0001; //include all tests, otherwise only executed
    static const int kPrintOutput = 0x0002; //print test output
  private:
    int m_nFlags; //flags
    std::shared_ptr<std::wstring> m_spReport;
  public:
    CWStringReporter(int flags = 0);
    virtual ~CWStringReporter() {};
    virtual void AddTestResult(const CTestBase& test) throw(ReportGenerationException);
    virtual void Begin() throw(ReportGenerationException);
    //=======================================================================
    //         Method: GetReport
    //    Description: Gets generated report
    //         return: std::shared_ptr<std::wstring> pointer to generated report, 
    //                 might be null
    //
    //       History:
    // 2015/03/09 TP: created
    //======================================================================
    std::shared_ptr<std::wstring> GetReport() const { return m_spReport; }
  };

  typedef std::shared_ptr<CWStringReporter> SpWStringReporter_t;

#pragma  warning( pop )
}


