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


static char stdout_buffer[1024 * 1024] = { 0 };

bool autoClose = false;

namespace video_capture_test_winrt {
  ref class VideoCaptureTestWinRT sealed :
    public Windows::UI::Xaml::Application{
  public:
    VideoCaptureTestWinRT() { }

  private:
    Windows::UI::Xaml::Controls::ProgressRing^ progressRing_;
    Windows::UI::Xaml::Controls::Button^ firstButton_;
    Windows::UI::Xaml::Controls::Button^ secondButton_;
    Windows::UI::Xaml::Controls::Button^ thirdButton_;

  protected:
    void OnLaunched(
        Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^
              e) override {
      auto layoutRoot = ref new Windows::UI::Xaml::Controls::Grid();
      layoutRoot->VerticalAlignment =
        Windows::UI::Xaml::VerticalAlignment::Center;
      layoutRoot->HorizontalAlignment =
        Windows::UI::Xaml::HorizontalAlignment::Center;

      auto containerStack = ref new Windows::UI::Xaml::Controls::StackPanel();
      containerStack->Orientation =
        Windows::UI::Xaml::Controls::Orientation::Vertical;

      auto buttonStack = ref new Windows::UI::Xaml::Controls::StackPanel();
      buttonStack->Orientation =
        Windows::UI::Xaml::Controls::Orientation::Horizontal;
      buttonStack->HorizontalAlignment =
        Windows::UI::Xaml::HorizontalAlignment::Center;

      firstButton_ = ref new Windows::UI::Xaml::Controls::Button();
      firstButton_->Width = 200;
      firstButton_->Height = 60;
      firstButton_->Content = "1";
      firstButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this,
        &video_capture_test_winrt::VideoCaptureTestWinRT::button1_Click);
      buttonStack->Children->Append(firstButton_);

      secondButton_ = ref new Windows::UI::Xaml::Controls::Button();
      secondButton_->Width = 200;
      secondButton_->Height = 60;
      secondButton_->Content = "2";
      secondButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(
        this, &video_capture_test_winrt::VideoCaptureTestWinRT::button1_Click);
      buttonStack->Children->Append(secondButton_);

      thirdButton_ = ref new Windows::UI::Xaml::Controls::Button();
      thirdButton_->Width = 200;
      thirdButton_->Height = 60;
      thirdButton_->Content = "3";
      thirdButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this,
        &video_capture_test_winrt::VideoCaptureTestWinRT::button1_Click);

      buttonStack->Children->Append(thirdButton_);

      containerStack->Children->Append(buttonStack);

      layoutRoot->Children->Append(containerStack);

      progressRing_ = ref new Windows::UI::Xaml::Controls::ProgressRing();
      progressRing_->Width = 50;
      progressRing_->Height = 50;
      layoutRoot->Children->Append(progressRing_);

      Windows::UI::Xaml::Window::Current->Content = layoutRoot;
      Windows::UI::Xaml::Window::Current->Activate();
      // RunCaptureTest();
    }

    void button1_Click(Platform::Object^ sender,
                       Windows::UI::Xaml::RoutedEventArgs^ e) {
    }

    void button2_Click(Platform::Object^ sender,
                       Windows::UI::Xaml::RoutedEventArgs^ e) {
    }

    void button3_Click(Platform::Object^ sender,
                       Windows::UI::Xaml::RoutedEventArgs^ e) {
    }

    void RunCaptureTest() {
      // Update the UI to indicate test execution is in progress
      progressRing_->IsActive = true;

      // Capture stdout
      setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

      // Run test cases in a separate thread not to block the UI thread
      // Pass the UI thread to continue using it after task execution
      auto ui = concurrency::task_continuation_context::use_current();
      concurrency::create_task([this, ui]() {
        int state = 0;
        if (state == 1) {
          webrtc::VideoCaptureModule* vcpm = NULL;
          webrtc::VideoCaptureCapability capability;
          vcpm->StartCapture(capability);
        }

        webrtc::VideoCaptureModule::DeviceInfo* dev_info =
          webrtc::VideoCaptureFactory::CreateDeviceInfo(0);

        int number_of_capture_devices = dev_info->NumberOfDevices();

        // int capture_device_id[10] = { 0 };
        webrtc::VideoCaptureModule** vcpms =
          new webrtc::VideoCaptureModule*[10];

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

          for (int cap_index = 0; cap_index < number_of_capabilities;
            ++cap_index) {
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
            webrtc::VideoCaptureFactory::Create(device_index,
            device_unique_name);
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
            // webrtc::VideoCaptureModule* vcpm = vcpms[device_index];
            // vcpm->StopCapture();
            // vcpms[device_index]->Release();
          }
        }

        state++;
      }).then([this]() {
        // Update the UI
        progressRing_->IsActive = false;

        // Exit the app
        VideoCaptureTestWinRT::Current->Exit();
      }, ui);
    }
  };

}  // namespace video_capture_test_winrt

int __cdecl main(::Platform::Array<::Platform::String^>^ args) {
  (void)args;  // Unused parameter
  Windows::UI::Xaml::Application::Start(
    ref new Windows::UI::Xaml::ApplicationInitializationCallback(
    [](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
    (void)p;  // Unused parameter
    auto app = ref new video_capture_test_winrt::VideoCaptureTestWinRT();
  }));

  return 0;
}
