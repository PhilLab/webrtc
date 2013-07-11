/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_H_

#import <AVFoundation/AVFoundation.h>
#include <stdio.h>

#include "../video_capture_impl.h"
#include "../video_capture_impl.h"
#include "../device_info_impl.h"
#include "video_capture_defines.h"

// Forward declaraion
@class VideoCaptureiPhoneObjC;
@class DeviceInfoIphoneObjC;

namespace webrtc
{
namespace videocapturemodule
{

class VideoCaptureiPhone: public VideoCaptureImpl
{
public:
    VideoCaptureiPhone(const WebRtc_Word32 id);
    virtual ~VideoCaptureiPhone();
    WebRtc_Word32 Init(const WebRtc_Word32 id,
                       const char* deviceUniqueIdUTF8);
    virtual WebRtc_Word32 StartCapture(
                        const VideoCaptureCapability& capability);
    virtual WebRtc_Word32 StopCapture();
    virtual bool CaptureStarted();
    virtual WebRtc_Word32 CaptureSettings(VideoCaptureCapability& settings);
protected:
    // Help functions
    WebRtc_Word32 SetCameraOutput();
private:
    VideoCaptureiPhoneObjC* _captureDevice;
    DeviceInfoIphoneObjC* _captureInfo;
    bool _isCapturing;
    WebRtc_Word32 _id;
    WebRtc_Word32 _captureWidth;
    WebRtc_Word32 _captureHeight;
    WebRtc_Word32 _captureFrameRate;
    WebRtc_Word32 _frameCount;
};

} // namespace videocapturemodule
} // namespace webrtc

#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_H_
