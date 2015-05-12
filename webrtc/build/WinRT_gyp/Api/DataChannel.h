#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "Delegates.h"

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;

namespace webrtc_winrt_api_internal
{
  class DataChannelObserver;
}

namespace webrtc_winrt_api
{
  public enum class RTCDataChannelState
  {
    connecting,
    open,
    closing,
    closed,
  };

  public ref class RTCDataChannelInit sealed
  {
  public:
    property IBox<bool>^ Ordered;
    property IBox<unsigned short>^ MaxPacketLifeTime;
    property IBox<unsigned short>^ MaxRetransmits;
    property String^ Protocol;
    property IBox<bool>^ Negotiated;
    property IBox<unsigned short>^ Id;
  };

  public ref class RTCDataChannelMessageEvent sealed
  {
  public:
    property String^ Data;
  };

  public ref class RTCDataChannel sealed
  {
  internal:
    RTCDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> impl);
    rtc::scoped_refptr<webrtc::DataChannelInterface> GetImpl();
    friend class webrtc_winrt_api_internal::DataChannelObserver;

  public:
    property String^ Label { String^ get(); }
    property bool Ordered { bool get(); }
    property IBox<unsigned short>^ MaxPacketLifeTime { IBox<unsigned short>^ get(); }
    property IBox<unsigned short>^ MaxRetransmits { IBox<unsigned short>^ get(); }
    property String^ Protocol { String^ get(); }
    property bool Negotiated { bool get(); }
    property unsigned short Id { unsigned short get(); }
    property RTCDataChannelState ReadyState { RTCDataChannelState get(); }
    property unsigned int BufferedAmount { unsigned int get(); }

    event RTCDataChannelMessageEventDelegate^ OnMessage;
    event EventDelegate^ OnOpen;
    event EventDelegate^ OnClose;
    // TODO: Figure out how OnError is received.
    event EventDelegate^ OnError;

    void Send(String^ data);

  private:
    rtc::scoped_refptr<webrtc::DataChannelInterface> _impl;
  };

  public ref class RTCDataChannelEvent sealed
  {
  public:
    property RTCDataChannel^ Channel;
  };
  public delegate void RTCDataChannelEventDelegate(RTCDataChannelEvent^);

}
