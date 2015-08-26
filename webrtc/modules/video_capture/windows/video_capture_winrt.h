/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_

#include <functional>
#include "webrtc/modules/video_capture/video_capture_impl.h"
#include "webrtc/modules/video_capture/windows/device_info_winrt.h"

namespace webrtc {
namespace videocapturemodule {

ref class CaptureDevice;
ref class DisplayOrientation;

class CaptureDeviceListener {
 public:
  virtual void OnIncomingFrame(uint8_t* video_frame,
                               size_t video_frame_length,
                               const VideoCaptureCapability& frame_info) = 0;
  virtual void OnCaptureDeviceFailed(HRESULT code,
    Platform::String^ message) = 0;
};

class DisplayOrientationListener {
 public:
  virtual void DisplayOrientationChanged(
    Windows::Graphics::Display::DisplayOrientations orientation) = 0;
};

class VideoCaptureWinRT
    : public VideoCaptureImpl,
      public CaptureDeviceListener,
      public DisplayOrientationListener{
 public:
  explicit VideoCaptureWinRT(const int32_t id);

  int32_t Init(const int32_t id, const char* device_id);

  // Overrides from VideoCaptureImpl.
  virtual int32_t StartCapture(const VideoCaptureCapability& capability);
  virtual int32_t StopCapture();
  virtual bool CaptureStarted();
  virtual int32_t CaptureSettings(VideoCaptureCapability& settings);

 protected:
  virtual ~VideoCaptureWinRT();

  virtual void OnIncomingFrame(uint8_t* video_frame,
                               size_t video_frame_length,
                               const VideoCaptureCapability& frame_info);

  virtual void OnCaptureDeviceFailed(HRESULT code,
                                     Platform::String^ message);

  virtual void DisplayOrientationChanged(
    Windows::Graphics::Display::DisplayOrientations orientation);
  virtual void ApplyDisplayOrientation(
    Windows::Graphics::Display::DisplayOrientations orientation);

 private:
  Platform::String^ device_id_;
  CaptureDevice^ device_;
  Windows::Devices::Enumeration::Panel camera_location_;
  DisplayOrientation^ display_orientation_;
};

// Helper function to run code on the WinRT CoreDispatcher
// and only return once the call completed.
void RunOnCoreDispatcher(std::function<void()> fn, bool async = false);

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_
