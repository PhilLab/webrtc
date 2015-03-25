//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

#include "webrtc/modules/video_capture/include/video_capture_factory.h"

namespace video_capture_test_winrt
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

  private:
    void MainButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
    int state_;
    webrtc::VideoCaptureModule** vcpms_;
  };
}
