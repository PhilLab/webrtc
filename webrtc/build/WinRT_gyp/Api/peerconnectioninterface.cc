#include "peerconnectioninterface.h"
#include "GlobalObserver.h"
#include "Marshalling.h"

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/thread.h"
#include "webrtc/test/field_trial.h"
#include "talk/app/webrtc/test/fakeconstraints.h"

#include <map>
#include <ppltasks.h>

using namespace webrtc_winrt_api;
using namespace webrtc_winrt_api_internal;
using namespace Platform;

Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;


// Any globals we need to keep around.
namespace {
  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> gPeerConnectionFactory;
  rtc::Thread gThread;
}

RTCPeerConnection::RTCPeerConnection(RTCConfiguration^ configuration)
{
  webrtc::PeerConnectionInterface::RTCConfiguration cc_configuration;
  FromCx(configuration, cc_configuration);

  webrtc::FakeConstraints constraints;


  _impl = gPeerConnectionFactory->CreatePeerConnection(
    cc_configuration, &constraints, nullptr, nullptr, &_observer);
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateOffer()
{
  IAsyncOperation<RTCSessionDescription^>^ asyncOp = Concurrency::create_async(
    [this]() -> RTCSessionDescription^ {
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

      rtc::scoped_refptr<OfferObserver> observer(new rtc::RefCountedObject<OfferObserver>());
      // TODO: Remove it once the callback has been received.
      _offerObservers.push_back(observer);

      webrtc::PeerConnectionInterface* impl = _impl.get();
      impl->CreateOffer(observer, nullptr);
      impl->signaling_state();

      RTCSessionDescription^ ret = nullptr;
      observer->Wait(
        [&ret](webrtc::SessionDescriptionInterface* sdi) -> void
        {
          ToCx(sdi, &ret);
        },
        [](const std::string& err) -> void
        {
          // TODO: Throw?
        });
      return ret;
  });

  return asyncOp;
}


void WebRTC::Initialize()
{
  rtc::EnsureWinsockInit();
  gThread.Start();
  rtc::ThreadManager::Instance()->SetCurrentThread(&gThread);
  rtc::InitializeSSL();

  webrtc::test::InitFieldTrialsFromString("");

  gPeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
}

