//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"
#include "webrtc\base\thread.h"

using namespace Platform;
using namespace Windows::UI::Xaml::Data;

namespace Test
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
  [Bindable]
  public ref class MainPage sealed : INotifyPropertyChanged
	{
	public:
		MainPage();

    property bool ThreadNotRunning;

    virtual event PropertyChangedEventHandler^ PropertyChanged;

    void NotifyPropertyChanged(Platform::String^ prop);

  private:
    void StartThreadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);

    rtc::Thread* thread_;
  };
}
