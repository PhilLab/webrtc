/*
*  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "webrtc/modules/video_capture/windows/device_info_winrt.h"

#include "webrtc/system_wrappers/interface/trace.h"

#include <windows.media.h>

#include <ppltasks.h>

using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureInitializationSettings;
using Windows::Media::Capture::MediaStreamType;
using Windows::Media::MediaProperties::IVideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;

extern Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

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
  ReadLockScoped cs(_apiLock);
  return GetDeviceInfo(255, 0, 0, 0, 0, 0, 0);
}

int32_t DeviceInfoWinRT::GetDeviceName(
    uint32_t deviceNumber,
    char* deviceNameUTF8,
    uint32_t deviceNameLength,
    char* deviceUniqueIdUTF8,
    uint32_t deviceUniqueIdUTF8Length,
    char* productUniqueIdUTF8,
    uint32_t productUniqueIdUTF8Length) {
  ReadLockScoped cs(_apiLock);
  const int32_t result = GetDeviceInfo(
      deviceNumber,
      deviceNameUTF8,
      deviceNameLength,
      deviceUniqueIdUTF8,
      deviceUniqueIdUTF8Length,
      productUniqueIdUTF8,
      productUniqueIdUTF8Length);
  return result > (int32_t)deviceNumber ? 0 : -1;
}

int32_t DeviceInfoWinRT::GetDeviceInfo(
    uint32_t deviceNumber,
    char* deviceNameUTF8,
    uint32_t deviceNameLength,
    char* deviceUniqueIdUTF8,
    uint32_t deviceUniqueIdUTF8Length,
    char* productUniqueIdUTF8,
    uint32_t productUniqueIdUTF8Length) {

  int deviceCount = -1;
  int* deviceCountPtr = &deviceCount;
  auto findAllAsyncTask = Concurrency::create_task(
      DeviceInformation::FindAllAsync(
          DeviceClass::VideoCapture)).then(
              [this,
              deviceNumber,
              deviceNameUTF8,
              deviceNameLength,
              deviceUniqueIdUTF8,
              deviceUniqueIdUTF8Length,
              productUniqueIdUTF8,
              productUniqueIdUTF8Length,
              deviceCountPtr](Concurrency::task<DeviceInformationCollection^> findTask) {
    try {
      DeviceInformationCollection^ devInfoCollection = findTask.get();
      if (devInfoCollection == nullptr || devInfoCollection->Size == 0) {
        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
          "No video capture device found");
      }
      *deviceCountPtr = devInfoCollection->Size;
      for (unsigned int i = 0; i < devInfoCollection->Size; i++) {
        if (i == static_cast<int>(deviceNumber))
        {
          auto devInfo = devInfoCollection->GetAt(i);
          Platform::String^ deviceName = devInfo->Name;
          Platform::String^ deviceUniqueId = devInfo->Id;
          int convResult = 0;
          convResult = WideCharToMultiByte(CP_UTF8, 0,
            deviceName->Data(), -1,
            (char*)deviceNameUTF8,
            deviceNameLength, NULL,
            NULL);
          if (convResult == 0)
          {
            WEBRTC_TRACE(webrtc::kTraceError,
              webrtc::kTraceVideoCapture, _id,
              "Failed to convert device name to UTF8. %d",
              GetLastError());
          }
          convResult = WideCharToMultiByte(CP_UTF8, 0,
            deviceUniqueId->Data(), -1,
            (char*)deviceUniqueIdUTF8,
            deviceUniqueIdUTF8Length, NULL,
            NULL);
          if (convResult == 0)
          {
            WEBRTC_TRACE(webrtc::kTraceError,
              webrtc::kTraceVideoCapture, _id,
              "Failed to convert device unique ID to UTF8. %d",
              GetLastError());
          }
          if (productUniqueIdUTF8 != NULL)
            productUniqueIdUTF8[0] = 0;
        }
      }
    } catch (Platform::Exception^ e) {
    }
  });

  findAllAsyncTask.wait();

  return deviceCount;
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

  _captureCapabilities.clear();

  const int32_t deviceUniqueIdUTF8Length =
    (int32_t)strlen((char*)deviceUniqueIdUTF8);
  if (deviceUniqueIdUTF8Length > kVideoCaptureUniqueNameLength)
  {
    WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
      "Device name too long");
    return -1;
  }
  WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
    "CreateCapabilityMap called for device %s", deviceUniqueIdUTF8);

  bool finished = false;
  bool* finishedPtr = &finished;
  auto findAllAsyncTask = Concurrency::create_task(
      DeviceInformation::FindAllAsync(
          DeviceClass::VideoCapture)).then(
              [this,
              deviceUniqueIdUTF8,
              deviceUniqueIdUTF8Length,
              finishedPtr](Concurrency::task<DeviceInformationCollection^> findTask) {
    try {
      DeviceInformationCollection^ devInfoCollection = findTask.get();
      if (devInfoCollection == nullptr || devInfoCollection->Size == 0) {
        WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
          "No video capture device found");
      }
      DeviceInformation^ chosenDevInfo = nullptr;
      for (unsigned int i = 0; i < devInfoCollection->Size; i++) {
        auto devInfo = devInfoCollection->GetAt(i);
        Platform::String^ deviceUniqueId = devInfo->Id;
        char currentDeviceUniqueIdUTF8[256];
        currentDeviceUniqueIdUTF8[0] = 0;
        WideCharToMultiByte(CP_UTF8, 0, deviceUniqueId->Data(), -1,
          currentDeviceUniqueIdUTF8,
          sizeof(currentDeviceUniqueIdUTF8), NULL,
          NULL);
        if (strncmp(currentDeviceUniqueIdUTF8,
          (const char*)deviceUniqueIdUTF8,
          deviceUniqueIdUTF8Length) == 0) {
          chosenDevInfo = devInfo;
          break;
        }
      }
      if (chosenDevInfo != nullptr) {
        auto settings = ref new MediaCaptureInitializationSettings();
        auto mediaCapture = ref new MediaCapture();
        Platform::Agile<MediaCapture> mediaCaptureAgile(mediaCapture);
        settings->VideoDeviceId = chosenDevInfo->Id;
        Windows::UI::Core::CoreDispatcher^ dispatcher = g_windowDispatcher;
        Windows::UI::Core::CoreDispatcherPriority priority = Windows::UI::Core::CoreDispatcherPriority::Normal;
        Concurrency::task<void> initializeAsyncTask;
        Windows::UI::Core::DispatchedHandler^ handler = ref new Windows::UI::Core::DispatchedHandler(
            [this,
            &initializeAsyncTask,
            mediaCaptureAgile,
            settings]() {
          initializeAsyncTask = Concurrency::create_task(mediaCaptureAgile->InitializeAsync(settings))
            .then([this, mediaCaptureAgile](Concurrency::task<void> initTask)
          {
            try {
              initTask.get();
              auto streamProperties = mediaCaptureAgile->VideoDeviceController->GetAvailableMediaStreamProperties(
                  MediaStreamType::VideoRecord);
              for (unsigned int i = 0; i < streamProperties->Size; i++)
              {
                IVideoEncodingProperties^ prop =
                  static_cast<IVideoEncodingProperties^>(streamProperties->GetAt(i));
                VideoCaptureCapability capability;
                capability.width = prop->Width;
                capability.height = prop->Height;
                capability.maxFPS = (int)((float)prop->FrameRate->Numerator / (float)prop->FrameRate->Denominator);
                if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Yv12->Data()) == 0)
                  capability.rawType = kVideoYV12;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Yuy2->Data()) == 0)
                  capability.rawType = kVideoYUY2;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Iyuv->Data()) == 0)
                  capability.rawType = kVideoIYUV;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Rgb24->Data()) == 0)
                  capability.rawType = kVideoRGB24;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Rgb32->Data()) == 0)
                  capability.rawType = kVideoARGB;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Mjpg->Data()) == 0)
                  capability.rawType = kVideoMJPEG;
                else if (_wcsicmp(prop->Subtype->Data(), MediaEncodingSubtypes::Nv12->Data()) == 0)
                  capability.rawType = kVideoNV12;
                else
                  capability.rawType = kVideoUnknown;
                _captureCapabilities.push_back(capability);
                int subtypeSize = WideCharToMultiByte(CP_UTF8, 0, prop->Subtype->Data(), wcslen(prop->Subtype->Data()), NULL, 0, NULL, NULL);
                std::string subtype(subtypeSize, 0);
                WideCharToMultiByte(CP_UTF8, 0, prop->Subtype->Data(), wcslen(prop->Subtype->Data()), &subtype[0], subtypeSize, NULL, NULL);
                WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoCapture, _id,
                  "Capture media stream properties: index: %d, width: %d, height: %d, frame rate: %d/%d, subtype: %s", 
                  i, prop->Width, prop->Height, prop->FrameRate->Numerator, prop->FrameRate->Denominator, subtype.c_str());
              }
            } catch (Platform::Exception^ e) {
              int messageSize = WideCharToMultiByte(CP_UTF8, 0, e->Message->Data(), wcslen(e->Message->Data()), NULL, 0, NULL, NULL);
              std::string message(messageSize, 0);
              WideCharToMultiByte(CP_UTF8, 0, e->Message->Data(), wcslen(e->Message->Data()), &message[0], messageSize, NULL, NULL);
              WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoCapture, _id,
                "Failed to get media stream properties. %s", message.c_str());
            }
          });
        });
        Windows::Foundation::IAsyncAction^ dispatcherAction = dispatcher->RunAsync(priority, handler);
        auto dispatcherTask = Concurrency::create_task(dispatcherAction);
        dispatcherTask.wait();
        initializeAsyncTask.wait();
      }
      *finishedPtr = true;
    } catch (Platform::Exception^ e) {
    }
  });

  findAllAsyncTask.wait();

  return _captureCapabilities.size();
}

}  // namespace videocapturemodule
}  // namespace webrtc
