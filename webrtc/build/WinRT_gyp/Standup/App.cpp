#include <collection.h>
#include <ppltasks.h>
#include <concrt.h>
#include <string>

#include "webrtc/modules/video_capture/include/video_capture.h"
#include "webrtc/modules/video_capture/include/video_capture_factory.h"
#include "webrtc/modules/video_render/include/video_render.h"
#include "webrtc/modules/video_render/windows/video_render_winrt.h"
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

#include "talk/media/devices/devicemanager.h"
#include "talk/media/base/videocapturer.h"

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
#define PREFERRED_FRAME_WIDTH 640
#define PREFERRED_FRAME_HEIGHT 480
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
  class MediaElementWrapper : public webrtc::IWinRTMediaElement
  {
  public:

    MediaElementWrapper(MediaElement^ mediaElement) :_mediaElement(mediaElement){};
    virtual void Play(){
      _mediaElement->Play();
    };
    virtual void SetMediaStreamSource(Windows::Media::Core::IMediaSource^ mss){
      _mediaElement->SetMediaStreamSource(mss);
    };

    MediaElement^ getMediaElement(){ return _mediaElement; }

  private:
    MediaElement^ _mediaElement;
  };

  class TestCaptureCallback : public webrtc::VideoCaptureDataCallback
  {
  public:

    virtual ~TestCaptureCallback() {};

    TestCaptureCallback(webrtc::VideoRenderCallback* rendererCallback) :
      _rendererCallback(rendererCallback)
    {

    }

    virtual void OnIncomingCapturedFrame(const int32_t id,
      const webrtc::I420VideoFrame& videoFrame)
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
      std::wstring messageUTF16 = rtc::ToUtf16(message, length + 2);
      messageUTF16[length - 1] = L'\r';
      messageUTF16[length] = L'\n';
      messageUTF16[length + 1] = 0;
      OutputDebugString(messageUTF16.c_str());

      rtc::LoggingSeverity sev = rtc::LS_VERBOSE;
      if (level == webrtc::kTraceError || level == webrtc::kTraceCritical)
          sev = rtc::LS_ERROR;
      else if (level == webrtc::kTraceWarning)
          sev = rtc::LS_WARNING;
      else if (level == webrtc::kTraceStateInfo || level == webrtc::kTraceInfo)
          sev = rtc::LS_INFO;
      else if (level == webrtc::kTraceTerseInfo)
          sev = rtc::LS_INFO;

      // Skip past boilerplate prefix text
      if (length < 72) {
          std::string msg(message, length);
          LOG(LS_ERROR) << "Malformed webrtc log message: ";
          LOG_V(sev) << msg;
      }
      else {
          std::string msg(message + 71, length - 72);
          LOG_V(sev) << "webrtc: " << msg;
      }
    }
  };

  ref class App sealed : public Windows::UI::Xaml::Application
  {
  public:
    App() :
      traceCallback_(new TestTraceCallback()),
      started_(false),
      startedVideo_(false),
      stoppedRendering_(false),
      voiceTransport_(NULL),
      videoTransport_(NULL),
      voiceChannel_(-1),
      captureId_(-1),
      videoChannel_(-1),
      localMediaWrapper_(NULL),
      remoteMediaWrapper_(NULL)
    {

      webrtc::test::InitFieldTrialsFromString("");
      webrtc::Trace::CreateTrace();
      webrtc::Trace::SetTraceCallback(traceCallback_);
      webrtc::Trace::set_level_filter(webrtc::kTraceAll);

      //provide some default values if user want to test on local machine 
      remoteIpAddress_ = "127.0.0.1";
      audioPort_ = 20000;
      videoPort_ = 20100;

      workerThread_.Start();

      workerThread_.Invoke<void>([this]() -> void
      {
        int error;

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
        }
        else if (videoBase_->LastError() > 0) {
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
      });

    }

    virtual ~App() {

      workerThread_.Invoke<void>([this]() -> void
      {
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
      });
    }

  private:
    TextBox^ ipTextBox_;
    TextBox^ videoPortTextBox_;
    TextBox^ audioPortTextBox_;
    TextBox^ ipRemoteTraces_;
    TextBox^ portRemoteTraces_;

    MediaElement^ localMedia_;
    MediaElement^ remoteMedia_;
    MediaElementWrapper* localMediaWrapper_;
    MediaElementWrapper* remoteMediaWrapper_;

    Button^ startStopButton_;
    Button^ startStopVideoButton_;
    Button^ stopRenderingButton_;
    ComboBox^ videoDeviceComboBox_;
    ComboBox^ videoFormatComboBox_;
    Button^ startTracingButton_;
    Button^ stopTracingButton_;
    Button^ sendTracesButton_;
    Button^ settingsButton_;

    Flyout^ settingsFlyout_;

    webrtc::VideoRender* vrm_;
    webrtc::VideoCaptureDataCallback* captureCallback_;
    webrtc::TraceCallback* traceCallback_;
    Windows::UI::Core::CoreDispatcher^ dispatcher_;
    bool started_;
    bool startedVideo_;
    bool stoppedRendering_;

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
    webrtc::VideoCaptureModule* vcpm_;
    webrtc::VideoEngine* videoEngine_;
    webrtc::ViEBase* videoBase_;
    webrtc::ViENetwork* videoNetwork_;
    webrtc::ViERender* videoRender_;
    webrtc::ViECapture* videoCapture_;
    webrtc::ViERTP_RTCP* videoRtpRtcp_;
    webrtc::ViECodec* videoCodec_;

    rtc::Thread workerThread_;

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

      // First row (Traces, Settings)
      {
        auto row = ref new RowDefinition();
        row->Height = GridLength(40, GridUnitType::Pixel);

        layoutRoot->RowDefinitions->Append(row);

        auto stackPanel = ref new StackPanel();
        stackPanel->Orientation = Orientation::Horizontal;
        Grid::SetRow(stackPanel, 0);
        layoutRoot->Children->Append(stackPanel);

        startTracingButton_ = ref new Button();
        startTracingButton_->Content = "Start Tracing";
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

        settingsButton_ = ref new Button();
        settingsButton_->Content = "Settings";
        settingsButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        settingsButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnSettingsClick);
        stackPanel->Children->Append(settingsButton_);
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

      // Third row (Start/Stop buttons)
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

#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
        startStopVideoButton_ = ref new Button();
        startStopVideoButton_->Content = "Start Video";
        startStopVideoButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        startStopVideoButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStartStopVideoClick);
        startStopVideoButton_->IsEnabled = true;
        stackPanel->Children->Append(startStopVideoButton_);
