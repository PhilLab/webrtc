
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/PeerConnectionInterface.h"

#include <ppltasks.h>
#include <map>
#include <string>
#include <functional>
#include <codecvt>

#include "GlobalObserver.h"
#include "Marshalling.h"
#include "DataChannel.h"
#include "Media.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/event_tracer.h"
#include "webrtc/base/loggingserver.h"
#include "webrtc/base/tracelog.h"
#include "webrtc/test/field_trial.h"
#include "talk/app/webrtc/test/fakeconstraints.h"


using webrtc_winrt_api_internal::FromCx;
using webrtc_winrt_api_internal::ToCx;
using Platform::Collections::Vector;

Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;
Platform::Agile<Windows::Media::Capture::MediaCapture> capture_manager = nullptr;

// Any globals we need to keep around.
namespace webrtc_winrt_api {
namespace globals {
rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
  gPeerConnectionFactory;
  // The worker thread for webrtc.
  rtc::Thread gThread;
  rtc::TraceLog gTraceLog;
  rtc::scoped_ptr<rtc::LoggingServer> gLoggingServer;
}  // namespace globals

RTCIceCandidate::RTCIceCandidate() {
}

RTCIceCandidate::RTCIceCandidate(
  String^ candidate, String^ sdpMid, uint16 sdpMLineIndex) {
  Candidate = candidate;
  SdpMid = sdpMid;
  SdpMLineIndex = sdpMLineIndex;
}

RTCSessionDescription::RTCSessionDescription() {
}

RTCSessionDescription::RTCSessionDescription(RTCSdpType type, String^ sdp) {
  Type = type;
  Sdp = sdp;
}

RTCPeerConnection::RTCPeerConnection(RTCConfiguration^ configuration) {
  webrtc::PeerConnectionInterface::RTCConfiguration cc_configuration;
  FromCx(configuration, &cc_configuration);

  webrtc::FakeConstraints constraints;
  constraints.SetAllowDtlsSctpDataChannels();
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
  std::function<T2(T1)> onCallback) {
  Concurrency::task_completion_event<T1> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init] {
    globals::RunOnGlobalThread<void>([tce, init] {
      init(tce);
    });
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<T1>(tce)
    .then([onCallback](T1 arg) {
    // Then calls the callback with the return value.
    return onCallback(arg);
  });

  // Return an async operation that waits on the return value
  // of the callback and returns it.
  return Concurrency::create_async([tceTask] {
    return tceTask.get();
  });
}

// Specialized version for void callbacks.
IAsyncAction^ CreateCallbackBridge(
  std::function<void(Concurrency::task_completion_event<void>)> init) {
  Concurrency::task_completion_event<void> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init] {
    init(tce);
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<void>(tce);

  // Return an async operation that waits on the
  // task completetion event.
  return Concurrency::create_async([tceTask] {
    return tceTask.get();
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateOffer() {
  return CreateCallbackBridge
    <webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
      [this](Concurrency::task_completion_event
        <webrtc::SessionDescriptionInterface*> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<CreateSdpObserver> observer(
      new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateOffer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi) {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateAnswer() {
  return CreateCallbackBridge
    <webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
      [this](Concurrency::task_completion_event
      <webrtc::SessionDescriptionInterface*> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<CreateSdpObserver> observer(
      new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateAnswer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi) {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncAction^ RTCPeerConnection::SetLocalDescription(
  RTCSessionDescription^ description) {
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<SetSdpObserver> observer(
      new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    rtc::scoped_ptr<webrtc::SessionDescriptionInterface> nativeDescription;
    FromCx(description, &nativeDescription);

    _impl->SetLocalDescription(observer, nativeDescription.release());
  });
}

IAsyncAction^ RTCPeerConnection::SetRemoteDescription(
  RTCSessionDescription^ description) {
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    rtc::scoped_refptr<SetSdpObserver> observer(
      new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    rtc::scoped_ptr<webrtc::SessionDescriptionInterface> nativeDescription;
    FromCx(description, &nativeDescription);

    _impl->SetRemoteDescription(observer, nativeDescription.release());
  });
}

RTCConfiguration^ RTCPeerConnection::GetConfiguration() {
  RTCConfiguration^ ret = nullptr;
  globals::RunOnGlobalThread<void>([this, ret] {
    // TODO(WINRT): Figure out how to rebuild a configuration
    // object.  There doesn't seem to be a simple function
    // on the webrtc::PeerConnectionInterface.
  });
  return ret;
}

IVector<MediaStream^>^ RTCPeerConnection::GetLocalStreams() {
  auto ret = ref new Vector<MediaStream^>();
  globals::RunOnGlobalThread<void>([this, ret] {
    auto streams = _impl->local_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      ret->Append(ref new MediaStream(streams->at(i)));
    }
  });
  return ret;
}

IVector<MediaStream^>^ RTCPeerConnection::GetRemoteStreams() {
  auto ret = ref new Vector<MediaStream^>();
  globals::RunOnGlobalThread<void>([this, ret] {
    auto streams = _impl->remote_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      ret->Append(ref new MediaStream(streams->at(i)));
    }
  });
  return ret;
}

MediaStream^ RTCPeerConnection::GetStreamById(String^ streamId) {
  MediaStream^ ret = nullptr;
  globals::RunOnGlobalThread<void>([this, streamId, &ret] {
    std::string streamIdStr = FromCx(streamId);
    // Look through the local streams.
    auto streams = _impl->local_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      auto stream = streams->at(i);
      // TODO(WINRT): Is the label the stream id?
      if (stream->label() == streamIdStr) {
        ret = ref new MediaStream(stream);
        return;
      }
    }
    // Look through the remote streams.
    streams = _impl->remote_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      auto stream = streams->at(i);
      // TODO(WINRT): Is the label the stream id?
      if (stream->label() == streamIdStr) {
        ret = ref new MediaStream(stream);
        return;
      }
    }
  });
  return ret;
}

void RTCPeerConnection::AddStream(MediaStream^ stream) {
  globals::RunOnGlobalThread<void>([this, stream] {
    _impl->AddStream(stream->GetImpl());
  });
}

void RTCPeerConnection::RemoveStream(MediaStream^ stream) {
  globals::RunOnGlobalThread<void>([this, stream] {
    _impl->RemoveStream(stream->GetImpl());
  });
}

RTCDataChannel^ RTCPeerConnection::CreateDataChannel(
  String^ label, RTCDataChannelInit^ init) {
  webrtc::DataChannelInit nativeInit;
  if (init != nullptr) {
    FromCx(init, &nativeInit);
  }

  auto channel = _impl->CreateDataChannel(
    FromCx(label), init != nullptr ? &nativeInit : nullptr);
  auto ret = ref new RTCDataChannel(channel);

  auto observer = new webrtc_winrt_api_internal::DataChannelObserver(ret);
  // TODO(WINRT): Figure out when to remove this observer.
  _dataChannelObservers.PushBack(observer);
  channel->RegisterObserver(observer);
  return ret;
}

IAsyncAction^ RTCPeerConnection::AddIceCandidate(RTCIceCandidate^ candidate) {
  return Concurrency::create_async([this, candidate] {
    rtc::scoped_ptr<webrtc::IceCandidateInterface> nativeCandidate;
    FromCx(candidate, &nativeCandidate);

    _impl->AddIceCandidate(nativeCandidate.get());
  });
}

void RTCPeerConnection::Close() {
  globals::RunOnGlobalThread<void>([this] {
    _impl->Close();
  });
}

RTCSessionDescription^ RTCPeerConnection::LocalDescription::get() {
  RTCSessionDescription^ ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    if (_impl->local_description() != nullptr) {
      ToCx(_impl->local_description(), &ret);
    }
  });
  return ret;
}

RTCSessionDescription^ RTCPeerConnection::RemoteDescription::get() {
  RTCSessionDescription^ ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    if (_impl->remote_description() != nullptr) {
      ToCx(_impl->remote_description(), &ret);
    }
  });
  return ret;
}

