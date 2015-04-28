/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_

#include "webrtc/modules/video_capture/device_info_impl.h"

#include <map>

#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

#include <ppltasks.h>

namespace webrtc {
namespace videocapturemodule {

private ref class MediaCaptureDevicesWinRT sealed
{
 private:
  MediaCaptureDevicesWinRT();

 public:
  virtual ~MediaCaptureDevicesWinRT();

  static MediaCaptureDevicesWinRT^ Instance();

 internal:
  Platform::Agile<Windows::Media::Capture::MediaCapture> GetMediaCapture(Platform::String^ deviceId);
  void RemoveMediaCapture(Platform::String^ deviceId);

 private:
   std::map<Platform::String^, Platform::Agile<Windows::Media::Capture::MediaCapture> > media_capture_map_;
   CriticalSectionWrapper* critical_section_;
};

class DeviceInfoWinRT : public DeviceInfoImpl {
 public:
  // Factory function.
  static DeviceInfoWinRT* Create(const int32_t id);
   
  explicit DeviceInfoWinRT(const int32_t id);
  virtual ~DeviceInfoWinRT();

  int32_t Init();
  virtual uint32_t NumberOfDevices();

  virtual int32_t GetDeviceName(uint32_t deviceNumber, char* deviceNameUTF8,
                                uint32_t deviceNameLength,
                                char* deviceUniqueIdUTF8,
                                uint32_t deviceUniqueIdUTF8Length,
                                char* productUniqueIdUTF8,
                                uint32_t productUniqueIdUTF8Length);

  virtual int32_t DisplayCaptureSettingsDialogBox(
      const char* deviceUniqueIdUTF8, const char* dialogTitleUTF8,
      void* parentWindow, uint32_t positionX, uint32_t positionY);

 protected:

  int32_t GetDeviceInfo(uint32_t deviceNumber,
      char* deviceNameUTF8,
      uint32_t deviceNameLength,
      char* deviceUniqueIdUTF8,
      uint32_t deviceUniqueIdUTF8Length,
      char* productUniqueIdUTF8,
      uint32_t productUniqueIdUTF8Length);

   virtual int32_t
       CreateCapabilityMap(const char* deviceUniqueIdUTF8);
};

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_
