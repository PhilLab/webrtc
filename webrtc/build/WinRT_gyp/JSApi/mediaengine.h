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


    virtual ~WinJSMediaEngine();

    //ToDo: remove this function and the global windows dispatcher
    void OnLaunched()
    {
      g_windowDispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

    }

    static void initial()
    {
      g_windowDispatcher = Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;


      webrtc_winrt_api::WebRTC::Initialize(g_windowDispatcher);

      webrtc_winrt_api::Media^ mediaObject = ref new webrtc_winrt_api::Media();

    }

    static Windows::Foundation::IAsyncOperation<bool>^ initialize(){
      return webrtc_winrt_api::WebRTC::RequestAccessForMediaCapture();
    }



  private:
    /**
    * private contructor
    */
    WinJSMediaEngine();


  };



}