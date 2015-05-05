#include <collection.h>
#include <ppltasks.h>
#include <string>

#include "webrtc/modules/video_capture/include/video_capture.h"
#include "webrtc/modules/video_capture/include/video_capture_factory.h"
#include "webrtc/modules/video_render/include/video_render.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"
#include "webrtc/system_wrappers/interface/trace.h"
#include "webrtc/video_frame.h"

#include "webrtc/voice_engine/include/voe_base.h"
#include "webrtc/voice_engine/include/voe_codec.h"
#include "webrtc/voice_engine/include/voe_network.h"
#include "webrtc/voice_engine/include/voe_rtp_rtcp.h"
#include "webrtc/voice_engine/include/voe_audio_processing.h"
#include "webrtc/voice_engine/include/voe_volume_control.h"
#include "webrtc/voice_engine/include/voe_hardware.h"
#include "webrtc/voice_engine/include/voe_file.h"

#include "webrtc/video_engine/include/vie_base.h"
#include "webrtc/video_engine/include/vie_network.h"
#include "webrtc/video_engine/include/vie_render.h"
#include "webrtc/video_engine/include/vie_capture.h"
#include "webrtc/video_engine/include/vie_codec.h"
#include "webrtc/video_engine/include/vie_rtp_rtcp.h"

#include "webrtc/test/channel_transport/include/channel_transport.h"
#include "webrtc/test/field_trial.h"

#include "webrtc/base/loggingserver.h"
#include "webrtc/base/socketaddress.h"
#include "webrtc/base/logging.h"
#include "webrtc\system_wrappers\interface\event_tracer.h"
#include "webrtc\base\tracelog.h"

#define VOICE
#define VIDEO

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Windows::Graphics::Display;

bool autoClose = false;
Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#define PREFERRED_FRAME_WIDTH 640
#define PREFERRED_FRAME_HEIGHT 480
#define PREFERRED_MAX_FPS 30
#define CAPTURE_DEVICE_INDEX 0
#define MAX_BITRATE 500
#else
#define PREFERRED_FRAME_WIDTH 800
#define PREFERRED_FRAME_HEIGHT 600
#define PREFERRED_MAX_FPS 30
#define CAPTURE_DEVICE_INDEX 0
#define MAX_BITRATE 1000
#endif

rtc::TraceLog tl;

const unsigned char* __cdecl GetCategoryGroupEnabled(const char* category_group)
{
  return reinterpret_cast<const unsigned char*>("webrtc");
}

void __cdecl AddTraceEvent(char phase,
  const unsigned char* category_group_enabled,
  const char* name,
  unsigned long long id,
  int num_args,
  const char** arg_names,
  const unsigned char* arg_types,
  const unsigned long long* arg_values,
  unsigned char flags)
{
  tl.Add(phase, category_group_enabled, name, id, num_args, arg_names, arg_types, arg_values, flags);
}

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
      started_(false),
      startedVideo_(false),
      voiceTransport_(NULL),
      videoTransport_(NULL),
      voiceChannel_(-1),
      captureId_(-1),
      videoChannel_(-1)
    {
      int error;

      webrtc::test::InitFieldTrialsFromString("");
      webrtc::Trace::CreateTrace();
      webrtc::Trace::SetTraceCallback(traceCallback_);
      webrtc::Trace::set_level_filter(webrtc::kTraceAll);

      //provide some default values if user want to test on local machine 
      remoteIpAddress_ = "127.0.0.1";
      audioPort_ = 20000;
      videoPort_ = 20100;

#ifdef VOICE
      voiceEngine_ = webrtc::VoiceEngine::Create();
      if (voiceEngine_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to create voice engine.");
        return;
      }
      voiceBase_ = webrtc::VoEBase::GetInterface(voiceEngine_);
      if (voiceBase_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for voice base.");
        return;
      }
      voiceCodec_ = webrtc::VoECodec::GetInterface(voiceEngine_);
      if (voiceCodec_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for voice codec.");
        return;
      }
      voiceNetwork_ = webrtc::VoENetwork::GetInterface(voiceEngine_);
      if (voiceNetwork_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for voice network.");
        return;
      }
      voiceRtpRtcp_ = webrtc::VoERTP_RTCP::GetInterface(voiceEngine_);
      if (voiceRtpRtcp_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for voice RTP/RTCP.");
        return;
      }
      voiceAudioProcessing_ = webrtc::VoEAudioProcessing::GetInterface(voiceEngine_);
      if (voiceAudioProcessing_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for audio processing.");
        return;
      }
      voiceVolumeControl_ = webrtc::VoEVolumeControl::GetInterface(voiceEngine_);
      if (voiceVolumeControl_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for volume control.");
        return;
      }
      voiceHardware_ = webrtc::VoEHardware::GetInterface(voiceEngine_);
      if (voiceHardware_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for audio hardware.");
        return;
      }
      voiceFile_ = webrtc::VoEFile::GetInterface(voiceEngine_);
      if (voiceFile_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to get interface for voice file.");
        return;
      }

      error = voiceHardware_->SetAudioDeviceLayer(webrtc::kAudioWindowsWasapi);
      if (error < 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set audio device layer type. Error: %d", voiceBase_->LastError());
        return;
      }

      error = voiceBase_->Init();
      if (error < 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to initialize voice base. Error: %d", voiceBase_->LastError());
        return;
      }
      else if (voiceBase_->LastError() > 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "An error has occured during voice base init. Error: %d", voiceBase_->LastError());
      }
