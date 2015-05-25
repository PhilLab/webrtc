#include "RTMediaStreamSource.h"
#include <mfapi.h>
#include <ppltasks.h>
#include <mfidl.h>
#include "talk/media/base/videoframe.h"
#include "libyuv/video_common.h"

using namespace Windows::Media::MediaProperties;
using namespace Microsoft::WRL;
using namespace webrtc_winrt_api;

namespace webrtc_winrt_api_internal
{
  IMediaSource^ RTMediaStreamSource::CreateMediaSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 frameRate)
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
    auto ret = ref new RTMediaStreamSource(track);
    ret->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(new RTCRenderer(ret));
    ret->_mediaStreamSource = streamSource;
    ret->_mediaStreamSource->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, 
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(ret, &RTMediaStreamSource::OnSampleRequested);
    ret->_frameRate = frameRate;
    track->SetRenderer(ret->_rtcRenderer.get());
    return ret;
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

  void RTMediaStreamSource::AddProtectionKey(IMediaStreamDescriptor^ streamDescriptor, const Array<uint8>^ keyIdentifier,
    const Array<uint8>^ licenseData)
  {
    _mediaStreamSource->AddProtectionKey(streamDescriptor, keyIdentifier, licenseData);
  }

  void RTMediaStreamSource::AddStreamDescriptor(IMediaStreamDescriptor^ descriptor)
  {
    _mediaStreamSource->AddStreamDescriptor(descriptor);
  }

  void RTMediaStreamSource::NotifyError(MediaStreamSourceErrorStatus errorStatus)
  {
    _mediaStreamSource->NotifyError(errorStatus);
  }

  void RTMediaStreamSource::SetBufferedRange(TimeSpan startOffset, TimeSpan endOffset)
  {
    _mediaStreamSource->SetBufferedRange(startOffset, endOffset);
  }

  TimeSpan RTMediaStreamSource::BufferTime::get()
  {
    return _mediaStreamSource->BufferTime;
  }

  void RTMediaStreamSource::BufferTime::set(TimeSpan value)
  {
    _mediaStreamSource->BufferTime = value;
  }

  bool RTMediaStreamSource::CanSeek::get()
  {
    // This is a live feed to seeking is always unavailable
    return false;
  }

  void RTMediaStreamSource::CanSeek::set(bool value)
  {
    if (!value)
    {
      throw ref new AccessDeniedException("Cannot seek on live media");
    }
  }

  TimeSpan RTMediaStreamSource::Duration::get()
  {
    throw ref new NotImplementedException("Live media does not have a duration");
  }

  void RTMediaStreamSource::Duration::set(TimeSpan value)
  {
    throw ref new AccessDeniedException("Cannot set the duration of live media");
  }

  MediaProtectionManager^ RTMediaStreamSource::MediaProtectionManager::get()
  {
    return _mediaStreamSource->MediaProtectionManager;
  }

  void RTMediaStreamSource::MediaProtectionManager::set(Windows::Media::Protection::MediaProtectionManager^ value)
  {
    _mediaStreamSource->MediaProtectionManager = value;
  }

  Windows::Storage::FileProperties::MusicProperties^ RTMediaStreamSource::MusicProperties::get()
  {
    return _mediaStreamSource->MusicProperties;
  }

  Windows::Storage::Streams::IRandomAccessStreamReference^  RTMediaStreamSource::Thumbnail::get()
  {
    return _mediaStreamSource->Thumbnail;
  }

  void RTMediaStreamSource::Thumbnail::set(Windows::Storage::Streams::IRandomAccessStreamReference^ value)
  {
    _mediaStreamSource->Thumbnail = value;
  }

  Windows::Storage::FileProperties::VideoProperties^ RTMediaStreamSource::VideoProperties::get()
  {
    return _mediaStreamSource->VideoProperties;
  }

  EventRegistrationToken RTMediaStreamSource::Closed::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceClosedEventArgs^>^ value)
  {
    return (_mediaStreamSource->Closed += value);
  }

  void RTMediaStreamSource::Closed::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Closed -= token;
  }

  EventRegistrationToken RTMediaStreamSource::Paused::add(TypedEventHandler<MediaStreamSource^, Object^>^ value)
  {
    return (_mediaStreamSource->Paused += value);
  }

  void RTMediaStreamSource::Paused::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Paused -= token;
  }

  EventRegistrationToken RTMediaStreamSource::SampleRequested::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>^ value)
  {
    return (_mediaStreamSource->SampleRequested += value);
  }

  void RTMediaStreamSource::SampleRequested::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->SampleRequested -= token;
  }

  EventRegistrationToken RTMediaStreamSource::Starting::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>^ value)
  {
    return (_mediaStreamSource->Starting += value);
  }

  void RTMediaStreamSource::Starting::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Starting -= token;
  }

  EventRegistrationToken RTMediaStreamSource::SwitchStreamsRequested::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSwitchStreamsRequestedEventArgs^>^ value)
  {
    return (_mediaStreamSource->SwitchStreamsRequested += value);
  }

  void RTMediaStreamSource::SwitchStreamsRequested::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->SwitchStreamsRequested -= token;
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
    // Destroy inner stream object and re-create
    EnterCriticalSection(&_lock);
    if (_rtcRenderer != nullptr)
    {
      _videoTrack->UnsetRenderer(_rtcRenderer.get());
    }
    if (_buffer != nullptr)
    {
      delete[] _buffer;
      _buffer = nullptr;
    }
    _sourceWidth = width;
    _sourceHeight = height;
    auto videoProperties =
      Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(
      Windows::Media::MediaProperties::MediaEncodingSubtypes::Bgra8, width, height);
    auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
    videoDesc->EncodingProperties->FrameRate->Numerator = _frameRate;
    videoDesc->EncodingProperties->FrameRate->Denominator = 1;
    videoDesc->EncodingProperties->Bitrate = (uint32)(videoDesc->EncodingProperties->FrameRate->Numerator*
      videoDesc->EncodingProperties->FrameRate->Denominator * width * height * 4);
    auto newMediaStreamSource = ref new MediaStreamSource(videoDesc);
    _mediaStreamSource = newMediaStreamSource;
    _mediaStreamSource->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^,
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(this, &RTMediaStreamSource::OnSampleRequested);
    _videoTrack->SetRenderer(_rtcRenderer.get());
    // TODO: re-create events
    LeaveCriticalSection(&_lock);
  }

}
