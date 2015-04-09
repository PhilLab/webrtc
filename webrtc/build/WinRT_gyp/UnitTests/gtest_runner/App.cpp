#include <collection.h>
#include <ppltasks.h>
#include <string>
#include "webrtc/test/test_suite.h"
#include "webrtc/base/win32.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/gunit.h"

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

static char stdout_buffer[1024 * 1024] = { 0 };

bool autoClose = false;

Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

namespace gtest_runner
{
  ref class GTestApp sealed : public Windows::UI::Xaml::Application
  {
  public:
    GTestApp()
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
      RunAllTests();
    }

    void RunAllTests()
    {
      testing::FLAGS_gtest_output = "xml";

      // Update the UI to indicate test execution is in progress
      progressRing_->IsActive = true;
      outputTextBox_->PlaceholderText = "Executing test cases. Please wait...";

      // Capture stdout
      setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

      // Initialize SSL which are used by several tests.
      rtc::InitializeSSL();

      // Run test cases in a separate thread not to block the UI thread
      // Pass the UI thread to continue using it after task execution
      auto ui = task_continuation_context::use_current();
      create_task([this, ui]()
      {
        char* argv[] = { "." };
        webrtc::test::TestSuite test_suite(1, argv);
        test_suite.Run();
      }).then([this]()
      {
        // Cleanup SSL initialized
        rtc::CleanupSSL();

        // Update the UI
        outputTextBox_->Text = ref new String(rtc::ToUtf16(stdout_buffer, strlen(stdout_buffer)).data());
        progressRing_->IsActive = false;

        // Exit the app
        GTestApp::Current->Exit();
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
    auto app = ref new gtest_runner::GTestApp();
  }));

  return 0;
}
