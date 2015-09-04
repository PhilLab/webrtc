
#include "App.h"
#include "WinRTMainWnd.h"
#include "conductor.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include <collection.h>
#include <ppltasks.h>
#include <string>

using namespace Platform::Collections;
using namespace Windows::UI;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Core;
using namespace Windows::UI::Popups;
using namespace Windows::Foundation::Collections;

using namespace peerconnectionclient;

CoreDispatcher^ g_windowDispatcher;

App::App()
{
  rtc::EnsureWinsockInit();
  rtc::InitializeSSL();
  workerThread_.Start();

  workerThread_.Invoke<void>([this]
  {
    mainWnd_ = new WinRTMainWnd();
    mainWnd_->RegisterParentApp(this);
    peerConnectionClient_ = new PeerConnectionClient();
    conductor_ = new rtc::RefCountedObject<Conductor>(peerConnectionClient_, mainWnd_);
    conductor_->AddRef();
  });
}

App::~App()
{
  workerThread_.Invoke<void>([this]
  {
    conductor_->Release();
    delete peerConnectionClient_;
    delete mainWnd_;
    rtc::CleanupSSL();
  });
}

//WinRTMainWnd callbacks
void App::MessageBox(::Platform::String^ caption, ::Platform::String^ text, bool is_error)
{
	MessageDialog^ msgDialog = ref new MessageDialog(text, caption);
	msgDialog->ShowAsync();
}

void App::UpdatePeersList(IMap<int32, Platform::String^>^ peers)
{
  g_windowDispatcher->RunAsync(CoreDispatcherPriority::Normal,
    ref new DispatchedHandler([this, peers]
  {
    for (auto peer : peers)
    {
      peersListBox_->Items->Append(peer);
    }
  }));
}

void App::StartLocalRenderer(webrtc::VideoTrackInterface* local_video)
{

}

void App::StopLocalRenderer()
{

}

void App::StartRemoteRenderer(webrtc::VideoTrackInterface* remote_video)
{

}

void App::StopRemoteRenderer()
{

}

//Helper methods
InputScope^ App::CreateInputScope()
{
	auto inputScope = ref new InputScope();
	auto scopeName = ref new InputScopeName();
	scopeName->NameValue = InputScopeNameValue::Number;
	inputScope->Names->Append(scopeName);
	return inputScope;
}

