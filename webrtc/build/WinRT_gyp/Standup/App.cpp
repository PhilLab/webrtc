#include <collection.h>
#include <ppltasks.h>
#include <string>

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

bool autoClose = false;

namespace StandupWinRT
{
  ref class App sealed : public Windows::UI::Xaml::Application
  {
  public:
    App()
    {
    }

  private:
    TextBox^ ipTextBox_;
    TextBox^ portTextBox_;

    MediaElement^ localMedia_;
    MediaElement^ remoteMedia_;

    Button^ startStopButton_;
    Button^ switchCameraButton_;

  protected:
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      auto layoutRoot = ref new Grid();
      layoutRoot->Margin = ThicknessHelper::FromUniformLength(32);

      // First row (ip and port fields)
      {
        auto row = ref new RowDefinition();
        row->Height = GridLength(32, GridUnitType::Pixel);
        layoutRoot->RowDefinitions->Append(row);

        auto stackPanel = ref new StackPanel();
        stackPanel->Orientation = Orientation::Horizontal;
        Grid::SetRow(stackPanel, 0);
        layoutRoot->Children->Append(stackPanel);

        auto label = ref new TextBlock();
        label->Text = "IP: ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(4, 0, 4, 0);
        stackPanel->Children->Append(label);

        ipTextBox_ = ref new TextBox();
        ipTextBox_->Width = 150;
        stackPanel->Children->Append(ipTextBox_);

        label = ref new TextBlock();
        label->Text = "Port: ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(8, 0, 4, 0);
        stackPanel->Children->Append(label);

        portTextBox_ = ref new TextBox();
        stackPanel->Children->Append(portTextBox_);
      }

      // Second row
      {
        auto row = ref new RowDefinition();
        row->Height = GridLength(1, GridUnitType::Star);
        layoutRoot->RowDefinitions->Append(row);

        // (2x1) Grid to contain the rendering surfaces
        auto grid = ref new Grid();
        grid->ColumnDefinitions->Append(ref new ColumnDefinition());
        grid->ColumnDefinitions->Append(ref new ColumnDefinition());
        layoutRoot->Children->Append(grid);
        Grid::SetRow(grid, 1);

        auto makeRenderSurface = [grid](MediaElement^& elem, int index) {
          auto surface = ref new MediaElement();
          auto border = ref new Border();
          border->BorderBrush = ref new SolidColorBrush(ColorHelper::FromArgb(255, 0, 0, 255));
          border->BorderThickness = ThicknessHelper::FromUniformLength(2);
          border->Margin = ThicknessHelper::FromUniformLength(8);
          grid->Children->Append(border);
          Grid::SetColumn(border, index);
        };

        makeRenderSurface(localMedia_, 0);
        makeRenderSurface(remoteMedia_, 1);
      }

      // Third row (Start/Stop button)
      {
        auto row = ref new RowDefinition();
        row->Height = GridLength(40, GridUnitType::Pixel);
        layoutRoot->RowDefinitions->Append(row);

        auto stackPanel = ref new StackPanel();
        stackPanel->Orientation = Orientation::Horizontal;
        Grid::SetRow(stackPanel, 2);
        layoutRoot->Children->Append(stackPanel);

        startStopButton_ = ref new Button();
        startStopButton_->Content = "Start";
        startStopButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStartStopClick);
        stackPanel->Children->Append(startStopButton_);

        switchCameraButton_ = ref new Button();
        switchCameraButton_->Content = "Switch Camera";
        switchCameraButton_->Margin = ThicknessHelper::FromLengths(40, 0, 0, 0);
        switchCameraButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnSwitchCameraClick);
        switchCameraButton_->IsEnabled = false;
        stackPanel->Children->Append(switchCameraButton_);
      }

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
    }

    void OnStartStopClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
};

}

int __cdecl main(::Platform::Array<::Platform::String^>^ args)
{
  (void)args; // Unused parameter
  Windows::UI::Xaml::Application::Start(
    ref new Windows::UI::Xaml::ApplicationInitializationCallback(
    [](Windows::UI::Xaml::ApplicationInitializationCallbackParams^ p) {
    (void)p; // Unused parameter
    auto app = ref new StandupWinRT::App();
  }));

  return 0;
}


void StandupWinRT::App::OnStartStopClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  // TODO: Start and stop the audio/video.
}


void StandupWinRT::App::OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  throw ref new Platform::NotImplementedException();
}
