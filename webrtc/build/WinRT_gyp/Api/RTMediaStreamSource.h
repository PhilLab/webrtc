#pragma once

#include "Media.h"
#include "talk/app/webrtc/mediastreaminterface.h"

using namespace Windows::Media::Core;
using namespace Windows::Media::Protection;
using namespace Windows::Foundation;
using namespace Platform;
using namespace Platform::Collections;
using namespace webrtc_winrt_api;
using namespace Windows::Foundation::Collections;

namespace webrtc_winrt_api_internal
{
  ref class RTMediaStreamSource sealed
  {
  public:
    virtual ~RTMediaStreamSource();
  internal:
    static MediaStreamSource^ CreateMediaSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 frameRate);
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
    void ResizeSource(uint32 width, uint32 height);
    static void OnClosed(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args);

    MediaVideoTrack^ _videoTrack;
    MediaStreamSource^ _mediaStreamSource;
    rtc::scoped_ptr<RTCRenderer> _rtcRenderer;
    CRITICAL_SECTION _lock;
    rtc::scoped_ptr<cricket::VideoFrame> _frame;
    uint32 _stride;
    uint32 _sourceWidth;
    uint32 _sourceHeight;
    uint64 _timeStamp;
    uint32 _frameRate;
  };

}