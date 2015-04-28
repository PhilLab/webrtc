// Class1.cpp
#include "peerconnectioninterface.h"

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/thread.h"

#include <map>

using namespace webrtc_winrt_api;
using namespace Platform;

Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

#define BEGIN_ENUM_CONVERT_FROM_CX(inObj, outObj, inName, outName, def)

// Marshalling functions to convert from WinRT objects to native cpp.
namespace {
  void FromCx(
    RTCBundlePolicy inObj,
    webrtc::PeerConnectionInterface::BundlePolicy& outObj)
  {
    std::map<RTCBundlePolicy, webrtc::PeerConnectionInterface::BundlePolicy> enumMap =
    {
      { RTCBundlePolicy::kBundlePolicyBalanced, webrtc::PeerConnectionInterface::kBundlePolicyBalanced },
      { RTCBundlePolicy::kBundlePolicyMaxBundle, webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle },
      { RTCBundlePolicy::kBundlePolicyMaxCompat, webrtc::PeerConnectionInterface::kBundlePolicyMaxCompat },
    };

    outObj = enumMap[inObj];
  }

  void FromCx(
    RTCIceTransportPolicy inObj,
    webrtc::PeerConnectionInterface::IceTransportsType& outObj)
  {
    std::map<RTCIceTransportPolicy, webrtc::PeerConnectionInterface::IceTransportsType> enumMap =
    {
      { RTCIceTransportPolicy::kNone, webrtc::PeerConnectionInterface::kNone },
      { RTCIceTransportPolicy::kRelay, webrtc::PeerConnectionInterface::kRelay },
      { RTCIceTransportPolicy::kNoHost, webrtc::PeerConnectionInterface::kNoHost },
      { RTCIceTransportPolicy::kAll, webrtc::PeerConnectionInterface::kAll },
    };

    outObj = enumMap[inObj];
  }

  void FromCx(
    RTCIceServer inObj,
    webrtc::PeerConnectionInterface::IceServer& outObj)
  {
    if (inObj.Url != nullptr)
      outObj.uri = rtc::ToUtf8(inObj.Url->Data());
    if (inObj.Username != nullptr)
      outObj.username = rtc::ToUtf8(inObj.Username->Data());
    if (inObj.Credential != nullptr)
      outObj.password = rtc::ToUtf8(inObj.Credential->Data());
  }

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
    webrtc::PeerConnectionInterface::RTCConfiguration& outObj,
    RTCConfiguration^ inObj)
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
}

// Any globals we need to keep around.
namespace {
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> gPeerConnectionFactory;
  rtc::Thread gThread;

}

RTCPeerConnection::RTCPeerConnection(RTCConfiguration^ configuration)
{
  webrtc::PeerConnectionInterface::RTCConfiguration cc_configuration;
  FromCx(cc_configuration, configuration);
  _impl = gPeerConnectionFactory->CreatePeerConnection(
    cc_configuration, nullptr, nullptr, nullptr, nullptr);
}


void WebRTC::Initialize()
{
  rtc::EnsureWinsockInit();
  rtc::ThreadManager::Instance()->SetCurrentThread(&gThread);
  rtc::InitializeSSL();

  gPeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
}