#endif
#ifdef VIDEO
      videoEngine_ = webrtc::VideoEngine::Create();
      if (videoEngine_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to create video engine.");
        return;
      }

      videoBase_ = webrtc::ViEBase::GetInterface(videoEngine_);
      if (videoBase_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get interface for video base.");
        return;
      }
      videoCapture_ = webrtc::ViECapture::GetInterface(videoEngine_);
      if (videoCapture_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed get interface for video capture.");
        return;
      }
      videoRtpRtcp_ = webrtc::ViERTP_RTCP::GetInterface(videoEngine_);
      if (videoRtpRtcp_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get interface for video RTP/RTCP.");
        return;
      }
      videoNetwork_ = webrtc::ViENetwork::GetInterface(videoEngine_);
      if (videoNetwork_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get interface for video network.");
        return;
      }
      videoRender_ = webrtc::ViERender::GetInterface(videoEngine_);
      if (videoRender_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get interface for video render.");
        return;
      }
      videoCodec_ = webrtc::ViECodec::GetInterface(videoEngine_);
      if (videoCodec_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get interface for video codec.");
        return;
      }

      error = videoBase_->Init();
      if (error < 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to initialize video base. Error: %d", videoBase_->LastError());
        return;
      } else if (videoBase_->LastError() > 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "An error has occured during video base init. Error: %d", videoBase_->LastError());
      }
#ifdef VOICE
      error = videoBase_->SetVoiceEngine(voiceEngine_);
      if (error < 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set voice engine for video base. Error: %d", videoBase_->LastError());
        return;
      }
#endif
#endif
    }

    virtual ~App() {

      int error;
#ifdef VOICE
      if (voiceBase_) {
        error = voiceBase_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release voice base. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceCodec_) {
        error = voiceCodec_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release voice codec. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceNetwork_) {
        error = voiceNetwork_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release voice network. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceRtpRtcp_) {
        error = voiceRtpRtcp_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release voice RTP/RTCP. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceAudioProcessing_) {
        error = voiceAudioProcessing_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release audio processing. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceVolumeControl_) {
        error = voiceVolumeControl_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release volume control. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceHardware_) {
        error = voiceHardware_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release audio hardware. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (voiceFile_) {
        error = voiceFile_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to release voice file. Error: %d", voiceBase_->LastError());
          return;
        }
      }

      if (!webrtc::VoiceEngine::Delete(voiceEngine_)) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to delete voice engine.");
        return;
      }
