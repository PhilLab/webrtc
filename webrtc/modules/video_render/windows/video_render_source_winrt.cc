// Own include file
#include "webrtc/modules/video_render/windows/video_render_source_winrt.h"

// System include files
#include <windows.h>

// WebRtc include files
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/event_wrapper.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
#include "webrtc/system_wrappers/interface/trace.h"

namespace webrtc {

VideoRenderSourceWinRT::VideoRenderSourceWinRT(void)
    : _cRef(1)
{
}

VideoRenderSourceWinRT::~VideoRenderSourceWinRT(void)
{
}

// IMFMediaSource methods
IFACEMETHODIMP VideoRenderSourceWinRT::CreatePresentationDescriptor(
    IMFPresentationDescriptor **ppPresentationDescriptor)
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::GetCharacteristics(DWORD *pdwCharacteristics)
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::Pause()
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::Shutdown()
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::Start(
    IMFPresentationDescriptor *pPresentationDescriptor,
    const GUID *pguidTimeFormat,
    const PROPVARIANT *pvarStartPos)
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::Stop()
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::GetService(_In_ REFGUID guidService, _In_ REFIID riid, _Out_opt_ LPVOID *ppvObject)
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::SetRate(BOOL fThin, float flRate)
{
  return S_OK;
}

IFACEMETHODIMP VideoRenderSourceWinRT::GetRate(_Inout_opt_ BOOL *pfThin, _Inout_opt_ float *pflRate)
{
  return S_OK;
}

}  // namespace webrtc
