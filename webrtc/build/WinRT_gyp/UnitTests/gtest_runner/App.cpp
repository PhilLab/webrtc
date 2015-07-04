#include <collection.h>
#include <ppltasks.h>
#include <string>
#include "webrtc/test/test_suite.h"
#include "webrtc/base/win32.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/gunit.h"
#include "testing/gtest/include/gtest/gtest.h"

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
      progressTimer  = ref new DispatcherTimer;
      progressTimer->Tick += ref new Windows::Foundation::EventHandler<Object^>(this, &GTestApp::progressUpdate);
      Windows::Foundation::TimeSpan t;
      t.Duration = 10* 10*1000*1000; //10sec
      progressTimer->Interval = t;
    }

  private:
    TextBox^ outputTextBox_;
    ProgressRing^ progressRing_;
    DispatcherTimer^ progressTimer;

  protected:
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      g_windowDispatcher = Window::Current->Dispatcher;

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

      progressTimer->Start();
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

    void progressUpdate(Platform::Object^ sender, Platform::Object^ e) {

      if (!::testing::UnitTest::GetInstance()->current_test_case())
        return;

      std::ostringstream stringStream;

      SYSTEMTIME st;
      GetLocalTime(&st);

      std::string currentTestCase = ::testing::UnitTest::GetInstance()->current_test_case()->name();
      //FixMe:
      // Test count is not thread safe. fortunately. we will add check it when we fixed the threading issue.
      //int total = ::testing::UnitTest::GetInstance()->test_to_run_count();
      //int finished = ::testing::UnitTest::GetInstance()->successful_test_count() + ::testing::UnitTest::GetInstance()->failed_test_count();

      stringStream << "Executing test cases. Please wait...\n"
        << "Current Test case:" << currentTestCase << "\n"
        //<< finished << "/" << total << "test suite finished" << "\n"
        << "\n" << "Last Status updated at " << st.wHour << ":"<<st.wMinute<<":"<<st.wSecond;

      std::string s_str = stringStream.str();
      std::wstring wid_str = std::wstring(s_str.begin(), s_str.end());


      outputTextBox_->Text = ref new Platform::String(wid_str.c_str());

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
