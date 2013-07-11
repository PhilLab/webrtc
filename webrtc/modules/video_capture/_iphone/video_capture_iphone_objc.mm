/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import "video_capture_iphone_objc.h"

#include "trace.h"


using namespace webrtc;
using namespace webrtc::videocapturemodule;

VideoCaptureiPhone*         _owner;
int                         _frameRate;
int                         _frameWidth;
int                         _frameHeight;
bool                        _isOSSupported;
bool                        _isRunning;
bool                        _ignoreNewFrames;
bool                        _isUsingFrontCamera;
AVCaptureSession*           _captureSession;
NSString*                   _captureQuality;
NSAutoreleasePool*          _pool;
NSRecursiveLock*            _rLock;
WebRtc_Word32               _ID;

@interface VideoCaptureiPhoneObjC (hidden)

- (bool)initializeVariables;
- (void)swapWidthAndHeight:(VideoCaptureCapability&) capability;
- (int)createCaptureSession;
- (int)changeCaptureSessionInputDevice:(NSString*)captureDeviceName;
- (int)newChangeCaptureInput:(NSString*)captureDeviceName;

@end

@implementation VideoCaptureiPhoneObjC

#pragma mark @synthesize
@synthesize frameRotation = _frameRotation;


#pragma mark Overridden NSObject functions

// custom initializer MUST be used
-(id)init{
    return nil;
}

// custom initializer MUST be used
- (id)initWithCoder:(NSCoder*)coder{
    return nil;
}

#pragma mark **** public implementations

- (id)initWithVideoAPIiPhone:(VideoCaptureiPhone*)iOwner AndID:(WebRtc_Word32)iId{
    _pool = [[NSAutoreleasePool alloc] init];

    //LOG_FUNCTION_ENTER;

    //NSString* osVersion = [[UIDevice currentDevice] systemVersion];
    _isOSSupported = YES;
    Class osSupportedTest = NSClassFromString(@"AVCaptureDevice");
    if(nil == osSupportedTest){
        _isOSSupported = NO;
        return nil;
    }
    osSupportedTest = NSClassFromString(@"AVCaptureSession");
    if(nil == osSupportedTest){
        _isOSSupported = NO;
        return nil;
    }
    osSupportedTest = NSClassFromString(@"AVCaptureDeviceInput");
    if(nil == osSupportedTest){
        _isOSSupported = NO;
        return nil;
    }
    osSupportedTest = NSClassFromString(@"AVCaptureVideoDataOutput");
    if(nil == osSupportedTest){
        _isOSSupported = NO;
        return nil;
    }

    if(self == [super init]){
        _owner = iOwner;
        _ID = iId;
        [self initializeVariables];
    }
    else{
        return nil;
    }
    //LOG_FUNCTION_LEAVE;
    return self;
}

- (int)setCaptureDeviceByName:(char*)name{

    //LOG_FUNCTION_ENTER;
    if(NO == _isOSSupported){
        return 0;
    }

    // user wants to destroy capture session
    if(!name || (0 == strcmp("", name))){
        return 0;
    }

    // check to see if the camera is already set
    NSString* newCaptureDeviceName = [NSString stringWithFormat:@"%s", name];
    if(_captureSession){
        NSArray* currentInputs = [NSArray arrayWithArray:[_captureSession inputs]];
        if(currentInputs != nil && [currentInputs count] > 0){
            AVCaptureDeviceInput* currentInput = (AVCaptureDeviceInput*)[currentInputs objectAtIndex:0];
            if(NSOrderedSame == [newCaptureDeviceName caseInsensitiveCompare:[currentInput.device localizedName]]){
                return 0;
            }
        }
        [currentInputs release];
    }

    //LOG_FUNCTION_LEAVE;
    int ret = (int)[self newChangeCaptureInput:newCaptureDeviceName];
    return ret;
}

