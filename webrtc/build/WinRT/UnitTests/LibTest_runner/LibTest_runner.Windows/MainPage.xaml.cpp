//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include <ppltasks.h>
#include "MainPage.xaml.h"

using namespace LibTest_runner;

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
  OutputBox->Text = L"Tests results appears here\n";
}

void LibTest_runner::MainPage::RunAll_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  SpWStringReporter_t spStringReporter(new CWStringReporter(CWStringReporter::kPrintOutput));

  // Update the UI to indicate test execution is in progress
  ProgressRingCtrl->IsActive = true;

  // Run test cases in a separate thread not to block the UI thread
  // Pass the UI thread to continue using it after task execution
  auto ui = task_continuation_context::use_current();
  create_task([this, ui, sender, spStringReporter]()
  {
    libSrtpTests::TestSolution::Instance().AddReporter(spStringReporter);
    //libSrtpTests::TestSolution::Instance().Execute(L"RdbxDriverValidationTest");
    libSrtpTests::TestSolution::Instance().Execute(L"RocDriverTest");
    //libSrtpTests::TestSolution::Instance().Execute();
    libSrtpTests::TestSolution::Instance().GenerateReport();
  }).then([this, sender, spStringReporter]()
  {
    // Update the UI
    if (spStringReporter->GetReport() != NULL)
    {
      OutputBox->Text = ref new String((*spStringReporter->GetReport()).c_str());
      OutputBox->Text += L"Execution finished.\n";
    }

    ProgressRingCtrl->IsActive = false;
  }, ui);
}
