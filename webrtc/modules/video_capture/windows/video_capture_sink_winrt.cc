#include "webrtc/modules/video_capture/windows/video_capture_sink_winrt.h"

#include <assert.h>

namespace webrtc {
namespace videocapturemodule {

VideoCaptureSinkWinRT::VideoCaptureSinkWinRT()
    : _cRef(1),
      _IsShutdown(false),
      _IsConnected(false),
      _llStartTime(0)
{
}

VideoCaptureSinkWinRT::~VideoCaptureSinkWinRT()
{
  assert(_IsShutdown);
}

///  IMFMediaSink
IFACEMETHODIMP VideoCaptureSinkWinRT::GetCharacteristics(DWORD *pdwCharacteristics)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::AddStreamSink(
    DWORD dwStreamSinkIdentifier,
    IMFMediaType *pMediaType,
    IMFStreamSink **ppStreamSink)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::RemoveStreamSink(DWORD dwStreamSinkIdentifier)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::GetStreamSinkCount(_Out_ DWORD *pcStreamSinkCount)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::GetStreamSinkByIndex(
    DWORD dwIndex,
    _Outptr_ IMFStreamSink **ppStreamSink)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::GetStreamSinkById(
    DWORD dwStreamSinkIdentifier,
    IMFStreamSink **ppStreamSink)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::SetPresentationClock(IMFPresentationClock *pPresentationClock)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::GetPresentationClock(IMFPresentationClock **ppPresentationClock)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::Shutdown()
{
  return S_OK;
}

// IMFClockStateSink
IFACEMETHODIMP VideoCaptureSinkWinRT::OnClockStart(
    MFTIME hnsSystemTime,
    LONGLONG llClockStartOffset)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::OnClockStop(
    MFTIME hnsSystemTime)
{
  return S_OK;
}


IFACEMETHODIMP VideoCaptureSinkWinRT::OnClockPause(
    MFTIME hnsSystemTime)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::OnClockRestart(
    MFTIME hnsSystemTime)
{
  return S_OK;
}

IFACEMETHODIMP VideoCaptureSinkWinRT::OnClockSetRate(
    /* [in] */ MFTIME hnsSystemTime,
    /* [in] */ float flRate)
{
  return S_OK;
}

}  // namespace videocapturemodule
}  // namespace webrtc
