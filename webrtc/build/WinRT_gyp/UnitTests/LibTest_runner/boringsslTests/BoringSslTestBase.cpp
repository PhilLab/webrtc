#include "common.h"

namespace BoringSSLTests
{
  void CBoringSSLTestBase::InterchangeableVerifyResult()
  {
    static const wchar_t* kPassKyeword = L"PASS";
    SetSucceed(m_wsOutput.find(kPassKyeword) != std::wstring::npos);
  }

}