#endif

        stopRenderingButton_ = ref new Button();
        stopRenderingButton_->Width = 100;
        stopRenderingButton_->Content = "Stop Left";
        stopRenderingButton_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        stopRenderingButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStopRenderingClick);
        stopRenderingButton_->IsEnabled = false;
        stackPanel->Children->Append(stopRenderingButton_);

        videoDeviceComboBox_ = ref new ComboBox();
        videoDeviceComboBox_->Width = 180;
        videoDeviceComboBox_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        videoDeviceComboBox_->SelectionChanged += ref new Windows::UI::Xaml::Controls::SelectionChangedEventHandler(this, &StandupWinRT::App::OnSelectionChanged);
        videoDeviceComboBox_->IsEnabled = false;
        stackPanel->Children->Append(videoDeviceComboBox_);

        videoFormatComboBox_ = ref new ComboBox();
        videoFormatComboBox_->Width = 180;
        videoFormatComboBox_->Margin = ThicknessHelper::FromLengths(20, 0, 0, 0);
        videoFormatComboBox_->IsEnabled = false;
        stackPanel->Children->Append(videoFormatComboBox_);
      }

      localMediaWrapper_ = new MediaElementWrapper(localMedia_);
      remoteMediaWrapper_ = new MediaElementWrapper(remoteMedia_);

      Concurrency::create_async([this]
      {
        workerThread_.Invoke<void>([this]() -> void
        {
          setCameraDevices();
          setCameraDeviceCapabilities();
        });
      });

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
      CreateSettingsFlyout();
    }

    void OnStartStopClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStopRenderingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSelectionChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^e);
    void OnStartTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStopTracingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSendTracesClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSettingsClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
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

    /**
    * read camera capabilities and fill the combo box for video format selection.
    */
    void setCameraDevices();

    /**
    * read camera capabilities and fill the combo box for video format selection.
    */
    void setCameraDeviceCapabilities();

    void SaveSettings();

    void CreateSettingsFlyout();
  };

}

