#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/scopedptrcollection.h"
#include "GlobalObserver.h"
#include "DataChannel.h"

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
  ref class MediaStream;
  ref class MediaStreamTrack;

  [Windows::Foundation::Metadata::WebHostHidden]
  public ref class WebRTC sealed
  {
  public:
    static void Initialize(Windows::UI::Core::CoreDispatcher^ dispatcher);

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

  public enum class RTCSignalingState
  {
    stable,
    haveLocalOffer,
    haveRemoteOffer,
    haveLocalPranswer,
    haveRemotePranswer,
    closed
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
    RTCIceCandidate();
    RTCIceCandidate(String^ candidate, String^ sdpMid, unsigned short sdpMLineIndex);
    property String^ Candidate;
    property String^ SdpMid;
    property unsigned short SdpMLineIndex;
  };

  public ref class RTCSessionDescription sealed
  {
  public:
    RTCSessionDescription();
    RTCSessionDescription(RTCSdpType type, String^ sdp);
    property IBox<RTCSdpType>^ Type;
    property String^ Sdp;
  };

  // Events and delegates
#if 1
  // ------------------
  public ref class RTCPeerConnectionIceEvent sealed
  {
  public:
    property RTCIceCandidate^ Candidate;
  };

  // ------------------
  public ref class MediaStreamEvent sealed
  {
  public:
    property MediaStream^ Stream;
  };
#endif

  public ref class RTCPeerConnection sealed
  {
  public:
    // Required so the observer can raise events in this class.
    // By default event raising is protected.
    friend class webrtc_winrt_api_internal::GlobalObserver;

    RTCPeerConnection(RTCConfiguration^ configuration);

    event RTCPeerConnectionIceEventDelegate^ OnIceCandidate;
    event MediaStreamEventEventDelegate^ OnAddStream;
    event MediaStreamEventEventDelegate^ OnRemoveStream;
    event EventDelegate^ OnNegotiationNeeded;
    event EventDelegate^ OnSignalingStateChange;
    event RTCDataChannelEventDelegate^ OnDataChannel;

    IAsyncOperation<RTCSessionDescription^>^ CreateOffer();
    IAsyncOperation<RTCSessionDescription^>^ CreateAnswer();
    IAsyncAction^ SetLocalDescription(RTCSessionDescription^ description);
    IAsyncAction^ SetRemoteDescription(RTCSessionDescription^ description);
    void AddStream(MediaStream^ stream);
    RTCDataChannel^ CreateDataChannel(String^ label, RTCDataChannelInit^ init);
    IAsyncAction^ AddIceCandidate(RTCIceCandidate^ candidate);

    property RTCSessionDescription^ LocalDescription { RTCSessionDescription^ get(); }
    property RTCSessionDescription^ RemoteDescription { RTCSessionDescription^ get(); }
    property RTCSignalingState SignalingState { RTCSignalingState get(); }

  private:

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> _impl;
    webrtc_winrt_api_internal::GlobalObserver _observer;

    typedef std::vector<rtc::scoped_refptr<webrtc_winrt_api_internal::CreateSdpObserver>> CreateSdpObservers;
    typedef std::vector<rtc::scoped_refptr<webrtc_winrt_api_internal::SetSdpObserver>> SetSdpObservers;
    typedef rtc::ScopedPtrCollection<webrtc_winrt_api_internal::DataChannelObserver> DataChannelObservers;
    CreateSdpObservers _createSdpObservers;
    SetSdpObservers _setSdpObservers;
    DataChannelObservers _dataChannelObservers;
  };

  namespace globals
  {
    extern rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> gPeerConnectionFactory;
    // The worker thread for webrtc.
    extern rtc::Thread gThread;

    template <typename T>
    T RunOnGlobalThread(std::function<T()> fn)
    {
      return gThread.Invoke<T, std::function<T()>>(fn);
    }
  }
}