#endif
#ifdef VIDEO
      if (videoBase_) {
        error = videoBase_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video base. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (videoNetwork_) {
        error = videoNetwork_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video network. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (videoRender_) {
        error = videoRender_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video render. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (videoCapture_) {
        error = videoCapture_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video capture. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (videoRtpRtcp_) {
        error = videoRtpRtcp_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video RTP/RTCP. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (videoCodec_) {
        error = videoCodec_->Release();
        if (error < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video codec. Error: %d", videoBase_->LastError());
          return;
        }
      }

      if (!webrtc::VideoEngine::Delete(videoEngine_)) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to delete video engine.");
        return;
      }
#endif
    }

  private:
    TextBox^ ipTextBox_;
    TextBox^ videoPortTextBox_;
    TextBox^ audioPortTextBox_;
    TextBox^ ipRemoteTraces_;
    TextBox^ portRemoteTraces_;

    MediaElement^ localMedia_;
    MediaElement^ remoteMedia_;

    Button^ startStopButton_;
    Button^ switchCameraButton_;
    Button^ startStopVideoButton_;
    Button^ startTracingButton_;
    Button^ stopTracingButton_;
    Button^ sendTracesButton_;

    webrtc::VideoRender* vrm_;
    webrtc::VideoCaptureDataCallback* captureCallback_;
    webrtc::TraceCallback* traceCallback_;
    Windows::UI::Core::CoreDispatcher^ dispatcher_;
    bool started_;
    bool startedVideo_;

    int audioPort_;
    int videoPort_;
    std::string remoteIpAddress_;

    int voiceChannel_;
    webrtc::test::VoiceChannelTransport* voiceTransport_;
    webrtc::VoiceEngine* voiceEngine_;
    webrtc::VoEBase* voiceBase_;
    webrtc::VoECodec* voiceCodec_;
    webrtc::VoENetwork* voiceNetwork_;
    webrtc::VoERTP_RTCP* voiceRtpRtcp_;
    webrtc::VoEAudioProcessing* voiceAudioProcessing_;
    webrtc::VoEVolumeControl* voiceVolumeControl_;
    webrtc::VoEHardware* voiceHardware_;
    webrtc::VoEFile* voiceFile_;

    int videoChannel_;
    webrtc::test::VideoChannelTransport* videoTransport_;
    int captureId_;
    char deviceUniqueId_[512];
	Concurrency::event stopEvent_;
    webrtc::VideoCaptureModule* vcpm_;
    webrtc::VideoEngine* videoEngine_;
    webrtc::ViEBase* videoBase_;
    webrtc::ViENetwork* videoNetwork_;
    webrtc::ViERender* videoRender_;
    webrtc::ViECapture* videoCapture_;
    webrtc::ViERTP_RTCP* videoRtpRtcp_;
    webrtc::ViECodec* videoCodec_;

  protected:

    InputScope^ CreateInputScope() {
      auto inputScope = ref new Windows::UI::Xaml::Input::InputScope();
      auto scopeName = ref new Windows::UI::Xaml::Input::InputScopeName();
      scopeName->NameValue = Windows::UI::Xaml::Input::InputScopeNameValue::Number;
      inputScope->Names->Append(scopeName);
      return inputScope;
    }

    virtual void OnLaunched(Windows::ApplicationModel::Activation::LaunchActivatedEventArgs^ e) override
    {
      g_windowDispatcher = dispatcher_ = Window::Current->Dispatcher;

      auto layoutRoot = ref new Grid();
      layoutRoot->Margin = ThicknessHelper::FromUniformLength(32);

      auto settings = ApplicationData::Current->LocalSettings->Values;

      // First row (ip and port fields)
      {
        auto row = ref new RowDefinition();
        row->Height = GridLength(32, GridUnitType::Pixel);
        layoutRoot->RowDefinitions->Append(row);

        auto viewBox = ref new Viewbox;
        Grid::SetRow(viewBox, 0);
        layoutRoot->Children->Append(viewBox);

        auto stackPanel = ref new StackPanel();
        stackPanel->Orientation = Orientation::Horizontal;
        viewBox->Child = stackPanel;

        auto label = ref new TextBlock();
        label->Text = "IP: ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(4, 0, 4, 0);
        stackPanel->Children->Append(label);

        ipTextBox_ = ref new TextBox();
        ipTextBox_->Width = 150;
        ipTextBox_->Text = settings->Lookup("remote_ip")->ToString();
        ipTextBox_->InputScope = CreateInputScope();
        stackPanel->Children->Append(ipTextBox_);

        label = ref new TextBlock();
        label->Text = "Video Port: ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(8, 0, 4, 0);
        stackPanel->Children->Append(label);

        videoPortTextBox_ = ref new TextBox();
        videoPortTextBox_->Text = settings->Lookup("video_port")->ToString();
        videoPortTextBox_->InputScope = CreateInputScope();
        stackPanel->Children->Append(videoPortTextBox_);

        label = ref new TextBlock();
        label->Text = "Audio Port: ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(8, 0, 4, 0);
        stackPanel->Children->Append(label);

        audioPortTextBox_ = ref new TextBox();
        audioPortTextBox_->Text = settings->Lookup("audio_port")->ToString();
        audioPortTextBox_->InputScope = CreateInputScope();
        stackPanel->Children->Append(audioPortTextBox_);
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
#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
          surface->Width = PREFERRED_FRAME_WIDTH / 4;
          surface->Height = PREFERRED_FRAME_HEIGHT / 4;
#else
          surface->Width = PREFERRED_FRAME_WIDTH;
          surface->Height = PREFERRED_FRAME_HEIGHT;
#endif
          border->BorderBrush = ref new SolidColorBrush(ColorHelper::FromArgb(255, 0, 0, 255));
          border->BorderThickness = ThicknessHelper::FromUniformLength(2);
          border->Margin = ThicknessHelper::FromUniformLength(8);
          border->Child = surface;
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
        switchCameraButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        switchCameraButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnSwitchCameraClick);
        switchCameraButton_->IsEnabled = false;
        stackPanel->Children->Append(switchCameraButton_);

        startStopVideoButton_ = ref new Button();
        startStopVideoButton_->Content = "Start Video";
        startStopVideoButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        startStopVideoButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStartStopVideoClick);
        startStopVideoButton_->IsEnabled = true;
        stackPanel->Children->Append(startStopVideoButton_);


        startTracingButton_ = ref new Button();
        startTracingButton_->Content = "Start Tracing";
        startTracingButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        startTracingButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStartTracingClick);
        stackPanel->Children->Append(startTracingButton_);

        stopTracingButton_ = ref new Button();
        stopTracingButton_->Content = "Stop Tracing";
        stopTracingButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        stopTracingButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStopTracingClick);
        stackPanel->Children->Append(stopTracingButton_);

        sendTracesButton_ = ref new Button();
        sendTracesButton_->Content = "Send Traces";
        sendTracesButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        sendTracesButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnSendTracesClick);
        stackPanel->Children->Append(sendTracesButton_);

        auto label = ref new TextBlock();
        label->Text = "IP(Traces): ";
        label->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(4, 0, 4, 0);
        stackPanel->Children->Append(label);

        ipRemoteTraces_ = ref new TextBox();
        ipRemoteTraces_->Width = 150;
        ipRemoteTraces_->Height = 32;
        stackPanel->Children->Append(ipRemoteTraces_);

        label = ref new TextBlock();
        label->Text = "Port(Traces): ";
        label->VerticalAlignment = VerticalAlignment::Center;
        label->Margin = ThicknessHelper::FromLengths(8, 0, 4, 0);
        stackPanel->Children->Append(label);

        portRemoteTraces_ = ref new TextBox();
        portRemoteTraces_->Height = 32;
        stackPanel->Children->Append(portRemoteTraces_);
      }

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
    }

    void OnStartStopClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStartTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStopTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSendTracesClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
  private:

    /**
     * initialize transport information provided by user input.
     * WARNING: this function has be called from Main UI thread.
     */
    void initializeTranportInfo();

    /**
     * string representation of raw video format.
     */
    std::string getRawVideoFormatString(webrtc::RawVideoType videoType);

    void SaveSettings();
};

}