static std::string GetIP(String^ stringIP) {

  std::wstring ipW(stringIP->Begin());
  std::string ip(ipW.begin(), ipW.end());
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
    Concurrency::create_async([this]
    {
      workerThread_.Invoke<void>([this]() -> void
      {
        int error;
#ifdef VOICE
        error = voiceBase_->StopSend(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to stop sending voice. Error: %d", voiceBase_->LastError());
          return;
        }
        error = voiceBase_->StopPlayout(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to stop playout. Error: %d", voiceBase_->LastError());
          return;
        }
        error = voiceBase_->StopReceive(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to stop receiving voice. Error: %d", voiceBase_->LastError());
          return;
        }
        if (voiceTransport_)
          delete voiceTransport_;
        error = voiceBase_->DeleteChannel(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to delete voice channel. Error: %d", voiceBase_->LastError());
          return;
        }

        voiceChannel_ = -1;
#endif
#ifdef VIDEO
        error = videoRender_->StopRender(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop rendering video channel. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRender_->RemoveRenderer(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to remove renderer for video channel. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoBase_->StopSend(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop sending video. Error: %d", videoBase_->LastError());
          return;
        }
        error = videoBase_->StopReceive(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop receiving video. Error: %d", videoBase_->LastError());
          return;
        }
        error = videoCapture_->DisconnectCaptureDevice(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to disconnect capture device from video channel. Error: %d", videoBase_->LastError());
          return;
        }
        if (videoTransport_)
          delete videoTransport_;
        error = videoBase_->DeleteChannel(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to delete video channel. Error: %d", videoBase_->LastError());
          return;
        }

        videoChannel_ = -1;

        error = videoRender_->StopRender(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop rendering video capture. Error: %d", videoBase_->LastError());
          return;
        }
        error = videoRender_->RemoveRenderer(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to remove renderer for video capture. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoCapture_->StopCapture(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop video capturing. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoCapture_->ReleaseCaptureDevice(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to release video capture device. Error: %d", videoBase_->LastError());
          return;
        }

        if (vcpm_ != NULL)
          vcpm_->Release();

        vcpm_ = NULL;

        captureId_ = -1;
#endif

        started_ = false;

        dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
          startStopButton_->Content = "Start";
#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
          startStopVideoButton_->IsEnabled = true;
#endif
          videoDeviceComboBox_->IsEnabled = true;
          videoFormatComboBox_->IsEnabled = true;
          stopRenderingButton_->Content = "Stop Left";
          stopRenderingButton_->IsEnabled = false;
          stoppedRendering_ = false;
        }));
      });
    });
  }
  else {
    SaveSettings();
    initializeTranportInfo();

    Concurrency::create_async([this]
    {
      workerThread_.Invoke<void>([this]() -> void
      {
        int error;
#ifdef VOICE
        voiceChannel_ = voiceBase_->CreateChannel();
        if (voiceChannel_ < 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Could not create voice channel. Error: %d", voiceBase_->LastError());
          voiceChannel_ = -1;
          return;
        }

        voiceTransport_ = new webrtc::test::VoiceChannelTransport(voiceNetwork_, voiceChannel_);
        if (voiceTransport_ == NULL) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Could not create voice channel transport.");
          return;
        }

        webrtc::EcModes ecMode = webrtc::kEcAec;
        error = voiceAudioProcessing_->SetEcStatus(true, ecMode);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set acoustic echo canceller status. Error: %d", voiceBase_->LastError());
          return;
        }
        if (ecMode == webrtc::kEcAecm) {
          error = voiceAudioProcessing_->SetAecmMode(webrtc::kAecmSpeakerphone);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
              "Failed to set acoustic echo canceller mobile mode. Error: %d", voiceBase_->LastError());
            return;
          }
        }

        error = voiceAudioProcessing_->SetAgcStatus(true, webrtc::kAgcAdaptiveDigital);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set automatic gain control status. Error: %d", voiceBase_->LastError());
          return;
        }

        error = voiceAudioProcessing_->SetNsStatus(true, webrtc::kNsLowSuppression);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set noise suppression status. Error: %d", voiceBase_->LastError());
          return;
        }

        error = voiceVolumeControl_->SetInputMute(-1, false);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set microphone mute. Error: %d", voiceBase_->LastError());
          return;
        }

        webrtc::CodecInst cinst;
        memset(&cinst, 0, sizeof(webrtc::CodecInst));
        for (int idx = 0; idx < voiceCodec_->NumOfCodecs(); idx++) {
          error = voiceCodec_->GetCodec(idx, cinst);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
              "Failed to get voice codec. Error: %d", voiceBase_->LastError());
            return;
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
              return;
            }
            break;
          }
        }

        error = voiceTransport_->SetSendDestination(remoteIpAddress_.c_str(), audioPort_);

        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set send destination for voice channel.");
          return;
        }

        error = voiceTransport_->SetLocalReceiver(audioPort_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to set local receiver for voice channel.");
          return;
        }

        error = voiceBase_->StartSend(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to start sending voice. Error: %d", voiceBase_->LastError());
          return;
        }

        error = voiceBase_->StartReceive(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to start receiving voice. Error: %d", voiceBase_->LastError());
          return;
        }

        error = voiceBase_->StartPlayout(voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
            "Failed to start playout. Error: %d", voiceBase_->LastError());
          return;
        }

