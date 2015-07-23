
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/Marshalling.h"
#include <map>

#include "webrtc/p2p/base/candidate.h"
#include "talk/app/webrtc/webrtcsdp.h"

using webrtc_winrt_api::RTCBundlePolicy;
using webrtc_winrt_api::RTCIceTransportPolicy;
using webrtc_winrt_api::RTCSignalingState;
using webrtc_winrt_api::RTCDataChannelState;
using webrtc_winrt_api::RTCIceGatheringState;
using webrtc_winrt_api::RTCIceConnectionState;
using webrtc_winrt_api::RTCIceServer;
using webrtc_winrt_api::RTCConfiguration;

namespace webrtc_winrt_api_internal {

#define DEFINE_MARSHALLED_ENUM(name, winrtType, nativeType)\
  typedef winrtType name##_winrtType;\
  typedef nativeType name##_nativeType;\
  std::map<winrtType, nativeType> name##_enumMap = {
#define DEFINE_MARSHALLED_ENUM_END(name)\
  };\
  void FromCx(name##_winrtType inObj, name##_nativeType* outObj) {\
    (*outObj) = name##_enumMap[inObj];\
  }\
  void ToCx(name##_nativeType const& inObj, name##_winrtType* outObj) {\
    for (auto& kv : name##_enumMap) {\
      if (kv.second == inObj) {\
        (*outObj) = kv.first;\
        return;\
      }\
    }\
    throw "Marshalling failed";\
  }\

  DEFINE_MARSHALLED_ENUM(BundlePolicy,
    RTCBundlePolicy, webrtc::PeerConnectionInterface::BundlePolicy)
      { RTCBundlePolicy::Balanced,
        webrtc::PeerConnectionInterface::kBundlePolicyBalanced },
      { RTCBundlePolicy::MaxBundle,
        webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle },
      { RTCBundlePolicy::MaxCompat,
        webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat },
  DEFINE_MARSHALLED_ENUM_END(BundlePolicy)

  DEFINE_MARSHALLED_ENUM(IceTransportPolicy,
    RTCIceTransportPolicy, webrtc::PeerConnectionInterface::IceTransportsType)
      { RTCIceTransportPolicy::None,
        webrtc::PeerConnectionInterface::kNone },
      { RTCIceTransportPolicy::Relay,
        webrtc::PeerConnectionInterface::kRelay },
      { RTCIceTransportPolicy::NoHost,
        webrtc::PeerConnectionInterface::kNoHost },
      { RTCIceTransportPolicy::All,
        webrtc::PeerConnectionInterface::kAll },
  DEFINE_MARSHALLED_ENUM_END(IceTransportPolicy)

  DEFINE_MARSHALLED_ENUM(SignalingState,
    RTCSignalingState, webrtc::PeerConnectionInterface::SignalingState)
      { RTCSignalingState::Stable,
        webrtc::PeerConnectionInterface::kStable },
      { RTCSignalingState::HaveLocalOffer,
        webrtc::PeerConnectionInterface::kHaveLocalOffer },
      { RTCSignalingState::HaveRemoteOffer,
        webrtc::PeerConnectionInterface::kHaveRemoteOffer },
      { RTCSignalingState::HaveLocalPranswer,
        webrtc::PeerConnectionInterface::kHaveLocalPrAnswer },
      { RTCSignalingState::HaveRemotePranswer,
        webrtc::PeerConnectionInterface::kHaveRemotePrAnswer },
      { RTCSignalingState::Closed,
        webrtc::PeerConnectionInterface::kClosed },
  DEFINE_MARSHALLED_ENUM_END(SignalingState)

  DEFINE_MARSHALLED_ENUM(DataChannelState,
    RTCDataChannelState,
    webrtc::DataChannelInterface::DataState)
      { RTCDataChannelState::Connecting,
        webrtc::DataChannelInterface::kConnecting },
      { RTCDataChannelState::Open,
        webrtc::DataChannelInterface::kOpen },
      { RTCDataChannelState::Closing,
        webrtc::DataChannelInterface::kClosing },
      { RTCDataChannelState::Closed,
        webrtc::DataChannelInterface::kClosed },
  DEFINE_MARSHALLED_ENUM_END(DataChannelState)

  DEFINE_MARSHALLED_ENUM(IceGatheringState,
    RTCIceGatheringState, webrtc::PeerConnectionInterface::IceGatheringState)
      { RTCIceGatheringState::New,
        webrtc::PeerConnectionInterface::kIceGatheringNew },
      { RTCIceGatheringState::Gathering,
        webrtc::PeerConnectionInterface::kIceGatheringGathering },
      { RTCIceGatheringState::Complete,
        webrtc::PeerConnectionInterface::kIceGatheringComplete },
  DEFINE_MARSHALLED_ENUM_END(IceGatheringState)

  DEFINE_MARSHALLED_ENUM(IceConnectionState,
    RTCIceConnectionState, webrtc::PeerConnectionInterface::IceConnectionState)
      { RTCIceConnectionState::New,
        webrtc::PeerConnectionInterface::kIceConnectionNew },
      { RTCIceConnectionState::Checking,
        webrtc::PeerConnectionInterface::kIceConnectionChecking },
      { RTCIceConnectionState::Connected,
        webrtc::PeerConnectionInterface::kIceConnectionConnected },
      { RTCIceConnectionState::Completed,
        webrtc::PeerConnectionInterface::kIceConnectionCompleted },
      { RTCIceConnectionState::Failed,
        webrtc::PeerConnectionInterface::kIceConnectionFailed },
      { RTCIceConnectionState::Disconnected,
        webrtc::PeerConnectionInterface::kIceConnectionDisconnected },
      { RTCIceConnectionState::Closed,
        webrtc::PeerConnectionInterface::kIceConnectionClosed },
  DEFINE_MARSHALLED_ENUM_END(IceConnectionState)

  std::string FromCx(String^ inObj) {
    return rtc::ToUtf8(inObj->Data());
  }

  String^ ToCx(std::string const& inObj) {
    return ref new String(rtc::ToUtf16(inObj).c_str());
  }

  void FromCx(
    RTCIceServer^ inObj,
    webrtc::PeerConnectionInterface::IceServer* outObj) {
    if (inObj->Url != nullptr)
      outObj->uri = rtc::ToUtf8(inObj->Url->Data());
    if (inObj->Username != nullptr)
      outObj->username = rtc::ToUtf8(inObj->Username->Data());
    if (inObj->Credential != nullptr)
      outObj->password = rtc::ToUtf8(inObj->Credential->Data());
  }

  void FromCx(
    RTCConfiguration^ inObj,
    webrtc::PeerConnectionInterface::RTCConfiguration* outObj) {

    // BundlePolicy
    if (inObj->BundlePolicy == nullptr) {
      // Default as defined in the WebApi spec.
      outObj->bundle_policy =
        webrtc::PeerConnectionInterface::kBundlePolicyBalanced;
    } else {
      FromCx(inObj->BundlePolicy->Value, &outObj->bundle_policy);
    }

    // IceTransportPolicy
    if (inObj->IceTransportPolicy == nullptr) {
      // Default as defined in the WebApi spec.
      outObj->type = webrtc::PeerConnectionInterface::kAll;
    } else {
      FromCx(inObj->IceTransportPolicy->Value, &outObj->type);
    }

    // IceServers
    if (inObj->IceServers != nullptr) {
      FromCx(inObj->IceServers, &outObj->servers);
    }
    // TODO(WINRT): Other fields once they are added.
  }

  void FromCx(
    webrtc_winrt_api::RTCDataChannelInit^ inObj,
    webrtc::DataChannelInit* outObj) {
    // Use ternary operators to handle default values.
    outObj->ordered = inObj->Ordered != nullptr ? inObj->Ordered->Value : true;
    outObj->maxRetransmitTime = inObj->MaxPacketLifeTime != nullptr ?
      inObj->MaxPacketLifeTime->Value : -1;
    outObj->maxRetransmits = inObj->MaxRetransmits != nullptr ?
      inObj->MaxRetransmits->Value : -1;
    outObj->protocol = FromCx(inObj->Protocol);
    outObj->negotiated = inObj->Negotiated != nullptr ?
      inObj->Negotiated->Value : false;
    outObj->id = inObj->Id != nullptr ? inObj->Id->Value : -1;
  }

  void FromCx(
    webrtc_winrt_api::RTCIceCandidate^ inObj,
    rtc::scoped_ptr<webrtc::IceCandidateInterface>* outObj) {
    outObj->reset(webrtc::CreateIceCandidate(
      FromCx(inObj->SdpMid),
      inObj->SdpMLineIndex,
      FromCx(inObj->Candidate), nullptr));
  }

  void ToCx(
    webrtc::IceCandidateInterface const& inObj,
    webrtc_winrt_api::RTCIceCandidate^* outObj) {
    (*outObj) = ref new webrtc_winrt_api::RTCIceCandidate();
    (*outObj)->Candidate = ToCx(webrtc::SdpSerializeCandidate(inObj));
    (*outObj)->SdpMid = ToCx(inObj.sdp_mid());
    (*outObj)->SdpMLineIndex = inObj.sdp_mline_index();
  }

  std::string FromCx(
    webrtc_winrt_api::RTCSdpType const& inObj) {
    if (inObj == webrtc_winrt_api::RTCSdpType::Offer)
      return webrtc::SessionDescriptionInterface::kOffer;
    if (inObj == webrtc_winrt_api::RTCSdpType::Answer)
      return webrtc::SessionDescriptionInterface::kAnswer;
    if (inObj == webrtc_winrt_api::RTCSdpType::Pranswer)
      return webrtc::SessionDescriptionInterface::kPrAnswer;
    // TODO(WINRT): Throw?
    return "";
  }

  void ToCx(
    std::string const& inObj,
    webrtc_winrt_api::RTCSdpType* outObj) {
    if (inObj == webrtc::SessionDescriptionInterface::kOffer)
      (*outObj) = webrtc_winrt_api::RTCSdpType::Offer;
    if (inObj == webrtc::SessionDescriptionInterface::kAnswer)
      (*outObj) = webrtc_winrt_api::RTCSdpType::Answer;
    if (inObj == webrtc::SessionDescriptionInterface::kPrAnswer)
      (*outObj) = webrtc_winrt_api::RTCSdpType::Pranswer;
  }

  void FromCx(
    webrtc_winrt_api::RTCSessionDescription^ inObj,
    rtc::scoped_ptr<webrtc::SessionDescriptionInterface>* outObj) {
    outObj->reset(webrtc::CreateSessionDescription(
      FromCx(inObj->Type->Value),
      FromCx(inObj->Sdp), nullptr));
  }

  void ToCx(
    const webrtc::SessionDescriptionInterface* inObj,
    webrtc_winrt_api::RTCSessionDescription^* outObj) {
    (*outObj) = ref new webrtc_winrt_api::RTCSessionDescription();

    std::string sdp;
    inObj->ToString(&sdp);
    (*outObj)->Sdp = ToCx(sdp);

    webrtc_winrt_api::RTCSdpType type;
    ToCx(inObj->type(), &type);
    (*outObj)->Type = type;
  }

}  // namespace webrtc_winrt_api_internal

