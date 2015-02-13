//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#include "webrtc\base\common.h"

using namespace Test;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=234238

MainPage::MainPage()
  : thread_(nullptr)
{
	InitializeComponent();
  ThreadNotRunning = true;
  thread_ = new rtc::Thread();
  DataContext = this;
}

class MyRunnable : public rtc::Runnable
{
public:
  MyRunnable(MainPage^ page)
    : page_(page)
  {

  }

  virtual void Run(rtc::Thread* thread)
  {
    page_->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
      [this]()
    {
      page_->ThreadNotRunning = false;
      page_->NotifyPropertyChanged("ThreadNotRunning");
    }));

    rtc::Thread::SleepMs(5000);

    page_->Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler(
      [this]()
    {
      page_->ThreadNotRunning = true;
      page_->NotifyPropertyChanged("ThreadNotRunning");
    }));
  }

private:
  MainPage^ page_;
};

void Test::MainPage::NotifyPropertyChanged(Platform::String^ prop){
  PropertyChanged(this, ref new PropertyChangedEventArgs(prop));
}

void Test::MainPage::StartThreadButton_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
  thread_->Start(new MyRunnable(this));
}

