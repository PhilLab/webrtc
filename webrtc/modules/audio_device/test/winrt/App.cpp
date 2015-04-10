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

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

static char stdout_buffer[1024 * 1024] = { 0 };

bool autoClose = false;

namespace audio_device_test_winrt
{
  ref class AudioDeviceTestWinRT sealed : public Windows::UI::Xaml::Application
  {
  public:
    AudioDeviceTestWinRT()
    {
    }

  private:
    ProgressRing^ progressRing_;
    Button^ startButton_;

  protected:
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      auto layoutRoot = ref new Grid();
      layoutRoot->VerticalAlignment = VerticalAlignment::Center;
      layoutRoot->HorizontalAlignment = HorizontalAlignment::Center;

      auto containerStack = ref new StackPanel();
      containerStack->Orientation = Orientation::Vertical;

      auto buttonStack = ref new StackPanel();
      buttonStack->Orientation = Orientation::Horizontal;
      buttonStack->HorizontalAlignment = HorizontalAlignment::Center;

      startButton_ = ref new Button();
      startButton_->Width = 200;
      startButton_->Height = 60;
      startButton_->Content = "Start";
      startButton_->Click += ref new RoutedEventHandler(this, &audio_device_test_winrt::AudioDeviceTestWinRT::startButton_Click);
      buttonStack->Children->Append(startButton_);

      containerStack->Children->Append(buttonStack);
      layoutRoot->Children->Append(containerStack);

      progressRing_ = ref new ProgressRing();
      progressRing_->Width = 50;
      progressRing_->Height = 50;
      layoutRoot->Children->Append(progressRing_);

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
    }

    void startButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
    {
      RunTest();
    }

    void RunTest()
    {
      // Update the UI to indicate test execution is in progress
      progressRing_->IsActive = true;

      // Capture stdout
      setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

      // Run test cases in a separate thread not to block the UI thread
      // Pass the UI thread to continue using it after task execution
      auto ui = task_continuation_context::use_current();
      create_task([this, ui]()
      {
        // do the tests here
      }).then([this]()
      {
        // Update the UI
        progressRing_->IsActive = false;

        // Exit the app
        AudioDeviceTestWinRT::Current->Exit();
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
    auto app = ref new audio_device_test_winrt::AudioDeviceTestWinRT();
  }));

  return 0;
}
