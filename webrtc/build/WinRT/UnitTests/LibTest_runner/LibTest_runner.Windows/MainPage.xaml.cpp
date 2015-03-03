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
}

void LibTest_runner::MainPage::RunAll_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  static const size_t kBufferSize = 2 * 1024 * 1024; //1MB
  std::string stdout_buf;
  // Capture stdout
  stdout_buf.resize(kBufferSize);

  setvbuf(stdout, const_cast<char*>(stdout_buf.c_str()), _IOFBF, kBufferSize);

  //Run tests
  TestSolution::Instance().Execute();
    
  //convert output to wchar_t
  int nRequiredSize = ::MultiByteToWideChar(CP_ACP, 0, const_cast<char*>(stdout_buf.c_str()), -1, NULL, 0);
  std::wstring wStdOut;
  if (nRequiredSize >= 0)
  {
    wStdOut.resize(nRequiredSize);
    ::MultiByteToWideChar(CP_ACP, 0, const_cast<char*>(stdout_buf.c_str()), -1, const_cast<wchar_t*>(wStdOut.c_str()), nRequiredSize);
  }

  OutputBox->Text = ref new String(wStdOut.c_str());
}
