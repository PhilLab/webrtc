/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video_capture_iphone.h"
#include "video_capture_iphone_objc.h"
#include "device_info_iphone_objc.h"
#include "trace.h"
#include "critical_section_wrapper.h"
#include "../video_capture_config.h"

namespace webrtc
{
namespace videocapturemodule
{

VideoCaptureModule* VideoCaptureImpl::Create(const WebRtc_Word32 id,
                                            const char* deviceUniqueIdUTF8)
{
    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoCapture, id,
                 "Create %s", deviceUniqueIdUTF8);

    VideoCaptureiPhone* newCaptureModule =
    new VideoCaptureiPhone(id);
    if (!newCaptureModule ||
        newCaptureModule->Init(id, deviceUniqueIdUTF8) != 0)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, id,
                     "could not Create for unique device %s",
                     deviceUniqueIdUTF8);
        std::_Destroy(newCaptureModule);
        newCaptureModule = NULL;
    }
    WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, id,
                 "VideoCaptureModule created for unique device %s",
                 deviceUniqueIdUTF8);

    return newCaptureModule;
}

VideoCaptureiPhone::VideoCaptureiPhone(const WebRtc_Word32 id) :
VideoCaptureImpl(id),
_isCapturing(false),
_id(id),
_captureWidth(0),
_captureHeight(0),
_captureFrameRate(30),
_frameCount(0)
{

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoCapture, id,
                 "%s:%d", __FUNCTION__, __LINE__);

}

VideoCaptureiPhone::~VideoCaptureiPhone()
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoCapture, _id,
                 "~VideoCaptureiPhone() called");
    if (_captureDevice)
    {
        [_captureDevice stopCapture];
    // [_captureDevice release];
    }
    if(_captureInfo)
    {
        // [_captureInfo release];
    }
}

WebRtc_Word32 VideoCaptureiPhone::Init(const WebRtc_Word32 id,
                                       const char* iDeviceUniqueIdUTF8)
{
    CriticalSectionScoped cs(_apiCs);

    WEBRTC_TRACE(kTraceModuleCall,
                 kTraceVideoCapture,
                 id, "VideoCaptureiPhone::Init() called with id %d "
                 "and unique device %s",
                 id, iDeviceUniqueIdUTF8);

    //WebRtc_Word32 result=0;

    const WebRtc_Word32 nameLength=
        (WebRtc_Word32) strlen((char*)iDeviceUniqueIdUTF8);
    if(nameLength>kVideoCaptureUniqueNameLength)
        return -1;

    // Store the device name
    _deviceUniqueId = new char[nameLength+1];
    memset(_deviceUniqueId, 0, nameLength+1);
    memcpy(_deviceUniqueId, iDeviceUniqueIdUTF8, nameLength+1);

    _captureDevice =
        [[VideoCaptureiPhoneObjC alloc] initWithVideoAPIiPhone:this AndID:_id];
    if(NULL == _captureDevice)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, id,
                     "Failed to create an instance of VideoCaptureiPhoneObjC");
        return -1;
    }

    if(0 == strcmp((char*)iDeviceUniqueIdUTF8, ""))
    {
        // the user doesn't want to set a capture device at this time
        return 0;
    }

    _captureInfo = [[DeviceInfoIphoneObjC alloc]init];
    if(nil == _captureInfo)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, id,
                     "Failed to create an instance of DeviceInfoIphoneObjC");
        return -1;
    }

    int captureDeviceCount = [[_captureInfo getCaptureDeviceCount]intValue];
    if(captureDeviceCount < 0)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture,
                     id, "No Capture Devices Present");
        return -1;
    }

    const int NAME_LENGTH = 1024;
    char deviceNameUTF8[1024] = "";
    char deviceUniqueIdUTF8[1024] = "";
    char deviceProductUniqueIDUTF8[1024] = "";

    bool captureDeviceFound = false;
    for(int index = 0; index < captureDeviceCount; index++)
    {

        memset(deviceNameUTF8, 0, NAME_LENGTH);
        memset(deviceUniqueIdUTF8, 0, NAME_LENGTH);
        memset(deviceProductUniqueIDUTF8, 0, NAME_LENGTH);

        if(-1 == [[_captureInfo getDeviceNamesFromIndex:index
            DefaultName:deviceNameUTF8 WithLength:NAME_LENGTH
            AndUniqueID:deviceUniqueIdUTF8 WithLength:NAME_LENGTH ]intValue])
        {
            WEBRTC_TRACE(kTraceError, kTraceVideoCapture, _id,
                         "GetDeviceName returned -1 for index %d", index);
            return -1;
        }
        if(0 == strcmp((const char*)iDeviceUniqueIdUTF8,
                       (char*)deviceUniqueIdUTF8))
        {
            // we have a match
            captureDeviceFound = true;
            break;
        }
    }

    if(false == captureDeviceFound)
    {
        WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _id,
                     "Failed to find capture device unique ID %s",
                     iDeviceUniqueIdUTF8);
        return -1;
    }

    // at this point we know that the user has passed in a valid camera.
    // Let's set it as the current.
    if(-1 == [_captureDevice setCaptureDeviceByName:(char*)deviceUniqueIdUTF8])
    {
        strcpy((char*)_deviceUniqueId, (char*)deviceNameUTF8);
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, _id,
                     "Failed to set capture device %s (unique ID %s) even "
                     "though it was a valid return from DeviceInfoIphone");
        return -1;
    }

    WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _id,
                 "successfully Init VideoCaptureiPhone" );

    return 0;
}

WebRtc_Word32 VideoCaptureiPhone::StartCapture(
    const VideoCaptureCapability& capability)
{
    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoCapture, _id,
                 "StartCapture width %d, height %d, frameRate %d",
                 capability.width, capability.height, capability.maxFPS);

    _captureWidth = capability.width;
    _captureHeight = capability.height;
    _captureFrameRate = capability.maxFPS;

    if(-1 == [_captureDevice setCaptureHeight:_captureHeight
                AndWidth:_captureWidth AndFrameRate:_captureFrameRate])
    {
        WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _id,
                     "Could not set width=%d height=%d frameRate=%d",
                     _captureWidth, _captureHeight, _captureFrameRate);
        return -1;
    }

    if(-1 == [_captureDevice startCapture])
    {
        return -1;
    }

    _isCapturing = true;

    return 0;
}

WebRtc_Word32 VideoCaptureiPhone::StopCapture()
{

    [_captureDevice stopCapture];

    _isCapturing = false;
    return 0;
}

bool VideoCaptureiPhone::CaptureStarted()
{
    return _isCapturing;
}

WebRtc_Word32 VideoCaptureiPhone::CaptureSettings(
    VideoCaptureCapability& settings)
{
    settings.width = _captureWidth;
    settings.height = _captureHeight;
    settings.maxFPS = _captureFrameRate;
    settings.rawType= kVideoNV12;
    return 0;
}

} // videocapturemodule
} // namespace webrtc

