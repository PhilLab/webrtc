#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "GlobalObserver.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

namespace webrtc_winrt_api_internal
{
  class GlobalObserver;
}

namespace webrtc_winrt_api
{
  public ref class WebRTC sealed
  {
  public:
    static void Initialize();

  private:
    // This type is not meant to be created.
    WebRTC();
  };

  public enum class RTCBundlePolicy
  {
    kBundlePolicyBalanced,
    kBundlePolicyMaxBundle,
    kBundlePolicyMaxCompat,
  };

  public enum class RTCIceTransportPolicy
  {
    kNone,
    kRelay,
    kNoHost,
    kAll
  };

  public enum class RTCSdpType
  {
    offer,
    pranswer,
    answer,
  };

  public ref class RTCIceServer sealed
  {
  public:
    property String^ Url;
    property String^ Username;
    property String^ Credential;
  };

  public ref class RTCConfiguration sealed
  {
  public:
    property IVector<RTCIceServer^>^ IceServers;
    property IBox<RTCIceTransportPolicy>^ IceTransportPolicy;
    property IBox<RTCBundlePolicy>^ BundlePolicy;
    // TODO: DOMString PeerIdentity
  };

  public ref class RTCIceCandidate sealed
  {
  public:
    property String^ Candidate;
    property String^ SdpMid;
    property unsigned short sdpMLineIndex;
  };

  public ref class RTCSessionDescription sealed
  {
  public:
    property IBox<RTCSdpType>^ Type;
    property String^ Sdp;
  internal:
    RTCSessionDescription(webrtc::SessionDescriptionInterface* impl);
    webrtc::SessionDescriptionInterface* GetImpl();
  private:
    webrtc::SessionDescriptionInterface* _impl;
  };

  // Events and delegates
#if 1
  public delegate void EventDelegate();

  public ref class RTCTrackEvent sealed
  {
  public:
    // TODO: event properties
  };
  public delegate void RTCTrackEventDelegate(RTCTrackEvent^);

  public ref class RTCPeerConnectionIceEvent sealed
  {
  public:
    property RTCIceCandidate^ Candidate;
  };
  public delegate void RTCPeerConnectionIceEventDelegate(RTCPeerConnectionIceEvent^);
#endif

  public ref class RTCPeerConnection sealed
  {
  public:
    // Required so the observer can raise events in this class.
    // By default event raising is protected.
    friend class webrtc_winrt_api_internal::GlobalObserver;

    RTCPeerConnection(RTCConfiguration^ configuration);

    event RTCPeerConnectionIceEventDelegate^ OnIceCandidate;
    event EventDelegate^ OnNegotiationNeeded;
    event RTCTrackEventDelegate^ OnTrack;

    IAsyncOperation<RTCSessionDescription^>^ CreateOffer();
    IAsyncOperation<RTCSessionDescription^>^ CreateAnswer();
    IAsyncAction^ SetLocalDescription(RTCSessionDescription^ description);
    IAsyncAction^ SetRemoteDescription(RTCSessionDescription^ description);

  private:

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> _impl;
    webrtc_winrt_api_internal::GlobalObserver _observer;

    typedef std::vector<rtc::scoped_refptr<webrtc_winrt_api_internal::CreateSdpObserver>> CreateSdpObservers;
    typedef std::vector<rtc::scoped_refptr<webrtc_winrt_api_internal::SetSdpObserver>> SetSdpObservers;
    CreateSdpObservers _createSdpObservers;
    SetSdpObservers _setSdpObservers;
  };

}
