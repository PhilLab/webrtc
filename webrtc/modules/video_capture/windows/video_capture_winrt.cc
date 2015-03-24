#include "webrtc/modules/video_capture/windows/video_capture_winrt.h"

namespace webrtc {
namespace videocapturemodule {

VideoCaptureWinRT::VideoCaptureWinRT(const int32_t id) : VideoCaptureImpl(id) {}
VideoCaptureWinRT::~VideoCaptureWinRT() {}

int32_t VideoCaptureWinRT::Init(const int32_t id, const char* device_id) {
  return 0;
}

int32_t VideoCaptureWinRT::StartCapture(
    const VideoCaptureCapability& capability) {
  return -1;
}

int32_t VideoCaptureWinRT::StopCapture() {
  return -1;
}

bool VideoCaptureWinRT::CaptureStarted() {
  return false;
}

int32_t VideoCaptureWinRT::CaptureSettings(
    VideoCaptureCapability& settings) {
  return -1;
}

}  // namespace videocapturemodule
}  // namespace webrtc
