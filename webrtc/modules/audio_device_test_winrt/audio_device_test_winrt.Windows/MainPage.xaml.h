//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include <ppltasks.h>

namespace audio_device_test_winrt
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

  private:
    void Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_1(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_2(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_3(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_Skip(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    Windows::Foundation::IAsyncAction^ TestTransportAsync();
    Windows::Foundation::IAsyncAction^ TestLoopBackAsync();
    Windows::Foundation::IAsyncAction^ TestSpeakerVolumeAsync();
    Windows::Foundation::IAsyncAction^ TestMicrophoneVolumeAsync();
    Windows::Foundation::IAsyncAction^ TestSpeakerMuteAsync();
    Windows::Foundation::IAsyncAction^ TestMicrophoneMuteAsync();
    Windows::Foundation::IAsyncAction^ TestMicrophoneAGCAsync();

    void Button_Click_4(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_5(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_6(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_7(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    void Button_Click_8(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
  };
}
