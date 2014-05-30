/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "trace.h"
#include "../../video_capture_config.h"
#include "video_capture_avfoundation_info_objc.h"

#include "../../include/video_capture.h"

namespace webrtc
{
namespace videocapturemodule
{

VideoCaptureIPhoneAVFoundationInfo::VideoCaptureIPhoneAVFoundationInfo(const int32_t id) :
    DeviceInfoImpl(id)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    _captureInfo = [[VideoCaptureIPhoneAVFoundationInfoObjC alloc] init];
}

VideoCaptureIPhoneAVFoundationInfo::~VideoCaptureIPhoneAVFoundationInfo()
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    [_captureInfo release];

}

int32_t VideoCaptureIPhoneAVFoundationInfo::Init()
{

    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    return 0;
}

uint32_t VideoCaptureIPhoneAVFoundationInfo::NumberOfDevices()
{

    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    uint32_t captureDeviceCount =
        [[_captureInfo getCaptureDeviceCount]intValue];
    return captureDeviceCount;

}

int32_t VideoCaptureIPhoneAVFoundationInfo::GetDeviceName(
    uint32_t deviceNumber, char* deviceNameUTF8,
    uint32_t deviceNameLength, char* deviceUniqueIdUTF8,
    uint32_t deviceUniqueIdUTF8Length, char* productUniqueIdUTF8,
    uint32_t productUniqueIdUTF8Length)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    int errNum = [[_captureInfo getDeviceNamesFromIndex:deviceNumber
                   DefaultName:deviceNameUTF8 WithLength:deviceNameLength
                   AndUniqueID:deviceUniqueIdUTF8
                   WithLength:deviceUniqueIdUTF8Length
                   AndProductID:productUniqueIdUTF8
                   WithLength:productUniqueIdUTF8Length]intValue];
    return errNum;
}

int32_t VideoCaptureIPhoneAVFoundationInfo::NumberOfCapabilities(
    const char* deviceUniqueIdUTF8)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    // Not implemented. iPhone doesn't use discrete steps in capabilities, rather
    // "analog". QTKit will do it's best to convert frames to what ever format
    // you ask for.
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                 "NumberOfCapabilities is not supported on the iPhone platform.");
    return -1;
}


int32_t VideoCaptureIPhoneAVFoundationInfo::GetCapability(
    const char* deviceUniqueIdUTF8,
    const uint32_t deviceCapabilityNumber,
    VideoCaptureCapability& capability)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    // Not implemented. iPhone doesn't use discrete steps in capabilities, rather
    // "analog". QTKit will do it's best to convert frames to what ever format
    // you ask for.
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                 "NumberOfCapabilities is not supported on the iPhone platform.");
    return -1;
}


int32_t VideoCaptureIPhoneAVFoundationInfo::GetBestMatchedCapability(
    const char*deviceUniqueIdUTF8,
    const VideoCaptureCapability& requested, VideoCaptureCapability& resulting)
{
    // Not implemented. iPhone doesn't use discrete steps in capabilities, rather
    // "analog". QTKit will do it's best to convert frames to what ever format
    // you ask for.
    WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
                 "NumberOfCapabilities is not supported on the iPhone platform.");
    return -1;
}
  
