#include "webrtc/modules/video_capture/windows/device_info_winrt.h"

namespace webrtc {
namespace videocapturemodule {

// static
DeviceInfoWinRT* DeviceInfoWinRT::Create(const int32_t id) {
  DeviceInfoWinRT* winrtInfo = new DeviceInfoWinRT(id);
  if (!winrtInfo || winrtInfo->Init() != 0) {
    delete winrtInfo;
    winrtInfo = NULL;
  }
  return winrtInfo;
}

DeviceInfoWinRT::DeviceInfoWinRT(const int32_t id) : DeviceInfoImpl(id) {
}

DeviceInfoWinRT::~DeviceInfoWinRT() {
}

int32_t DeviceInfoWinRT::Init() {
  return 0;
}

uint32_t DeviceInfoWinRT::NumberOfDevices() {
  return 1;
}

int32_t DeviceInfoWinRT::GetDeviceName(
    uint32_t deviceNumber,
    char* deviceNameUTF8,
    uint32_t deviceNameLength,
    char* deviceUniqueIdUTF8,
    uint32_t deviceUniqueIdUTF8Length,
    char* productUniqueIdUTF8,
    uint32_t productUniqueIdUTF8Length) {
  deviceNameUTF8[0] = 'a';
  deviceNameUTF8[1] = 0;
  deviceUniqueIdUTF8[0] = 'a';
  deviceUniqueIdUTF8[1] = 0;
  return 0;
}

int32_t DeviceInfoWinRT::DisplayCaptureSettingsDialogBox(
    const char* deviceUniqueIdUTF8,
    const char* dialogTitleUTF8,
    void* parentWindow,
    uint32_t positionX,
    uint32_t positionY) {
  return -1;
}

int32_t DeviceInfoWinRT::CreateCapabilityMap(
  const char* deviceUniqueIdUTF8) {
  return 0;
}

}  // namespace videocapturemodule
}  // namespace webrtc
