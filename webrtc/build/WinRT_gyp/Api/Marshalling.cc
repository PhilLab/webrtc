#include "Marshalling.h"

#include "webrtc/p2p/base/candidate.h"
#include "talk/app/webrtc/webrtcsdp.h"

using namespace webrtc_winrt_api;

namespace webrtc_winrt_api_internal {

#define DEFINE_MARSHALLED_ENUM_FROM(winrtType, nativeType)\
  void FromCx(winrtType inObj, nativeType& outObj) {\
    std::map<winrtType, nativeType> enumMap = {
#define DEFINE_MARSHALLED_ENUM_TO(winrtType, nativeType)\
    };\
    outObj = enumMap[inObj];\
  }\
  void ToCx(nativeType const& inObj, winrtType& outObj) {\
    std::map<nativeType, winrtType> enumMap = {
#define DEFINE_MARSHALLED_ENUM_END()\
    };\
    outObj = enumMap[inObj];\
  }\


  DEFINE_MARSHALLED_ENUM_FROM(RTCBundlePolicy, webrtc::PeerConnectionInterface::BundlePolicy)
      { RTCBundlePolicy::kBundlePolicyBalanced, webrtc::PeerConnectionInterface::kBundlePolicyBalanced },
      { RTCBundlePolicy::kBundlePolicyMaxBundle, webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle },
      { RTCBundlePolicy::kBundlePolicyMaxCompat, webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat },
  DEFINE_MARSHALLED_ENUM_TO(RTCBundlePolicy, webrtc::PeerConnectionInterface::BundlePolicy)
      { webrtc::PeerConnectionInterface::kBundlePolicyBalanced, RTCBundlePolicy::kBundlePolicyBalanced },
      { webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle, RTCBundlePolicy::kBundlePolicyMaxBundle },
      { webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat, RTCBundlePolicy::kBundlePolicyMaxCompat },
  DEFINE_MARSHALLED_ENUM_END()

  DEFINE_MARSHALLED_ENUM_FROM(RTCIceTransportPolicy, webrtc::PeerConnectionInterface::IceTransportsType)
      { RTCIceTransportPolicy::kNone, webrtc::PeerConnectionInterface::kNone },
      { RTCIceTransportPolicy::kRelay, webrtc::PeerConnectionInterface::kRelay },
      { RTCIceTransportPolicy::kNoHost, webrtc::PeerConnectionInterface::kNoHost },
      { RTCIceTransportPolicy::kAll, webrtc::PeerConnectionInterface::kAll },
  DEFINE_MARSHALLED_ENUM_TO(RTCIceTransportPolicy, webrtc::PeerConnectionInterface::IceTransportsType)
      { webrtc::PeerConnectionInterface::kNone, RTCIceTransportPolicy::kNone },
      { webrtc::PeerConnectionInterface::kRelay, RTCIceTransportPolicy::kRelay },
      { webrtc::PeerConnectionInterface::kNoHost, RTCIceTransportPolicy::kNoHost },
      { webrtc::PeerConnectionInterface::kAll, RTCIceTransportPolicy::kAll },
  DEFINE_MARSHALLED_ENUM_END()

  DEFINE_MARSHALLED_ENUM_FROM(RTCSignalingState, webrtc::PeerConnectionInterface::SignalingState)
    { RTCSignalingState::stable, webrtc::PeerConnectionInterface::kStable },
    { RTCSignalingState::haveLocalOffer, webrtc::PeerConnectionInterface::kHaveLocalOffer },
    { RTCSignalingState::haveRemoteOffer, webrtc::PeerConnectionInterface::kHaveRemoteOffer },
    { RTCSignalingState::haveLocalPranswer, webrtc::PeerConnectionInterface::kHaveLocalPrAnswer },
    { RTCSignalingState::haveRemotePranswer, webrtc::PeerConnectionInterface::kHaveRemotePrAnswer },
    { RTCSignalingState::closed, webrtc::PeerConnectionInterface::kClosed },
  DEFINE_MARSHALLED_ENUM_TO(RTCSignalingState, webrtc::PeerConnectionInterface::SignalingState)
    { webrtc::PeerConnectionInterface::kStable, RTCSignalingState::stable },
    { webrtc::PeerConnectionInterface::kHaveLocalOffer, RTCSignalingState::haveLocalOffer },
    { webrtc::PeerConnectionInterface::kHaveRemoteOffer, RTCSignalingState::haveRemoteOffer },
    { webrtc::PeerConnectionInterface::kHaveLocalPrAnswer, RTCSignalingState::haveLocalPranswer },
    { webrtc::PeerConnectionInterface::kHaveRemotePrAnswer, RTCSignalingState::haveRemotePranswer },
    { webrtc::PeerConnectionInterface::kClosed, RTCSignalingState::closed },
  DEFINE_MARSHALLED_ENUM_END()

  DEFINE_MARSHALLED_ENUM_FROM(webrtc_winrt_api::RTCDataChannelState, webrtc::DataChannelInterface::DataState)
    { RTCDataChannelState::connecting, webrtc::DataChannelInterface::kConnecting },
    { RTCDataChannelState::open, webrtc::DataChannelInterface::kOpen },
    { RTCDataChannelState::closing, webrtc::DataChannelInterface::kClosing },
    { RTCDataChannelState::closed, webrtc::DataChannelInterface::kClosed },
  DEFINE_MARSHALLED_ENUM_TO(webrtc_winrt_api::RTCDataChannelState, webrtc::DataChannelInterface::DataState)
    { webrtc::DataChannelInterface::kConnecting, RTCDataChannelState::connecting },
    { webrtc::DataChannelInterface::kOpen, RTCDataChannelState::open },
    { webrtc::DataChannelInterface::kClosing, RTCDataChannelState::closing },
    { webrtc::DataChannelInterface::kClosed, RTCDataChannelState::closed },
  DEFINE_MARSHALLED_ENUM_END()

