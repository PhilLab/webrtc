#include "RTMediaStreamSource.h"

namespace webrtc_winrt_api
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
    auto ret = ref new RTMediaStreamSource();
    ret->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(new RTCRenderer(ret));
    ret->_mediaStreamSource = streamSource;
    ret->_mediaStreamSource->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, 
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(ret, &webrtc_winrt_api::RTMediaStreamSource::OnSampleRequested);
    return ret;
  }

  RTMediaStreamSource::RTMediaStreamSource() : 
    _stride(0), _buffer(nullptr), _sourceWidth(0), _sourceHeight(0)
  {
    InitializeCriticalSection(&_lock);
  }

  RTMediaStreamSource::~RTMediaStreamSource()
  {
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
    _mediaStreamSource->Closed += value;
  }

  void RTMediaStreamSource::Closed::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Closed -= token;
  }

  EventRegistrationToken RTMediaStreamSource::Paused::add(TypedEventHandler<MediaStreamSource^, Object^>^ value)
  {
    _mediaStreamSource->Paused += value;
  }

  void RTMediaStreamSource::Paused::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Paused -= token;
  }

  EventRegistrationToken RTMediaStreamSource::SampleRequested::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>^ value)
  {
    _mediaStreamSource->SampleRequested += value;
  }

  void RTMediaStreamSource::SampleRequested::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->SampleRequested -= token;
  }

  EventRegistrationToken RTMediaStreamSource::Starting::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>^ value)
  {
    _mediaStreamSource->Starting += value;
  }

  void RTMediaStreamSource::Starting::remove(Windows::Foundation::EventRegistrationToken token)
  {
    _mediaStreamSource->Starting -= token;
  }

  EventRegistrationToken RTMediaStreamSource::SwitchStreamsRequested::add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSwitchStreamsRequestedEventArgs^>^ value)
  {
    _mediaStreamSource->SwitchStreamsRequested += value;
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
    EnterCriticalSection(&_lock);
    LeaveCriticalSection(&_lock);
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
    LeaveCriticalSection(&_lock);
  }

}
