#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_

#include <mfidl.h>

#include <windows.media.h>

#include <wrl\implements.h>

namespace webrtc {
namespace videocapturemodule {

class VideoCaptureSinkWinRT
    : public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<
            Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
        ABI::Windows::Media::IMediaExtension,
        Microsoft::WRL::FtmBase,
        IMFMediaSink,
        IMFClockStateSink> {
 public:
  VideoCaptureSinkWinRT();
  ~VideoCaptureSinkWinRT();

  // IMediaExtension
  IFACEMETHOD(SetProperties) (
      ABI::Windows::Foundation::Collections::IPropertySet *pConfiguration) {
    return S_OK;
  }

  // IMFMediaSink methods
  IFACEMETHOD(GetCharacteristics) (DWORD *pdwCharacteristics);

  IFACEMETHOD(AddStreamSink)(
    /* [in] */ DWORD dwStreamSinkIdentifier,
    /* [in] */ IMFMediaType *pMediaType,
    /* [out] */ IMFStreamSink **ppStreamSink);

  IFACEMETHOD(RemoveStreamSink) (DWORD dwStreamSinkIdentifier);
  IFACEMETHOD(GetStreamSinkCount) (_Out_ DWORD *pcStreamSinkCount);
  IFACEMETHOD(GetStreamSinkByIndex) (
      DWORD dwIndex,
      _Outptr_ IMFStreamSink **ppStreamSink);
  IFACEMETHOD(GetStreamSinkById) (
      DWORD dwIdentifier,
      IMFStreamSink **ppStreamSink);
  IFACEMETHOD(SetPresentationClock) (
      IMFPresentationClock *pPresentationClock);
  IFACEMETHOD(GetPresentationClock) (
      IMFPresentationClock **ppPresentationClock);
  IFACEMETHOD(Shutdown) ();

  // IMFClockStateSink methods
  IFACEMETHOD(OnClockStart) (MFTIME hnsSystemTime, LONGLONG llClockStartOffset);
  IFACEMETHOD(OnClockStop) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockPause) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockRestart) (MFTIME hnsSystemTime);
  IFACEMETHOD(OnClockSetRate) (MFTIME hnsSystemTime, float flRate);

 private:
  int64 _cRef;
  bool _isShutdown;
  bool _isConnected;
  LONGLONG _llStartTime;

  Microsoft::WRL::ComPtr<IMFPresentationClock> _spClock;
};

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_SINK_WINRT_H_
