#include "common.h"
#include <collection.h>
#include <ppltasks.h>
#include <string>

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

bool autoClose = false;

namespace LibTest_runner
{
  ref class LibTestApp sealed : public Windows::UI::Xaml::Application
  {
  public:
    LibTestApp()
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
      SpWStringReporter_t spStringReporter(new CWStringReporter(/*CWStringReporter::kPrintOutput*/));

      // Update the UI to indicate test execution is in progress
      progressRing_->IsActive = true;

      // Run test cases in a separate thread not to block the UI thread
      // Pass the UI thread to continue using it after task execution
      auto ui = task_continuation_context::use_current();
      create_task([this, ui, spStringReporter]()
      {
        libSrtpTests::TestSolution::Instance().AddReporter(spStringReporter);
        libSrtpTests::TestSolution::Instance().AddReporter(SpXmlReporter_t(new CXmlReporter(ref new String(L"tests.xml"))));
        libSrtpTests::TestSolution::Instance().Execute();
        libSrtpTests::TestSolution::Instance().GenerateReport();
      }).then([this, spStringReporter]()
      {
        // Update the UI
        if (spStringReporter->GetReport() != NULL)
        {
          outputTextBox_->Text = ref new String((*spStringReporter->GetReport()).c_str());
          outputTextBox_->Text += L"Execution finished.\n";
        }

        progressRing_->IsActive = false;
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
    auto app = ref new LibTest_runner::LibTestApp();
  }));

  return 0;
}
