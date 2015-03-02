#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_

#include <mfidl.h>

#include <windows.media.h>

#include <wrl\implements.h>

using namespace ABI::Windows::Media;
using namespace Microsoft::WRL;

namespace webrtc {
namespace videocapturemodule {

class VideoCaptureSinkWinRT
    : public RuntimeClass<
                          RuntimeClassFlags< RuntimeClassType::WinRtClassicComMix >,
                          IMediaExtension,
                          FtmBase,
                          IMFMediaSink,
                          IMFClockStateSink>
{
 public:
  VideoCaptureSinkWinRT();
  ~VideoCaptureSinkWinRT();

  // IMediaExtension
  IFACEMETHOD(SetProperties) (ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration) { return S_OK; }

  // IMFMediaSink methods
  IFACEMETHOD(GetCharacteristics) (DWORD *pdwCharacteristics);

  IFACEMETHOD(AddStreamSink)(
    /* [in] */ DWORD dwStreamSinkIdentifier,
    /* [in] */ IMFMediaType *pMediaType,
    /* [out] */ IMFStreamSink **ppStreamSink);

  IFACEMETHOD(RemoveStreamSink) (DWORD dwStreamSinkIdentifier);
  IFACEMETHOD(GetStreamSinkCount) (_Out_ DWORD *pcStreamSinkCount);
  IFACEMETHOD(GetStreamSinkByIndex) (DWORD dwIndex, _Outptr_ IMFStreamSink **ppStreamSink);
  IFACEMETHOD(GetStreamSinkById) (DWORD dwIdentifier, IMFStreamSink **ppStreamSink);
  IFACEMETHOD(SetPresentationClock) (IMFPresentationClock *pPresentationClock);
  IFACEMETHOD(GetPresentationClock) (IMFPresentationClock **ppPresentationClock);
  IFACEMETHOD(Shutdown) ();

  // IMFClockStateSink methods
  IFACEMETHOD(OnClockStart) (MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
  IFACEMETHOD(OnClockStop) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockPause) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockRestart) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockSetRate) (MFTIME hnsSystemTime, float flRate);

 private:
  long                            _cRef;                      // reference count

  bool                            _IsShutdown;                // Flag to indicate if Shutdown() method was called.
  bool                            _IsConnected;
  LONGLONG                        _llStartTime;

  ComPtr<IMFPresentationClock>    _spClock;                   // Presentation clock.
};

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_
