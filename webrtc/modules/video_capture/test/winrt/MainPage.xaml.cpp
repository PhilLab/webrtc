//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace video_capture_test_winrt;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

Windows::UI::Xaml::Controls::CaptureElement^ g_capturePreview;

MainPage::MainPage()
  : state_(0)
{
	InitializeComponent();
  g_capturePreview = CapturePreview;
}

void video_capture_test_winrt::MainPage::MainButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  if (state_ == 1) {
    webrtc::VideoCaptureModule* vcpm = vcpms_[0];
    webrtc::VideoCaptureCapability capability;
    vcpm->StartCapture(capability);
  }

  webrtc::VideoCaptureModule::DeviceInfo* dev_info =
    webrtc::VideoCaptureFactory::CreateDeviceInfo(0);

  int number_of_capture_devices = dev_info->NumberOfDevices();

  int capture_device_id[10] = { 0 };
  webrtc::VideoCaptureModule** vcpms = new webrtc::VideoCaptureModule*[10];
  vcpms_ = vcpms;

  // Check capabilities
  for (int device_index = 0; device_index < number_of_capture_devices;
    ++device_index) {
    char device_name[128];
    char device_unique_name[512];

    dev_info->GetDeviceName(device_index,
      device_name,
      sizeof(device_name),
      device_unique_name,
      sizeof(device_unique_name));

    int number_of_capabilities =
      dev_info->NumberOfCapabilities(device_unique_name);

    for (int cap_index = 0; cap_index < number_of_capabilities; ++cap_index) {
      webrtc::VideoCaptureCapability capability;
      dev_info->GetCapability(device_unique_name, cap_index,
        capability);
    }
  }
  // Check allocation. Try to allocate them all after each other.
  for (int device_index = 0; device_index < number_of_capture_devices;
    ++device_index) {
    char device_name[128];
    char device_unique_name[512];
    dev_info->GetDeviceName(device_index,
      device_name,
      sizeof(device_name),
      device_unique_name,
      sizeof(device_unique_name));
    webrtc::VideoCaptureModule* vcpm =
      webrtc::VideoCaptureFactory::Create(device_index, device_unique_name);
    if (!vcpm)
      continue;

    vcpm->AddRef();
    vcpms[device_index] = vcpm;

    webrtc::VideoCaptureCapability capability;
    dev_info->GetCapability(device_unique_name, 0, capability);

    MainButton->Content = "Start";
  }

  /// **************************************************************
  //  Testing finished. Tear down Video Engine
  /// **************************************************************
  delete dev_info;

  // Stop all started capture devices.
  for (int device_index = 0; device_index < number_of_capture_devices;
    ++device_index) {
    if (vcpms[device_index]) {
      webrtc::VideoCaptureModule* vcpm = vcpms[device_index];
      //vcpm->StopCapture();
      //vcpms[device_index]->Release();
    }
  }

  state_++;
}
