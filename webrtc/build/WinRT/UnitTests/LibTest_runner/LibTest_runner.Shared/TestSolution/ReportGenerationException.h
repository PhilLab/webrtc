#pragma once

namespace LibTest_runner
{
  //=============================================================================
  //         class: ReportGenerationException
  //   Description: thrown when errors appears during generation tests report
  // History: 
  // 2015/03/09 TP: created
  //=============================================================================
  class ReportGenerationException : public std::exception
  {
  public:
    ReportGenerationException(const char * const &message)
      : std::exception(message) {};
  };
}