static std::string GetIP(String^ stringIP) {

  std::wstring ipW(stringIP->Begin());
  std::string ip(ipW.begin(),ipW.end());
  //ToDo, trim the string
  return ip;
}

int __cdecl main(::Platform::Array<::Platform::String^>^ args)
{
  rtc::SocketAddress sa(INADDR_ANY, 47002);
  rtc::LoggingServer ls;
  ls.Listen(sa, rtc::LS_INFO);

  webrtc::SetupEventTracer(&GetCategoryGroupEnabled, &AddTraceEvent);

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
  if (started_) {
	  stopEvent_.set();
	  started_ = false;
  }
  else {
	  SaveSettings();
	  stopEvent_.reset();
      initializeTranportInfo();

    Concurrency::create_task([this]() {
      int error;
#ifdef VOICE
      voiceChannel_ = voiceBase_->CreateChannel();
      if (voiceChannel_ < 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Could not create voice channel. Error: %d", voiceBase_->LastError());
          voiceChannel_ = -1;
          return Concurrency::task<void>();
      }

      voiceTransport_ = new webrtc::test::VoiceChannelTransport(voiceNetwork_, voiceChannel_);
      if (voiceTransport_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Could not create voice channel transport.");
        return Concurrency::task<void>();
      }

      webrtc::EcModes ecMode = webrtc::kEcAec;
      error = voiceAudioProcessing_->SetEcStatus(true, ecMode);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set acoustic echo canceller status. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }
      if (ecMode == webrtc::kEcAecm) {
        error = voiceAudioProcessing_->SetAecmMode(webrtc::kAecmSpeakerphone);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set acoustic echo canceller mobile mode. Error: %d", voiceBase_->LastError());
          return Concurrency::task<void>();
        }
      }

      error = voiceAudioProcessing_->SetAgcStatus(true, webrtc::kAgcAdaptiveDigital);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set automatic gain control status. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      error = voiceAudioProcessing_->SetNsStatus(true, webrtc::kNsLowSuppression);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set noise suppression status. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      error = voiceVolumeControl_->SetInputMute(-1, false);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set microphone mute. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      webrtc::CodecInst cinst;
      memset(&cinst, 0, sizeof(webrtc::CodecInst));
      for (int idx = 0; idx < voiceCodec_->NumOfCodecs(); idx++) {
        error = voiceCodec_->GetCodec(idx, cinst);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to get voice codec. Error: %d", voiceBase_->LastError());
          return Concurrency::task<void>();
        }
        if (strcmp(cinst.plname, "OPUS") == 0) {
          strcpy(cinst.plname, "OPUS");
          cinst.pltype = 110;
          cinst.rate = 20000;
          cinst.pacsize = 320; // 20ms
          cinst.plfreq = 16000;
          cinst.channels = 1;
          error = voiceCodec_->SetSendCodec(voiceChannel_, cinst);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
              "Failed to set send voice codec. Error: %d", voiceBase_->LastError());
            return Concurrency::task<void>();
          }
          break;
        }
      }

      error = voiceTransport_->SetSendDestination(remoteIpAddress_.c_str(), audioPort_);

      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set send destination for voice channel.");
        return Concurrency::task<void>();
      }

      error = voiceTransport_->SetLocalReceiver(audioPort_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set local receiver for voice channel.");
        return Concurrency::task<void>();
      }

      error = voiceBase_->StartSend(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to start sending voice. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      error = voiceBase_->StartReceive(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to start receiving voice. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      error = voiceBase_->StartPlayout(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to start playout. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

#endif
#ifdef VIDEO
      const unsigned int KMaxDeviceNameLength = 128;
      const unsigned int KMaxUniqueIdLength = 256;
      char deviceName[KMaxDeviceNameLength];
      memset(deviceName, 0, KMaxDeviceNameLength);
      char uniqueId[KMaxUniqueIdLength];
      memset(uniqueId, 0, KMaxUniqueIdLength);
      uint32_t captureIdx = CAPTURE_DEVICE_INDEX;

      int devicesNumber = videoCapture_->NumberOfCaptureDevices();

      for (int i = 0; i < devicesNumber; i++) {

        error = videoCapture_->GetCaptureDevice(i, deviceName,
          KMaxDeviceNameLength, uniqueId,
          KMaxUniqueIdLength);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to get video device name.");
          return Concurrency::task<void>();
        }

        webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
          "Capture device - index: %d, name: %s, unique ID: %s", i, deviceName, uniqueId);
      }

      error = videoCapture_->GetCaptureDevice(captureIdx, deviceName,
        KMaxDeviceNameLength, uniqueId,
        KMaxUniqueIdLength);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get video device name.");
        return Concurrency::task<void>();
      }

      strcpy(deviceUniqueId_, uniqueId);

      webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
        "Selected capture device - index: %d, name: %s, unique ID: %s", captureIdx, deviceName, uniqueId);

      vcpm_ = webrtc::VideoCaptureFactory::Create(1, uniqueId);
      if (vcpm_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to create video capture module.");
        return Concurrency::task<void>();
      }

      error = videoCapture_->AllocateCaptureDevice(*vcpm_, captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to allocate video capture device. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }
      vcpm_->AddRef();

      int capabilitiesNumber = videoCapture_->NumberOfCapabilities(uniqueId, KMaxUniqueIdLength);

      webrtc::CaptureCapability capability;
      int minWidthDiff = INT_MAX;
      int minHeightDiff = INT_MAX;
      int minFpsDiff = INT_MAX;

      for (int i = 0; i < capabilitiesNumber; i++) {

        webrtc::CaptureCapability deviceCapability;

        error = videoCapture_->GetCaptureCapability(uniqueId, KMaxUniqueIdLength, i, deviceCapability);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to get capture capability.");
          return Concurrency::task<void>();
        }

        std::string deviceRawVideoFormat = getRawVideoFormatString(deviceCapability.rawType);
        webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
          "Capture capability - index: %d, width: %d, height: %d, max fps: %d, video format: %s",
          i, deviceCapability.width, deviceCapability.height, deviceCapability.maxFPS, deviceRawVideoFormat.c_str());

        if (deviceCapability.rawType == webrtc::kVideoMJPEG || deviceCapability.rawType == webrtc::kVideoUnknown)
          continue;

        int widthDiff = abs((int)(deviceCapability.width - PREFERRED_FRAME_WIDTH));
        if (widthDiff < minWidthDiff) {
          capability = deviceCapability;
          minWidthDiff = widthDiff;
        }
        else if (widthDiff == minWidthDiff) {
          int heightDiff = abs((int)(deviceCapability.height - PREFERRED_FRAME_HEIGHT));
          if (heightDiff < minHeightDiff) {
            capability = deviceCapability;
            minHeightDiff = heightDiff;
          }
          else if (heightDiff == minHeightDiff) {
            int fpsDiff = abs((int)(deviceCapability.maxFPS - PREFERRED_MAX_FPS));
            if (fpsDiff < minFpsDiff) {
              capability = deviceCapability;
              minFpsDiff = fpsDiff;
            }
          }
        }
      }

      std::string rawVideoFormat = getRawVideoFormatString(capability.rawType);
      webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
        "Selected capture capability - width: %d, height: %d, max fps: %d, video format: %s",
        capability.width, capability.height, capability.maxFPS, rawVideoFormat.c_str());

      error = videoCapture_->StartCapture(captureId_, capability);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to start capturing. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      IInspectable* captureRendererPtr = reinterpret_cast<IInspectable*>(localMedia_);

      error = videoRender_->AddRenderer(captureId_, captureRendererPtr, 0, 0.0F, 0.0F, 1.0F, 1.0F);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to add renderer for video capture. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRender_->StartRender(captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to start rendering video capture. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoBase_->CreateChannel(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Could not create video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      videoTransport_ = new webrtc::test::VideoChannelTransport(videoNetwork_, videoChannel_);
        if (videoTransport_ == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Could not create video channel transport.");
        return Concurrency::task<void>();
        }

      error = videoCapture_->ConnectCaptureDevice(captureId_, videoChannel_);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to connect capture device to video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRtpRtcp_->SetRTCPStatus(videoChannel_, webrtc::kRtcpCompound_RFC4585);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set video RTCP status. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRtpRtcp_->SetKeyFrameRequestMethod(videoChannel_, webrtc::kViEKeyFrameRequestPliRtcp);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set key frame request method. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRtpRtcp_->SetTMMBRStatus(videoChannel_, true);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set temporary max media bit rate status. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      webrtc::VideoCodec videoCodec;
      memset(&videoCodec, 0, sizeof(webrtc::VideoCodec));
      for (int idx = 0; idx < videoCodec_->NumberOfCodecs(); idx++) {
        error = videoCodec_->GetCodec(idx, videoCodec);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to get video codec. Error: %d", videoBase_->LastError());
          return Concurrency::task<void>();
        }
        if (videoCodec.codecType == webrtc::kVideoCodecVP8) {
          videoCodec.width = PREFERRED_FRAME_WIDTH;
          videoCodec.height = PREFERRED_FRAME_HEIGHT;
          videoCodec.maxFramerate = PREFERRED_MAX_FPS;
          videoCodec.maxBitrate = MAX_BITRATE;
          error = videoCodec_->SetSendCodec(videoChannel_, videoCodec);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
              "Failed to set send video codec. Error: %d", videoBase_->LastError());
            return Concurrency::task<void>();
          }
          break;
        }
      }

      error = videoTransport_->SetSendDestination(remoteIpAddress_.c_str(), videoPort_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set send destination for video channel.");
        return Concurrency::task<void>();
      }

      error = videoTransport_->SetLocalReceiver(videoPort_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set local receiver for video channel.");
        return Concurrency::task<void>();
      }

      error = videoBase_->StartSend(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to start sending video. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoBase_->StartReceive(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to start receiving video. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      IInspectable* channelRendererPtr = reinterpret_cast<IInspectable*>(remoteMedia_);

      error = videoRender_->AddRenderer(videoChannel_, channelRendererPtr, 0, 0.0F, 0.0F, 1.0F, 1.0F);
      if (0 != error) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to add renderer for video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRender_->StartRender(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to start rendering video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

#endif

      Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Stop";
        startStopVideoButton_->IsEnabled = false;
      })));

    
	  started_ = true;
	  stopEvent_.wait();

