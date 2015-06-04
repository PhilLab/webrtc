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

#include <ppltasks.h>

#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/modules/video_capture/windows/video_capture_sink_winrt.h"

using Microsoft::WRL::ComPtr;
using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureInitializationSettings;
using Windows::Media::Capture::MediaCaptureFailedEventArgs;
using Windows::Media::Capture::MediaCaptureFailedEventHandler;
using Windows::Media::Capture::MediaStreamType;
using Windows::Media::IMediaExtension;
using Windows::Media::MediaProperties::IVideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingProfile;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;
using Windows::Media::MediaProperties::VideoEncodingProperties;

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

delegate void CaptureFailedHandler(CaptureDevice^ sender,
                                   CaptureFailedEventArgs^ error_event_args);

ref class CaptureDevice sealed {
 internal:
  event CaptureFailedHandler^ Failed;

  CaptureDevice(IncomingFrameCallback* incoming_frame_callback);

  void Initialize(Platform::String^ device_id);

  void CleanupSink();

  void DoCleanup();

  void Cleanup();

  void StartCapture(MediaEncodingProfile^ media_encoding_profile,
                    IVideoEncodingProperties^ video_encoding_properties);

  void StopCapture();

  bool CaptureStarted() { return capture_started_; }

  VideoCaptureCapability GetFrameInfo() { return frame_info_; }

  void OnCaptureFailed(MediaCapture^ sender,
                       MediaCaptureFailedEventArgs^ error_event_args) {
    Failed(this, ref new CaptureFailedEventArgs(error_event_args->Code,
                                                error_event_args->Message));
  }

  void OnMediaSample(Object^ sender, MediaSampleEventArgs^ args);

  property Platform::Agile<Windows::Media::Capture::MediaCapture> MediaCapture {
    Platform::Agile<Windows::Media::Capture::MediaCapture> get();
  }

private:
  Platform::Agile<Windows::Media::Capture::MediaCapture> media_capture_;
  VideoCaptureMediaSinkProxyWinRT^ media_sink_;
  Windows::Foundation::EventRegistrationToken
    media_capture_failed_event_registration_token_;
  Windows::Foundation::EventRegistrationToken
    media_sink_video_sample_event_registration_token_;

  IncomingFrameCallback* incoming_frame_callback_;

  bool capture_started_;
  VideoCaptureCapability frame_info_;
};

CaptureDevice::CaptureDevice(IncomingFrameCallback* incoming_frame_callback)
  : media_capture_(nullptr),
    media_sink_(nullptr),
    incoming_frame_callback_(incoming_frame_callback),
    capture_started_(false) {
}

void CaptureDevice::Initialize(Platform::String^ device_id) {
  try {
    media_capture_ =
      MediaCaptureDevicesWinRT::Instance()->GetMediaCapture(device_id);
    media_capture_failed_event_registration_token_ =
      media_capture_->Failed +=
        ref new MediaCaptureFailedEventHandler(this,
                                               &CaptureDevice::OnCaptureFailed);
  } catch (Platform::Exception^ e) {
    DoCleanup();
    throw e;
  }
}

void CaptureDevice::CleanupSink() {
  if (media_sink_) {
    media_sink_->MediaSampleEvent -=
      media_sink_video_sample_event_registration_token_;
    delete media_sink_;
    media_sink_ = nullptr;
    capture_started_ = false;
  }
}

void CaptureDevice::DoCleanup() {
  Windows::Media::Capture::MediaCapture^ media_capture = media_capture_.Get();
  if (media_capture != nullptr) {
    media_capture->Failed -= media_capture_failed_event_registration_token_;
  }

  CleanupSink();
}

Platform::Agile<Windows::Media::Capture::MediaCapture>
CaptureDevice::MediaCapture::get() {
  return media_capture_;
}

void CaptureDevice::Cleanup() {
  Windows::Media::Capture::MediaCapture^ media_capture = media_capture_.Get();
  if (media_capture == nullptr && !media_sink_) {
    return;
  }

  if (media_capture != nullptr) {
    media_capture->Failed -= media_capture_failed_event_registration_token_;
  }

  if (media_capture != nullptr && capture_started_) {
    Concurrency::create_task(
      media_capture->StopRecordAsync()).then([this](Concurrency::task<void>&) {
      DoCleanup();
    }).wait();
  } else {
    DoCleanup();
  }
}

