#include "webrtc/modules/video_capture/windows/video_capture_winrt.h"

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

  Concurrency::task<void> InitializeAsync();

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

Concurrency::task<void> CaptureDevice::InitializeAsync() {
  try {
    auto media_capture = ref new Windows::Media::Capture::MediaCapture();
    media_capture_ = media_capture;
    media_capture_failed_event_registration_token_ = media_capture->Failed +=
        ref new Windows::Media::Capture::MediaCaptureFailedEventHandler(this, &CaptureDevice::OnCaptureFailed);

    return Concurrency::create_task(media_capture->InitializeAsync());
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

int32_t VideoCaptureWinRT::Init(const int32_t id, const char* device_id) {

  CaptureDevice^ device = ref new CaptureDevice();

  device->InitializeAsync().then([this, device](Concurrency::task<void> asyncInfo)
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
    Windows::Media::MediaProperties::MediaEncodingSubtypes::Nv12, 640, 480);
 
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
