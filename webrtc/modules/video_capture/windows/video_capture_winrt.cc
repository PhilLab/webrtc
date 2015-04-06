/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "webrtc/modules/video_capture/windows/video_capture_winrt.h"

#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/modules/video_capture/windows/video_capture_sink_winrt.h"

#include <ppltasks.h>

extern Windows::UI::Xaml::Controls::CaptureElement^ g_capturePreview;

using Microsoft::WRL::ComPtr;
using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureFailedEventArgs;
using Windows::Media::Capture::MediaCaptureFailedEventHandler;
using Windows::Media::IMediaExtension;
using Windows::Media::MediaProperties::MediaEncodingProfile;
using Windows::Media::MediaProperties::VideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;

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

  CaptureDevice(IncomingFrameCallback* incoming_frame_callback);

  Concurrency::task<void> InitializeAsync(
      Platform::String^ deviceId);

  void CleanupSink();

  void DoCleanup();

  Concurrency::task<void> CleanupAsync();

  Concurrency::task<void> StartCaptureAsync(
      MediaEncodingProfile^ mediaEncodingProfile);

  Concurrency::task<void> StopCaptureAsync();

  void OnCaptureFailed(
      MediaCapture^ sender,
      MediaCaptureFailedEventArgs^ errorEventArgs) {
      // Forward the error to listeners.
      Failed(this, ref new CaptureFailedEventArgs(errorEventArgs->Code, errorEventArgs->Message));
  }

  void OnMediaSample(Object^ sender, MediaSampleEventArgs^ args);

  property Platform::Agile<Windows::Media::Capture::MediaCapture> MediaCapture
  {
    Platform::Agile<Windows::Media::Capture::MediaCapture> get();
  }

private:
  Platform::Agile<Windows::Media::Capture::MediaCapture> media_capture_;
  VideoCaptureMediaSinkProxyWinRT^ media_sink_;
  Windows::Foundation::EventRegistrationToken media_capture_failed_event_registration_token_;
  Windows::Foundation::EventRegistrationToken media_sink_video_sample_event_registration_token_;

  IncomingFrameCallback* incoming_frame_callback_;

  bool capture_started_;
};

CaptureDevice::CaptureDevice(IncomingFrameCallback* incoming_frame_callback)
  : media_capture_(nullptr),
    media_sink_(nullptr),
    incoming_frame_callback_(incoming_frame_callback) {
}

Concurrency::task<void> CaptureDevice::InitializeAsync(
    Platform::String^ deviceId) {
  try {
    //auto settings = ref new Windows::Media::Capture::MediaCaptureInitializationSettings();
    auto media_capture = ref new Windows::Media::Capture::MediaCapture();
    media_capture_ = media_capture;
    media_capture_failed_event_registration_token_ = media_capture->Failed +=
        ref new MediaCaptureFailedEventHandler(this, &CaptureDevice::OnCaptureFailed);
    //settings->VideoDeviceId = deviceId;
    return Concurrency::create_task(media_capture->InitializeAsync());
  } catch (Platform::Exception^ e) {
    DoCleanup();
    throw e;
  }
}

void CaptureDevice::CleanupSink() {
  if (media_sink_) {
    media_sink_->MediaSampleEvent -= media_sink_video_sample_event_registration_token_;
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
    MediaEncodingProfile^ mediaEncodingProfile)
{
  // We cannot start recording twice.
  if (media_sink_ && capture_started_) {
    throw ref new Platform::Exception(__HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
  }

  // Release sink if there is one already.
  CleanupSink();

  // Create new sink
  media_sink_ = ref new VideoCaptureMediaSinkProxyWinRT();
  media_sink_video_sample_event_registration_token_ = media_sink_->MediaSampleEvent += 
    ref new Windows::Foundation::EventHandler<MediaSampleEventArgs^>(this, &CaptureDevice::OnMediaSample);

  return Concurrency::create_task(media_sink_->InitializeAsync(mediaEncodingProfile->Video)).
     then([this, mediaEncodingProfile](IMediaExtension^ mediaExtension)
  {
    return Concurrency::create_task(media_capture_->StartRecordToCustomSinkAsync(mediaEncodingProfile, mediaExtension)).then([this](Concurrency::task<void>& asyncInfo)
    {
      try
      {
        asyncInfo.get();
        capture_started_ = true;
      }
      catch (Platform::Exception^ e)
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

void CaptureDevice::OnMediaSample(Object^ sender, MediaSampleEventArgs^ args) {
  if (incoming_frame_callback_) {
    Microsoft::WRL::ComPtr<IMFSample> spMediaSample = args->GetMediaSample();
    ComPtr<IMFMediaBuffer> spMediaBuffer;
    HRESULT hr = spMediaSample->GetBufferByIndex(0, &spMediaBuffer);
    uint8_t* videoFrame;
    size_t videoFrameLength;
    VideoCaptureCapability frameInfo;
    frameInfo.width = 640;
    frameInfo.height = 480;
    frameInfo.rawType = kVideoNV12;
    int64_t captureTime = 0;
    DWORD maxLength;
    DWORD currentLength;
    hr = spMediaBuffer->Lock(&videoFrame, &maxLength, &currentLength);
    videoFrameLength = currentLength;
    if (SUCCEEDED(hr)) {
      incoming_frame_callback_->OnIncomingFrame(videoFrame, videoFrameLength, frameInfo, captureTime);
      spMediaBuffer->Unlock();
      spMediaSample.Get()->Release();
    }
  }
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

  Concurrency::create_task(
      DeviceInformation::FindAllAsync(
          DeviceClass::VideoCapture)).then(
              [this,
              device_unique_id,
              device_unique_id_length](Concurrency::task<DeviceInformationCollection^> findTask) {
    try {
      DeviceInformationCollection^ devInfoCollection = findTask.get();
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
  }, Concurrency::task_continuation_context::use_arbitrary());

  while (device_id_ != nullptr) {
    Sleep(100);
  }

  CaptureDevice^ device = ref new CaptureDevice(this);

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

  MediaEncodingProfile^ mediaEncodingProfile =
    ref new MediaEncodingProfile();
  mediaEncodingProfile->Audio = nullptr;
  mediaEncodingProfile->Container = nullptr;
  mediaEncodingProfile->Video =
    VideoEncodingProperties::CreateUncompressed(
    MediaEncodingSubtypes::Nv12, /*capability.width*/ 640, /*capability.height*/ 480);
 
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

void VideoCaptureWinRT::OnIncomingFrame(
    uint8_t* videoFrame,
    size_t videoFrameLength,
    const VideoCaptureCapability& frameInfo,
    int64_t captureTime) {

  IncomingFrame(videoFrame, videoFrameLength, frameInfo, captureTime);
}

}  // namespace videocapturemodule
}  // namespace webrtc