#endif
#ifdef VIDEO
        const unsigned int KMaxDeviceNameLength = 128;
        const unsigned int KMaxUniqueIdLength = 256;
        char deviceName[KMaxDeviceNameLength];
        memset(deviceName, 0, KMaxDeviceNameLength);
        char uniqueId[KMaxUniqueIdLength];
        memset(uniqueId, 0, KMaxUniqueIdLength);

        int selectedDeviceIndex;

        Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([this, &selectedDeviceIndex]() {
          selectedDeviceIndex = videoDeviceComboBox_->SelectedIndex;
        }))).wait();

        // return if Video Device combo box is empty
        if (selectedDeviceIndex == -1)
          return;

        error = videoCapture_->GetCaptureDevice(selectedDeviceIndex, deviceName,
          KMaxDeviceNameLength, uniqueId,
          KMaxUniqueIdLength);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to get video device name.");
          return;
        }

        webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
          "Selected capture device - index: %d, name: %s, unique ID: %s", selectedDeviceIndex, deviceName, uniqueId);

        vcpm_ = webrtc::VideoCaptureFactory::Create(1, uniqueId);
        if (vcpm_ == NULL) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to create video capture module.");
          return;
        }

        error = videoCapture_->AllocateCaptureDevice(*vcpm_, captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to allocate video capture device. Error: %d", videoBase_->LastError());
          return;
        }
        vcpm_->AddRef();

        webrtc::CaptureCapability capability;

        int selectedCaptureCapabilityIndex;

        Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
          ref new Windows::UI::Core::DispatchedHandler([this, &selectedCaptureCapabilityIndex]() {
          selectedCaptureCapabilityIndex = videoFormatComboBox_->SelectedIndex;
        }))).wait();

        // return if Video Format combo box is empty
        if (selectedCaptureCapabilityIndex == -1)
          return;

        error = videoCapture_->GetCaptureCapability(uniqueId, KMaxUniqueIdLength, selectedCaptureCapabilityIndex, capability);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to get capture capability.");
              return;
          }

          std::string rawVideoFormat = getRawVideoFormatString(capability.rawType);
          webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
            "Selected capture capability - width: %d, height: %d, max fps: %d, video format: %s",
            capability.width, capability.height, capability.maxFPS, rawVideoFormat.c_str());

        if (capability.rawType == webrtc::kVideoMJPEG || capability.rawType == webrtc::kVideoUnknown) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Wrong video format raw type.");
          return;
        }

        error = videoCapture_->StartCapture(captureId_, capability);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start capturing. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRender_->AddRenderer(captureId_, localMediaWrapper_, 0, 0.0F, 0.0F, 1.0F, 1.0F);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to add renderer for video capture. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRender_->StartRender(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start rendering video capture. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoBase_->CreateChannel(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Could not create video channel. Error: %d", videoBase_->LastError());
          return;
        }

#if defined(VOICE)
        error = videoBase_->ConnectAudioChannel(videoChannel_, voiceChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Could not connect audio channel. Error: %d", videoBase_->LastError());
          return;
        }
#endif

        videoTransport_ = new webrtc::test::VideoChannelTransport(videoNetwork_, videoChannel_);
        if (videoTransport_ == NULL) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Could not create video channel transport.");
          return;
        }

        error = videoCapture_->ConnectCaptureDevice(captureId_, videoChannel_);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to connect capture device to video channel. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRtpRtcp_->SetRTCPStatus(videoChannel_, webrtc::kRtcpCompound_RFC4585);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to set video RTCP status. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRtpRtcp_->SetKeyFrameRequestMethod(videoChannel_, webrtc::kViEKeyFrameRequestPliRtcp);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to set key frame request method. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRtpRtcp_->SetTMMBRStatus(videoChannel_, true);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to set temporary max media bit rate status. Error: %d", videoBase_->LastError());
          return;
        }

        webrtc::VideoCodec videoCodec;
        memset(&videoCodec, 0, sizeof(webrtc::VideoCodec));
        for (int idx = 0; idx < videoCodec_->NumberOfCodecs(); idx++) {
          error = videoCodec_->GetCodec(idx, videoCodec);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
              "Failed to get video codec. Error: %d", videoBase_->LastError());
            return;
          }
          if (videoCodec.codecType == webrtc::kVideoCodecVP8) {
          videoCodec.width = capability.width;
          videoCodec.height = capability.height;
          videoCodec.maxFramerate = capability.maxFPS;
            videoCodec.maxBitrate = MAX_BITRATE;
            error = videoCodec_->SetSendCodec(videoChannel_, videoCodec);
            if (error != 0) {
              webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
                "Failed to set send video codec. Error: %d", videoBase_->LastError());
              return;
            }
            break;
          }
        }

        error = videoTransport_->SetSendDestination(remoteIpAddress_.c_str(), videoPort_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to set send destination for video channel.");
          return;
        }

        error = videoTransport_->SetLocalReceiver(videoPort_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to set local receiver for video channel.");
          return;
        }

        error = videoBase_->StartSend(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start sending video. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoBase_->StartReceive(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start receiving video. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRender_->AddRenderer(videoChannel_, remoteMediaWrapper_, 0, 0.0F, 0.0F, 1.0F, 1.0F);
        if (0 != error) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to add renderer for video channel. Error: %d", videoBase_->LastError());
          return;
        }

        error = videoRender_->StartRender(videoChannel_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start rendering video channel. Error: %d", videoBase_->LastError());
          return;
        }

#endif

        dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
          startStopButton_->Content = "Stop";
#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
          startStopVideoButton_->IsEnabled = false;
#endif
          videoDeviceComboBox_->IsEnabled = false;
          videoFormatComboBox_->IsEnabled = false;
          stopRenderingButton_->IsEnabled = true;
        }));

        started_ = true;
      });
    });
  }
}

