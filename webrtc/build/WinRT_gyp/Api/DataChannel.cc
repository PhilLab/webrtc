
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/DataChannel.h"
#include "Marshalling.h"

using webrtc_winrt_api_internal::ToCx;

namespace webrtc_winrt_api {

RTCDataChannel::RTCDataChannel(
  rtc::scoped_refptr<webrtc::DataChannelInterface> impl)
  : _impl(impl) {
}

rtc::scoped_refptr<webrtc::DataChannelInterface> RTCDataChannel::GetImpl() {
  return _impl;
}

String^ RTCDataChannel::Label::get() {
  return ToCx(_impl->label());
}

bool RTCDataChannel::Ordered::get() {
  return _impl->ordered();
}

IBox<uint16>^ RTCDataChannel::MaxPacketLifeTime::get() {
  return _impl->maxRetransmitTime() != -1
    ? _impl->maxRetransmitTime()
    : (IBox<uint16>^)nullptr;
}

IBox<uint16>^ RTCDataChannel::MaxRetransmits::get() {
  return _impl->maxRetransmits() != -1
    ? _impl->maxRetransmits()
    : (IBox<uint16>^)nullptr;
}

String^ RTCDataChannel::Protocol::get() {
  return ToCx(_impl->protocol());
}

bool RTCDataChannel::Negotiated::get() {
  return _impl->negotiated();
}

uint16 RTCDataChannel::Id::get() {
  return _impl->id();
}

void RTCDataChannel::Close() {
  _impl->Close();
}

RTCDataChannelState RTCDataChannel::ReadyState::get() {
  RTCDataChannelState state;
  ToCx(_impl->state(), &state);
  return state;
}

unsigned int RTCDataChannel::BufferedAmount::get() {
  return _impl->buffered_amount();
}

void RTCDataChannel::Send(IDataChannelMessage^ data) {
  if (!data->binary) {
    webrtc::DataBuffer buffer(rtc::ToUtf8(data->DataString->Data()));
    _impl->Send(buffer);
  }
  else {
    std::vector<byte> binaryDataVector;
    binaryDataVector.reserve(data->DataBinary->Size);

    // convert IVector from to std::vector
    webrtc_winrt_api_internal::FromCx(data->DataBinary, &binaryDataVector);

    byte* byteArr = (&binaryDataVector[0]);
    const rtc::Buffer rtcBuffer(byteArr, binaryDataVector.size());
    webrtc::DataBuffer buffer(*(&rtcBuffer), true);

    _impl->Send(buffer);
  }
}

}  // namespace webrtc_winrt_api
