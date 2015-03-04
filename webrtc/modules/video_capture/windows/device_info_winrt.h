#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_

#include "webrtc/modules/video_capture/device_info_impl.h"

namespace webrtc {
namespace videocapturemodule {

class DeviceInfoWinRT : public DeviceInfoImpl {
 public:
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
};

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_DEVICE_INFO_WINRT_H_