void CaptureDevice::StartCapture(MediaEncodingProfile^ media_encoding_profile, IVideoEncodingProperties^ video_encoding_properties) {
  if (media_sink_ && capture_started_) {
    throw ref new Platform::Exception(
      __HRESULT_FROM_WIN32(ERROR_INVALID_STATE));
  }

  CleanupSink();

  frame_info_.width = media_encoding_profile->Video->Width;
  frame_info_.height = media_encoding_profile->Video->Height;
  frame_info_.maxFPS =
    static_cast<int>(
    static_cast<float>(media_encoding_profile->Video->FrameRate->Numerator) /
    static_cast<float>(media_encoding_profile->Video->FrameRate->Denominator));
  if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Yv12->Data()) == 0)
    frame_info_.rawType = kVideoYV12;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Yuy2->Data()) == 0)
    frame_info_.rawType = kVideoYUY2;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Iyuv->Data()) == 0)
    frame_info_.rawType = kVideoIYUV;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Rgb24->Data()) == 0)
    frame_info_.rawType = kVideoRGB24;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Rgb32->Data()) == 0)
    frame_info_.rawType = kVideoARGB;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Mjpg->Data()) == 0)
    frame_info_.rawType = kVideoMJPEG;
  else if (_wcsicmp(media_encoding_profile->Video->Subtype->Data(),
    MediaEncodingSubtypes::Nv12->Data()) == 0)
    frame_info_.rawType = kVideoNV12;
  else
    frame_info_.rawType = kVideoUnknown;

  media_sink_ = ref new VideoCaptureMediaSinkProxyWinRT();
  media_sink_video_sample_event_registration_token_ =
    media_sink_->MediaSampleEvent +=
      ref new Windows::Foundation::EventHandler<MediaSampleEventArgs^>
        (this, &CaptureDevice::OnMediaSample);

  Concurrency::create_task(
    media_sink_->InitializeAsync(media_encoding_profile->Video)).
      then([this, media_encoding_profile, video_encoding_properties](IMediaExtension^ media_extension) {
    return Concurrency::create_task(media_capture_->VideoDeviceController->
      SetMediaStreamPropertiesAsync(MediaStreamType::VideoRecord, video_encoding_properties)).
        then([this, media_encoding_profile, media_extension](Concurrency::task<void> async_info) {
      return Concurrency::create_task(media_capture_->
        StartRecordToCustomSinkAsync(media_encoding_profile, media_extension)).then([this](Concurrency::task<void> async_info)
      {
        try {
          async_info.get();
          capture_started_ = true;
        } catch (Platform::Exception^ e) {
          CleanupSink();
          throw;
        }
      });
    });
  }).wait();
}

void CaptureDevice::StopCapture() {
  if (capture_started_) {
    Concurrency::create_task(
      media_capture_.Get()->StopRecordAsync()).then([this]() {
      CleanupSink();
    }).wait();
  }
}

