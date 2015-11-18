
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
#include "webrtc/system_wrappers/include/tick_util.h"
#include "webrtc/system_wrappers/include/critical_section_wrapper.h"

using Windows::Media::Core::MediaStreamSource;
using Platform::WeakReference;
using webrtc_winrt_api::MediaVideoTrack;

namespace webrtc_winrt_api {
// Delegate used to notify an update of the frame per second on a video stream.
public delegate void FramesPerSecondChangedEventHandler(String^ id,
  Platform::String^ fps);
public delegate void ResolutionChangedEventHandler(String^ id,
  unsigned int width, unsigned int height);

public ref class FrameCounterHelper sealed {
  public:
    static event FramesPerSecondChangedEventHandler^ FramesPerSecondChanged;
  internal:
    static void FireEvent(String^ id, Platform::String^ str);
  };

public ref class ResolutionHelper sealed {
public:
  static event ResolutionChangedEventHandler^ ResolutionChanged;
internal:
  static void FireEvent(String^ id, unsigned int width, unsigned int height);
};
}  // namespace webrtc_winrt_api

namespace webrtc_winrt_api_internal {
ref class RTMediaStreamSource sealed {
  public:
    virtual ~RTMediaStreamSource();

    void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender,
      Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);

  internal:
    static MediaStreamSource^ CreateMediaSource(
      MediaVideoTrack^ track, uint32 frameRate, String^ id);
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
    void ProcessReceivedFrame(const cricket::VideoFrame *frame);
    bool ConvertFrame(IMFMediaBuffer* mediaBuffer);
    void BlankFrame(IMFMediaBuffer* mediaBuffer);
    void ResizeSource(uint32 width, uint32 height);
    static void OnClosed(Windows::Media::Core::MediaStreamSource ^sender,
      Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args);

    MediaVideoTrack^ _videoTrack;
    String^ _id;  // Provided by the calling API.

    // Keep a weak reference here.
    // Its _mediaStreamSource that keeps a reference to this object.
    WeakReference _mediaStreamSource;
    rtc::scoped_ptr<RTCRenderer> _rtcRenderer;
    rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;
    rtc::scoped_ptr<cricket::VideoFrame> _frame;
    bool _isNewFrame;  // If the frame in _frame hasn't been rendered yet.
    uint32 _stride;
    uint64 _timeStamp;

    uint32 _frameRate;
    Windows::Media::Core::VideoStreamDescriptor^ _videoDesc;

    // State related to calculating FPS.
    int _frameCounter;
    webrtc::TickTime _lastTimeFPSCalculated;
};

}  // namespace webrtc_winrt_api_internal

#endif  // WEBRTC_BUILD_WINRT_GYP_API_RTMEDIASTREAMSOURCE_H_
