/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#pragma mark **** imports/includes

#import <AVFoundation/AVFoundation.h>
#import "device_info_iphone_objc.h"

#pragma mark **** hidden class interface

bool                                _OSSupportedInfo;
NSArray*                            _captureDevicesInfo;
NSAutoreleasePool*                    _poolInfo;

@interface DeviceInfoIphoneObjC (hidden)
/*
bool                                _OSSupportedInfo;
NSArray*                            _captureDevicesInfo;
NSAutoreleasePool*                    _poolInfo;
*/
- (NSNumber*)getCaptureDevices;
- (NSNumber*)initializeVariables;
- (void)checkOSSupported;
@end

@implementation DeviceInfoIphoneObjC

// *********** over-written OS methods ****************************************
#pragma mark **** over-written OS methods
-(id)init{

    self = [super init];
    if(nil != self){
        [self checkOSSupported];
        [self initializeVariables];
    }
    else{
        return nil;
    }
    return self;
}

- (void)dealloc {
    [super dealloc];
//    [_pool release];
}

// ****************** public methods ******************************************
#pragma mark **** public method implementations
- (NSNumber*)displayCaptureSettingsDialogBoxWithDevice:(const WebRtc_UWord8*)deviceUniqueIdUTF8
                    AndTitle:(const WebRtc_UWord8*)dialogTitleUTF8
                    AndParentWindow:(void*) parentWindow
                    AtX:(WebRtc_UWord32)positionX
                    AndY:(WebRtc_UWord32) positionY
{
    // not currently supported
    return [NSNumber numberWithInt:-1];
}

- (NSNumber*)getCaptureDeviceCount{
  [self getCaptureDevices];
  return [NSNumber numberWithInt:[_captureDevicesInfo count]];
}


- (NSNumber*)getDeviceNamesFromIndex:(WebRtc_UWord32)index DefaultName:(char*)deviceName WithLength:(WebRtc_UWord32)deviceNameLength
                    AndUniqueID:(char*)deviceUniqueID
                    WithLength:(WebRtc_UWord32)deviceUniqueIDLength
{
    if(NO == _OSSupportedInfo){
        return [NSNumber numberWithInt:0];
    }

    if(index > (WebRtc_UWord32)[_captureDevicesInfo count] - 1){
        return [NSNumber numberWithInt:-1];
    }

    AVCaptureDevice*    tempCaptureDevice = (AVCaptureDevice*)[_captureDevicesInfo objectAtIndex:index];
    if(!tempCaptureDevice){
        return [NSNumber numberWithInt:-1];
    }

    memset(deviceName, 0, deviceNameLength);
    memset(deviceUniqueID, 0, deviceUniqueIDLength);
    bool successful = NO;

    // Get localizedName or return -1
    NSString* tempString = [tempCaptureDevice localizedName];
    successful = [tempString getCString:(char*)deviceName maxLength:deviceNameLength encoding:NSUTF8StringEncoding];
    if(NO == successful)
    {
        memset(deviceName, 0, deviceNameLength);
        return [NSNumber numberWithInt:-1];
    }

    // Get uniqueID or return -1
    tempString = [tempCaptureDevice uniqueID];
    successful = [tempString getCString:(char*)deviceUniqueID maxLength:deviceUniqueIDLength encoding:NSUTF8StringEncoding];
    if(NO == successful)
    {
        memset(deviceUniqueID, 0, deviceNameLength);
        return [NSNumber numberWithInt:-1];
    }

    return [NSNumber numberWithInt:0];

}

// ****************** hidden functions below here *****************************
#pragma mark **** hidden method implementations

- (NSNumber*)getCaptureDeviceWithIndex:(int)index ToString:(char*)name WithLength:(int)length{

    return [NSNumber numberWithInt:0];

}

- (NSNumber*)setCaptureDeviceByIndex:(int)index{

    return [NSNumber numberWithInt:-1];
}

- (NSNumber*)initializeVariables{

    return [NSNumber numberWithInt:-1];
}

/// ***** Checks to see if the QTCaptureSession framework is available in the OS.
/// ***** If it is not, isOSSupprted = NO.
/// ***** Throughout the rest of the class isOSSupprted is checked and functions are/aren't called depending
/// ***** The user can use weak linking to the QTKit framework and run on older versions of the OS
/// ***** I.E. Backwards compaitibility
/// ***** Returns nothing. Sets member variable
- (void)checkOSSupported{

    Class osSupportedTest = NSClassFromString(@"AVCaptureSession");
    _OSSupportedInfo = NO;
    if(nil == osSupportedTest){

    }

    _OSSupportedInfo = YES;

}

/// ***** Retrieves the number of capture devices currently available
/// ***** Stores them in an NSArray instance
/// ***** Returns 0 on success, -1 otherwise.
- (NSNumber*)getCaptureDevices{

    if(NO == _OSSupportedInfo){
        return [NSNumber numberWithInt:0];
    }

    if(_captureDevicesInfo){
        [_captureDevicesInfo release];
    }
    _captureDevicesInfo = [[NSArray alloc] initWithArray:[AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]];
    return [NSNumber numberWithInt:0];
}

@end









