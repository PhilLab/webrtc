#pragma once

#include <collection.h>
#include <ppltasks.h>
#include <string>
#include <Mfidl.h>

#include "mediastreamsource.h"

//--temp code, need to revisist , may removed
#include "webrtc/test/channel_transport/include/channel_transport.h"
#include "webrtc/test/field_trial.h"
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


//-----

bool autoClose = false;
Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

namespace webrtc_winjs_api {

  public ref class WinJSMediaEngine sealed
  {
  public:

    static property WinJSMediaEngine^ Instance
    {
      WinJSMediaEngine^ get()
      {
        static WinJSMediaEngine^ instance = ref new WinJSMediaEngine();
        return instance;
      }
    }


    /**
     * start the media engine
     */
    Windows::Foundation::IAsyncAction^ start();

    /**
    *stop the media engine
    */
    Windows::Foundation::IAsyncAction^ stop();

    Windows::Media::Core::IMediaSource^ getLocalMediaStreamSource() { return this->localMediaWrapper_->getMediaStreamSource(); };

    Windows::Media::Core::IMediaSource^ getRemoteMediaStreamSource(){ return this->remoteMediaWrapper_->getMediaStreamSource(); };

    virtual ~WinJSMediaEngine();

    //ToDo: remove this function and the global windows dispatcher
    void OnLaunched()
    {
      g_windowDispatcher = dispatcher_ = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

      webrtc_winrt_api::WebRTC::Initialize(dispatcher_);


    }

    static void initial()
    {
      g_windowDispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

      webrtc_winrt_api::WebRTC::Initialize(g_windowDispatcher);


    }

    bool initialized(){ return this->started_; }


  private:
    /**
    * private contructor
    */
    WinJSMediaEngine();

  private:

//Reviewer: you can ignore all the private memebers for now.
//They are used to testing the winjs app. eventually, most of them will be gone or move to c++/CX API for C#
    /**
    * string representation of raw video format.
    */
    std::string getRawVideoFormatString(webrtc::RawVideoType videoType);

    MediaElementWrapper* localMediaWrapper_;
    MediaElementWrapper* remoteMediaWrapper_;
    Concurrency::event stopEvent_;

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
    webrtc::VideoCaptureModule* vcpm_;
    webrtc::VideoEngine* videoEngine_;
    webrtc::ViEBase* videoBase_;
    webrtc::ViENetwork* videoNetwork_;
    webrtc::ViERender* videoRender_;
    webrtc::ViECapture* videoCapture_;
    webrtc::ViERTP_RTCP* videoRtpRtcp_;
    webrtc::ViECodec* videoCodec_;

  };



}