void CaptureDevice::OnMediaSample(Object^ sender, MediaSampleEventArgs^ args) {
  if (incoming_frame_callback_) {
    Microsoft::WRL::ComPtr<IMFSample> spMediaSample = args->GetMediaSample();
    ComPtr<IMFMediaBuffer> spMediaBuffer;
    HRESULT hr = spMediaSample->GetBufferByIndex(0, &spMediaBuffer);
    uint8_t* video_frame;
    size_t video_frame_length;
    int64_t capture_time;
    LONGLONG hnsSampleTime;
    BYTE* pbBuffer;
    DWORD cbMaxLength;
    DWORD cbCurrentLength;
    hr = spMediaSample->GetSampleTime(&hnsSampleTime);
    hr = spMediaBuffer->Lock(&pbBuffer, &cbMaxLength, &cbCurrentLength);
    video_frame = pbBuffer;
    video_frame_length = cbCurrentLength;
    // conversion from 100-nanosecond to millisecond units
    capture_time = hnsSampleTime / 10000;
    if (SUCCEEDED(hr)) {
      WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, 0,
        "Video Capture - OnMediaSample - video frame length: %d, capture time: %lld",
        video_frame_length, capture_time);
      incoming_frame_callback_->OnIncomingFrame(video_frame,
                                                video_frame_length,
                                                frame_info_);
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

int32_t VideoCaptureWinRT::Init(const int32_t id,
                                const char* device_unique_id) {
  const int32_t device_unique_id_length = strlen(device_unique_id);
  if (device_unique_id_length > kVideoCaptureUniqueNameLength) {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
      "Device name too long");
    return -1;
  }

  WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
    "Init called for device %s", device_unique_id);

  device_id_ = nullptr;

  Concurrency::create_task(
    DeviceInformation::FindAllAsync(DeviceClass::VideoCapture)).
      then([this,
           device_unique_id,
           device_unique_id_length](
        Concurrency::task<DeviceInformationCollection^> find_task) {
    try {
      DeviceInformationCollection^ dev_info_collection = find_task.get();
      if (dev_info_collection == nullptr || dev_info_collection->Size == 0) {
        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
          "No video capture device found");
      }
      for (unsigned int i = 0; i < dev_info_collection->Size; i++) {
        auto dev_info = dev_info_collection->GetAt(i);
        Platform::String^ deviceUniqueId = dev_info->Id;
        char current_device_unique_id_utf8[256];
        current_device_unique_id_utf8[0] = 0;
        WideCharToMultiByte(CP_UTF8, 0, deviceUniqueId->Begin(), -1,
          current_device_unique_id_utf8,
          sizeof(current_device_unique_id_utf8), NULL,
          NULL);
        if (strncmp(current_device_unique_id_utf8,
          (const char*)device_unique_id,
          device_unique_id_length) == 0) {
          device_id_ = dev_info->Id;
          break;
        }
      }
    } catch (Platform::Exception^ e) {
    }
  }).wait();

  CaptureDevice^ device = ref new CaptureDevice(this);

  try {
    device->Initialize(device_id_);

    device_ = device;
  } catch (Platform::Exception^ e) {
    if (device_) {
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
  Platform::String^ subtype;
  switch (capability.rawType) {
  case kVideoYV12:
    subtype = MediaEncodingSubtypes::Yv12;
    break;
  case kVideoYUY2:
    subtype = MediaEncodingSubtypes::Yuy2;
    break;
  case kVideoI420:
  case kVideoIYUV:
    subtype = MediaEncodingSubtypes::Iyuv;
    break;
  case kVideoRGB24:
    subtype = MediaEncodingSubtypes::Rgb24;
    break;
  case kVideoARGB:
    subtype = MediaEncodingSubtypes::Rgb24;
    break;
  case kVideoMJPEG:
    subtype = MediaEncodingSubtypes::Mjpg;
    break;
  case kVideoNV12:
    subtype = MediaEncodingSubtypes::Nv12;
    break;
  default:
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
      "The specified raw video format is not supported on this plaform.");
    return -1;
  }

  MediaEncodingProfile^ media_encoding_profile =
    ref new MediaEncodingProfile();
  media_encoding_profile->Audio = nullptr;
  media_encoding_profile->Container = nullptr;
  media_encoding_profile->Video =
    VideoEncodingProperties::CreateUncompressed(
      subtype, capability.width, capability.height);
  media_encoding_profile->Video->FrameRate->Numerator = capability.maxFPS;
  media_encoding_profile->Video->FrameRate->Denominator = 1;

  IVideoEncodingProperties^ video_encoding_properties;
  int min_width_diff = INT_MAX;
  int min_height_diff = INT_MAX;
  int min_fps_diff = INT_MAX;
  auto mediaCapture = MediaCaptureDevicesWinRT::Instance()->GetMediaCapture(device_id_);
  auto streamProperties = mediaCapture->VideoDeviceController->GetAvailableMediaStreamProperties(
    MediaStreamType::VideoRecord);
  for (unsigned int i = 0; i < streamProperties->Size; i++)
  {
    IVideoEncodingProperties^ prop =
      static_cast<IVideoEncodingProperties^>(streamProperties->GetAt(i));

    if (_wcsicmp(prop->Subtype->Data(), subtype->Data()) != 0)
      continue;

    int width_diff = abs((int)(prop->Width - capability.width));
    int height_diff = abs((int)(prop->Height - capability.height));
    int prop_fps = (int)((float)prop->FrameRate->Numerator / (float)prop->FrameRate->Denominator);
    int fps_diff = abs((int)(prop_fps - capability.maxFPS));

    if (width_diff < min_width_diff)
    {
      video_encoding_properties = prop;
      min_width_diff = width_diff;
      min_height_diff = height_diff;
      min_fps_diff = fps_diff;
    }
    else if (width_diff == min_width_diff)
    {
      if (height_diff < min_height_diff)
      {
        video_encoding_properties = prop;
        min_height_diff = height_diff;
        min_fps_diff = fps_diff;
      }
      else if (height_diff == min_height_diff)
      {
        if (fps_diff < min_fps_diff) {
          video_encoding_properties = prop;
          min_fps_diff = fps_diff;
        }
      }
    }
  }

  try {
    device_->StartCapture(media_encoding_profile, video_encoding_properties);
  } catch (Platform::Exception^ e) {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
      "Failed to start capture on device");
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

int32_t VideoCaptureWinRT::CaptureSettings(VideoCaptureCapability& settings) {
  settings = device_->GetFrameInfo();
  return 0;
}

void VideoCaptureWinRT::OnIncomingFrame(
  uint8_t* video_frame,
  size_t video_frame_length,
  const VideoCaptureCapability& frame_info) {
  IncomingFrame(video_frame, video_frame_length, frame_info);
}

}  // namespace videocapturemodule
}  // namespace webrtc
