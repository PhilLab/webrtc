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

RTCSessionDescription::RTCSessionDescription(webrtc::SessionDescriptionInterface* impl)
  : _impl(impl)
{

}

webrtc::SessionDescriptionInterface* RTCSessionDescription::GetImpl()
{
  return _impl;
}

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

      rtc::scoped_refptr<CreateSdpObserver> observer(new rtc::RefCountedObject<CreateSdpObserver>());
      // TODO: Remove it once the callback has been received.
      _createSdpObservers.push_back(observer);

      _impl->CreateOffer(observer, nullptr);

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

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateAnswer()
{
  // HACK: Trigger OnNegotiationNeeded
  this->OnNegotiationNeeded();

  IAsyncOperation<RTCSessionDescription^>^ asyncOp = Concurrency::create_async(
    [this]() -> RTCSessionDescription^ {
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

      rtc::scoped_refptr<CreateSdpObserver> observer(new rtc::RefCountedObject<CreateSdpObserver>());
      // TODO: Remove it once the callback has been received.
      _createSdpObservers.push_back(observer);

      _impl->CreateAnswer(observer, nullptr);

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

IAsyncAction^ RTCPeerConnection::SetLocalDescription(RTCSessionDescription^ description)
{
  IAsyncAction^ asyncOp = Concurrency::create_async(
    [this, description]() -> void {

    rtc::scoped_refptr<SetSdpObserver> observer(new rtc::RefCountedObject<SetSdpObserver>());
    // TODO: Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    _impl->SetLocalDescription(observer, description->GetImpl());

    observer->Wait(
      []() -> void
      {
      },
      [](const std::string& err) -> void
      {
      // TODO: Throw?
      });
  });

  return asyncOp;
}

IAsyncAction^ RTCPeerConnection::SetRemoteDescription(RTCSessionDescription^ description)
{
  IAsyncAction^ asyncOp = Concurrency::create_async(
    [this, description]() -> void {

    rtc::scoped_refptr<SetSdpObserver> observer(new rtc::RefCountedObject<SetSdpObserver>());
    // TODO: Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    _impl->SetRemoteDescription(observer, description->GetImpl());

    observer->Wait(
      []() -> void
      {
      },
      [](const std::string& err) -> void
      {
      // TODO: Throw?
      });
  });

  return asyncOp;
}



void WebRTC::Initialize()
{
  rtc::EnsureWinsockInit();
  // Create a worker thread
  gThread.Start();
  rtc::ThreadManager::Instance()->SetCurrentThread(&gThread);
  rtc::InitializeSSL();

  webrtc::test::InitFieldTrialsFromString("");

  gPeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
}