void App::OnLaunched(LaunchActivatedEventArgs^ e)
{
	g_windowDispatcher = dispatcher_ = Window::Current->Dispatcher;

	auto rootGrid = ref new Grid();
	rootGrid->Margin = ThicknessHelper::FromUniformLength(32);

	auto column = ref new ColumnDefinition();
	column->Width = GridLength(300, GridUnitType::Pixel);
	rootGrid->ColumnDefinitions->Append(column);

	column = ref new ColumnDefinition();
	column->Width = GridLength(1, GridUnitType::Star);
	rootGrid->ColumnDefinitions->Append(column);

	auto leftStackPanel = ref new StackPanel();
	leftStackPanel->Orientation = Orientation::Vertical;
	leftStackPanel->VerticalAlignment = VerticalAlignment::Top;
	Grid::SetColumn(leftStackPanel, 0);
	rootGrid->Children->Append(leftStackPanel);

	auto ipStackPanel = ref new StackPanel();
	ipStackPanel->Orientation = Orientation::Horizontal;
	ipStackPanel->VerticalAlignment = VerticalAlignment::Top;
	ipStackPanel->HorizontalAlignment = HorizontalAlignment::Right;
	leftStackPanel->Children->Append(ipStackPanel);

	ipLabel_ = ref new TextBlock();
	ipLabel_->Text = "IP";
	ipLabel_->VerticalAlignment = VerticalAlignment::Center;
	ipLabel_->Margin = ThicknessHelper::FromLengths(4, 4, 4, 4);
	ipStackPanel->Children->Append(ipLabel_);

	ipTextBox_ = ref new TextBox();
	ipTextBox_->Width = 180;
	ipTextBox_->Margin = ThicknessHelper::FromLengths(4, 4, 4, 4);
	ipTextBox_->Text = "localhost";
	ipTextBox_->InputScope = CreateInputScope();
	ipStackPanel->Children->Append(ipTextBox_);

	auto portStackPanel = ref new StackPanel();
	portStackPanel->Orientation = Orientation::Horizontal;
	portStackPanel->VerticalAlignment = VerticalAlignment::Top;
	portStackPanel->HorizontalAlignment = HorizontalAlignment::Right;
	leftStackPanel->Children->Append(portStackPanel);

	portLabel_ = ref new TextBlock();
	portLabel_->Text = "Port";
	portLabel_->VerticalAlignment = VerticalAlignment::Center;
	portLabel_->Margin = ThicknessHelper::FromLengths(4, 4, 4, 4);
	portStackPanel->Children->Append(portLabel_);

	portTextBox_ = ref new TextBox();
	portTextBox_->Width = 180;
	portTextBox_->Margin = ThicknessHelper::FromLengths(4, 4, 4, 4);
	portTextBox_->Text = "8888";
	portTextBox_->InputScope = CreateInputScope();
	portStackPanel->Children->Append(portTextBox_);

	connectButton_ = ref new Button();
	connectButton_->Content = "Connect";
	connectButton_->Margin = ThicknessHelper::FromLengths(4, 4, 0, 4);
	connectButton_->HorizontalAlignment = HorizontalAlignment::Right;
	connectButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &App::OnStartStopClick);
	leftStackPanel->Children->Append(connectButton_);

	peersLabel_ = ref new TextBlock();
	peersLabel_->Text = "Connected peers";
	peersLabel_->HorizontalAlignment = HorizontalAlignment::Left;
	peersLabel_->Margin = ThicknessHelper::FromLengths(4, 4, 4, 4);
	leftStackPanel->Children->Append(peersLabel_);

	peersListBox_ = ref new ListView();
	peersListBox_->Margin = ThicknessHelper::FromLengths(4, 4, 0, 4);
	peersListBox_->Height = 400;
  peersListBox_->Background = ref new SolidColorBrush(Colors::White);
  peersListBox_->Foreground = ref new SolidColorBrush(Colors::Black);
	leftStackPanel->Children->Append(peersListBox_);

	localMedia_ = ref new MediaElement();
	localMedia_->Width = 300;
	localMedia_->Height = 300;
	auto localMediaBorder = ref new Border();
	localMediaBorder->BorderBrush = ref new SolidColorBrush(Colors::Blue);
	localMediaBorder->BorderThickness = ThicknessHelper::FromUniformLength(2);
	localMediaBorder->Margin = ThicknessHelper::FromUniformLength(4);
	localMediaBorder->Child = localMedia_;
	leftStackPanel->Children->Append(localMediaBorder);

	remoteMedia_ = ref new MediaElement();
	auto remoteMediaBorder = ref new Border();
	remoteMediaBorder->BorderBrush = ref new SolidColorBrush(Colors::Blue);
	remoteMediaBorder->BorderThickness = ThicknessHelper::FromUniformLength(2);
	remoteMediaBorder->Margin = ThicknessHelper::FromUniformLength(4);
	remoteMediaBorder->Child = remoteMedia_;
	Grid::SetColumn(remoteMediaBorder, 1);
	rootGrid->Children->Append(remoteMediaBorder);

	Window::Current->Content = rootGrid;
	Window::Current->Activate();
}

void App::OnStartStopClick(Platform::Object ^sender, RoutedEventArgs ^e)
{
	auto finalIp = ref new Platform::String();
	if (ipTextBox_->Text->Equals(L"localhost"))
	{
		finalIp = "127.0.0.1";
	}
	else
	{
		finalIp = ipTextBox_->Text;
	}
	int port = _wtof(portTextBox_->Text->Data());

  workerThread_.Invoke<void>([this, finalIp, port]
  {
    mainWnd_->StartLogin(finalIp, port);
  });
}


int __cdecl main(::Platform::Array<::Platform::String^>^ args)
{
	(void)args; // Unused parameter
	Windows::UI::Xaml::Application::Start(
		ref new Windows::UI::Xaml::ApplicationInitializationCallback([](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p)
	{
		(void)p; // Unused parameter
		auto app = ref new peerconnectionclient::App();
	}));
	return 0;
}