#ifdef VOICE

      error = voiceBase_->StopSend(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to stop sending voice. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }
      error = voiceBase_->StopPlayout(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to stop playout. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }
      error = voiceBase_->StopReceive(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to stop receiving voice. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }
      if (voiceTransport_)
        delete voiceTransport_;
      error = voiceBase_->DeleteChannel(voiceChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to delete voice channel. Error: %d", voiceBase_->LastError());
        return Concurrency::task<void>();
      }

      voiceChannel_ = -1;
#endif
#ifdef VIDEO
      error = videoRender_->StopRender(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to stop rendering video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoRender_->RemoveRenderer(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to remove renderer for video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoBase_->StopSend(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to stop sending video. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }
      error = videoBase_->StopReceive(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to stop receiving video. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }
      error = videoCapture_->DisconnectCaptureDevice(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to disconnect capture device from video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }
      if (videoTransport_)
        delete videoTransport_;
      error = videoBase_->DeleteChannel(videoChannel_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to delete video channel. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      videoChannel_ = -1;

      error = videoRender_->StopRender(captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to stop rendering video capture. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }
      error = videoRender_->RemoveRenderer(captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to remove renderer for video capture. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoCapture_->StopCapture(captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to stop video capturing. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      error = videoCapture_->ReleaseCaptureDevice(captureId_);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to release video capture device. Error: %d", videoBase_->LastError());
        return Concurrency::task<void>();
      }

      if (vcpm_ != NULL)
        vcpm_->Release();

      vcpm_ = NULL;

      captureId_ = -1;
#endif

	  started_ = false;

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Start";
        startStopVideoButton_->IsEnabled = true;
      })));
    });
  }
}

