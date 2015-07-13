
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_API_RTMEDIASTREAMSOURCE_H_
#define WEBRTC_BUILD_WINRT_GYP_API_RTMEDIASTREAMSOURCE_H_

#include "Media.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "webrtc/system_wrappers/interface/tick_util.h"

using Windows::Media::Core::MediaStreamSource;
using Platform::WeakReference;
using webrtc_winrt_api::MediaVideoTrack;

namespace webrtc_winrt_api {
public delegate void FramesPerSecondChangedEventHandler(Platform::String^ fps);
public ref class FrameCounterHelper sealed {
  public:
    static event FramesPerSecondChangedEventHandler^ FramesPerSecondChanged;
  internal:
    static void FireEvent(Platform::String^ str) {
      FramesPerSecondChanged(str);
    }
  };
}  // namespace webrtc_winrt_api

namespace webrtc_winrt_api_internal {
ref class RTMediaStreamSource sealed {
  public:
    virtual ~RTMediaStreamSource();
  internal:
    static MediaStreamSource^ CreateMediaSource(
      MediaVideoTrack^ track, uint32 frameRate);
  private:
    class RTCRenderer : public webrtc::VideoRendererInterface {
     public:
      explicit RTCRenderer(RTMediaStreamSource^ streamSource);
      virtual ~RTCRenderer();
      virtual void SetSize(uint32 width, uint32 height, uint32 reserved);
      virtual void RenderFrame(const cricket::VideoFrame *frame);
     private:
      // This object is owned by RTMediaStreamSource
      // so _streamSource must be a weak reference
      WeakReference _streamSource;
    };

    RTMediaStreamSource(MediaVideoTrack^ videoTrack);
    void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender,
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);
    void ProcessReceivedFrame(const cricket::VideoFrame *frame);
    bool ConvertFrame(IMFMediaBuffer* mediaBuffer);
    void ResizeSource(uint32 width, uint32 height);
    static void OnClosed(Windows::Media::Core::MediaStreamSource ^sender,
      Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args);

    MediaVideoTrack^ _videoTrack;
    MediaStreamSource^ _mediaStreamSource;
    rtc::scoped_ptr<RTCRenderer> _rtcRenderer;
    CRITICAL_SECTION _lock;
    rtc::scoped_ptr<cricket::VideoFrame> _frame;
    uint32 _stride;
    uint64 _timeStamp;
    uint32 _frameRate;
    Windows::Media::Core::VideoStreamDescriptor^ _videoDesc;
    int _frameCounter;
    webrtc::TickTime _lastTimeFPSCalculated;
    bool _framePassedToUI;
};

}  // namespace webrtc_winrt_api_internal

#endif  // WEBRTC_BUILD_WINRT_GYP_API_RTMEDIASTREAMSOURCE_H_
