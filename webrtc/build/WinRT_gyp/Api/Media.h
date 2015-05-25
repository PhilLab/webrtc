#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "GlobalObserver.h"
#include <mfidl.h>

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Media::Core;

namespace webrtc_winrt_api
{
  public interface class IMediaStreamTrack
  {
    property String^ Kind { String^ get(); }
    property String^ Id { String^ get(); }
    property bool Enabled { bool get(); void set(bool value); }
  };

  public ref class MediaVideoTrack sealed : public IMediaStreamTrack
  {
  internal:
    MediaVideoTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> impl);
  public:
    virtual ~MediaVideoTrack();
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
  internal:
    void SetRenderer(webrtc::VideoRendererInterface* renderer);
    void UnsetRenderer(webrtc::VideoRendererInterface* renderer);
  private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> _impl;
  };

  public ref class MediaAudioTrack sealed : public IMediaStreamTrack
  {
  internal:
    MediaAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> impl);
  public:
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
  private:
    rtc::scoped_refptr<webrtc::AudioTrackInterface> _impl;
  };


  public ref class MediaStream sealed
  {
  internal:
    MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl);
    rtc::scoped_refptr<webrtc::MediaStreamInterface> GetImpl();
  public:
    IVector<MediaAudioTrack^>^ GetAudioTracks();
    IVector<MediaVideoTrack^>^ GetVideoTracks();
    IVector<IMediaStreamTrack^>^ GetTracks();
  private:
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _impl;
  };

  public ref class Media sealed
  {
  public:
    // TODO: Arguments
    IAsyncOperation<MediaStream^>^ GetUserMedia();
    IMediaSource^ CreateMediaStreamSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 framerate);
  };

}
