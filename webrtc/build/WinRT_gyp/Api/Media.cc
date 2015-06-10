﻿
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/Media.h"
#include <ppltasks.h>
#include <mfapi.h>
#include <vector>
#include <string>
#include "PeerConnectionInterface.h"
#include "Marshalling.h"
#include "RTMediaStreamSource.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/videosourceinterface.h"

using Platform::Collections::Vector;
using webrtc_winrt_api_internal::ToCx;
using webrtc_winrt_api_internal::FromCx;

namespace webrtc_winrt_api {

// = MediaVideoTrack =========================================================

MediaVideoTrack::MediaVideoTrack(
  rtc::scoped_refptr<webrtc::VideoTrackInterface> impl) :
  _impl(impl) {
}

MediaVideoTrack::~MediaVideoTrack() {
}

String^ MediaVideoTrack::Kind::get() {
  return ToCx(_impl->kind());
}

String^ MediaVideoTrack::Id::get() {
  return ToCx(_impl->id());
}

bool MediaVideoTrack::Enabled::get() {
  return _impl->enabled();
}

void MediaVideoTrack::Enabled::set(bool value) {
  _impl->set_enabled(value);
}

void MediaVideoTrack::SetRenderer(webrtc::VideoRendererInterface* renderer) {
  _impl->AddRenderer(renderer);
}

void MediaVideoTrack::UnsetRenderer(webrtc::VideoRendererInterface* renderer) {
  _impl->RemoveRenderer(renderer);
}

// = MediaAudioTrack =========================================================

MediaAudioTrack::MediaAudioTrack(
  rtc::scoped_refptr<webrtc::AudioTrackInterface> impl) :
  _impl(impl) {
}

String^ MediaAudioTrack::Kind::get() {
  return ToCx(_impl->kind());
}

String^ MediaAudioTrack::Id::get() {
  return ToCx(_impl->id());
}

bool MediaAudioTrack::Enabled::get() {
  return _impl->enabled();
}

void MediaAudioTrack::Enabled::set(bool value) {
  _impl->set_enabled(value);
}

// = MediaStream =============================================================

MediaStream::MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl)
  : _impl(impl) {
}

rtc::scoped_refptr<webrtc::MediaStreamInterface> MediaStream::GetImpl() {
  return _impl;
}

IVector<MediaAudioTrack^>^ MediaStream::GetAudioTracks() {
  auto ret = ref new Vector<MediaAudioTrack^>();
  for (auto track : _impl->GetAudioTracks()) {
    ret->Append(ref new MediaAudioTrack(track));
  }
  return ret;
}

IVector<MediaVideoTrack^>^ MediaStream::GetVideoTracks() {
  auto ret = ref new Vector<MediaVideoTrack^>();
  for (auto track : _impl->GetVideoTracks()) {
    ret->Append(ref new MediaVideoTrack(track));
  }
  return ret;
}

IVector<IMediaStreamTrack^>^ MediaStream::GetTracks() {
  auto ret = ref new Vector<IMediaStreamTrack^>();
  for (auto track : _impl->GetAudioTracks()) {
    ret->Append(ref new MediaAudioTrack(track));
  }
  for (auto track : _impl->GetVideoTracks()) {
    ret->Append(ref new MediaVideoTrack(track));
  }
  return ret;
}

IMediaStreamTrack^ MediaStream::GetTrackById(String^ trackId) {
  IMediaStreamTrack^ ret = nullptr;
  std::string trackIdStr = FromCx(trackId);
  // Search the audio tracks.
  auto audioTrack = _impl->FindAudioTrack(trackIdStr);
  if (audioTrack != nullptr) {
    ret = ref new MediaAudioTrack(audioTrack);
  } else {
    // Search the video tracks.
    auto videoTrack = _impl->FindVideoTrack(trackIdStr);
    if (videoTrack != nullptr) {
      ret = ref new MediaVideoTrack(videoTrack);
    }
  }
  return ret;
}

void MediaStream::AddTrack(IMediaStreamTrack^ track) {
  std::string kind = FromCx(track->Kind);
  if (kind == "audio") {
    auto audioTrack = static_cast<MediaAudioTrack^>(track);
    _impl->AddTrack(audioTrack->GetImpl());
  } else if (kind == "video") {
    auto videoTrack = static_cast<MediaVideoTrack^>(track);
    _impl->AddTrack(videoTrack->GetImpl());
  } else {
    throw "Unknown track kind";
  }
}

void MediaStream::RemoveTrack(IMediaStreamTrack^ track) {
  std::string kind = FromCx(track->Kind);
  if (kind == "audio") {
    auto audioTrack = static_cast<MediaAudioTrack^>(track);
    _impl->RemoveTrack(audioTrack->GetImpl());
  } else if (kind == "video") {
    auto videoTrack = static_cast<MediaVideoTrack^>(track);
    _impl->RemoveTrack(videoTrack->GetImpl());
  } else {
    throw "Unknown track kind";
  }
}

bool MediaStream::Active::get() {
  bool ret = false;
  for (auto track : _impl->GetAudioTracks()) {
    if (track->state() < webrtc::MediaStreamTrackInterface::kEnded) {
      ret = true;
    }
  }
  for (auto track : _impl->GetVideoTracks()) {
    if (track->state() < webrtc::MediaStreamTrackInterface::kEnded) {
      ret = true;
    }
  }
  return ret;
}

// = Media ===================================================================

const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

IAsyncOperation<MediaStream^>^ Media::GetUserMedia() {
  IAsyncOperation<MediaStream^>^ asyncOp = Concurrency::create_async(
    [this]() -> MediaStream^ {
    // TODO(WINRT): Check if a stream already exists.  Create only once.

    return globals::RunOnGlobalThread<MediaStream^>([this]()->MediaStream^ {
      rtc::scoped_ptr<cricket::DeviceManagerInterface> dev_manager(
        cricket::DeviceManagerFactory::Create());

      if (!dev_manager->Init()) {
        LOG(LS_ERROR) << "Can't create device manager";
        return nullptr;
      }

      std::vector<cricket::Device> devs;
      if (!dev_manager->GetVideoCaptureDevices(&devs)) {
        LOG(LS_ERROR) << "Can't enumerate video devices";
        return nullptr;
      }

      // Select the first video device as the capturer.
      cricket::VideoCapturer* videoCapturer = NULL;
      for (auto videoDev : devs) {
        videoCapturer = dev_manager->CreateVideoCapturer(videoDev);
        if (videoCapturer != NULL)
          break;
      }

      // This is the stream returned.
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
        globals::gPeerConnectionFactory->CreateLocalMediaStream(kStreamLabel);

      // Add an audio track.
      rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
        globals::gPeerConnectionFactory->CreateAudioTrack(
        kAudioLabel, globals::gPeerConnectionFactory->CreateAudioSource(NULL)));
      stream->AddTrack(audio_track);

      // Add a video track
      if (videoCapturer != nullptr) {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
          globals::gPeerConnectionFactory->CreateVideoTrack(
          kVideoLabel,
          globals::gPeerConnectionFactory->CreateVideoSource(
            videoCapturer, NULL)));
        stream->AddTrack(video_track);
      }

      auto ret = ref new MediaStream(stream);
      return ret;
    });
  });

  return asyncOp;
}

IMediaSource^ Media::CreateMediaStreamSource(
  MediaVideoTrack^ track, uint32 width, uint32 height, uint32 framerate) {
  return webrtc_winrt_api_internal::RTMediaStreamSource::CreateMediaSource(
    track, width, height, framerate);
}

}  // namespace webrtc_winrt_api