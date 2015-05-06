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
#ifdef HAVE_LOGITECH_HEADERS
#include "third_party/logitech/files/logitechquickcam.h"
#endif
#include "webrtc/video_engine/include/vie_base.h"
#include "webrtc/video_engine/include/vie_capture.h"

using namespace concurrency;
using namespace Windows::Devices::Enumeration;

namespace cricket
{
  DeviceManagerInterface* DeviceManagerFactory::Create()
  {
    return new WinRTDeviceManager();
  }

  const CLSID WinRTDeviceManager::CLSID_VideoInputDeviceCategory = 
  { 0x860BB310, 0x5D01, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };

  const CLSID WinRTDeviceManager::CLSID_SystemDeviceEnum =
  { 0x62BE5D10, 0x60EB, 0x11d0, { 0xBD, 0x3B, 0x00, 0xA0, 0xC9, 0x11, 0xCE, 0x86 } };

  WinRTDeviceManager::WinRTDeviceManager() : needCoUninitialize_(false)
  {
  }

  WinRTDeviceManager::~WinRTDeviceManager()
  {
    if (initialized()) {
      Terminate();
    }
  }

  bool WinRTDeviceManager::Init()
  {
    //if (!initialized())
    //{
    //  HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    //  needCoUninitialize_ = SUCCEEDED(hr);
    //  if (FAILED(hr))
    //  {
    //    LOG(LS_ERROR) << "CoInitialize failed, hr=" << hr;
    //    if (hr != RPC_E_CHANGED_MODE)
    //    {
    //      return false;
    //    }
    //  }
    //  set_initialized(true);
    //}
    if (!initialized())
    {
      set_initialized(true);
    }
    return true;
  }

  void WinRTDeviceManager::Terminate()
  {
    if (initialized())
    {
      //if (needCoUninitialize_)
      //{
      //  CoUninitialize();
      //  needCoUninitialize_ = false;
      //}
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

  bool WinRTDeviceManager::GetDevices(REFCLSID catid, std::vector<Device>* devices)
  {
    return true;
  }

}