/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_OBJC_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_OBJC_H_

#define DEFAULT_FRAME_RATE              1

#import <Foundation/Foundation.h>
#import <CoreGraphics/CoreGraphics.h>
#import <UIKit/UIKit.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <CoreMedia/CoreMedia.h>

#include "video_capture_iphone.h"

#define SOME_MAX_LIMIT 100000000
#define SOME_MIN_LIMIT 0

@interface VideoCaptureiPhoneObjC : UIViewController <AVCaptureVideoDataOutputSampleBufferDelegate>{
    webrtc::VideoCaptureRotation _frameRotation;
}

@property webrtc::VideoCaptureRotation frameRotation;

// custom initializer. Instance of VideoCaptureiPhone is needed for callback purposes.
// default init methods have been overridden to return nil.
- (id)initWithVideoAPIiPhone:(webrtc::videocapturemodule::VideoCaptureiPhone*)iOwner AndID:(WebRtc_Word32)iId;
- (int)setCaptureDeviceByName:(char*)name;
- (int)setCaptureHeight:(int)height AndWidth:(int)width AndFrameRate:(int)frameRate;
- (int)stopCapture;
- (int)startCapture;

@end
#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_IPHONE_VIDEO_CAPTURE_IPHONE_OBJC_H_
