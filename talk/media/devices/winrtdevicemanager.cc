#include "talk/media/devices/winrtdevicemanager.h"
#define _ATL_STATIC_LIB_IMPL
#include <atlbase.h>
#include <dbt.h>
#include <strmif.h>
#include <ks.h>
#include <ksmedia.h>
#include <mmdeviceapi.h>
#include <mmsystem.h>
#include <functiondiscoverykeys_devpkey.h>
#include <wincodec.h>
#include <uuids.h>
#include <ppltasks.h>
#include <collection.h>

#include "webrtc/base/logging.h"
#include "webrtc/base/stringutils.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/win32.h"  // ToUtf8
#include "webrtc/base/win32window.h"
#include "talk/media/base/mediacommon.h"
#include "webrtc/video_engine/include/vie_base.h"
#include "webrtc/video_engine/include/vie_capture.h"

using namespace concurrency;
using namespace Windows::Devices::Enumeration;

namespace cricket
{
  const char* WinRTDeviceManager::kUsbDevicePathPrefix = "\\\\?\\usb";

  DeviceManagerInterface* DeviceManagerFactory::Create()
  {
    return new WinRTDeviceManager();
  }

  WinRTDeviceManager::WinRTDeviceManager() :
    watcher_(ref new WinRTWatcher())
  {
  }

  WinRTDeviceManager::~WinRTDeviceManager()
  {
  }

  bool WinRTDeviceManager::Init()
  {
    if (!initialized())
    {
      watcher_->deviceManager_ = this;
      watcher_->Start();
      set_initialized(true);
    }
    return true;
  }

  void WinRTDeviceManager::Terminate()
  {
    if (initialized())
    {
      watcher_->Stop();
      set_initialized(false);
    }
  }

  bool WinRTDeviceManager::GetAudioInputDevices(std::vector<Device>* devices)
  {
    devices->clear();
    auto deviceOp = DeviceInformation::FindAllAsync(
      DeviceClass::AudioCapture);
    auto deviceEnumTask = create_task(deviceOp);
    deviceEnumTask.then([&devices](DeviceInformationCollection^ deviceCollection)
    {
      for (size_t i = 0; i < deviceCollection->Size; i++)
      {
        DeviceInformation^ di = deviceCollection->GetAt(i);
        std::string nameUTF8(rtc::ToUtf8(di->Name->Data(), di->Name->Length()));
        std::string idUTF8(rtc::ToUtf8(di->Id->Data(), di->Id->Length()));
        devices->push_back(Device(nameUTF8, idUTF8));
      }
    }).wait();
    return true;
  }

  bool WinRTDeviceManager::GetAudioOutputDevices(std::vector<Device>* devices)
  {
    devices->clear();
    auto deviceOp = DeviceInformation::FindAllAsync(
      DeviceClass::AudioRender);
    auto deviceEnumTask = create_task(deviceOp);
    deviceEnumTask.then([&devices](DeviceInformationCollection^ deviceCollection)
    {
      for (size_t i = 0; i < deviceCollection->Size; i++)
      {
        DeviceInformation^ di = deviceCollection->GetAt(i);
        std::string nameUTF8(rtc::ToUtf8(di->Name->Data(), di->Name->Length()));
        std::string idUTF8(rtc::ToUtf8(di->Id->Data(), di->Id->Length()));
        devices->push_back(Device(nameUTF8, idUTF8));
      }
    }).wait();
    return true;
  }

  bool WinRTDeviceManager::GetVideoCaptureDevices(std::vector<Device>* devices)
  {
    devices->clear();
    webrtc::VideoEngine* ve = webrtc::VideoEngine::Create();
    webrtc::ViECapture* vc = webrtc::ViECapture::GetInterface(ve);
    int deviceCount = vc->NumberOfCaptureDevices();
    const unsigned int KMaxDeviceNameLength = 128;
    const unsigned int KMaxUniqueIdLength = 256;
    char deviceName[KMaxDeviceNameLength];
    char uniqueId[KMaxUniqueIdLength];
    for (int i = 0; i < deviceCount; i++)
    {
      vc->GetCaptureDevice(i, deviceName,
        KMaxDeviceNameLength, uniqueId,
        KMaxUniqueIdLength);
      devices->push_back(Device(std::string(deviceName), std::string(uniqueId)));
    }
    vc->Release();
    webrtc::VideoEngine::Delete(ve);
    return true;
  }

  bool WinRTDeviceManager::GetDefaultVideoCaptureDevice(Device* device)
  {
    std::vector<Device> devices;
    bool ret = (GetVideoCaptureDevices(&devices)) && (!devices.empty());
    if (ret)
    {
      *device = *devices.begin();
      for (std::vector<Device>::const_iterator it = devices.begin(); it != devices.end(); it++)
      {
        if (strnicmp((*it).id.c_str(), kUsbDevicePathPrefix,
          sizeof(kUsbDevicePathPrefix) - 1) == 0)
        {
          *device = *it;
          break;
        }
      }
    }
    return ret;
  }

  void WinRTDeviceManager::OnDeviceChange()
  {
    SignalDevicesChange();
  }

WinRTDeviceManager::WinRTWatcher::WinRTWatcher() :
  deviceManager_(nullptr), 
  videoCaptureWatcher_(DeviceInformation::CreateWatcher(DeviceClass::VideoCapture)),
  videoAudioInWatcher_(DeviceInformation::CreateWatcher(DeviceClass::AudioCapture)),
  videoAudioOutWatcher_(DeviceInformation::CreateWatcher(DeviceClass::AudioRender))
{
  videoCaptureWatcher_->Added += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformation ^>(this, &WinRTDeviceManager::WinRTWatcher::OnVideoCaptureAdded);
  videoCaptureWatcher_->Removed += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformationUpdate ^>(this, &WinRTDeviceManager::WinRTWatcher::OnVideoCaptureRemoved);

  videoAudioInWatcher_->Added += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformation ^>(this, &WinRTDeviceManager::WinRTWatcher::OnAudioInAdded);
  videoAudioInWatcher_->Removed += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformationUpdate ^>(this, &WinRTDeviceManager::WinRTWatcher::OnAudioInRemoved);

  videoAudioOutWatcher_->Added += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformation ^>(this, &WinRTDeviceManager::WinRTWatcher::OnAudioOutAdded);
  videoAudioOutWatcher_->Removed += ref new Windows::Foundation::TypedEventHandler<Windows::Devices::Enumeration::DeviceWatcher ^,
    DeviceInformationUpdate ^>(this, &WinRTDeviceManager::WinRTWatcher::OnAudioOutRemoved);
}

void WinRTDeviceManager::WinRTWatcher::OnVideoCaptureAdded(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformation ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::OnVideoCaptureRemoved(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformationUpdate ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::OnAudioInAdded(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformation ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::OnAudioInRemoved(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformationUpdate ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::OnAudioOutAdded(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformation ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::OnAudioOutRemoved(
  Windows::Devices::Enumeration::DeviceWatcher ^sender,
  DeviceInformationUpdate ^args)
{
  OnDeviceChange();
}

void WinRTDeviceManager::WinRTWatcher::Start()
{
  videoCaptureWatcher_->Start();
  videoAudioInWatcher_->Start();
  videoAudioOutWatcher_->Start();
}

void WinRTDeviceManager::WinRTWatcher::Stop()
{
  videoCaptureWatcher_->Stop();
  videoAudioInWatcher_->Stop();
  videoAudioOutWatcher_->Stop();
}

void WinRTDeviceManager::WinRTWatcher::OnDeviceChange()
{
  deviceManager_->OnDeviceChange();
}

}