void StandupWinRT::App::OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  if (!startedVideo_)
  {
      Concurrency::create_async([this]
      {
        workerThread_.Invoke<void>([this]() -> void
        {
          webrtc::VideoCaptureModule::DeviceInfo* dev_info =
            webrtc::VideoCaptureFactory::CreateDeviceInfo(0);

          const unsigned int KMaxDeviceNameLength = 128;
          const unsigned int KMaxUniqueIdLength = 256;
          char deviceName[KMaxDeviceNameLength];
          memset(deviceName, 0, KMaxDeviceNameLength);
          char uniqueId[KMaxUniqueIdLength];
          memset(uniqueId, 0, KMaxUniqueIdLength);
          int selectedDeviceIndex;

          Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler([this, &selectedDeviceIndex]() {
            selectedDeviceIndex = videoDeviceComboBox_->SelectedIndex;
          }))).wait();

          // return if Video Device combo box is empty
          if (selectedDeviceIndex == -1)
            return;

          dev_info->GetDeviceName(selectedDeviceIndex,
            deviceName,
            KMaxDeviceNameLength,
            uniqueId,
            KMaxUniqueIdLength);

          vcpm_ = webrtc::VideoCaptureFactory::Create(0, uniqueId);
          vcpm_->AddRef();

          vrm_ = webrtc::VideoRender::CreateVideoRender(1, localMediaWrapper_, false);

          webrtc::VideoRenderCallback* rendererCallback = vrm_->AddIncomingRenderStream(1, 0, 0.0, 0.0, 1.0, 1.0);

          captureCallback_ = new TestCaptureCallback(rendererCallback);

          vcpm_->RegisterCaptureDataCallback(*captureCallback_);

          webrtc::VideoCaptureCapability capability;
          int selectedCaptureCapabilityIndex;

          Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
            ref new Windows::UI::Core::DispatchedHandler([this, &selectedCaptureCapabilityIndex]() {
            selectedCaptureCapabilityIndex = videoFormatComboBox_->SelectedIndex;
          }))).wait();

          // return if Video Format combo box is empty
          if (selectedCaptureCapabilityIndex == -1)
            return;

          dev_info->GetCapability(uniqueId, selectedCaptureCapabilityIndex, capability);

          delete dev_info;

          vcpm_->StartCapture(capability);

          vrm_->StartRender(1);

          startedVideo_ = true;

          dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
            startStopVideoButton_->Content = "Stop Video";
#endif
            startStopButton_->IsEnabled = false;
            videoDeviceComboBox_->IsEnabled = false;
            videoFormatComboBox_->IsEnabled = false;
          }));
        });
    });
  }
  else
  {
    Concurrency::create_async([this]
    {
      workerThread_.Invoke<void>([this]() -> void
      {
        vrm_->StopRender(1);
        vcpm_->StopCapture();
        vcpm_->Release();
        delete (TestCaptureCallback*)captureCallback_;

        startedVideo_ = false;

        dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
#if (WINAPI_FAMILY != WINAPI_FAMILY_PHONE_APP)
          startStopVideoButton_->Content = "Start Video";
#endif
          startStopButton_->IsEnabled = true;
          videoDeviceComboBox_->IsEnabled = true;
          videoFormatComboBox_->IsEnabled = true;
        }));
      });
    });
  }
}

