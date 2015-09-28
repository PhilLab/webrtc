
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_API_DELEGATES_H_
#define WEBRTC_BUILD_WINRT_GYP_API_DELEGATES_H_

namespace webrtc_winrt_api {
public delegate void EventDelegate();

// ------------------
ref class RTCPeerConnectionIceEvent;
public delegate void RTCPeerConnectionIceEventDelegate(
  RTCPeerConnectionIceEvent^);

// ------------------
ref class RTCPeerConnectionIceStateChangeEvent;
public delegate void RTCPeerConnectionIceStateChangeEventDelegate(
  RTCPeerConnectionIceStateChangeEvent^);

// ------------------
ref class MediaStreamEvent;
public delegate void MediaStreamEventEventDelegate(
  MediaStreamEvent^);

ref class RTCDataChannelMessageEvent;
public delegate void RTCDataChannelMessageEventDelegate(
  RTCDataChannelMessageEvent^);

ref class MediaDevice;
public delegate void OnMediaCaptureDeviceFoundDelegate(MediaDevice^);

}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_DELEGATES_H_