- (int)setCaptureHeight:(int)newHeight AndWidth:(int)newWidth AndFrameRate:(int)newFrameRate{
    //LOG_FUNCTION_ENTER;
    if(NO == _isOSSupported){
        return 0;
    }

    if(!_captureSession){
        // no capture session to configure
        return -1;
    }


    // check limits
    if(newFrameRate < 1){
        return -1;
    }
    if(newFrameRate > 30){
        return -1;
    }
    if(newHeight < 0 || newHeight > SOME_MAX_LIMIT){
        return -1;
    }
    if(newWidth < 0 || newWidth > SOME_MAX_LIMIT){
        return -1;
    }

    _frameWidth = newWidth;
    _frameHeight = newHeight;
    _frameRate = newFrameRate;


    // get current capture output (before beginConfiguration)
    NSArray* currentOutputs = [_captureSession outputs];
    if(0 == [currentOutputs count]){
        return -1;
        // no outputs present
    }

    AVCaptureVideoDataOutput* currentOutput = (AVCaptureVideoDataOutput*)[currentOutputs objectAtIndex:0];

    // Currently, the iPhones have only 3 capture capabilites. Filter for the more appropriate one
    if(_frameWidth > 640 && _frameHeight > 480){
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetHigh];
    }
    else if(_frameWidth > 480 && _frameHeight > 360){
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetMedium];
    }
    else{   // from  480*360 -> 0
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetLow];
    }

    // begin configuration for the AVCaptureSession
    [_captureSession beginConfiguration];

    // picture resolution
    [_captureSession setSessionPreset:_captureQuality];

    // take care of capture framerate now
    CMTime cmTime = {1,
                _frameRate,
                kCMTimeFlags_Valid,
                0};
    //currentOutput.minFrameDuration = cmTime; // minFrameDuration is deprecated
    AVCaptureConnection* connection = [currentOutput connectionWithMediaType:AVMediaTypeVideo];
    [connection setVideoMinFrameDuration:cmTime];

    // finished configuring, commit settings to AVCaptureSession.
    [_captureSession commitConfiguration];

    return 0;
}

- (int)startCapture{

    //LOG_FUNCTION_ENTER;

    if(NO == _isOSSupported){
        return 0;
    }

    if(nil == _captureSession){
        return -1;
    }

    [_captureSession startRunning];

    _isRunning = YES;
    //LOG_FUNCTION_LEAVE;

    return 0;

}

- (int)stopCapture{

    //LOG_FUNCTION_ENTER;

    if(NO == _isOSSupported){
        return 0;
    }

    if(nil == _captureSession){
        return -1;
    }


    [_captureSession stopRunning];

    _isRunning = NO;

    //LOG_FUNCTION_LEAVE;
    return 0;

}

// ****************************************** private functions below here ************************************************
#pragma mark **** private function implementations

- (bool)initializeVariables{

    //LOG_FUNCTION_ENTER;

    if(NO == _isOSSupported){
        return YES;
    }

    _rLock = [[NSRecursiveLock alloc]init];

    _isRunning = NO;
    _captureQuality = AVCaptureSessionPresetLow;
    _frameRate = DEFAULT_FRAME_RATE;
    _captureSession = nil;

    [self createCaptureSession];



    //LOG_FUNCTION_LEAVE;

    return YES;
}

