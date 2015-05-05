#pragma once

#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/jsep.h"
#include "peerconnectioninterface.h"

#define DECLARE_MARSHALLED_ENUM(winrtType, nativeType) \
  void FromCx(winrtType inObj, nativeType& outObj);\
  void ToCx(nativeType const& inObj, winrtType& outObj)

// Marshalling functions to convert from WinRT objects to native cpp.
namespace webrtc_winrt_api_internal {

  DECLARE_MARSHALLED_ENUM(webrtc_winrt_api::RTCBundlePolicy, webrtc::PeerConnectionInterface::BundlePolicy);
  DECLARE_MARSHALLED_ENUM(webrtc_winrt_api::RTCIceTransportPolicy, webrtc::PeerConnectionInterface::IceTransportsType);

  void FromCx(
    webrtc_winrt_api::RTCIceServer^ inObj,
    webrtc::PeerConnectionInterface::IceServer& outObj);

  // Templated function to convert vectors.
  template <typename I, typename O>
  void FromCx(
    IVector<I>^ inArray,
    std::vector<O>& outArray)
  {
    for (auto inObj : inArray)
    {
      // TODO: Optimize with reference to object in vector.
      O outObj;
      FromCx(inObj, outObj);
      outArray.push_back(outObj);
    }
  }

  void FromCx(
    webrtc_winrt_api::RTCConfiguration^ inObj,
    webrtc::PeerConnectionInterface::RTCConfiguration& outObj);

  void ToCx(
    webrtc::IceCandidateInterface const& inObj,
    webrtc_winrt_api::RTCIceCandidate^* outObj);

  std::string FromCx(
    webrtc_winrt_api::RTCSdpType const& inObj);
  void ToCx(
    std::string const& inObj,
    webrtc_winrt_api::RTCSdpType& outObj);

  void ToCx(
    webrtc::SessionDescriptionInterface* inObj,
    webrtc_winrt_api::RTCSessionDescription^* outObj);
}
