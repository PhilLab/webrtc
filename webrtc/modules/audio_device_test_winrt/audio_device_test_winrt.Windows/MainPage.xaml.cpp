//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "webrtc/modules/audio_device/audio_device_config.h"
#include "webrtc/modules/audio_device/audio_device_impl.h"
#include <ppltasks.h>
using namespace concurrency;

using namespace audio_device_test_winrt;

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


void audio_device_test_winrt::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  WinRTTestManager *mManager = new WinRTTestManager();
  mManager->Init();
  mManager->TestDeviceEnumeration();
}


void audio_device_test_winrt::MainPage::Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  WinRTTestManager *mManager = new WinRTTestManager();
  mManager->Init();
  mManager->TestDeviceSelection();
}


void audio_device_test_winrt::MainPage::Button_Click_2(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  TestTransportAsync();
}

Windows::Foundation::IAsyncAction^  audio_device_test_winrt::MainPage::TestTransportAsync() {
  return create_async([this]
  {
    WinRTTestManager *mManager = new WinRTTestManager();
    mManager->Init();
    mManager->TestAudioTransport();

  });
}

void audio_device_test_winrt::MainPage::Button_Click_3(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  TestLoopBackAsync();
}


Windows::Foundation::IAsyncAction^ audio_device_test_winrt::MainPage::TestLoopBackAsync(){
  return create_async([this]
  {
    WinRTTestManager *mManager = new WinRTTestManager();
    mManager->Init();
    mManager->TestLoopback();
  });
}

void audio_device_test_winrt::MainPage::Button_Click_Skip(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e){
  WinRTTestManager::userSignalToContinue();
}