#include "webrtc/modules/video_capture/windows/video_capture_winrt.h"

#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/modules/video_capture/windows/video_capture_sink_winrt.h"

#include <ppltasks.h>

extern Windows::UI::Xaml::Controls::CaptureElement^ g_capturePreview;

namespace webrtc {
namespace videocapturemodule {

ref class CaptureFailedEventArgs {
 internal:
  CaptureFailedEventArgs(HRESULT code, Platform::String^ message)
    : code_(code),
      message_(message) {
  }

  property HRESULT Code {
    HRESULT get() {
      return code_;
    }
  }

  property Platform::String^ Message {
    Platform::String^ get() {
      return message_;
    }
  }

 private:
  HRESULT code_;
  Platform::String^ message_;
};

ref class CaptureDevice;

delegate void CaptureFailedHandler(
    CaptureDevice^ sender,
    CaptureFailedEventArgs^ errorEventArgs);

ref class CaptureDevice sealed {

 internal:
  event CaptureFailedHandler^ Failed;

  CaptureDevice();

  Concurrency::task<void> InitializeAsync(
      Platform::String^ deviceId);

  void CleanupSink();

  void DoCleanup();

  Concurrency::task<void> CleanupAsync();

  Concurrency::task<void> StartCaptureAsync(
      Windows::Media::MediaProperties::MediaEncodingProfile^ mediaEncodingProfile);

  Concurrency::task<void> StopCaptureAsync();

  void OnCaptureFailed(
      Windows::Media::Capture::MediaCapture^ sender,
      Windows::Media::Capture::MediaCaptureFailedEventArgs^ errorEventArgs) {
      // Forward the error to listeners.
      Failed(this, ref new CaptureFailedEventArgs(errorEventArgs->Code, errorEventArgs->Message));
  }

  property Platform::Agile<Windows::Media::Capture::MediaCapture> MediaCapture
  {
    Platform::Agile<Windows::Media::Capture::MediaCapture> get();
  }

private:
  Platform::Agile<Windows::Media::Capture::MediaCapture> media_capture_;
  VideoCaptureMediaSinkProxyWinRT^ media_sink_;
  Windows::Foundation::EventRegistrationToken media_capture_failed_event_registration_token_;

  bool capture_started_;
};

CaptureDevice::CaptureDevice()
  : media_capture_(nullptr) {
}

Concurrency::task<void> CaptureDevice::InitializeAsync(
    Platform::String^ deviceId) {
  try {
    auto settings = ref new Windows::Media::Capture::MediaCaptureInitializationSettings();
    auto media_capture = ref new Windows::Media::Capture::MediaCapture();
    media_capture_ = media_capture;
    media_capture_failed_event_registration_token_ = media_capture->Failed +=
        ref new Windows::Media::Capture::MediaCaptureFailedEventHandler(this, &CaptureDevice::OnCaptureFailed);
    settings->VideoDeviceId = deviceId;
    return Concurrency::create_task(media_capture->InitializeAsync(settings));
  } catch (Platform::Exception^ e) {
    DoCleanup();
    throw e;
  }
}

void CaptureDevice::CleanupSink() {
  if (media_sink_) {
    delete media_sink_;
    media_sink_ = nullptr;
    capture_started_ = false;
  }
}

void CaptureDevice::DoCleanup() {
  Windows::Media::Capture::MediaCapture ^mediaCapture = media_capture_.Get();
  if (mediaCapture != nullptr) {
    mediaCapture->Failed -= media_capture_failed_event_registration_token_;
  }

  CleanupSink();
}

Platform::Agile<Windows::Media::Capture::MediaCapture>
    CaptureDevice::MediaCapture::get()
{
  return media_capture_;
}

Concurrency::task<void> CaptureDevice::CleanupAsync()
{
  Windows::Media::Capture::MediaCapture ^mediaCapture = media_capture_.Get();
  if (mediaCapture == nullptr && !media_sink_)
  {
    return Concurrency::create_task([](){});
  }

  if (mediaCapture != nullptr)
  {
    mediaCapture->Failed -= media_capture_failed_event_registration_token_;
  }

  if (mediaCapture != nullptr && capture_started_)
  {
    return Concurrency::create_task(
        mediaCapture->StopRecordAsync()).then([this](Concurrency::task<void>&)
    {
      DoCleanup();
    });
  }
  else
  {
    DoCleanup();
  }
  return Concurrency::create_task([](){});
}

Concurrency::task<void> CaptureDevice::StartCaptureAsync(
    Windows::Media::MediaProperties::MediaEncodingProfile^ mediaEncodingProfile)
{
  // We cannot start recording twice.
  if (media_sink_ && capture_started_) {
    throw ref new Platform::Exception(__HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
  }

  // Release sink if there is one already.
  CleanupSink();

  // Create new sink
  media_sink_ = ref new VideoCaptureMediaSinkProxyWinRT();

  return Concurrency::create_task(media_sink_->InitializeAsync(mediaEncodingProfile->Video)).
     then([this, mediaEncodingProfile](Windows::Media::IMediaExtension^ mediaExtension)
  {
    return Concurrency::create_task(media_capture_->StartRecordToCustomSinkAsync(mediaEncodingProfile, mediaExtension)).then([this](Concurrency::task<void>& asyncInfo)
    {
      try
      {
        asyncInfo.get();
        capture_started_ = true;
      }
      catch (Platform::Exception^)
      {
        CleanupSink();
        throw;
      }
    });
  });
}

Concurrency::task<void> CaptureDevice::StopCaptureAsync()
{
  if (capture_started_)
  {
    return Concurrency::create_task(media_capture_.Get()->StopRecordAsync()).then([this]()
    {
      CleanupSink();
    });
  }
  else
  {
    // If recording not started just do nothing
  }
  return Concurrency::create_task([](){});
}

VideoCaptureWinRT::VideoCaptureWinRT(const int32_t id)
  : VideoCaptureImpl(id),
    device_(nullptr) {
}

VideoCaptureWinRT::~VideoCaptureWinRT() {
}

int32_t VideoCaptureWinRT::Init(const int32_t id, const char* device_unique_id) {

  const int32_t device_unique_id_length =
    (int32_t)strlen((char*)device_unique_id);
  if (device_unique_id_length > kVideoCaptureUniqueNameLength)
  {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
      "Device name too long");
    return -1;
  }
  WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
    "Init called for device %s", device_unique_id);