int32_t VideoCaptureIPhoneAVFoundationInfo::GetOrientation(const char* deviceUniqueIdUTF8,
                                                                 VideoCaptureRotation& orientation)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
  

    if (!_orientationLock)
    {
        NSNumber* deviceIndex = [_captureInfo getCaptureDeviceIndex:deviceUniqueIdUTF8];
        UIDeviceOrientation deviceOrientation = [_captureInfo getDeviceOrientation];
        if ([deviceIndex isEqualToNumber:[_captureInfo getFrontCameraIndex]])
        {
            if (deviceOrientation == UIDeviceOrientationLandscapeLeft)
            {
                orientation = kCameraRotate180;
            }
            else if (deviceOrientation == UIDeviceOrientationPortraitUpsideDown)
            {
                orientation = kCameraRotate270;
            }
            else if (deviceOrientation == UIDeviceOrientationLandscapeRight)
            {
                orientation = kCameraRotate0;
            }
            else if (deviceOrientation == UIDeviceOrientationPortrait)
            {
                orientation = kCameraRotate90;
            }
            else
            {
                if (_defaultOrientation == kOrientationLandscapeLeft)
                    orientation = kCameraRotate180;
                else if (_defaultOrientation == kOrientationPortraitUpsideDown)
                    orientation = kCameraRotate270;
                else if (_defaultOrientation == kOrientationLandscapeRight)
                    orientation = kCameraRotate0;
                else if (_defaultOrientation == kOrientationPortrait)
                    orientation = kCameraRotate90;
                else
                    orientation = kCameraRotate180;
            }
        }
        else if ([deviceIndex isEqualToNumber:[_captureInfo getBackCameraIndex]])
        {
            if (deviceOrientation == UIDeviceOrientationLandscapeLeft)
            {
                orientation = kCameraRotate0;
            }
            else if (deviceOrientation == UIDeviceOrientationPortraitUpsideDown)
            {
                orientation = kCameraRotate270;
            }
            else if (deviceOrientation == UIDeviceOrientationLandscapeRight)
            {
                orientation = kCameraRotate180;
            }
            else if (deviceOrientation == UIDeviceOrientationPortrait)
            {
                orientation = kCameraRotate90;
            }
            else
            {
                if (_defaultOrientation == kOrientationLandscapeLeft)
                    orientation = kCameraRotate0;
                else if (_defaultOrientation == kOrientationPortraitUpsideDown)
                    orientation = kCameraRotate270;
                else if (_defaultOrientation == kOrientationLandscapeRight)
                    orientation = kCameraRotate180;
                else if (_defaultOrientation == kOrientationPortrait)
                    orientation = kCameraRotate90;
                else
                    orientation = kCameraRotate0;
            }
        }
        else
        {
            orientation = kCameraRotate0;
        }
    }
    else
    {
        NSNumber* deviceIndex = [_captureInfo getCaptureDeviceIndex:deviceUniqueIdUTF8];
        if ([deviceIndex isEqualToNumber:[_captureInfo getFrontCameraIndex]])
        {
            if (_lockedOrientation == kOrientationLandscapeLeft)
                orientation = kCameraRotate180;
            else if (_lockedOrientation == kOrientationPortraitUpsideDown)
                orientation = kCameraRotate270;
            else if (_lockedOrientation == kOrientationLandscapeRight)
                orientation = kCameraRotate0;
            else if (_lockedOrientation == kOrientationPortrait)
                orientation = kCameraRotate90;
            else
                orientation = kCameraRotate180;
        }
        else if ([deviceIndex isEqualToNumber:[_captureInfo getBackCameraIndex]])
        {
            if (_lockedOrientation == kOrientationLandscapeLeft)
                orientation = kCameraRotate0;
            else if (_lockedOrientation == kOrientationPortraitUpsideDown)
                orientation = kCameraRotate270;
            else if (_lockedOrientation == kOrientationLandscapeRight)
                orientation = kCameraRotate180;
            else if (_lockedOrientation == kOrientationPortrait)
                orientation = kCameraRotate90;
            else
                orientation = kCameraRotate0;
        }
        else
        {
            orientation = kCameraRotate0;
        }
    }

    return 0;
}

int32_t VideoCaptureIPhoneAVFoundationInfo::DisplayCaptureSettingsDialogBox(
    const char* deviceUniqueIdUTF8,
    const char* dialogTitleUTF8, void* parentWindow,
    uint32_t positionX, uint32_t positionY)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);

    return [[_captureInfo
             displayCaptureSettingsDialogBoxWithDevice:deviceUniqueIdUTF8
             AndTitle:dialogTitleUTF8
             AndParentWindow:parentWindow AtX:positionX AndY:positionY]
             intValue];
}

int32_t VideoCaptureIPhoneAVFoundationInfo::CreateCapabilityMap(
    const char* deviceUniqueIdUTF8)
{
    WEBRTC_TRACE(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                 "%s:%d", __FUNCTION__, __LINE__);
    // Not implemented. iPhone doesn't use discrete steps in capabilities, rather
    // "analog". QTKit will do it's best to convert frames to what ever format
    // you ask for.
    WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
                 "NumberOfCapabilities is not supported on the iPhone platform.");
     return -1;
}
}  // namespace videocapturemodule
}  // namespace webrtc
