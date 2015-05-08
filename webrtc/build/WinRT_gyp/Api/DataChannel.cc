#include "DataChannel.h"
#include "Marshalling.h"

using namespace webrtc_winrt_api;
using namespace webrtc_winrt_api_internal;
using namespace Platform;


RTCDataChannel::RTCDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> impl)
  : _impl(impl)
{

}

rtc::scoped_refptr<webrtc::DataChannelInterface> RTCDataChannel::GetImpl()
{
  return _impl;
}

String^ RTCDataChannel::Label::get()
{
  return ToCx(_impl->label());
}

bool RTCDataChannel::Ordered::get()
{
  return _impl->ordered();
}

IBox<unsigned short>^ RTCDataChannel::MaxPacketLifeTime::get()
{
  return _impl->maxRetransmitTime() != -1
    ? _impl->maxRetransmitTime()
    : (IBox<unsigned short>^)nullptr;
}

IBox<unsigned short>^ RTCDataChannel::MaxRetransmits::get()
{
  return _impl->maxRetransmits() != -1
    ? _impl->maxRetransmits()
    : (IBox<unsigned short>^)nullptr;
}

String^ RTCDataChannel::Protocol::get()
{
  return ToCx(_impl->protocol());
}

bool RTCDataChannel::Negotiated::get()
{
  return _impl->negotiated();
}

unsigned short RTCDataChannel::Id::get()
{
  return _impl->id();

}

RTCDataChannelState RTCDataChannel::ReadyState::get()
{
  RTCDataChannelState state;
  ToCx(_impl->state(), state);
  return state;
}

unsigned int RTCDataChannel::BufferedAmount::get()
{
  return _impl->buffered_amount();

}

void RTCDataChannel::Send(String^ data)
{
  webrtc::DataBuffer buffer(rtc::ToUtf8(data->Data()));
  _impl->Send(buffer);
}