- (int)createCaptureSession{
    // destroy/recreate capture session
    if(_captureSession){
        [_captureSession release];
        _captureSession = nil;
    }

    _captureSession = [[AVCaptureSession alloc] init];

    // create and configure a new output (using callbacks)
    AVCaptureVideoDataOutput* newCaptureOutput = [[AVCaptureVideoDataOutput alloc] init];
    [newCaptureOutput setSampleBufferDelegate:self queue:dispatch_get_current_queue()];
    NSString* key = (NSString*)kCVPixelBufferPixelFormatTypeKey;
    NSNumber* val = [NSNumber numberWithUnsignedInt:kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange]; // Bi-Planar Component Y' CbCr 8-bit 4:2:0, video-range (luma=[16,235] chroma=[16,240]).  baseAddr points to a big-endian CVPlanarPixelBufferInfo_YCbCrBiPlanar struct
    NSDictionary* videoSettings = [NSDictionary dictionaryWithObject:val forKey:key];
    [newCaptureOutput setVideoSettings:videoSettings];

    // Currently, the iPhones have only 3 capture capabilites. Filter for the more appropriate one
    if(_frameWidth > 640 && _frameHeight > 480){
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetHigh];
        //[_captureSession setSessionPreset:AVCaptureSessionPresetHigh];
    }
    else if(_frameWidth > 480 && _frameHeight > 360){
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetMedium];
        //[_captureSession setSessionPreset:AVCaptureSessionPresetMedium];
    }
    else{   // from  480*360 -> 0
        _captureQuality = [NSString stringWithString:AVCaptureSessionPresetLow];
        //[_captureSession setSessionPreset:AVCaptureSessionPresetLow];
    }

    // ************************************  Begin session configuration *****
    [_captureSession beginConfiguration];

    // add new output
    if([_captureSession canAddOutput:newCaptureOutput]){
        [_captureSession addOutput:newCaptureOutput];
        WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _ID, "Added AVCaptureSession output (callback) ", __FILE__, __FUNCTION__, __LINE__);
    }
    else{
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, _ID, "%s:%s:%d Could not add output to AVCaptureSession ", __FILE__, __FUNCTION__, __LINE__);
    }

    // set picture quality
    [_captureSession setSessionPreset:_captureQuality];

    [_captureSession commitConfiguration];
    // ************************************  End session configuration *****

    return 0;
}

- (int)newChangeCaptureInput:(NSString*)captureDeviceName{
    if(!_captureSession){
        return -1;
    }

    // ************************************************ get current input, if any, and remove it
    // GET current input so we can remove it from the session
    bool hasCurrentInput = YES;
    NSArray* currentInputs = [_captureSession inputs];
    if([currentInputs count] <= 0){
        hasCurrentInput = NO;
    }

    // remove current input
    if(YES == hasCurrentInput){
        AVCaptureInput* currentInput = (AVCaptureInput*)[currentInputs objectAtIndex:0];
        WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _ID, "%s:%s:%d Removing AVCaptureSession input", __FILE__, __FUNCTION__, __LINE__);
        [_captureSession removeInput:currentInput];
    }
    else{
        WEBRTC_TRACE(kTraceDebug, kTraceVideoCapture, _ID, "%s:%s:%d There is no AVCaptureSession input to remove", __FILE__, __FUNCTION__, __LINE__);
    }

    // ************************************************ Look for input device with the name requested (as our input param)
    // get list of available capture devices
    NSArray* captureDevices = [[NSArray alloc] initWithArray:[AVCaptureDevice devicesWithMediaType:AVMediaTypeVideo]];
    int captureDeviceCount = [captureDevices count];
    if(captureDeviceCount <= 0){
        return -1;
    }

    // create new input from param NSString captureDeviceName
    bool captureDeviceFound = NO;
    AVCaptureDevice* captureDevice = nil;
    for(NSUInteger index = 0; index < [captureDevices count]; index++){
        captureDevice = (AVCaptureDevice*)[captureDevices objectAtIndex:index];
        // lazy checking. If input param (capture device) matched either localized name OR uniqueID, then yes.
        if(NSOrderedSame == [captureDeviceName caseInsensitiveCompare:[captureDevice localizedName]]){
            captureDeviceFound = YES;
            break;
        }
        if(NSOrderedSame == [captureDeviceName caseInsensitiveCompare:[captureDevice uniqueID]]){
            captureDeviceFound = YES;
            break;
        }
    }
    if(NO == captureDeviceFound){
        // no device was found that matches param 1
        return -1;
    }

    // we have found our capture device. Get Cstring for  logging
    const int nameLength = 1024;
    char captureDeviceNameUTF8[nameLength] = "";
    [[captureDevice localizedName] getCString:captureDeviceNameUTF8 maxLength:nameLength encoding:NSUTF8StringEncoding];
    WEBRTC_TRACE(kTraceDebug, kTraceVideoCapture, _ID, "%s:%d captureDevice=%s", __FUNCTION__, __LINE__, captureDeviceNameUTF8);

    // now create capture session input out of AVCaptureDevice
    AVCaptureDeviceInput* newCaptureInput = [AVCaptureDeviceInput deviceInputWithDevice:captureDevice error:nil];

    // ************************************************ try to add our new capture device to the capture session
    [_captureSession beginConfiguration];

    bool addedCaptureInput = NO;
    if([_captureSession canAddInput:newCaptureInput]){
        [_captureSession addInput:newCaptureInput];
        addedCaptureInput = YES;
        WEBRTC_TRACE(kTraceInfo, kTraceVideoCapture, _ID, "%s:%s:%d Added new AVCaptureSession input", __FILE__, __FUNCTION__, __LINE__);
    }
    else{
        WEBRTC_TRACE(kTraceError, kTraceVideoCapture, _ID, "%s:%s:%d Could not add new AVCaptureSession input", __FILE__, __FUNCTION__, __LINE__);
        addedCaptureInput = NO;
    }

    [_captureSession setSessionPreset:_captureQuality];

    [_captureSession commitConfiguration];

    // set class variable so we know how to rotate the incoming frames depending on which camera is being used
    NSString* frontCameraName = [NSString stringWithFormat:@"Front Camera"];
    if(NSOrderedSame == [frontCameraName caseInsensitiveCompare:[newCaptureInput.device localizedName]]){
        _isUsingFrontCamera = YES;
    }
    else{
        _isUsingFrontCamera = NO;
    }

    return 0;
}

