#include "pch.h"

namespace LibTest_runner
{

#pragma warning ( push )
#pragma warning( disable : 4290 )

  //=======================================================================
  //         Method: CWStringReporter
  //    Description: ctor
  //       Argument: int flags - reporter flags
  //         return: 
  //
  //       History:
  // 2015/03/09 TP: created
  //======================================================================
  CWStringReporter::CWStringReporter(int flags)
    : m_nFlags(flags)
  {
  }

  void CWStringReporter::AddTestResult(const CTestBase& test)
  {
    //just simple output for now
    if (test.Executed() || (m_nFlags & kAllTests))
    {
      (*m_spReport) += test.Name() + L": " + (test.Succeed() ? L"Pass" : L"Failed") + L"\n";
    }
  }

  void CWStringReporter::Begin() throw(ReportGenerationException)
  {
    m_spReport.reset(new std::wstring());
  }

#pragma warning ( pop )
}
