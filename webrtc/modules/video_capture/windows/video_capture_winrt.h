#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_

#include "webrtc/modules/video_capture/video_capture_impl.h"
#include "webrtc/modules/video_capture/windows/device_info_winrt.h"

namespace webrtc {
namespace videocapturemodule {

ref class CaptureDevice;

class VideoCaptureWinRT : public VideoCaptureImpl {
 public:
  explicit VideoCaptureWinRT(const int32_t id);

  int32_t Init(const int32_t id, const char* device_id);

  // Overrides from VideoCaptureImpl.
  virtual int32_t StartCapture(const VideoCaptureCapability& capability);
  virtual int32_t StopCapture();
  virtual bool CaptureStarted();
  virtual int32_t CaptureSettings(
      VideoCaptureCapability& settings);

 protected:
  virtual ~VideoCaptureWinRT();

 private:
  Platform::String^ device_id_;
  CaptureDevice^ device_;
};

}  // namespace videocapturemodule
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_CAPTURE_WINDOWS_VIDEO_CAPTURE_WINRT_H_