void StandupWinRT::App::OnStopRenderingClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e) {
  if (stoppedRendering_) {
    Concurrency::create_async([this]
    {
      workerThread_.Invoke<void>([this]() -> void
      {
        int error = videoRender_->StartRender(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to start rendering video capture. Error: %d", videoBase_->LastError());
        }

        stoppedRendering_ = false;

        dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
          stopRenderingButton_->Content = "Stop Left";
          stopRenderingButton_->IsEnabled = true;
        }));
      });
    });
  }
  else {
    Concurrency::create_async([this]
    {
      workerThread_.Invoke<void>([this]() -> void
      {
        int error = videoRender_->StopRender(captureId_);
        if (error != 0) {
          webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
            "Failed to stop rendering video capture. Error: %d", videoBase_->LastError());
        }

        stoppedRendering_ = true;

        dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
          stopRenderingButton_->Content = "Start Left";
          stopRenderingButton_->IsEnabled = true;
        }));
      });
    });
  }
}

void StandupWinRT::App::OnSelectionChanged(Platform::Object ^sender, Windows::UI::Xaml::Controls::SelectionChangedEventArgs ^e)
{
  videoFormatComboBox_->IsEnabled = false;
  Concurrency::create_async([this]
  {
    workerThread_.Invoke<void>([this]() -> void
    {
      setCameraDeviceCapabilities();
    });
  });
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

void StandupWinRT::App::OnSettingsClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  CreateSettingsFlyout();
  settingsFlyout_->ShowAt((FrameworkElement^)(Window::Current->Content));
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

void StandupWinRT::App::setCameraDevices()
{
#ifdef VIDEO
  int error;
  const unsigned int KMaxDeviceNameLength = 128;
  const unsigned int KMaxUniqueIdLength = 256;
  char deviceName[KMaxDeviceNameLength];
  memset(deviceName, 0, KMaxDeviceNameLength);
  char uniqueId[KMaxUniqueIdLength];
  memset(uniqueId, 0, KMaxUniqueIdLength);

  int devicesNumber = videoCapture_->NumberOfCaptureDevices();

  std::vector<std::string> devices;

  for (int i = 0; i < devicesNumber; i++) {

    error = videoCapture_->GetCaptureDevice(i, deviceName,
      KMaxDeviceNameLength, uniqueId,
      KMaxUniqueIdLength);
    if (error != 0) {
      webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
        "Failed to get video device name.");
      return;
    }

    webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
      "Capture device - index: %d, name: %s, unique ID: %s", i, deviceName, uniqueId);

    devices.push_back(deviceName);
  }

  error = videoCapture_->GetCaptureDevice(CAPTURE_DEVICE_INDEX, deviceName,
    KMaxDeviceNameLength, uniqueId,
    KMaxUniqueIdLength);
  if (error != 0) {
    webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
      "Failed to get video device name.");
    return;
  }

  webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
    "Selected capture device - index: %d, name: %s, unique ID: %s", CAPTURE_DEVICE_INDEX, deviceName, uniqueId);

  Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
    ref new Windows::UI::Core::DispatchedHandler([this, &devices]() {
    std::vector<std::string>::iterator iter = devices.begin();
    while (iter != devices.end()) {
      std::wstring deviceUTF16 = rtc::ToUtf16(iter->c_str(), iter->size());
      videoDeviceComboBox_->Items->Append(ref new String(deviceUTF16.c_str()));
      iter++;
    }
    videoDeviceComboBox_->SelectedIndex = CAPTURE_DEVICE_INDEX;
    videoDeviceComboBox_->IsEnabled = true;
  }))).wait();
