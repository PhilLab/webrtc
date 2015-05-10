#pragma once
#include <Mfidl.h>
#include <ppltasks.h>

//--temp code, need to revisist , may removed
#include "webrtc/modules/video_render/windows/video_render_winrt.h"

#include "webrtc/test/channel_transport/include/channel_transport.h"
#include "webrtc/test/field_trial.h"

//-----

/**
 * Implement custom mediastreamsource to wrapper the IMediasource object from media engine so that the javascript can use it as
 * src of the <video> element
 */
namespace webrtc_winjs_api
{
  /***
   * winrt media stream queue eventhandler which implmented the IMFAsyncCallback to help to retrieve Video sample from
   * the event queue in the underlying media engine
   */
  class MssQueueEventhandler : public IMFAsyncCallback{
  public:
    // IUnknown
    IFACEMETHOD(QueryInterface) (REFIID iid, void **ppv);
    IFACEMETHOD_(ULONG, AddRef) ();
    IFACEMETHOD_(ULONG, Release) ();

    MssQueueEventhandler(){ _cRef = 1; };

    void setStreamSource(Windows::Media::Core::IMediaSource^ winrtMss, Windows::Media::Core::MediaStreamSource^ jsMss){
      _winrtMss = winrtMss;
      _jsMss = jsMss;
    }

  public:
    void handleSampleRequestedEvent(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);

    virtual HRESULT STDMETHODCALLTYPE GetParameters(DWORD *pdwFlags, DWORD *pdwQueue);
    virtual HRESULT STDMETHODCALLTYPE Invoke(IMFAsyncResult *pAsyncResult);
  private:
    Windows::Media::Core::IMediaSource^ _winrtMss;

    Windows::Media::Core::MediaStreamSource^ _jsMss;
    Microsoft::WRL::ComPtr<IMFMediaStreamSourceSampleRequest> _currentRequest;
    Windows::Media::Core::MediaStreamSourceSampleRequestDeferral^ _currentRequsetDeferral;
    ULONG _cRef;
  };

  public ref class MediaStreamSourceHelper sealed{
  public:
    MediaStreamSourceHelper() {
    }

    void setStreamSource(Windows::Media::Core::IMediaSource^ winrtMss, Windows::Media::Core::MediaStreamSource^ jsMss){
      _winrtMss = winrtMss;
      _jsMss = jsMss;
      _eventHelper.setStreamSource(winrtMss, jsMss);

    }
    void OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args);
    void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);
  private:
    Windows::Media::Core::IMediaSource^ _winrtMss;

    Windows::Media::Core::MediaStreamSource^ _jsMss;

    MssQueueEventhandler _eventHelper;
  };

  class MediaElementWrapper : public webrtc::IWinRTMediaElement
  {
  public:

    MediaElementWrapper(bool isLocal) :_mss(nullptr), _jsMss(nullptr), _isLocal(isLocal){};
    virtual void Play(){
    };
    virtual void SetMediaStreamSource(Windows::Media::Core::IMediaSource^ mss){
      _mss = mss;

      createJSMediaStreamSource();
    };

    Windows::Media::Core::MediaStreamSource^ getMediaStreamSource(){ return _jsMss; }

  private:
    void createJSMediaStreamSource();
    Windows::Media::Core::IMediaSource^ _mss;

    Windows::Media::Core::MediaStreamSource^ _jsMss;

    Windows::Media::Core::VideoStreamDescriptor^ _videoDesc;

    MediaStreamSourceHelper^ _helper;
    bool _isLocal; //ToDo, remove this attribute, this shall not needed if we can detect the frame width/height from the media engine.
  };

}
