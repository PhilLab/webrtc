//
//  VideoCaptureIPhoneAVFoundationInfoObjC.cpp
//
//

#pragma mark **** imports/includes


#import "video_capture_avfoundation_info_objc.h"

#include "webrtc/system_wrappers/interface/trace.h"



#pragma mark **** hidden class interface


@implementation VideoCaptureIPhoneAVFoundationInfoObjC

// ****************** over-written OS methods ***********************
#pragma mark **** over-written OS methods

/// ***** Objective-C. Similar to C++ constructor, although invoked manually
/// ***** Potentially returns an instance of self
-(id)init{
    self = [super init];
    if(nil != self){
        [self checkOSSupported];
        [self initializeVariables];
    }
    else
    {
        return nil;
    }
    return self;
}

/// ***** Objective-C. Similar to C++ destructor
/// ***** Returns nothing
- (void)dealloc {
     webrtc::Trace::Add(webrtc::kTraceModuleCall, webrtc::kTraceVideoCapture, 0,
                  "%s:%d", __FUNCTION__, __LINE__);
}

// ****************** public methods ******************
#pragma mark **** public method implementations

/// ***** Creates a message box with Cocoa framework
/// ***** Returns 0 on success, -1 otherwise.
- (NSNumber*)displayCaptureSettingsDialogBoxWithDevice:(const char*)deviceUniqueIdUTF8
                    AndTitle:(const char*)dialogTitleUTF8
                    AndParentWindow:(void*) parentWindow
                    AtX:(uint32_t)positionX
                    AndY:(uint32_t) positionY
{
    NSString* strTitle = [NSString stringWithFormat:@"%s", dialogTitleUTF8];
    NSString* strButton = @"Alright";
    NSString* strMessage = [NSString stringWithFormat:@"Device %s is capturing", deviceUniqueIdUTF8];
    UIAlertView* alertView = [[UIAlertView alloc] initWithTitle:strTitle 
                                            message:strMessage
                                            delegate:self
                                            cancelButtonTitle:@"Cancel"
                                            otherButtonTitles:strButton,nil];
    [alertView show];
    return [NSNumber numberWithInt:0];
}

- (NSNumber*)getCaptureDeviceCount{
    [self getCaptureDevices];
    return [NSNumber numberWithInt:_captureDeviceCountInfo];
}


- (NSNumber*)getDeviceNamesFromIndex:(uint32_t)index
    DefaultName:(char*)deviceName
    WithLength:(uint32_t)deviceNameLength
    AndUniqueID:(char*)deviceUniqueID
    WithLength:(uint32_t)deviceUniqueIDLength
    AndProductID:(char*)deviceProductID
    WithLength:(uint32_t)deviceProductIDLength
{
    if(NO == _OSSupportedInfo)
    {
        return [NSNumber numberWithInt:0];
    }

    if(index > (uint32_t)_captureDeviceCountInfo)
    {
        return [NSNumber numberWithInt:-1];
    }

    AVCaptureDevice* tempCaptureDevice =
        (AVCaptureDevice*)[_captureDevicesInfo objectAtIndex:index];
    if(!tempCaptureDevice)
    {
        return [NSNumber numberWithInt:-1];
    }

    memset(deviceName, 0, deviceNameLength);
    memset(deviceUniqueID, 0, deviceUniqueIDLength);

    bool successful = NO;

    NSString* tempString = [tempCaptureDevice localizedName];
    successful = [tempString getCString:(char*)deviceName
                  maxLength:deviceNameLength encoding:NSUTF8StringEncoding];
    if(NO == successful)
    {
        memset(deviceName, 0, deviceNameLength);
        return [NSNumber numberWithInt:-1];
    }

    tempString = [tempCaptureDevice uniqueID];
    successful = [tempString getCString:(char*)deviceUniqueID
                  maxLength:deviceUniqueIDLength encoding:NSUTF8StringEncoding];
    if(NO == successful)
    {
        memset(deviceUniqueID, 0, deviceNameLength);
        return [NSNumber numberWithInt:-1];
    }

    return [NSNumber numberWithInt:0];

}