#endif
}

void StandupWinRT::App::setCameraDeviceCapabilities()
{
#ifdef VIDEO
  int error;
  const unsigned int KMaxDeviceNameLength = 128;
  const unsigned int KMaxUniqueIdLength = 256;
  char deviceName[KMaxDeviceNameLength];
  memset(deviceName, 0, KMaxDeviceNameLength);
  char uniqueId[KMaxUniqueIdLength];
  memset(uniqueId, 0, KMaxUniqueIdLength);

  int selectedDeviceIndex;

  Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, 
    ref new Windows::UI::Core::DispatchedHandler([this, &selectedDeviceIndex]() {
    selectedDeviceIndex = videoDeviceComboBox_->SelectedIndex;
    videoFormatComboBox_->Items->Clear();
  }))).wait();

  // return if Video Device combo box is empty
  if (selectedDeviceIndex == -1)
    return;

  error = videoCapture_->GetCaptureDevice(selectedDeviceIndex, deviceName,
    KMaxDeviceNameLength, uniqueId,
    KMaxUniqueIdLength);
  if (error != 0) {
    webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
      "Failed to get video device name.");
    return;
  }

  int capabilitiesNumber = videoCapture_->NumberOfCapabilities(uniqueId, KMaxUniqueIdLength);

  int selectedCapabilityIndex = 0;
  int minWidthDiff = INT_MAX;
  int minHeightDiff = INT_MAX;
  int minFpsDiff = INT_MAX;
  std::vector<webrtc::CaptureCapability> captureCapabilities;


	for (int i = 0; i < capabilitiesNumber; i++) {

		webrtc::CaptureCapability capability;

		error = videoCapture_->GetCaptureCapability(uniqueId, KMaxUniqueIdLength, i, capability);
		if (error != 0) {
			webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
				"Failed to get capture capability.");
			return;
		}

		std::string rawVideoFormat = getRawVideoFormatString(capability.rawType);
		webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
			"Capture capability - index: %d, width: %d, height: %d, max fps: %d, video format: %s",
			i, capability.width, capability.height, capability.maxFPS, rawVideoFormat.c_str());

		captureCapabilities.push_back(capability);

		if (capability.rawType == webrtc::kVideoMJPEG || capability.rawType == webrtc::kVideoUnknown)
			continue;

		int widthDiff = abs((int)(capability.width - PREFERRED_FRAME_WIDTH));
		if (widthDiff < minWidthDiff) {
			selectedCapabilityIndex = i;
			minWidthDiff = widthDiff;
		}
		else if (widthDiff == minWidthDiff) {
			int heightDiff = abs((int)(capability.height - PREFERRED_FRAME_HEIGHT));
			if (heightDiff < minHeightDiff) {
				selectedCapabilityIndex = i;
				minHeightDiff = heightDiff;
			}
			else if (heightDiff == minHeightDiff) {
				int fpsDiff = abs((int)(capability.maxFPS - PREFERRED_MAX_FPS));
				if (fpsDiff < minFpsDiff) {
					selectedCapabilityIndex = i;
					minFpsDiff = fpsDiff;
				}
			}
		}
	}
	auto settings = ApplicationData::Current->LocalSettings->Values;
	if (settings->HasKey("video_format"))
	{
		int vf = safe_cast<IPropertyValue^>(settings->Lookup("video_format"))->GetInt32();
		if (vf >= 0 && vf < capabilitiesNumber)
		{
			selectedCapabilityIndex = vf;
		}
	}


  webrtc::CaptureCapability selectedCapability = captureCapabilities[selectedCapabilityIndex];

  std::string selectedRawVideoFormat = getRawVideoFormatString(selectedCapability.rawType);
  webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideo, -1,
    "Selected capture capability - width: %d, height: %d, max fps: %d, video format: %s",
    selectedCapability.width, selectedCapability.height, selectedCapability.maxFPS, selectedRawVideoFormat.c_str());

  Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal,
    ref new Windows::UI::Core::DispatchedHandler([this, &captureCapabilities, selectedCapabilityIndex]() {
    std::vector<webrtc::CaptureCapability>::iterator iter = captureCapabilities.begin();
    while (iter != captureCapabilities.end()) {
      wchar_t videoFormat[128];
      std::string rawVideoFormat = getRawVideoFormatString(iter->rawType);
      std::wstring rawVideoFormatUTF16 = rtc::ToUtf16(rawVideoFormat.c_str(), rawVideoFormat.size());
      swprintf(videoFormat, L"%dx%d@%d - %s", iter->width, iter->height, iter->maxFPS, rawVideoFormatUTF16.c_str());
      videoFormatComboBox_->Items->Append(ref new String(videoFormat));
      iter++;
    }
    videoFormatComboBox_->SelectedIndex = selectedCapabilityIndex;
    videoFormatComboBox_->IsEnabled = true;
  }))).wait();
