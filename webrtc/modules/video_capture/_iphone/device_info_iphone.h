/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_H_

#include "../video_capture_impl.h"
#include "../device_info_impl.h"

#include "map_wrapper.h"

@class DeviceInfoIphoneObjC;

namespace webrtc
{
namespace videocapturemodule
{
class DeviceInfoIphone: public DeviceInfoImpl
{
public:
    DeviceInfoIphone(const WebRtc_Word32 id);
    virtual ~DeviceInfoIphone();
    WebRtc_Word32 Init();
    virtual WebRtc_UWord32 NumberOfDevices();
    virtual WebRtc_Word32 GetDeviceName(
        WebRtc_UWord32 deviceNumber,
        char* deviceNameUTF8,
        WebRtc_UWord32 deviceNameLength,
        char* deviceUniqueIdUTF8,
        WebRtc_UWord32 deviceUniqueIdUTF8Length,
        char* productUniqueIdUTF8=0,
        WebRtc_UWord32 productUniqueIdUTF8Length=0);
    virtual WebRtc_Word32 NumberOfCapabilities(const char* deviceUniqueIdUTF8);
    virtual WebRtc_Word32 GetCapability(
        const char* deviceUniqueIdUTF8,
        const WebRtc_UWord32     deviceCapabilityNumber,
        VideoCaptureCapability& capability);
    virtual WebRtc_Word32 GetBestMatchedCapability(
        const char* deviceUniqueIdUTF8,
        const VideoCaptureCapability& requested,
        VideoCaptureCapability& resulting);
    virtual WebRtc_Word32 DisplayCaptureSettingsDialogBox(
        const char* deviceUniqueIdUTF8,
        const char* dialogTitleUTF8,
        void* parentWindow,
        WebRtc_UWord32 positionX,
        WebRtc_UWord32 positionY);
    virtual WebRtc_Word32 GetOrientation(
        const char* deviceUniqueIdUTF8,
        VideoCaptureRotation& orientation);
protected:
    virtual WebRtc_Word32 CreateCapabilityMap (const char* deviceUniqueIdUTF8);
    DeviceInfoIphoneObjC*    _captureInfo;
};
} // namespace videocapturemodule
} // namespace webrtc

#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_H_