RTCSignalingState RTCPeerConnection::SignalingState::get() {
  RTCSignalingState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    ToCx(_impl->signaling_state(), &ret);
  });
  return ret;
}

RTCIceGatheringState RTCPeerConnection::IceGatheringState::get() {
  RTCIceGatheringState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    ToCx(_impl->ice_gathering_state(), &ret);
  });
  return ret;
}

RTCIceConnectionState RTCPeerConnection::IceConnectionState::get() {
  RTCIceConnectionState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    ToCx(_impl->ice_connection_state(), &ret);
  });
  return ret;
}
/*
IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection^>^ WebRTC::GetAllDeviceInfo(){
  return Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(Windows::Devices::Enumeration::DeviceClass::VideoCapture);
}
*/
Windows::Foundation::IAsyncAction^  WebRTC::InitializeMediaEngine(){

  capture_manager = ref new  Windows::Media::Capture::MediaCapture();

  Windows::Media::Capture::MediaCaptureInitializationSettings^ mediaSettings = ref new  Windows::Media::Capture::MediaCaptureInitializationSettings();

  mediaSettings->AudioDeviceId = "";
  mediaSettings->VideoDeviceId = "";
  mediaSettings->StreamingCaptureMode = Windows::Media::Capture::StreamingCaptureMode::AudioAndVideo;

  mediaSettings->PhotoCaptureSource = Windows::Media::Capture::PhotoCaptureSource::VideoPreview;
  /*
  IAsyncOperation<Windows::Devices::Enumeration::DeviceInformationCollection^>^ devicesOp = Windows::Devices::Enumeration::DeviceInformation::FindAllAsync(Windows::Devices::Enumeration::DeviceClass::VideoCapture);
  auto aTask = Concurrency::create_task(devicesOp);
  aTask.then([mediaSettings](Windows::Devices::Enumeration::DeviceInformationCollection^ devices){

    mediaSettings->VideoDeviceId = devices->GetAt(0)->Id;
    mediaSettings->StreamingCaptureMode = Windows::Media::Capture::StreamingCaptureMode::AudioAndVideo;

    mediaSettings->PhotoCaptureSource = Windows::Media::Capture::PhotoCaptureSource::VideoPreview;


  }, Concurrency::task_continuation_context::use_current()).wait();*/
  
  return capture_manager->InitializeAsync(mediaSettings);
}

