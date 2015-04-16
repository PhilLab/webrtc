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

#define VOICE
#define VIDEO

using namespace Platform;
using namespace concurrency;
using namespace Windows::ApplicationModel::Activation;
using namespace Windows::UI;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Media;

bool autoClose = false;
Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

#if (WINAPI_FAMILY == WINAPI_FAMILY_PHONE_APP)
#define HARDCODED_FRAME_WIDTH 1280
#define HARDCODED_FRAME_HEIGHT 720
#else
#define HARDCODED_FRAME_WIDTH 640
#define HARDCODED_FRAME_HEIGHT 480
#endif

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
    TextBox^ portTextBox_;

    MediaElement^ localMedia_;
    MediaElement^ remoteMedia_;

    Button^ startStopButton_;
    Button^ switchCameraButton_;
    Button^ startStopVideoButton_;

    webrtc::VideoRender* vrm_;
    webrtc::VideoCaptureDataCallback* captureCallback_;
    webrtc::TraceCallback* traceCallback_;
    Windows::UI::Core::CoreDispatcher^ dispatcher_;
    bool started_;
    bool startedVideo_;

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
    webrtc::VideoCaptureModule* vcpm_;
    webrtc::VideoEngine* videoEngine_;
    webrtc::ViEBase* videoBase_;
    webrtc::ViENetwork* videoNetwork_;
    webrtc::ViERender* videoRender_;
    webrtc::ViECapture* videoCapture_;
    webrtc::ViERTP_RTCP* videoRtpRtcp_;
    webrtc::ViECodec* videoCodec_;

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
          surface->Width = HARDCODED_FRAME_WIDTH;
          surface->Height = HARDCODED_FRAME_HEIGHT;
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
        switchCameraButton_->Margin = ThicknessHelper::FromLengths(40, 0, 0, 0);
        switchCameraButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnSwitchCameraClick);
        switchCameraButton_->IsEnabled = false;
        stackPanel->Children->Append(switchCameraButton_);

        startStopVideoButton_ = ref new Button();
        startStopVideoButton_->Content = "Start Video";
        startStopVideoButton_->Margin = ThicknessHelper::FromLengths(40, 0, 0, 0);
        startStopVideoButton_->Click += ref new Windows::UI::Xaml::RoutedEventHandler(this, &StandupWinRT::App::OnStartStopVideoClick);
        startStopVideoButton_->IsEnabled = true;
        stackPanel->Children->Append(startStopVideoButton_);
      }

      Window::Current->Content = layoutRoot;
      Window::Current->Activate();
    }

    void OnStartStopClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e);
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
#ifdef VOICE
    Concurrency::create_task([this]() {
      int error;
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

      error = voiceTransport_->SetSendDestination("127.0.0.1", 20010);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVoice, -1,
          "Failed to set send destination for voice channel.");
        return Concurrency::task<void>();
      }

      error = voiceTransport_->SetLocalReceiver(20010);
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

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Stop";
        startStopVideoButton_->IsEnabled = false;
      })));
    });
#endif
#ifdef VIDEO
    Concurrency::create_task([this]() {
      int error;
      const unsigned int KMaxDeviceNameLength = 128;
      const unsigned int KMaxUniqueIdLength = 256;
      char deviceName[KMaxDeviceNameLength];
      memset(deviceName, 0, KMaxDeviceNameLength);
      char uniqueId[KMaxUniqueIdLength];
      memset(uniqueId, 0, KMaxUniqueIdLength);
      uint32_t captureIdx = 0;

      webrtc::VideoCaptureModule::DeviceInfo *devInfo = webrtc::VideoCaptureFactory::CreateDeviceInfo(0);
      if (devInfo == NULL) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to create video capture device info.");
        return Concurrency::task<void>();
      }

      error = devInfo->GetDeviceName(captureIdx, deviceName,
        KMaxDeviceNameLength, uniqueId,
        KMaxUniqueIdLength);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to get video device name.");
        return Concurrency::task<void>();
      }

      strcpy(deviceUniqueId_, uniqueId);

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
      delete devInfo;

      int width = HARDCODED_FRAME_WIDTH, height = HARDCODED_FRAME_HEIGHT, maxFramerate = 30, maxBitrate = 512;

      webrtc::CaptureCapability capability;
      capability.width = width;
      capability.height = height;
      capability.maxFPS = maxFramerate;
      capability.rawType = webrtc::kVideoI420;
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
          videoCodec.width = width;
          videoCodec.height = height;
          videoCodec.maxFramerate = maxFramerate;
          videoCodec.maxBitrate = maxBitrate;
          error = videoCodec_->SetSendCodec(videoChannel_, videoCodec);
          if (error != 0) {
            webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
              "Failed to set send video codec. Error: %d", videoBase_->LastError());
            return Concurrency::task<void>();
          }
          break;
        }
      }

      error = videoTransport_->SetSendDestination("127.0.0.1", 20000);
      if (error != 0) {
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideo, -1,
          "Failed to set send destination for video channel.");
        return Concurrency::task<void>();
      }

      error = videoTransport_->SetLocalReceiver(20000);
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

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Stop";
        startStopVideoButton_->IsEnabled = false;
      })));
    });
#endif
    started_ = true;

  } else {
#ifdef VOICE
    Concurrency::create_task([this]() {
      int error;
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

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Start";
        startStopVideoButton_->IsEnabled = true;
      })));
    });
#endif
#ifdef VIDEO
    Concurrency::create_task([this]() {
      int error;
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

      return Concurrency::create_task(dispatcher_->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this]() {
        startStopButton_->Content = "Start";
        startStopVideoButton_->IsEnabled = true;
      })));
    });
#endif
    started_ = false;
  }
}

void StandupWinRT::App::OnSwitchCameraClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  throw ref new Platform::NotImplementedException();
}

void StandupWinRT::App::OnStartStopVideoClick(Platform::Object ^sender, Windows::UI::Xaml::RoutedEventArgs ^e)
{
  if (!startedVideo_) {
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
