
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_API_DATACHANNEL_H_
#define WEBRTC_BUILD_WINRT_GYP_API_DATACHANNEL_H_

#include <collection.h>
#include "GlobalObserver.h"
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "Delegates.h"

using Platform::String;
using Platform::IBox;
using Windows::Foundation::Collections::IVector;

namespace webrtc_winrt_api {
public enum class RTCDataChannelState {
  Connecting,
  Open,
  Closing,
  Closed,
};

public ref class RTCDataChannelInit sealed {
public:
  property IBox<bool>^ Ordered;
  property IBox<uint16>^ MaxPacketLifeTime;
  property IBox<uint16>^ MaxRetransmits;
  property String^ Protocol;
  property IBox<bool>^ Negotiated;
  property IBox<uint16>^ Id;
};

public enum class RTCDataChannelMessageType {
  String,
  Binary
};

public interface class IDataChannelMessage {
  property RTCDataChannelMessageType DataType;
};

public ref class StringDataChannelMessage sealed : IDataChannelMessage {
public:
  StringDataChannelMessage(String^ data);
  property String^ StringData;
  property RTCDataChannelMessageType DataType {
    virtual RTCDataChannelMessageType get() {
        return RTCDataChannelMessageType::String;
    }
    virtual void set(RTCDataChannelMessageType) { }
  };
};

public ref class BinaryDataChannelMessage sealed : IDataChannelMessage {
public:
  BinaryDataChannelMessage(IVector<byte>^ data);
  property IVector<byte>^ BinaryData;
  property RTCDataChannelMessageType DataType {
    virtual RTCDataChannelMessageType get() {
        return RTCDataChannelMessageType::Binary;
    }
    virtual void set(RTCDataChannelMessageType) { }
  };
};

public ref class RTCDataChannelMessageEvent sealed {
public:
  property IDataChannelMessage^ Data;
};

public ref class RTCDataChannel sealed {
internal:
  RTCDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> impl);
  rtc::scoped_refptr<webrtc::DataChannelInterface> GetImpl();
  friend class webrtc_winrt_api_internal::DataChannelObserver;

public:
  property String^ Label { String^ get(); }
  property bool Ordered { bool get(); }
  property IBox<uint16>^ MaxPacketLifeTime { IBox<uint16>^ get(); }
  property IBox<uint16>^ MaxRetransmits { IBox<uint16>^ get(); }
  property String^ Protocol { String^ get(); }
  property bool Negotiated { bool get(); }
  property uint16 Id { uint16 get(); }
  property RTCDataChannelState ReadyState { RTCDataChannelState get(); }
  property unsigned int BufferedAmount { unsigned int get(); }

  event RTCDataChannelMessageEventDelegate^ OnMessage;
  event EventDelegate^ OnOpen;
  event EventDelegate^ OnClose;
  // TODO(WINRT): Figure out how OnError is received.
  event EventDelegate^ OnError;

  void Close();
  void Send(IDataChannelMessage^ data);

private:
  rtc::scoped_refptr<webrtc::DataChannelInterface> _impl;
};

public ref class RTCDataChannelEvent sealed {
public:
  property RTCDataChannel^ Channel;
};
public delegate void RTCDataChannelEventDelegate(RTCDataChannelEvent^);

}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_DATACHANNEL_H_