  std::string FromCx(String^ inObj)
  {
    return rtc::ToUtf8(inObj->Data());
  }

  String^ ToCx(std::string const& inObj)
  {
    return ref new String(rtc::ToUtf16(inObj).c_str());
  }

  void FromCx(
    RTCIceServer^ inObj,
    webrtc::PeerConnectionInterface::IceServer& outObj)
  {
    if (inObj->Url != nullptr)
      outObj.uri = rtc::ToUtf8(inObj->Url->Data());
    if (inObj->Username != nullptr)
      outObj.username = rtc::ToUtf8(inObj->Username->Data());
    if (inObj->Credential != nullptr)
      outObj.password = rtc::ToUtf8(inObj->Credential->Data());
  }

  void FromCx(
    RTCConfiguration^ inObj,
    webrtc::PeerConnectionInterface::RTCConfiguration& outObj)
    {

    // BundlePolicy
    if (inObj->BundlePolicy == nullptr)
    {
      // Default as defined in the WebApi spec.
      outObj.bundle_policy = webrtc::PeerConnectionInterface::kBundlePolicyBalanced;
    }
    else
    {
      FromCx(inObj->BundlePolicy->Value, outObj.bundle_policy);
    }

    // IceTransportPolicy
    if (inObj->IceTransportPolicy == nullptr)
    {
      // Default as defined in the WebApi spec.
      outObj.type = webrtc::PeerConnectionInterface::kAll;
    }
    else
    {
      FromCx(inObj->IceTransportPolicy->Value, outObj.type);
    }

    // IceServers
    if (inObj->IceServers != nullptr)
    {
      FromCx(inObj->IceServers, outObj.servers);
    }
    // TODO: Other fields once they are added.
  }

  void FromCx(
    webrtc_winrt_api::RTCDataChannelInit^ inObj,
    webrtc::DataChannelInit& outObj)
  {
    // Use ternary operators to handle default values.
    outObj.ordered = inObj->Ordered != nullptr ? inObj->Ordered->Value : true;
    outObj.maxRetransmitTime = inObj->MaxPacketLifeTime != nullptr ? inObj->MaxPacketLifeTime->Value : -1;
    outObj.maxRetransmits = inObj->MaxRetransmits != nullptr ? inObj->MaxRetransmits->Value : -1;
    outObj.protocol = FromCx(inObj->Protocol);
    outObj.negotiated = inObj->Negotiated != nullptr ? inObj->Negotiated->Value : false;
    outObj.id = inObj->Id != nullptr ? inObj->Id->Value : -1;
  }

  void FromCx(
    webrtc_winrt_api::RTCIceCandidate^ inObj,
    rtc::scoped_ptr<webrtc::IceCandidateInterface>& outObj)
  {
    outObj.reset(webrtc::CreateIceCandidate(
      FromCx(inObj->SdpMid),
      inObj->sdpMLineIndex,
      FromCx(inObj->Candidate)));
  }

  void ToCx(
    webrtc::IceCandidateInterface const& inObj,
    webrtc_winrt_api::RTCIceCandidate^* outObj)
  {
    (*outObj) = ref new webrtc_winrt_api::RTCIceCandidate();
    (*outObj)->Candidate = ToCx(webrtc::SdpSerializeCandidate(inObj));
    (*outObj)->SdpMid = ToCx(inObj.sdp_mid());
    (*outObj)->sdpMLineIndex = inObj.sdp_mline_index();
  }

  std::string FromCx(
    webrtc_winrt_api::RTCSdpType const& inObj)
  {
    if (inObj == webrtc_winrt_api::RTCSdpType::offer)
      return webrtc::SessionDescriptionInterface::kOffer;
    if (inObj == webrtc_winrt_api::RTCSdpType::answer)
      return webrtc::SessionDescriptionInterface::kAnswer;
    if (inObj == webrtc_winrt_api::RTCSdpType::pranswer)
      return webrtc::SessionDescriptionInterface::kPrAnswer;
    // TODO: Throw?
    return "";
  }

  void ToCx(
    std::string const& inObj,
    webrtc_winrt_api::RTCSdpType& outObj)
  {
    if (inObj == webrtc::SessionDescriptionInterface::kOffer)
      outObj = webrtc_winrt_api::RTCSdpType::offer;
    if (inObj == webrtc::SessionDescriptionInterface::kAnswer)
      outObj = webrtc_winrt_api::RTCSdpType::answer;
    if (inObj == webrtc::SessionDescriptionInterface::kPrAnswer)
      outObj = webrtc_winrt_api::RTCSdpType::pranswer;
  }

  void FromCx(
    webrtc_winrt_api::RTCSessionDescription^ inObj,
    rtc::scoped_ptr<webrtc::SessionDescriptionInterface>& outObj)
  {
    outObj.reset(webrtc::CreateSessionDescription(
      FromCx(inObj->Type->Value),
      FromCx(inObj->Sdp)));
  }

  void ToCx(
    const webrtc::SessionDescriptionInterface* inObj,
    webrtc_winrt_api::RTCSessionDescription^* outObj)
  {
    (*outObj) = ref new webrtc_winrt_api::RTCSessionDescription();

    std::string sdp;
    inObj->ToString(&sdp);
    (*outObj)->Sdp = ToCx(sdp);

    webrtc_winrt_api::RTCSdpType type;
    ToCx(inObj->type(), type);
    (*outObj)->Type = type;
  }

}
