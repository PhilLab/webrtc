
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

// Class1.cpp
#include "webrtc/build/WinRT_gyp/Api/GlobalObserver.h"
#include <ppltasks.h>
#include "PeerConnectionInterface.h"
#include "Marshalling.h"
#include "Media.h"
#include "DataChannel.h"

extern Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

namespace webrtc_winrt_api_internal {

#define POST_EVENT(evt, statement) \
  g_windowDispatcher->RunAsync(\
    Windows::UI::Core::CoreDispatcherPriority::Normal, \
    ref new Windows::UI::Core::DispatchedHandler([this, evt] {\
    statement;\
  }));
#define POST_ACTION(statement) \
  g_windowDispatcher->RunAsync(\
    Windows::UI::Core::CoreDispatcherPriority::Normal, \
    ref new Windows::UI::Core::DispatchedHandler([this] {\
    statement;\
  }));

void GlobalObserver::SetPeerConnection(
  webrtc_winrt_api::RTCPeerConnection^ pc) {
  _pc = pc;
}

// Triggered when the SignalingState changed.
void GlobalObserver::OnSignalingChange(
  webrtc::PeerConnectionInterface::SignalingState new_state)
{
  if (_pc != nullptr)
  {
    POST_ACTION(_pc->OnSignalingStateChange());
  }
}

// Triggered when SignalingState or IceState have changed.
// TODO(bemasc): Remove once callers transition to OnSignalingChange.
void GlobalObserver::OnStateChange(StateType state_changed) {
}

// Triggered when media is received on a new stream from remote peer.
void GlobalObserver::OnAddStream(webrtc::MediaStreamInterface* stream) {
  if (_pc != nullptr) {
    auto evt = ref new webrtc_winrt_api::MediaStreamEvent();
    evt->Stream = ref new webrtc_winrt_api::MediaStream(stream);
    POST_EVENT(evt, _pc->OnAddStream(evt));

  }
}

// Triggered when a remote peer close a stream.
void GlobalObserver::OnRemoveStream(webrtc::MediaStreamInterface* stream) {
  if (_pc != nullptr) {
    auto evt = ref new webrtc_winrt_api::MediaStreamEvent();
    evt->Stream = ref new webrtc_winrt_api::MediaStream(stream);
    POST_EVENT(evt, _pc->OnRemoveStream(evt));
  }
}

// Triggered when a remote peer open a data channel.
void GlobalObserver::OnDataChannel(webrtc::DataChannelInterface* data_channel) {
  if (_pc != nullptr) {
    auto evt = ref new webrtc_winrt_api::RTCDataChannelEvent();
    evt->Channel = ref new webrtc_winrt_api::RTCDataChannel(data_channel);
    // TODO(WINRT): Figure out when this observer can be deleted.
    data_channel->RegisterObserver(new DataChannelObserver(evt->Channel));
    POST_EVENT(evt, _pc->OnDataChannel(evt));
  }
}

// Triggered when renegotiation is needed, for example the ICE has restarted.
void GlobalObserver::OnRenegotiationNeeded()
{
  if (_pc != nullptr)
  {
    POST_ACTION(_pc->OnNegotiationNeeded());
  }
}

// Called any time the IceConnectionState changes
void GlobalObserver::OnIceConnectionChange(
  webrtc::PeerConnectionInterface::IceConnectionState new_state) {
}

// Called any time the IceGatheringState changes
void GlobalObserver::OnIceGatheringChange(
  webrtc::PeerConnectionInterface::IceGatheringState new_state) {
}

// New Ice candidate have been found.
void GlobalObserver::OnIceCandidate(
  const webrtc::IceCandidateInterface* candidate) {
  if (_pc != nullptr && candidate != nullptr) {
    auto evt = ref new webrtc_winrt_api::RTCPeerConnectionIceEvent();
    webrtc_winrt_api::RTCIceCandidate^ cxCandidate;
    ToCx(*candidate, &cxCandidate);
    evt->Candidate = cxCandidate;
    POST_EVENT(evt, _pc->OnIceCandidate(evt));
  }
}

// TODO(bemasc): Remove this once callers transition to OnIceGatheringChange.
// All Ice candidates have been found.
void GlobalObserver::OnIceComplete() {
}

//============================================================================

CreateSdpObserver::CreateSdpObserver(
  Concurrency::task_completion_event<webrtc::SessionDescriptionInterface*> tce)
  : _tce(tce) {
}

void CreateSdpObserver::OnSuccess(webrtc::SessionDescriptionInterface* desc) {
  _tce.set(desc);
}

void CreateSdpObserver::OnFailure(const std::string& error) {
  _tce.set_exception(error);
}

//============================================================================

SetSdpObserver::SetSdpObserver(Concurrency::task_completion_event<void> tce)
  : _tce(tce) {
}

void SetSdpObserver::OnSuccess() {
  _tce.set();
}

void SetSdpObserver::OnFailure(const std::string& error) {
  _tce.set_exception(error);
}

//============================================================================

DataChannelObserver::DataChannelObserver(
  webrtc_winrt_api::RTCDataChannel^ channel)
  : _channel(channel) {
}

void DataChannelObserver::OnStateChange() {
  switch (_channel->GetImpl()->state()) {
  case webrtc::DataChannelInterface::kOpen:
    _channel->OnOpen();
    break;
  case webrtc::DataChannelInterface::kClosed:
    _channel->OnClose();
    break;
  }
}

void DataChannelObserver::OnMessage(const webrtc::DataBuffer& buffer) {
  auto evt = ref new webrtc_winrt_api::RTCDataChannelMessageEvent();
  if (!buffer.binary) {
    evt->Data = ToCx(std::string(buffer.data.data(), buffer.size()));
  }
  else
  {
    // TODO
    evt->Data = "<binary>";
  }
  POST_EVENT(evt, _channel->OnMessage(evt));
}

}  // namespace webrtc_winrt_api_internal