  device_id_ = nullptr;

  auto findAllTask = Concurrency::create_task(
      Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(
          Windows::Devices::Enumeration::DeviceClass::VideoCapture)).then(
              [this,
              device_unique_id,
              device_unique_id_length](Concurrency::task<Windows::Devices::Enumeration::DeviceInformationCollection^> findTask) {
    try {
      Windows::Devices::Enumeration::DeviceInformationCollection^ devInfoCollection = findTask.get();
      if (devInfoCollection == nullptr || devInfoCollection->Size == 0) {
        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
          "No video capture device found");
      }
      for (unsigned int i = 0; i < devInfoCollection->Size; i++) {
        auto devInfo = devInfoCollection->GetAt(i);
        Platform::String^ deviceUniqueId = devInfo->Id;
        char currentDeviceUniqueIdUTF8[256];
        currentDeviceUniqueIdUTF8[0] = 0;
        WideCharToMultiByte(CP_UTF8, 0, deviceUniqueId->Begin(), -1,
          currentDeviceUniqueIdUTF8,
          sizeof(currentDeviceUniqueIdUTF8), NULL,
          NULL);
        if (strncmp(currentDeviceUniqueIdUTF8,
          (const char*)device_unique_id,
          device_unique_id_length) == 0) {
          device_id_ = devInfo->Id;
          break;
        }
      }
    }
    catch (Platform::Exception^ e) {
    }
  });

  findAllTask.wait();

  CaptureDevice^ device = ref new CaptureDevice();

  device->InitializeAsync(device_id_).then([this, device](Concurrency::task<void> asyncInfo)
  {
    try
    {
      asyncInfo.get();
    }
    catch (Platform::Exception^ e)
    {
      if (device_)
      {
        device_->CleanupAsync().then([this](){});
        device_id_ = nullptr;
        device_ = nullptr;
      }
      return Concurrency::create_task([]{});
    }

    device_ = device;

    return Concurrency::create_task([]{});
  });

  return 0;
}

int32_t VideoCaptureWinRT::StartCapture(
    const VideoCaptureCapability& capability) {

  g_capturePreview->Source = device_->MediaCapture.Get();

  Concurrency::create_task(device_->MediaCapture.Get()->StartPreviewAsync()).then([this](Concurrency::task<void> asyncInfo)
  {
    try
    {
      asyncInfo.get();
    }
    catch (Platform::Exception^ e)
    {
      return Concurrency::create_task([]{});
    }

    return Concurrency::create_task([]{});
  });

  Windows::Media::MediaProperties::MediaEncodingProfile^ mediaEncodingProfile =
    ref new Windows::Media::MediaProperties::MediaEncodingProfile();
  mediaEncodingProfile->Audio = nullptr;
  mediaEncodingProfile->Container = nullptr;
  mediaEncodingProfile->Video =
    Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(
    Windows::Media::MediaProperties::MediaEncodingSubtypes::Nv12, capability.width, capability.height);
 
  device_->StartCaptureAsync(mediaEncodingProfile).then([this](Concurrency::task<void> asyncInfo)
  {
    try
    {
      asyncInfo.get();
    }
    catch (Platform::Exception^ e)
    {
      return Concurrency::create_task([]{});
    }

    return Concurrency::create_task([]{});
  });

  return 0;
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
