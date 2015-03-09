//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace LibTest_runner;

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
  //TODO: redesign not to block UI thread
  SpWStringReporter_t spStringReporter(new CWStringReporter());

  libSrtpTests::TestSolution::Instance().AddReporter(spStringReporter);
  /*libSrtpTests::TestSolution::Instance().Execute(L"CRdbxDriverTest");
  libSrtpTests::TestSolution::Instance().Execute(L"CReplayDriverTest");*/
  libSrtpTests::TestSolution::Instance().Execute();
  libSrtpTests::TestSolution::Instance().GenerateReport();

  OutputBox->Text = ref new String((*spStringReporter->GetReport()).c_str());
  OutputBox->Text += L"Execution finished.\n";
}
