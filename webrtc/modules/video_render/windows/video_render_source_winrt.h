#ifndef WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_
#define WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_

#include <mfidl.h>

#include "webrtc/modules/video_render/include/video_render_defines.h"

namespace webrtc {

class VideoRenderSourceWinRT :
    public IMFMediaSource,
    public IMFGetService,
    public IMFRateControl {
 public:
  VideoRenderSourceWinRT();
  ~VideoRenderSourceWinRT();

  // IMFMediaSource
  IFACEMETHOD(CreatePresentationDescriptor) (
      IMFPresentationDescriptor **ppPresentationDescriptor);
  IFACEMETHOD(GetCharacteristics) (DWORD *pdwCharacteristics);
  IFACEMETHOD(Pause) ();
  IFACEMETHOD(Shutdown) ();
  IFACEMETHOD(Start) (
      IMFPresentationDescriptor *pPresentationDescriptor,
      const GUID *pguidTimeFormat,
      const PROPVARIANT *pvarStartPosition);
  IFACEMETHOD(Stop)();

  // IMFGetService
  IFACEMETHOD(GetService) (
      _In_ REFGUID guidService,
      _In_ REFIID riid,
      _Out_opt_ LPVOID *ppvObject);

  // IMFRateControl
  IFACEMETHOD(SetRate) (BOOL fThin, float flRate);
  IFACEMETHOD(GetRate) (_Inout_opt_ BOOL *pfThin, _Inout_opt_ float *pflRate);

 private:
  int64 _cRef;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_
