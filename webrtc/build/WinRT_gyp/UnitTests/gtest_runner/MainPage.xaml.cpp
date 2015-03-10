//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include <ppltasks.h>
#include <string>
#include "MainPage.xaml.h"
#include "App.xaml.h"
#include "webrtc/test/test_suite.h"
#include "webrtc/base/win32.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/gunit.h"

using namespace gtest_runner;

using namespace concurrency;
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

MainPage::MainPage()
{
	InitializeComponent();
}

static char stdout_buffer[1024 * 1024] = { 0 };

void gtest_runner::MainPage::RunAll_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  testing::FLAGS_gtest_output = "xml";
  // Can only run once due to static initializations done in libraries.
  RunAll->IsEnabled = false;

  // Update the UI to indicate test execution is in progress
  ProgressRingCtrl->IsActive = true;
  OutputBox->PlaceholderText = "Executing test cases. Please wait...";

  // Capture stdout
  setvbuf(stdout, stdout_buffer, _IOFBF, sizeof(stdout_buffer));

  // Initialize SSL which are used by several tests.
  rtc::InitializeSSL();

  // Run test cases in a separate thread not to block the UI thread
  // Pass the UI thread to continue using it after task execution
  auto ui = task_continuation_context::use_current();
  create_task([this, ui, sender]()
  {
    char* argv[] = { "." };
    webrtc::test::TestSuite test_suite(1, argv);
    test_suite.Run();
  }).then([this, sender]()
  {
    // Cleanup SSL initialized
    rtc::CleanupSSL();

    // Update the UI
    OutputBox->Text = ref new String(rtc::ToUtf16(stdout_buffer, strlen(stdout_buffer)).data());
    ProgressRingCtrl->IsActive = false;
    if (sender == nullptr)
    {
      // The run operation was triggered by "autostart"
      // Exit the app
      App::Current->Exit();
    }
  }, ui);
}

void gtest_runner::MainPage::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
  if (e != nullptr && e->Parameter != nullptr && e->Parameter->ToString() == "autostart")
  {
    RunAll_Click(nullptr, nullptr);
  }
}