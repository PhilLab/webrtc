#include <collection.h>
#include <ppltasks.h>
#include <string>

#include "webrtc/modules/video_capture/include/video_capture.h"
#include "webrtc/modules/video_capture/include/video_capture_factory.h"
#include "webrtc/modules/video_render/include/video_render.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"
#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/video_frame.h"

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

bool autoClose = false;
Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

namespace StandupWinRT
{
  class TestCaptureCallback : public webrtc::VideoCaptureDataCallback
  {
  public:

    virtual ~TestCaptureCallback() {};

    TestCaptureCallback(webrtc::VideoRenderCallback* rendererCallback) :
      _rendererCallback(rendererCallback)
    {

    }

    virtual void OnIncomingCapturedFrame(const int32_t id,
      webrtc::I420VideoFrame& videoFrame)
    {
      _rendererCallback->RenderFrame(1, videoFrame);
    }

    virtual void OnCaptureDelayChanged(const int32_t id,
      const int32_t delay)
    {

    }

  private:
    webrtc::VideoRenderCallback* _rendererCallback;
  };

  class TestTraceCallback : public webrtc::TraceCallback
  {
  public:

    virtual void Print(webrtc::TraceLevel level, const char* message, int length)
    {
      WCHAR szTextBuf[1024];
      int cTextBufSize = MultiByteToWideChar(CP_UTF8, 0, message, length + 2, NULL, 0);
      MultiByteToWideChar(CP_UTF8, 0, message, length + 2, szTextBuf, cTextBufSize);
      szTextBuf[cTextBufSize - 3] = L'\r';
      szTextBuf[cTextBufSize - 2] = L'\n';
      szTextBuf[cTextBufSize - 1] = 0;
      OutputDebugString(szTextBuf);
    }
  };

  ref class App sealed : public Windows::UI::Xaml::Application
  {
  public:
    App() :
      traceCallback_(new TestTraceCallback()),
      started_(false)
    {
      webrtc::Trace::CreateTrace();
      webrtc::Trace::SetTraceCallback(traceCallback_);
      webrtc::Trace::set_level_filter(webrtc::kTraceAll);
    }

  private:
    TextBox^ ipTextBox_;
    TextBox^ portTextBox_;

    MediaElement^ localMedia_;
    MediaElement^ remoteMedia_;

    Button^ startStopButton_;
    Button^ switchCameraButton_;

    webrtc::VideoCaptureModule* vcpm_;
    webrtc::VideoRender* vrm_;
    webrtc::VideoCaptureDataCallback* captureCallback_;
    webrtc::TraceCallback* traceCallback_;
    Windows::UI::Core::CoreDispatcher^ dispatcher_;
    bool started_;

  protected:
    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      g_windowDispatcher = dispatcher_ = Window::Current->Dispatcher;

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
          elem = surface;
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
  if (!started_) {
    Concurrency::create_task([this]() {
      webrtc::VideoCaptureModule::DeviceInfo* dev_info =
        webrtc::VideoCaptureFactory::CreateDeviceInfo(0);

      char device_name[128];
      char device_unique_name[512];

      dev_info->GetDeviceName(0,
        device_name,
        sizeof(device_name),
        device_unique_name,
        sizeof(device_unique_name));

      webrtc::VideoCaptureModule* vcpm_ =
        webrtc::VideoCaptureFactory::Create(0, device_unique_name);

      vcpm_->AddRef();
      
      webrtc::VideoCaptureCapability capability;
      dev_info->GetCapability(device_unique_name, 0, capability);

      delete dev_info;

      IInspectable* videoRendererPtr = reinterpret_cast<IInspectable*>(localMedia_);

      vrm_ = webrtc::VideoRender::CreateVideoRender(1, videoRendererPtr, false);

      webrtc::VideoRenderCallback* rendererCallback = vrm_->AddIncomingRenderStream(1, 0, 0.0, 0.0, 1.0, 1.0);

      captureCallback_ = new TestCaptureCallback(rendererCallback);

      vcpm_->RegisterCaptureDataCallback(*captureCallback_);

      vcpm_->StartCapture(capability);

      vrm_->StartRender(1);

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Stop";
      })));
    });
  }
  else
  {
    Concurrency::create_task([this]() {
      vcpm_->StopCapture();
      vcpm_->Release();
      delete (TestCaptureCallback*)captureCallback_;

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Start";
      })));
    });
  }
}


void StandupWinRT::App::OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  throw ref new Platform::NotImplementedException();
}
