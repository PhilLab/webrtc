#include "peerconnectioninterface.h"
#include "GlobalObserver.h"
#include "Marshalling.h"
#include "Media.h"

#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/bind.h"
#include "webrtc/test/field_trial.h"
#include "talk/app/webrtc/test/fakeconstraints.h"

#include <map>
#include <functional>
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
namespace webrtc_winrt_api {
  namespace globals {
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> gPeerConnectionFactory;
    // The worker thread for webrtc.
    rtc::Thread gThread;
  }
}

RTCPeerConnection::RTCPeerConnection(RTCConfiguration^ configuration)
{
  webrtc::PeerConnectionInterface::RTCConfiguration cc_configuration;
  FromCx(configuration, cc_configuration);

  webrtc::FakeConstraints constraints;
  _observer.SetPeerConnection(this);


  _impl = globals::gPeerConnectionFactory->CreatePeerConnection(
    cc_configuration, &constraints, nullptr, nullptr, &_observer);
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateOffer()
{
  IAsyncOperation<RTCSessionDescription^>^ asyncOp = Concurrency::create_async(
    // Lambda that does the work of create offer and
    // returns with a session description.
    [this]() -> RTCSessionDescription^ {
      webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

      rtc::scoped_refptr<CreateSdpObserver> observer(new rtc::RefCountedObject<CreateSdpObserver>());
      // TODO: Remove it once the callback has been received.
      _createSdpObservers.push_back(observer);

      _impl->CreateOffer(observer, nullptr);

      RTCSessionDescription^ ret = nullptr;

      // Synchronous wait for the callback to be invoked.
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

void RTCPeerConnection::AddStream(MediaStream^ stream)
{
  _impl->AddStream(stream->GetImpl());
}

void WebRTC::Initialize(Windows::UI::Core::CoreDispatcher^ dispatcher)
{
  g_windowDispatcher = dispatcher;

  // Create a worker thread
  globals::gThread.Start();

  globals::RunOnGlobalThread<void>([]() -> void
  {
    rtc::EnsureWinsockInit();
    //rtc::ThreadManager::Instance()->SetCurrentThread(&globals::gThread);
    rtc::InitializeSSL();

    webrtc::test::InitFieldTrialsFromString("");

    globals::gPeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
  });
}

