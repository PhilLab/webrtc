#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "GlobalObserver.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

namespace webrtc_winrt_api
{
  public ref class MediaStreamTrack sealed
  {
  internal:
    MediaStreamTrack(rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> impl);
  public:
    property String^ Kind { String^ get(); }
    property String^ Id { String^ get(); }
    property bool Enabled { bool get(); void set(bool); }

  private:
    rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> _impl;
  };


  public ref class MediaStream sealed
  {
  internal:
    MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl);
    rtc::scoped_refptr<webrtc::MediaStreamInterface> GetImpl();
  public:
    IVector<MediaStreamTrack^>^ GetAudioTracks();
    IVector<MediaStreamTrack^>^ GetVideoTracks();
    IVector<MediaStreamTrack^>^ GetTracks();
  private:
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _impl;
  };

  public ref class Media sealed
  {
  public:
    // TODO: Arguments
    IAsyncOperation<MediaStream^>^ GetUserMedia();
  };
}
