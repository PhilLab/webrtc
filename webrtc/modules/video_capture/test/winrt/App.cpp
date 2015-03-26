/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include <collection.h>
#include <ppltasks.h>
#include <string>
#include "webrtc/modules/video_capture/include/video_capture_factory.h"

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

static char stdout_buffer[1024 * 1024] = { 0 };

bool autoClose = false;

Windows::UI::Xaml::Controls::CaptureElement^ g_capturePreview;

namespace video_capture_test_winrt
{
  ref class VideoCaptureTestWinRT sealed : public Windows::UI::Xaml::Application
  {
  public:
    VideoCaptureTestWinRT()
    {
    }

  private:
    TextBox^ outputTextBox_;
    ProgressRing^ progressRing_;

  protected:
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      auto layoutRoot = ref new Grid();
      layoutRoot->VerticalAlignment = VerticalAlignment::Center;
      layoutRoot->HorizontalAlignment = HorizontalAlignment::Center;

      outputTextBox_ = ref new TextBox();
      outputTextBox_->Width = 640;
      outputTextBox_->Height = 480;
      outputTextBox_->AcceptsReturn = true;
      outputTextBox_->PlaceholderText = "Test outputs appears here!";
      layoutRoot->Children->Append(outputTextBox_);

      progressRing_ = ref new ProgressRing();
      progressRing_->Width = 50;
      progressRing_->Height = 50;
      layoutRoot->Children->Append(progressRing_);

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
      RunCaptureTest();
    }

    void RunCaptureTest()
    {
      // Update the UI to indicate test execution is in progress
      progressRing_->IsActive = true;
      outputTextBox_->PlaceholderText = "Executing test cases. Please wait...";

      // Capture stdout
      setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

      // Run test cases in a separate thread not to block the UI thread
      // Pass the UI thread to continue using it after task execution
      auto ui = task_continuation_context::use_current();
      create_task([this, ui]()
      {
        int state = 0;
        if (state == 1) {
          webrtc::VideoCaptureModule* vcpm = NULL;
          webrtc::VideoCaptureCapability capability;
          vcpm->StartCapture(capability);
        }

        webrtc::VideoCaptureModule::DeviceInfo* dev_info =
          webrtc::VideoCaptureFactory::CreateDeviceInfo(0);

        int number_of_capture_devices = dev_info->NumberOfDevices();

        //int capture_device_id[10] = { 0 };
        webrtc::VideoCaptureModule** vcpms = new webrtc::VideoCaptureModule*[10];

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
        }

        /// **************************************************************
        //  Testing finished. Tear down Video Engine
        /// **************************************************************
        delete dev_info;

        // Stop all started capture devices.
        for (int device_index = 0; device_index < number_of_capture_devices;
          ++device_index) {
          if (vcpms[device_index]) {
            //webrtc::VideoCaptureModule* vcpm = vcpms[device_index];
            //vcpm->StopCapture();
            //vcpms[device_index]->Release();
          }
        }

        state++;
      }).then([this]()
      {
        // Update the UI
        progressRing_->IsActive = false;

        // Exit the app
        VideoCaptureTestWinRT::Current->Exit();
      }, ui);
    }
  };

}

int __cdecl main(::Platform::Array<::Platform::String^>^ args)
{
  (void)args; // Unused parameter
  Windows::UI::Xaml::Application::Start(
    ref new Windows::UI::Xaml::ApplicationInitializationCallback(
    [](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
    (void)p; // Unused parameter
    auto app = ref new video_capture_test_winrt::VideoCaptureTestWinRT();
  }));

  return 0;
}
