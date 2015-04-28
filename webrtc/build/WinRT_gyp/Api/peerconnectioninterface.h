#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"

using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

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

  public value struct RTCIceServer
  {
    String^ Url;
    String^ Username;
    String^ Credential;
  };

  public ref class RTCConfiguration sealed
  {
  public:
    property IVector<RTCIceServer>^ IceServers;
    property IBox<RTCIceTransportPolicy>^ IceTransportPolicy;
    property IBox<RTCBundlePolicy>^ BundlePolicy;
    // TODO: DOMString PeerIdentity
  };

  public ref class RTCPeerConnection sealed
  {
  public:
    RTCPeerConnection(RTCConfiguration^ configuration);

  internal:
  private:

    rtc::scoped_refptr<webrtc::PeerConnectionInterface> _impl;
  };

}
