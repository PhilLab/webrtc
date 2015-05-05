#include "Marshalling.h"

#include "webrtc/p2p/base/candidate.h"

using namespace webrtc_winrt_api;

namespace webrtc_winrt_api_internal {

#define DEFINE_MARSHALLED_ENUM_FROM(winrtType, nativeType)\
  void FromCx(winrtType inObj, nativeType& outObj) {\
    std::map<winrtType, nativeType> enumMap = {
#define DEFINE_MARSHALLED_ENUM_TO(winrtType, nativeType)\
    };\
    outObj = enumMap[inObj];\
  }\
  void ToCx(nativeType inObj, winrtType& outObj) {\
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

  void ToCx(
    webrtc::IceCandidateInterface const& inObj,
    webrtc_winrt_api::RTCIceCandidate^* outObj)
  {
    (*outObj) = ref new webrtc_winrt_api::RTCIceCandidate();
    (*outObj)->Candidate = ToCx(inObj.candidate().ToString());
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

  void ToCx(
    webrtc::SessionDescriptionInterface* inObj,
    webrtc_winrt_api::RTCSessionDescription^* outObj)
  {
    (*outObj) = ref new webrtc_winrt_api::RTCSessionDescription(inObj);

    std::string sdp;
    inObj->ToString(&sdp);
    (*outObj)->Sdp = ToCx(sdp);

    webrtc_winrt_api::RTCSdpType type;
    ToCx(inObj->type(), type);
    (*outObj)->Type = type;
  }

}