#endif
}

void StandupWinRT::App::SaveSettings()
{
  auto settings = ApplicationData::Current->LocalSettings;
  auto values = settings->Values;
  values->Insert("remote_ip", ipTextBox_->Text);
  values->Insert("audio_port", audioPortTextBox_->Text);
  values->Insert("video_port", videoPortTextBox_->Text);
	values->Insert("video_format", videoFormatComboBox_->SelectedIndex);
}

void StandupWinRT::App::CreateSettingsFlyout()
{
  if (!settingsFlyout_) {

    auto layoutRoot = ref new Grid();
    auto settings = ApplicationData::Current->LocalSettings->Values;

    auto row = ref new RowDefinition();
    auto stackPanel = ref new StackPanel();
    stackPanel->Width = 300;
    layoutRoot->Children->Append(stackPanel);
    auto hostNames = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
    String^ ipAddress = "";
    std::for_each(begin(hostNames), end(hostNames), [&](Windows::Networking::HostName^ hostname)
    {
      if (hostname->IPInformation != nullptr && hostname->IPInformation->PrefixLength->Value <= 32)
      {
        ipAddress = hostname->DisplayName;
      }
    });

    auto label = ref new TextBlock();
    label->Text = "IP: " + ipAddress;
    label->VerticalAlignment = VerticalAlignment::Center;
    label->Margin = ThicknessHelper::FromLengths(0, 0, 4, 0);
    stackPanel->Children->Append(label);

    ipTextBox_ = ref new TextBox();
    ipTextBox_->Text = settings->Lookup("remote_ip")->ToString();
    ipTextBox_->InputScope = CreateInputScope();
    stackPanel->Children->Append(ipTextBox_);

    label = ref new TextBlock();
    label->Text = "Video Port: ";
    label->VerticalAlignment = VerticalAlignment::Center;
    label->Margin = ThicknessHelper::FromLengths(0, 4, 4, 0);
    stackPanel->Children->Append(label);

    videoPortTextBox_ = ref new TextBox();
    videoPortTextBox_->Text = settings->Lookup("video_port")->ToString();
    videoPortTextBox_->InputScope = CreateInputScope();
    stackPanel->Children->Append(videoPortTextBox_);

    label = ref new TextBlock();
    label->Text = "Audio Port: ";
    label->VerticalAlignment = VerticalAlignment::Center;
    label->Margin = ThicknessHelper::FromLengths(0, 4, 4, 0);
    stackPanel->Children->Append(label);

    audioPortTextBox_ = ref new TextBox();
    audioPortTextBox_->Text = settings->Lookup("audio_port")->ToString();
    audioPortTextBox_->InputScope = CreateInputScope();
    stackPanel->Children->Append(audioPortTextBox_);

    label = ref new TextBlock();
    label->Text = "IP(Traces): ";
    label->VerticalAlignment = VerticalAlignment::Center;
    label->Margin = ThicknessHelper::FromLengths(0, 4, 4, 0);
    stackPanel->Children->Append(label);

    ipRemoteTraces_ = ref new TextBox();

    stackPanel->Children->Append(ipRemoteTraces_);

    label = ref new TextBlock();
    label->Text = "Port(Traces): ";
    label->VerticalAlignment = VerticalAlignment::Center;
    label->Margin = ThicknessHelper::FromLengths(0, 4, 4, 0);
    stackPanel->Children->Append(label);

    portRemoteTraces_ = ref new TextBox();
    stackPanel->Children->Append(portRemoteTraces_);

    settingsFlyout_ = ref new Flyout();
    settingsFlyout_->Content = layoutRoot;

  }
}