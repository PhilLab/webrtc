#pragma once

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/system_wrappers/interface/scoped_refptr.h"
#include "webrtc/system_wrappers/interface/condition_variable_wrapper.h"
#include "webrtc/base/event.h"
#include <functional>

namespace webrtc_winrt_api
{
  ref class RTCPeerConnection;
}

namespace webrtc_winrt_api_internal
{

  class GlobalObserver :
    public webrtc::PeerConnectionObserver
  {
  public:
    void SetPeerConnection(webrtc_winrt_api::RTCPeerConnection^ pc);

    // PeerConnectionObserver functions
#if 1
    virtual void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState new_state);

    virtual void OnStateChange(StateType state_changed);

    virtual void OnAddStream(webrtc::MediaStreamInterface* stream);

    virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream);

    virtual void OnDataChannel(webrtc::DataChannelInterface* data_channel);

    virtual void OnRenegotiationNeeded();

    virtual void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState new_state);

    virtual void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState new_state);

    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate);

    virtual void OnIceComplete();
#endif

  private:
    webrtc_winrt_api::RTCPeerConnection^ _pc;
  };

  // There is one of those per call to CreateOffer().
  class CreateSdpObserver : public webrtc::CreateSessionDescriptionObserver
  {
  public:
    // TODO: Get a handle on the async operation to unblock.
    CreateSdpObserver();

    // CreateSessionDescriptionObserver implementation
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc);
    virtual void OnFailure(const std::string& error);

    void Wait(
      std::function<void(webrtc::SessionDescriptionInterface*)> onSuccess,
      std::function<void(const std::string&)> onFailure);

  private:
    std::string _error;
    webrtc::SessionDescriptionInterface* _sdi;
    rtc::Event _callbackHappened;
  };

  // There is one of those per call to CreateOffer().
  class SetSdpObserver : public webrtc::SetSessionDescriptionObserver
  {
  public:
    // TODO: Get a handle on the async operation to unblock.
    SetSdpObserver();

    // CreateSessionDescriptionObserver implementation
    virtual void OnSuccess();
    virtual void OnFailure(const std::string& error);

    void Wait(
      std::function<void()> onSuccess,
      std::function<void(const std::string&)> onFailure);

  private:
    std::string _error;
    rtc::Event _callbackHappened;
  };
}

