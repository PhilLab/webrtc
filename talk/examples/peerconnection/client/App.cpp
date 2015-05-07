#include <collection.h>
#include <ppltasks.h>
#include <string>

using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Core;

CoreDispatcher^ g_windowDispatcher;

namespace peerconnectionclient
{
	ref class App sealed : public Windows::UI::Xaml::Application
	{
	private:
		TextBlock^ ipLabel_;
		TextBox^ ipTextBox_;
		TextBlock^ portLabel_;
		TextBox^ portTextBox_;
		Button^ connectButton_;
		TextBlock^ peersLabel_;
		ListView^ peersListBox_;
		MediaElement^ localMedia_;
		MediaElement^ remoteMedia_;
		CoreDispatcher^ dispatcher_;

	public:
		App()
		{

		}


	protected:
		InputScope^ CreateInputScope() {
			auto inputScope = ref new InputScope();
			auto scopeName = ref new InputScopeName();
			scopeName->NameValue = InputScopeNameValue::Number;
			inputScope->Names->Append(scopeName);
			return inputScope;
		}

		virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
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
			portTextBox_->InputScope = CreateInputScope();
			portStackPanel->Children->Append(portTextBox_);

			connectButton_ = ref new Button();
			connectButton_->Content = "Connect";
			connectButton_->Margin = ThicknessHelper::FromLengths(4, 4, 0, 4);
			connectButton_->HorizontalAlignment = HorizontalAlignment::Right;
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
	};
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
