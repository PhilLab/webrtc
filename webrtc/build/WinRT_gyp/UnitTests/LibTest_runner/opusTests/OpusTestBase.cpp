#include "common.h"

void OpusTests::COpusTestBase::InterchangeableVerifyResult()
{
  static const wchar_t kErrorString[] = L"A fatal error was detected";
  if (m_wsOutput.find(kErrorString) != std::wstring::npos)
  {
    SetSucceed(false);
    m_wsResultMessage.assign(L"See test output for details.");
  }
}