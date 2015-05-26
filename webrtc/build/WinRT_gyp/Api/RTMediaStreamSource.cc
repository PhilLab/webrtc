#include "RTMediaStreamSource.h"
#include <mfapi.h>
#include <ppltasks.h>
#include <mfidl.h>
#include "talk/media/base/videoframe.h"
#include "libyuv/video_common.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

using namespace Windows::Media::MediaProperties;
using namespace Microsoft::WRL;
using namespace webrtc_winrt_api;

namespace
{
  webrtc::CriticalSectionWrapper& gMediaStreamListLock(*webrtc::CriticalSectionWrapper::CreateCriticalSection());
  Vector<webrtc_winrt_api_internal::RTMediaStreamSource^>^ gMediaStreamList = ref new Vector<webrtc_winrt_api_internal::RTMediaStreamSource^>();
}

namespace webrtc_winrt_api_internal
{

  MediaStreamSource^ RTMediaStreamSource::CreateMediaSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 frameRate)
  {
    auto videoProperties =
      Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(
      Windows::Media::MediaProperties::MediaEncodingSubtypes::Bgra8, width, height);
    auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
    videoDesc->EncodingProperties->FrameRate->Numerator = frameRate;
    videoDesc->EncodingProperties->FrameRate->Denominator = 1;
    videoDesc->EncodingProperties->Bitrate = (uint32)(videoDesc->EncodingProperties->FrameRate->Numerator*
      videoDesc->EncodingProperties->FrameRate->Denominator * width * height * 4);
    auto streamSource = ref new MediaStreamSource(videoDesc);
    auto streamState = ref new RTMediaStreamSource(track);
    streamState->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(new RTCRenderer(streamState));
    streamState->_mediaStreamSource = streamSource;
    streamState->_mediaStreamSource->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^,
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(streamState, &RTMediaStreamSource::OnSampleRequested);
    streamState->_mediaStreamSource->Closed += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(&webrtc_winrt_api_internal::RTMediaStreamSource::OnClosed);
    streamState->_frameRate = frameRate;
    streamState->_sourceWidth = width;
    streamState->_sourceHeight = height;
    track->SetRenderer(streamState->_rtcRenderer.get());
    {
      webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
      gMediaStreamList->Append(streamState);
    }
    return streamState->_mediaStreamSource;
  }

  RTMediaStreamSource::RTMediaStreamSource(MediaVideoTrack^ videoTrack) :
    _videoTrack(videoTrack), _stride(0), _buffer(nullptr),
    _bufferSize(0), _sourceWidth(0), _sourceHeight(0), _timeStamp(0), _frameRate(0)
  {
    InitializeCriticalSection(&_lock);
  }

  RTMediaStreamSource::~RTMediaStreamSource()
  {
    if (_rtcRenderer != nullptr)
    {
      _videoTrack->UnsetRenderer(_rtcRenderer.get());
    }
    DeleteCriticalSection(&_lock);
    if (_buffer != nullptr)
    {
      delete[] _buffer;
    }
  }

  RTMediaStreamSource::RTCRenderer::RTCRenderer(RTMediaStreamSource^ streamSource) : _streamSource(streamSource)
  {
  }

  RTMediaStreamSource::RTCRenderer::~RTCRenderer()
  {
  }

  void RTMediaStreamSource::RTCRenderer::SetSize(int width, int height, int reserved)
  {
    auto stream = _streamSource.Resolve<RTMediaStreamSource>();
    if (stream != nullptr)
    {
      stream->ResizeSource((uint32)width, (uint32)height);
    }
  }

  void RTMediaStreamSource::RTCRenderer::RenderFrame(const cricket::VideoFrame *frame)
  {
    auto stream = _streamSource.Resolve<RTMediaStreamSource>();
    if (stream != nullptr)
    {
      stream->ProcessReceivedFrame(frame);
    }
  }

  void RTMediaStreamSource::OnSampleRequested(MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args)
  {
    auto request = args->Request;
    if (request == nullptr)
    {
      return;
    }
    ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
    HRESULT hr = reinterpret_cast<IInspectable*>(request)->QueryInterface(spRequest.ReleaseAndGetAddressOf());
    if (FAILED(hr))
    {
      return;
    }
    ComPtr<IMFSample> spSample;
    hr = MFCreateSample(spSample.GetAddressOf());
    if (FAILED(hr))
    {
      return;
    }
    ComPtr<IMFMediaBuffer> mediaBuffer;
    hr = MFCreate2DMediaBuffer(_sourceWidth, _sourceHeight, 20, FALSE, mediaBuffer.GetAddressOf());
    if (FAILED(hr))
    {
      return;
    }
    spSample->AddBuffer(mediaBuffer.Get());
    auto spVideoStreamDescriptor = (VideoStreamDescriptor^)request->StreamDescriptor;
    auto spEncodingProperties = spVideoStreamDescriptor->EncodingProperties;
    auto spRatio = spEncodingProperties->FrameRate;
    UINT32 ui32Numerator = spRatio->Numerator;
    UINT32 ui32Denominator = spRatio->Denominator;
    ULONGLONG ulTimeSpan = ((ULONGLONG)ui32Denominator) * 10000000 / ui32Numerator;

    spSample->SetSampleDuration(ulTimeSpan);
    spSample->SetSampleTime((LONGLONG)_timeStamp);
    if (ConvertFrame(mediaBuffer.Get()))
    {
      _timeStamp += ulTimeSpan;
    }
    spRequest->SetSample(spSample.Get());
  }

  void RTMediaStreamSource::ProcessReceivedFrame(const cricket::VideoFrame *frame)
  {
    EnterCriticalSection(&_lock);
    if (_buffer == nullptr)
    {
      // Not ready for frames yet
      LeaveCriticalSection(&_lock);
      return;
    }
    frame->ConvertToRgbBuffer(libyuv::FOURCC_ARGB, _buffer, _bufferSize, _stride);
    LeaveCriticalSection(&_lock);
  }

  bool RTMediaStreamSource::ConvertFrame(IMFMediaBuffer* mediaBuffer)
  {
    ComPtr<IMF2DBuffer> imageBuffer;
    if (FAILED(mediaBuffer->QueryInterface(imageBuffer.GetAddressOf())))
    {
      return false;
    }
    BYTE* destRawData;
    LONG pitch;
    if (FAILED(imageBuffer->Lock2D(&destRawData, &pitch)))
    {
      return false;
    }
    EnterCriticalSection(&_lock);
    if (!_buffer)
    {
      _stride = (uint32)pitch;
      _bufferSize = _sourceHeight * _stride;
      _buffer = new BYTE[_bufferSize];
      LeaveCriticalSection(&_lock);
      imageBuffer->Unlock2D();
      return false;
    }
    memcpy(destRawData, _buffer, _bufferSize);
    LeaveCriticalSection(&_lock);
    imageBuffer->Unlock2D();
    return true;
  }

  void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height)
  {
  }

  void RTMediaStreamSource::OnClosed(Windows::Media::Core::MediaStreamSource ^sender,
    Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args)
  {
    webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
    for (unsigned int i = 0; i < gMediaStreamList->Size; i++)
    {
      if (gMediaStreamList->GetAt(i)->Equals(sender))
      {
        gMediaStreamList->RemoveAt(i);
        break;
      }
    }
  }
}
