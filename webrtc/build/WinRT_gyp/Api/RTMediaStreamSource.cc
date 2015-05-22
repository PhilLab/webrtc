#include "RTMediaStreamSource.h"

namespace webrtc_winrt_api
{

  RTMediaStreamSource::RTMediaStreamSource()
  {
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

}