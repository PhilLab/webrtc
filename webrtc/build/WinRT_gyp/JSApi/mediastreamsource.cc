// mediastreamsource.cc
#include "mediastreamsource.h"
#include <string>
#include <Mfidl.h>
#include <collection.h>


using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;

using namespace  webrtc_winjs_api;

// Macro that releases a COM object if not NULL.
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)     do { if ((p)) { (p)->Release(); (p) = NULL; } } while(0)
#endif

//-----toDO, REMOVE THIS, THIS SHALL NOT NEEDED
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

//--------------------




// IUnknown methods
IFACEMETHODIMP MssQueueEventhandler::QueryInterface(REFIID riid, void **ppv)
{
  if (ppv == nullptr)
  {
    return E_POINTER;
  }
  (*ppv) = nullptr;

  HRESULT hr = S_OK;
  if (riid == IID_IUnknown ||
    riid == IID_IMFAsyncCallback)
  {
    (*ppv) = static_cast<IMFAsyncCallback*>(this);
    AddRef();
  }
  else
  {
    hr = E_NOINTERFACE;
  }

  return hr;
}

IFACEMETHODIMP_(ULONG) MssQueueEventhandler::AddRef()
{
  return InterlockedIncrement(&_cRef);
}

IFACEMETHODIMP_(ULONG) MssQueueEventhandler::Release()
{
  long cRef = InterlockedDecrement(&_cRef);
  if (cRef == 0)
  {
    delete this;
  }
  return cRef;
}


HRESULT MssQueueEventhandler::GetParameters(DWORD *pdwFlags, DWORD *pdwQueue){

  *pdwFlags = MFASYNC_BLOCKING_CALLBACK;
  *pdwQueue = MFASYNC_CALLBACK_QUEUE_STANDARD;


  return S_OK;
}
HRESULT MssQueueEventhandler::Invoke(IMFAsyncResult *pAsyncResult){
  HRESULT hr = S_OK;
  IMFMediaEvent* pEvent = NULL;
  MediaEventType meType = MEUnknown;
  BOOL fGetAnotherEvent = TRUE;
  HRESULT hrStatus = S_OK;

  webrtc::VideoRenderMediaSourceWinRT*  winrtms = reinterpret_cast<webrtc::VideoRenderMediaSourceWinRT*>(_winrtMss);


  if (winrtms == NULL) {
    //should never happen
    return S_FALSE;
  }


  // Get the event from the event queue.
  // Assume that m_pEventGenerator is a valid pointer to the
  // event generator's IMFMediaEventGenerator interface.
  hr = winrtms->getCurrentActiveStream()->EndGetEvent(pAsyncResult, &pEvent);

  // Get the event type.
  if (SUCCEEDED(hr))
  {
    hr = pEvent->GetType(&meType);
  }

  // Get the event status. If the operation that triggered the event 
  // did not succeed, the status is a failure code.
  if (SUCCEEDED(hr))
  {
    hr = pEvent->GetStatus(&hrStatus);
  }

  if (SUCCEEDED(hr) && meType == MEMediaSample)
  {
    // TODO: Handle the event.

    PROPVARIANT var;
    hr = pEvent->GetValue(&var);
    if (SUCCEEDED(hr)){

      IMFSample* aSample = NULL;

      if (var.vt == VT_UNKNOWN)
      {
        hr = var.punkVal->QueryInterface(&aSample);
      }
      else
      {
        hr = S_FALSE;
      }

      if (aSample != NULL){

        _currentRequest->SetSample(aSample);
        _currentRequsetDeferral->Complete();
        fGetAnotherEvent = FALSE;
      }

      PropVariantClear(&var);
    }
  }

  // If not finished, request another event.
  // Pass in a pointer to this instance of the application's
  // CEventHandler class, which implements the callback.
  if (fGetAnotherEvent)
  {
    hr = winrtms->getCurrentActiveStream()->BeginGetEvent(this, NULL);
  }

  SAFE_RELEASE(pEvent);
  return hr;
}

//Handling "sample request" from the mediastreamsource(from UI element)
void MssQueueEventhandler::handleSampleRequestedEvent(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args){

  MediaStreamSourceSampleRequest ^ request = args->Request;


  HRESULT hr = (request != nullptr) ? S_OK : E_POINTER;

  if (SUCCEEDED(hr))
  {
    hr = reinterpret_cast<IInspectable*>(request)->QueryInterface(_currentRequest.ReleaseAndGetAddressOf());
  }

  webrtc::VideoRenderMediaSourceWinRT*  winrtms = reinterpret_cast<webrtc::VideoRenderMediaSourceWinRT*>(_winrtMss);


  if (winrtms == NULL) {

    return;

  }

  _currentRequsetDeferral = request->GetDeferral();

  winrtms->requestSample(NULL);

  hr = winrtms->getCurrentActiveStream()->BeginGetEvent(this, NULL);

}

void MediaElementWrapper::createJSMediaStreamSource() {

  webrtc::VideoRenderMediaSourceWinRT*  winrtms = reinterpret_cast<webrtc::VideoRenderMediaSourceWinRT*>(_mss);


  if (winrtms == NULL) {

    return;

  }

  webrtc::StreamDescription streamInfo=winrtms->getCurrentActiveStream()->getCurrentStreamDescription();

  VideoEncodingProperties^ videoProperties = VideoEncodingProperties::CreateUncompressed(MediaEncodingSubtypes::Iyuv, streamInfo.dwFrameWidth, streamInfo.dwFrameHeight);
  _videoDesc = ref new Windows::Media::Core::VideoStreamDescriptor(videoProperties);
  _videoDesc->EncodingProperties->FrameRate->Numerator = streamInfo.dwFrameRateNumerator;
  _videoDesc->EncodingProperties->FrameRate->Denominator = streamInfo.dwFrameRateDenominator;
  _videoDesc->EncodingProperties->Bitrate = (unsigned int)(streamInfo.dwFrameRateNumerator * streamInfo.dwFrameRateDenominator * streamInfo.dwFrameWidth *  streamInfo.dwFrameHeight * 4);

  _jsMss = ref new Windows::Media::Core::MediaStreamSource(_videoDesc);

  //ToDo: not sure how the buffer time impact the playback jitters/speed, need to further investigate
  //TimeSpan spanBuffer;
  //spanBuffer.Duration = 250000;
  //_jsMss->BufferTime = spanBuffer;
  //_jsMss->Duration = spanBuffer;
  _helper = ref new MediaStreamSourceHelper();
  _helper->setStreamSource(_mss, _jsMss);
  _jsMss->Starting += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^>(_helper, &MediaStreamSourceHelper::OnStarting);
  _jsMss->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(_helper, &MediaStreamSourceHelper::OnSampleRequested);
}

//Handling "starting" event from the mediastreamsource(from UI element)
void MediaStreamSourceHelper::OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args){
  this->_eventHelper.setStreamSource(this->_winrtMss, this->_jsMss);

  webrtc::VideoRenderMediaSourceWinRT*  winrtms = reinterpret_cast<webrtc::VideoRenderMediaSourceWinRT*>(_winrtMss);


  if (winrtms == NULL) {

    return;

  }

  winrtms->jSStart();

}

//relay the "sample request" from the mediastreamsource(from UI element) to event handler
void MediaStreamSourceHelper::OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args){
  _eventHelper.handleSampleRequestedEvent(sender, args);
}


