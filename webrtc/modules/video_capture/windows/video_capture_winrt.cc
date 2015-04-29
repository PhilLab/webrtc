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

using Microsoft::WRL::ComPtr;
using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureInitializationSettings;
using Windows::Media::Capture::MediaCaptureFailedEventArgs;
using Windows::Media::Capture::MediaCaptureFailedEventHandler;
using Windows::Media::IMediaExtension;
using Windows::Media::MediaProperties::MediaEncodingProfile;
using Windows::Media::MediaProperties::VideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;

extern Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

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

  void Initialize(Platform::String^ deviceId);

  void CleanupSink();

  void DoCleanup();

  void Cleanup();

  void StartCapture(
      MediaEncodingProfile^ mediaEncodingProfile);

  void StopCapture();

  bool CaptureStarted() { return capture_started_; }

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
  int32_t frame_width_;
  int32_t frame_height_;
  int32_t max_fps_;
};

CaptureDevice::CaptureDevice(IncomingFrameCallback* incoming_frame_callback)
  : media_capture_(nullptr),
    media_sink_(nullptr),
    incoming_frame_callback_(incoming_frame_callback),
    capture_started_(false),
    frame_width_(0),
    frame_height_(0),
    max_fps_(0) {
}

void CaptureDevice::Initialize(Platform::String^ deviceId) {
  try {
    media_capture_ = MediaCaptureDevicesWinRT::Instance()->GetMediaCapture(deviceId);
    media_capture_failed_event_registration_token_ = media_capture_->Failed +=
        ref new MediaCaptureFailedEventHandler(this, &CaptureDevice::OnCaptureFailed);
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

void CaptureDevice::Cleanup()
{
  Windows::Media::Capture::MediaCapture ^mediaCapture = media_capture_.Get();
  if (mediaCapture == nullptr && !media_sink_) {
    return;
  }

  if (mediaCapture != nullptr) {
    mediaCapture->Failed -= media_capture_failed_event_registration_token_;
  }

  if (mediaCapture != nullptr && capture_started_) {
    auto stopRecorAsyncTask = Concurrency::create_task(
        mediaCapture->StopRecordAsync()).then([this](Concurrency::task<void>&)
    {
      DoCleanup();
    });

    stopRecorAsyncTask.wait();
  }
  else
  {
    DoCleanup();
  }
}

void CaptureDevice::StartCapture(MediaEncodingProfile^ mediaEncodingProfile)
{
  // We cannot start recording twice.
  if (media_sink_ && capture_started_) {
    throw ref new Platform::Exception(__HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
  }

  // Release sink if there is one already.
  CleanupSink();

  frame_width_ = mediaEncodingProfile->Video->Width;
  frame_height_ = mediaEncodingProfile->Video->Height;
  max_fps_ = (int)((float)mediaEncodingProfile->Video->FrameRate->Numerator / (float)mediaEncodingProfile->Video->FrameRate->Denominator);

  // Create new sink
  media_sink_ = ref new VideoCaptureMediaSinkProxyWinRT();
  media_sink_video_sample_event_registration_token_ = media_sink_->MediaSampleEvent += 
    ref new Windows::Foundation::EventHandler<MediaSampleEventArgs^>(this, &CaptureDevice::OnMediaSample);

  auto initializeAsyncTask = Concurrency::create_task(media_sink_->InitializeAsync(mediaEncodingProfile->Video)).
     then([this, mediaEncodingProfile](IMediaExtension^ mediaExtension)
  {
    auto startRecordToCustomSinkAsyncTask = Concurrency::create_task(media_capture_->StartRecordToCustomSinkAsync(mediaEncodingProfile, mediaExtension)).then([this](Concurrency::task<void>& asyncInfo)
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

    startRecordToCustomSinkAsyncTask.wait();

    return startRecordToCustomSinkAsyncTask;
  });

  initializeAsyncTask.wait();
}

void CaptureDevice::StopCapture()
{
  if (capture_started_)
  {
    auto stopRecordAsyncTask = Concurrency::create_task(media_capture_.Get()->StopRecordAsync()).then([this]()
    {
      CleanupSink();
    });

    stopRecordAsyncTask.wait();
  }
}

void CaptureDevice::OnMediaSample(Object^ sender, MediaSampleEventArgs^ args) {
  if (incoming_frame_callback_) {
    Microsoft::WRL::ComPtr<IMFSample> spMediaSample = args->GetMediaSample();
    ComPtr<IMFMediaBuffer> spMediaBuffer;
    HRESULT hr = spMediaSample->GetBufferByIndex(0, &spMediaBuffer);
    uint8_t* videoFrame;
    size_t videoFrameLength;
    VideoCaptureCapability frameInfo;
    frameInfo.width = frame_width_;
    frameInfo.height = frame_height_;
    frameInfo.maxFPS = max_fps_;
    frameInfo.rawType = kVideoNV12;
    int64_t captureTime;
    DWORD maxLength;
    DWORD currentLength;
    hr = spMediaSample->GetSampleTime(&captureTime);
    hr = spMediaBuffer->Lock(&videoFrame, &maxLength, &currentLength);
    videoFrameLength = currentLength;
    if (SUCCEEDED(hr)) {
      incoming_frame_callback_->OnIncomingFrame(videoFrame, videoFrameLength, frameInfo, captureTime);
      spMediaBuffer->Unlock();
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

  auto findAllAsyncTask = Concurrency::create_task(
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
    } catch (Platform::Exception^ e) {
    }
  });

  findAllAsyncTask.wait();

  CaptureDevice^ device = ref new CaptureDevice(this);

  try
  {
    device->Initialize(device_id_);

    device_ = device;
  }
  catch (Platform::Exception^ e)
  {
    if (device_)
    {
      device_->Cleanup();
      device_id_ = nullptr;
      device_ = nullptr;
      return -1;
    }
  }

  return 0;
}

int32_t VideoCaptureWinRT::StartCapture(
    const VideoCaptureCapability& capability) {

  MediaEncodingProfile^ mediaEncodingProfile =
    ref new MediaEncodingProfile();
  mediaEncodingProfile->Audio = nullptr;
  mediaEncodingProfile->Container = nullptr;
  mediaEncodingProfile->Video =
    VideoEncodingProperties::CreateUncompressed(
      MediaEncodingSubtypes::Nv12, capability.width, capability.height);

  try
  {
    device_->StartCapture(mediaEncodingProfile);
  }
  catch (Platform::Exception^ e)
  {
    return -1;
  }

  return 0;
}

int32_t VideoCaptureWinRT::StopCapture() {
  device_->StopCapture();
  return 0;
}

bool VideoCaptureWinRT::CaptureStarted() {
  return device_->CaptureStarted();
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