void StandupWinRT::App::OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  throw ref new Platform::NotImplementedException();
}

void StandupWinRT::App::OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  if (!startedVideo_) {
    initializeTranportInfo();
    SaveSettings();

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

      vcpm_ = webrtc::VideoCaptureFactory::Create(0, device_unique_name);
      vcpm_->AddRef();

      delete dev_info;

      IInspectable* videoRendererPtr = reinterpret_cast<IInspectable*>(localMedia_);

      vrm_ = webrtc::VideoRender::CreateVideoRender(1, videoRendererPtr, false);

      webrtc::VideoRenderCallback* rendererCallback = vrm_->AddIncomingRenderStream(1, 0, 0.0, 0.0, 1.0, 1.0);

      captureCallback_ = new TestCaptureCallback(rendererCallback);

      vcpm_->RegisterCaptureDataCallback(*captureCallback_);

      webrtc::VideoCaptureCapability capability;
      capability.width = PREFERRED_FRAME_WIDTH;
      capability.height = PREFERRED_FRAME_HEIGHT;
      capability.maxFPS = PREFERRED_MAX_FPS;
      capability.rawType = webrtc::kVideoNV12;

      vcpm_->StartCapture(capability);

      vrm_->StartRender(1);

      startedVideo_ = true;

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopVideoButton_->Content = "Stop Video";
        startStopButton_->IsEnabled = false;
      })));
    });
  }
  else
  {
    Concurrency::create_task([this]() {

      vrm_->StopRender(1);
      vcpm_->StopCapture();
      vcpm_->Release();
      delete (TestCaptureCallback*)captureCallback_;

      startedVideo_ = false;

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopVideoButton_->Content = "Start Video";
        startStopButton_->IsEnabled = true;
      })));
    });
  }
}

