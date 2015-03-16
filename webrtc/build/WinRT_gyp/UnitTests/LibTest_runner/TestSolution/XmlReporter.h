#pragma once

namespace LibTest_runner
{
#pragma warning ( push )
#pragma warning( disable : 4290 )
  //=============================================================================
  //         class: CXmlReporter
  //   Description: Test reporter generating xml file
  //              
  // History: 
  // 2015/03/09 TP: created
  //=============================================================================
  class CXmlReporter : public CTestsReporterBase
  {
  public:
    CXmlReporter();
    virtual ~CXmlReporter();

    virtual void AddTestResult(const CTestBase& test) throw(ReportGenerationException);

  };

#pragma warning ( pop )
}

