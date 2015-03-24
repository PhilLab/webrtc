#include "pch.h"
#include "XmlReporter.h"

namespace LibTest_runner
{
#pragma warning ( push )
#pragma warning( disable : 4290 )

  CXmlReporter::CXmlReporter()
  {
  }


  CXmlReporter::~CXmlReporter()
  {
  }

  void CXmlReporter::AddTestResult(const CTestBase& test) throw(ReportGenerationException)
  {
    //#TODO
    throw std::exception("The method or operation is not implemented.");
  }

#pragma warning ( pop )
}