// ****************** "private" category functions below here  ******************
#pragma mark **** "private" method implementations

- (NSNumber*)getCaptureDeviceWithIndex:(int)index ToString:(char*)name
    WithLength:(int)length
{
    return [NSNumber numberWithInt:0];
}

- (NSNumber*)setCaptureDeviceByIndex:(int)index
{
    return [NSNumber numberWithInt:0];
}

- (NSNumber*)initializeVariables
{
    if(NO == _OSSupportedInfo)
    {
        return [NSNumber numberWithInt:0];
    }

    _captureDeviceCountInfo = 0;
    _frontCameraIndex = -1;
    _backCameraIndex = -1;
    [self getCaptureDevices];

    return [NSNumber numberWithInt:0];
}

// ***** Checks to see if the QTCaptureSession framework is available in the OS
// ***** If it is not, isOSSupprted = NO
// ***** Throughout the rest of the class isOSSupprted is checked and functions
// ***** are/aren't called depending
// ***** The user can use weak linking to the QTKit framework and run on older
// ***** versions of the OS
// ***** I.E. Backwards compaitibility
// ***** Returns nothing. Sets member variable
- (void)checkOSSupported
{
    Class osSupportedTest = NSClassFromString(@"AVCaptureSession");
    _OSSupportedInfo = NO;
    if(nil == osSupportedTest)
    {
    }
    _OSSupportedInfo = YES;
}

/// ***** Retrieves the number of capture devices currently available
/// ***** Stores them in an NSArray instance
/// ***** Returns 0 on success, -1 otherwise.
- (NSNumber*)getCaptureDevices
{
    if(NO == _OSSupportedInfo)
    {
        return [NSNumber numberWithInt:0];
    }

    _captureDevicesInfo = [[NSArray alloc]
                            initWithArray:[AVCaptureDevice
                                           devicesWithMediaType:AVMediaTypeVideo]];

    _captureDeviceCountInfo = _captureDevicesInfo.count;
    if(_captureDeviceCountInfo < 1){
        return [NSNumber numberWithInt:0];
    }
  
    if (_captureDeviceCountInfo == 1)
    {
        _frontCameraIndex = 0;
        _backCameraIndex = -1;
    }
    else if (_captureDeviceCountInfo >= 2)
    {
        _frontCameraIndex = 1;
        _backCameraIndex = 0;
    }
  
    return [NSNumber numberWithInt:0];
}

- (UIDeviceOrientation)getDeviceOrientation
{
    return [[UIDevice currentDevice] orientation];
}

- (NSNumber*)getCaptureDeviceIndex:(const char*)deviceUniqueIdUTF8
{
    if(NO == _OSSupportedInfo)
    {
        return [NSNumber numberWithInt:-1];
    }
  
    _captureDevicesInfo = [[NSArray alloc]
                         initWithArray:[AVCaptureDevice
                                        devicesWithMediaType:AVMediaTypeVideo]];
    AVCaptureDevice* tempCaptureDevice;
    for(int index = 0; index < (int)_captureDevicesInfo.count; index++)
    {
        tempCaptureDevice = (AVCaptureDevice*)[_captureDevicesInfo
                                               objectAtIndex:index];
        char tempCaptureDeviceUniqueID[1024] = "";
        [[tempCaptureDevice uniqueID]
        getCString:tempCaptureDeviceUniqueID maxLength:1024
        encoding:NSUTF8StringEncoding];
        if(0 == strcmp(deviceUniqueIdUTF8, tempCaptureDeviceUniqueID))
        {
            return [NSNumber numberWithInt:index];
        }
    }
    return [NSNumber numberWithInt:-1];
}

- (NSNumber*)getFrontCameraIndex
{
    return [NSNumber numberWithInt:_frontCameraIndex];
}

- (NSNumber*)getBackCameraIndex
{
    return [NSNumber numberWithInt:_backCameraIndex];
}

@end
