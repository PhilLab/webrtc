#pragma once

#include "Media.h"
#include "talk/app/webrtc/mediastreaminterface.h"


using namespace Windows::Media::Core;
using namespace Windows::Media::Protection;
using namespace Windows::Foundation;
using namespace Platform;
using namespace webrtc_winrt_api;

namespace webrtc_winrt_api_internal
{
  ref class RTMediaStreamSource sealed :
    IMediaSource
  {
  public:
    virtual ~RTMediaStreamSource();
    // IMediaStreamSource
    // Methods
    virtual void AddProtectionKey(IMediaStreamDescriptor^ streamDescriptor, const Array<uint8>^ keyIdentifier,
      const Array<uint8>^ licenseData);
    virtual void AddStreamDescriptor(IMediaStreamDescriptor^ descriptor);
    virtual void NotifyError(MediaStreamSourceErrorStatus errorStatus);
    virtual void SetBufferedRange(TimeSpan startOffset, TimeSpan endOffset);
    // Properties
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
    property Windows::Storage::Streams::IRandomAccessStreamReference^ Thumbnail
    {
      virtual Windows::Storage::Streams::IRandomAccessStreamReference^ get();
      virtual void set(Windows::Storage::Streams::IRandomAccessStreamReference^ value);
    }
    property Windows::Storage::FileProperties::VideoProperties^ VideoProperties
    {
      virtual Windows::Storage::FileProperties::VideoProperties^ get();
    }
    // Events
    event TypedEventHandler<MediaStreamSource^, MediaStreamSourceClosedEventArgs^>^ Closed
    {
      virtual EventRegistrationToken add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceClosedEventArgs^>^ value);
      virtual void remove(Windows::Foundation::EventRegistrationToken token);
    }
    event TypedEventHandler<MediaStreamSource^, Object^>^ Paused
    {
      virtual EventRegistrationToken add(TypedEventHandler<MediaStreamSource^, Object^>^ value);
      virtual void remove(Windows::Foundation::EventRegistrationToken token);
    }
    event TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>^ SampleRequested
    {
      virtual EventRegistrationToken add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSampleRequestedEventArgs^>^ value);
      virtual void remove(Windows::Foundation::EventRegistrationToken token);
    }
    event TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>^ Starting
    {
      virtual EventRegistrationToken add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceStartingEventArgs^>^ value);
      virtual void remove(Windows::Foundation::EventRegistrationToken token);
    }
    event TypedEventHandler<MediaStreamSource^, MediaStreamSourceSwitchStreamsRequestedEventArgs^>^ SwitchStreamsRequested
    {
      virtual EventRegistrationToken add(TypedEventHandler<MediaStreamSource^, MediaStreamSourceSwitchStreamsRequestedEventArgs^>^ value);
      virtual void remove(Windows::Foundation::EventRegistrationToken token);
    }
  internal:
    static IMediaSource^ CreateMediaSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 frameRate);
  private:

    class RTCRenderer : public webrtc::VideoRendererInterface
    {
    public:
      RTCRenderer(RTMediaStreamSource^ streamSource);
      virtual ~RTCRenderer();
      virtual void SetSize(int width, int height, int reserved);
      virtual void RenderFrame(const cricket::VideoFrame *frame);
    private:
      // This object is owned by RTMediaStreamSource so _streamSource must be a weak reference
      WeakReference _streamSource;
    };

    RTMediaStreamSource(MediaVideoTrack^ videoTrack);
    void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);
    void ProcessReceivedFrame(const cricket::VideoFrame *frame);
    bool ConvertFrame(IMFMediaBuffer* mediaBuffer);

    MediaVideoTrack^ _videoTrack;
    MediaStreamSource^ _mediaStreamSource;
    rtc::scoped_ptr<RTCRenderer> _rtcRenderer;
    CRITICAL_SECTION _lock;
    uint32 _stride;
    byte* _buffer;
    uint32 _bufferSize;
    uint32 _sourceWidth;
    uint32 _sourceHeight;
    uint64 _timeStamp;
  };

}