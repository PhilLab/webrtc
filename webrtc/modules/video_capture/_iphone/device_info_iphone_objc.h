/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_OBJC_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_OBJC_H_

#import <Foundation/Foundation.h>
#include "device_info_iphone.h"

@interface DeviceInfoIphoneObjC : NSObject
{
}

/**************************************************************************
 *
 *   The following functions are called by DeviceInfoIphone class
 *
 ***************************************************************************/

- (NSNumber*)getCaptureDeviceCount;

- (NSNumber*)getDeviceNamesFromIndex:(WebRtc_UWord32)index DefaultName:(char*)deviceName WithLength:(WebRtc_UWord32)deviceNameLength
                                      AndUniqueID:(char*)deviceUniqueID WithLength:(WebRtc_UWord32)deviceUniqueIDLength;

- (NSNumber*)displayCaptureSettingsDialogBoxWithDevice:(const WebRtc_UWord8*)deviceUniqueIdUTF8
                            AndTitle:(const WebRtc_UWord8*)dialogTitleUTF8
                            AndParentWindow:(void*) parentWindow
                            AtX:(WebRtc_UWord32)positionX
                            AndY:(WebRtc_UWord32) positionY;
@end

#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_DEVICE_INFO_IPHONE_OBJC_H__