void StandupWinRT::App::initializeTranportInfo(){

  std::string userIp = GetIP(ipTextBox_->Text);
  if (!userIp.empty()) {
    remoteIpAddress_ = userIp;
  }
  int userInputVideoPort = _wtoi(videoPortTextBox_->Text->Data());
  if (userInputVideoPort > 0)
  {
    videoPort_ = userInputVideoPort;
  }

  int userInputAudioPort = _wtoi(audioPortTextBox_->Text->Data());
  if (userInputAudioPort > 0)
  {
    audioPort_ = userInputAudioPort;
  }
}

void StandupWinRT::App::OnStartTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  tl.StartTracing();
}

void StandupWinRT::App::OnStopTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  tl.StopTracing();
}

void StandupWinRT::App::OnSendTracesClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  std::wstring tmp(ipRemoteTraces_->Text->Data());
  std::string ip(tmp.begin(), tmp.end());
  tl.Save(ip, _wtoi(portRemoteTraces_->Text->Data()));
}

std::string StandupWinRT::App::getRawVideoFormatString(webrtc::RawVideoType videoType) {

  std::string videoFormatString;
  switch (videoType)
  {
  case webrtc::kVideoYV12:
    videoFormatString = "YV12";
    break;
  case webrtc::kVideoYUY2:
    videoFormatString = "YUY2";
    break;
  case webrtc::kVideoI420:
    videoFormatString = "I420";
    break;
  case webrtc::kVideoIYUV:
    videoFormatString = "IYUV";
    break;
  case webrtc::kVideoRGB24:
    videoFormatString = "RGB24";
    break;
  case webrtc::kVideoARGB:
    videoFormatString = "ARGB";
    break;
  case webrtc::kVideoMJPEG:
    videoFormatString = "MJPEG";
    break;
  case webrtc::kVideoNV12:
    videoFormatString = "NV12";
    break;
  default:
    videoFormatString = "Not supported";
    break;
  }
  return videoFormatString;
}

void StandupWinRT::App::SaveSettings()
{
  auto settings = ApplicationData::Current->LocalSettings;
  auto values = settings->Values;
  values->Insert("remote_ip", ipTextBox_->Text);
  values->Insert("audio_port", audioPortTextBox_->Text);
  values->Insert("video_port", videoPortTextBox_->Text);
}
