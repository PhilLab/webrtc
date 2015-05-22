#pragma once

using namespace Windows::Media::Core;
using namespace Windows::Media::Protection;
using namespace Windows::Foundation;
using namespace Platform;

namespace webrtc_winrt_api
{

  ref class RTMediaStreamSource sealed :
    IMediaSource,
    IMediaStreamSource
  {
  public:
    RTMediaStreamSource();
    // IMediaStreamSource
    virtual void AddProtectionKey(IMediaStreamDescriptor^ streamDescriptor, const Array<uint8>^ keyIdentifier,
      const Array<uint8>^ licenseData);
    virtual void AddStreamDescriptor(IMediaStreamDescriptor^ descriptor);
    virtual void NotifyError(MediaStreamSourceErrorStatus errorStatus);
    virtual void SetBufferedRange(TimeSpan startOffset, TimeSpan endOffset);
    property TimeSpan BufferTime
    {
      virtual TimeSpan get();
      virtual void set(TimeSpan value);
    }
    property bool CanSeek
    {
      virtual bool get();
      virtual void set(bool value);
    }
    property TimeSpan Duration
    {
      virtual TimeSpan get();
      virtual void set(TimeSpan value);
    }
    property MediaProtectionManager^ MediaProtectionManager
    {
      virtual Windows::Media::Protection::MediaProtectionManager^ get();
      virtual void set(Windows::Media::Protection::MediaProtectionManager^ value);
    }
    property Windows::Storage::FileProperties::MusicProperties^ MusicProperties
    {
      virtual Windows::Storage::FileProperties::MusicProperties^ get();
    }

  private:
    MediaStreamSource^ _mediaStreamSource;
  };

}