#include "peerconnectioninterface.h"
#include "GlobalObserver.h"
#include "Marshalling.h"
#include "DataChannel.h"
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
  constraints.SetAllowRtpDataChannels();
  _observer.SetPeerConnection(this);


  _impl = globals::gPeerConnectionFactory->CreatePeerConnection(
    cc_configuration, &constraints, nullptr, nullptr, &_observer);
}

// Utility function to create an async operation
// which wraps a callback based async function.
// Use std::tuple<> for callbacks with more than one argument.
// Different types T1 and T2 where additional processing
// needs to be done in the callback.
template <typename T1, typename T2>
IAsyncOperation<T2>^ CreateCallbackBridge(
  std::function<void(Concurrency::task_completion_event<T1>)> init,
  std::function<T2(T1)> onCallback)
{
  Concurrency::task_completion_event<T1> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init]
  {
    init(tce);
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<T1>(tce)
    .then([onCallback](T1 arg)
  {
    // Then calls the callback with the return value.
    return onCallback(arg);
  });

  // Return an async operation that waits on the return value
  // of the callback and returns it.
  return Concurrency::create_async([tceTask]
  {
    return tceTask.get();
  });
}

// Specialized version for void callbacks.
IAsyncAction^ CreateCallbackBridge(
  std::function<void(Concurrency::task_completion_event<void>)> init)
{
  Concurrency::task_completion_event<void> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init]
  {
    init(tce);
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<void>(tce);

  // Return an async operation that waits on the
  // task completetion event.
  return Concurrency::create_async([tceTask]
  {
    return tceTask.get();
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateOffer()
{
  return CreateCallbackBridge<webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
    [this](Concurrency::task_completion_event<webrtc::SessionDescriptionInterface*> tce)
  {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<CreateSdpObserver> observer(new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO: Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateOffer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi)
  {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateAnswer()
{
  return CreateCallbackBridge<webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
    [this](Concurrency::task_completion_event<webrtc::SessionDescriptionInterface*> tce)
  {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<CreateSdpObserver> observer(new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO: Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateAnswer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi)
  {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncAction^ RTCPeerConnection::SetLocalDescription(RTCSessionDescription^ description)
{
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce)
  {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<SetSdpObserver> observer(new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO: Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    _impl->SetLocalDescription(observer, description->GetImpl());
  });
}

IAsyncAction^ RTCPeerConnection::SetRemoteDescription(RTCSessionDescription^ description)
{
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce)
  {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<SetSdpObserver> observer(new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO: Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    _impl->SetRemoteDescription(observer, description->GetImpl());
  });
}

void RTCPeerConnection::AddStream(MediaStream^ stream)
{
  _impl->AddStream(stream->GetImpl());
}

RTCDataChannel^ RTCPeerConnection::CreateDataChannel(String^ label, RTCDataChannelInit^ init)
{
  webrtc::DataChannelInit nativeInit;
  if (init != nullptr)
  {
    FromCx(init, nativeInit);
  }

  auto channel = _impl->CreateDataChannel(FromCx(label), init != nullptr ? &nativeInit : nullptr);
  auto ret = ref new RTCDataChannel(channel);

  auto observer = new webrtc_winrt_api_internal::DataChannelObserver(ret);
  // TODO: Figure out when to remove this observer.
  _dataChannelObservers.PushBack(observer);
  channel->RegisterObserver(observer);
  return ret;
}

RTCSessionDescription^ RTCPeerConnection::LocalDescription::get()
{
  // HACK: Temporary, cast the const away.
  //       Need to figure out a way to distinguish between a
  //       RTCSessionDescription^ containing a const and not.
  return ref new RTCSessionDescription(
    const_cast<webrtc::SessionDescriptionInterface*>(_impl->local_description()));
}

RTCSignalingState RTCPeerConnection::SignalingState::get()
{
  RTCSignalingState ret;
  ToCx(_impl->signaling_state(), ret);
  return ret;
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

