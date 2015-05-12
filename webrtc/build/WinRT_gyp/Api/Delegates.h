#pragma once

namespace webrtc_winrt_api
{
  public delegate void EventDelegate();

  // ------------------
  ref class RTCPeerConnectionIceEvent;
  public delegate void RTCPeerConnectionIceEventDelegate(RTCPeerConnectionIceEvent^);

  // ------------------
  ref class MediaStreamEvent;
  public delegate void MediaStreamEventEventDelegate(MediaStreamEvent^);

  ref class RTCDataChannelMessageEvent;
  public delegate void RTCDataChannelMessageEventDelegate(RTCDataChannelMessageEvent^);

}