-(void)captureOutput:(AVCaptureOutput *)captureOutput didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer fromConnection:(AVCaptureConnection *)connection{
    //LOG_FUNCTION_ENTER;
    if(NO == _isOSSupported){
        //LOG_FUNCTION_LEAVE;
        return;
    }

    [_rLock lock];

    NSLock* tLock = [[NSLock alloc]init];
    [tLock lock];
    const int Y_BUFFER = 0;
    //const int CBCR_BUFFER = 1;

    //int BUFFER = Y_BUFFER;

    CVImageBufferRef yBuffer = CMSampleBufferGetImageBuffer(sampleBuffer);

    // Lock the frame, get properties etc....
    CVPixelBufferLockBaseAddress(yBuffer, Y_BUFFER);

    uint8_t* baseAddress = (uint8_t*)CVPixelBufferGetBaseAddressOfPlane(yBuffer, Y_BUFFER);
    //size_t bytesPerRow = CVPixelBufferGetBytesPerRow(yBuffer);
    _frameWidth = CVPixelBufferGetWidth(yBuffer);
    _frameHeight = CVPixelBufferGetHeight(yBuffer);

    int frameSize = _frameWidth * _frameHeight * 1.5;
    VideoCaptureCapability tempCaptureCapability;
    tempCaptureCapability.width = _frameWidth;
    tempCaptureCapability.height = _frameHeight;
    tempCaptureCapability.maxFPS = _frameRate;
    tempCaptureCapability.rawType = kVideoNV12;

    _owner->IncomingFrame(baseAddress, frameSize, tempCaptureCapability, 0);

    CVPixelBufferUnlockBaseAddress(yBuffer, Y_BUFFER);


    [_rLock unlock];
}

- (void)swapWidthAndHeight:(VideoCaptureCapability&)capability{
    WebRtc_Word32 temp;
    temp = capability.width;
    capability.width = capability.height;
    capability.height = temp;
}

- (void)dealloc {

    [_rLock unlock];
    [super dealloc];
    [_pool release];
}

@end