void WebRTC::Initialize(Windows::UI::Core::CoreDispatcher^ dispatcher) {
  g_windowDispatcher = dispatcher;

  // Create a worker thread
  globals::gThread.Start();

  globals::RunOnGlobalThread<void>([] {
    rtc::EnsureWinsockInit();
    rtc::InitializeSSL();

    globals::gPeerConnectionFactory = webrtc::CreatePeerConnectionFactory();
    webrtc::SetupEventTracer(&WebRTC::GetCategoryGroupEnabled,
      &WebRTC::AddTraceEvent);
  });
}

bool WebRTC::IsTracing() {
  return globals::gTraceLog.IsTracing();
}

void WebRTC::StartTracing() {
  globals::gTraceLog.StartTracing();
}

void WebRTC::StopTracing() {
  globals::gTraceLog.StopTracing();
}

bool WebRTC::SaveTrace(Platform::String^ filename) {
  std::string filenameStr = FromCx(filename);
  return globals::gTraceLog.Save(filenameStr);
}

bool WebRTC::SaveTrace(Platform::String^ host, int port) {
  std::string hostStr = FromCx(host);
  return globals::gTraceLog.Save(hostStr, port);
}

void WebRTC::EnableLogging(LogLevel level) {
  rtc::SocketAddress sa(INADDR_ANY, 47003);
  globals::gLoggingServer = rtc::scoped_ptr<rtc::LoggingServer>(
    new rtc::LoggingServer());
  globals::gLoggingServer->Listen(sa, static_cast<int>(level));
  LOG(LS_INFO) << "WebRTC logging enabled";
}

void WebRTC::DisableLogging() {
  LOG(LS_INFO) << "WebRTC logging disabled";
  globals::gLoggingServer.reset();
}

const unsigned char* /*__cdecl*/ WebRTC::GetCategoryGroupEnabled(
  const char* category_group) {
  return reinterpret_cast<const unsigned char*>("webrtc");
}

void __cdecl WebRTC::AddTraceEvent(char phase,
  const unsigned char* category_group_enabled,
  const char* name,
  unsigned long long id,
  int num_args,
  const char** arg_names,
  const unsigned char* arg_types,
  const unsigned long long* arg_values,
  unsigned char flags) {
  globals::gTraceLog.Add(phase, category_group_enabled, name, id,
    num_args, arg_names, arg_types, arg_values, flags);
}

}  // namespace webrtc_winrt_api
