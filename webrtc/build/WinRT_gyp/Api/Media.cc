
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/Media.h"
#include <stdio.h>
#include <ppltasks.h>
#include <mfapi.h>
#include <vector>
#include <string>
#include <set>
#include "PeerConnectionInterface.h"
#include "Marshalling.h"
#include "RTMediaStreamSource.h"
#include "webrtc/base/logging.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "webrtc/modules/audio_device/audio_device_config.h"
#include "webrtc/modules/audio_device/audio_device_impl.h"
#include "webrtc/modules/audio_device/include/audio_device_defines.h"
#include "webrtc/modules/video_capture/windows/device_info_winrt.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

using Platform::Collections::Vector;
using webrtc_winrt_api_internal::ToCx;
using webrtc_winrt_api_internal::FromCx;
using Windows::Media::Capture::MediaStreamType;

namespace {
  std::vector<cricket::Device> g_videoDevices;
  webrtc::CriticalSectionWrapper& gMediaStreamListLock(
    *webrtc::CriticalSectionWrapper::CreateCriticalSection());
}

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

void MediaVideoTrack::Stop() {
  _impl->GetSource()->Stop();
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

void MediaAudioTrack::Stop() {
}

// = MediaStream =============================================================

MediaStream::MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl)
  : _impl(impl) {
}

MediaStream::~MediaStream() {
  LOG(LS_INFO) << "MediaStream::~MediaStream";
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

String^ MediaStream::Id::get() {
  return ToCx(_impl->label());
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

void MediaStream::Stop() {
  // TODO(winrt): Investigate if this is the proper way
  // to stop the stream. If something else holds
  // a reference, the stream may not stop.
  _impl.release();
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
const char kStreamLabel[] = "stream_label_%x";
// we will append current time (uint32 in Hex, e.g.:
// 8chars to the end to generate a unique string)

Media::Media() {
  _dev_manager = rtc::scoped_ptr<cricket::DeviceManagerInterface>
    (cricket::DeviceManagerFactory::Create());

  if (!_dev_manager->Init()) {
    LOG(LS_ERROR) << "Can't create device manager";
    return;
  }

  globals::RunOnGlobalThread<int>([this]()->int {
      _audioDevice = webrtc::AudioDeviceModuleImpl::Create(555,
          webrtc::AudioDeviceModule::kWindowsWasapiAudio);
      if (_audioDevice == NULL) {
          LOG(LS_ERROR) << "Can't create audio device manager";
          return 0;
      }
      _audioDevice->Init();
      return 0;
  });
}

IAsyncOperation<MediaStream^>^ Media::GetUserMedia(
  RTCMediaStreamConstraints^ mediaStreamConstraints) {
  // add to separate sets of constraints
  IAsyncOperation<MediaStream^>^ asyncOp = Concurrency::create_async(
    [this, mediaStreamConstraints]() -> MediaStream^ {
    // TODO(WINRT): Check if a stream already exists.  Create only once.
    return globals::RunOnGlobalThread<MediaStream^>([this,
                                      mediaStreamConstraints]()->MediaStream^ {
      // This is the stream returned.
      char streamLabel[32];
      _snprintf(streamLabel, sizeof(streamLabel), kStreamLabel, rtc::Time());
      rtc::scoped_refptr<webrtc::MediaStreamInterface> stream =
        globals::gPeerConnectionFactory->CreateLocalMediaStream(streamLabel);

      if (mediaStreamConstraints->audioEnabled) {
        if (_selectedAudioDevice == -1) {
          // select default device if wasn't set
          _audioDevice->SetRecordingDevice(
            webrtc::AudioDeviceModule::kDefaultDevice);
        } else {
          _audioDevice->SetRecordingDevice(_selectedAudioDevice);
        }
        // Add an audio track.
        LOG(LS_INFO) << "Creating audio track.";
        rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track(
          globals::gPeerConnectionFactory->CreateAudioTrack(
            kAudioLabel,
            globals::gPeerConnectionFactory->CreateAudioSource(NULL)));
        LOG(LS_INFO) << "Adding audio track to stream.";
        stream->AddTrack(audio_track);
      }

      if (mediaStreamConstraints->videoEnabled) {
        cricket::VideoCapturer* videoCapturer = NULL;
        if (_selectedVideoDevice.id == "") {
          // Select the first video device as the capturer.
          webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
          for (auto videoDev : g_videoDevices) {
            videoCapturer = _dev_manager->CreateVideoCapturer(videoDev);
            if (videoCapturer != NULL)
              break;
          }
        } else {
          videoCapturer = _dev_manager->CreateVideoCapturer(
                                                        _selectedVideoDevice);
        }

        // Add a video track
        if (videoCapturer != nullptr) {
          LOG(LS_INFO) << "Creating video track.";
          rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
            globals::gPeerConnectionFactory->CreateVideoTrack(
            kVideoLabel,
            globals::gPeerConnectionFactory->CreateVideoSource(
            videoCapturer, NULL)));
          LOG(LS_INFO) << "Adding video track to stream.";
          stream->AddTrack(video_track);
        }
      }

      auto ret = ref new MediaStream(stream);
      return ret;
    });
  });

  return asyncOp;
}

IMediaSource^ Media::CreateMediaStreamSource(
    MediaVideoTrack^ track, uint32 framerate, String^ id) {
    return globals::RunOnGlobalThread<MediaStreamSource^>([track, framerate,
      id]()->MediaStreamSource^{
        return webrtc_winrt_api_internal::RTMediaStreamSource::
          CreateMediaSource(track, framerate, id);
    });
}

IVector<MediaDevice^>^ Media::GetVideoCaptureDevices() {
  auto ret = ref new Vector<MediaDevice^>();
  for (auto videoDev : g_videoDevices) {
    ret->Append(ref new MediaDevice(ToCx(videoDev.id), ToCx(videoDev.name)));
  }
  return ret;
}

IVector<MediaDevice^>^ Media::GetAudioCaptureDevices() {
  auto ret = ref new Vector<MediaDevice^>();
  char name[webrtc::kAdmMaxDeviceNameSize];
  char guid[webrtc::kAdmMaxGuidSize];
  int16_t recordingDeviceCount = _audioDevice->RecordingDevices();
  for (int i = 0; i < recordingDeviceCount; i++) {
    _audioDevice->RecordingDeviceName(i, name, guid);

    ret->Append(ref new MediaDevice(ToCx(guid), ToCx(name)));
  }

  return ret;
}

IAsyncOperation<bool>^ Media::EnumerateAudioVideoCaptureDevices() {
  IAsyncOperation<bool>^ asyncOp = Concurrency::create_async(
    [this]() -> bool {
    std::vector<cricket::Device> videoDevices;
    if (!_dev_manager->GetVideoCaptureDevices(&videoDevices)) {
      LOG(LS_ERROR) << "Can't enumerate video devices";
      return false;
    }
    webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
    g_videoDevices.clear();
    for (auto videoDev : videoDevices) {
      g_videoDevices.push_back(videoDev);
      OnVideoCaptureDeviceFound(ref new MediaDevice(ToCx(videoDev.id),
                                                    ToCx(videoDev.name)));
    }

    char name[webrtc::kAdmMaxDeviceNameSize];
    char guid[webrtc::kAdmMaxGuidSize];

    int16_t recordingDeviceCount = _audioDevice->RecordingDevices();
    for (int i = 0; i < recordingDeviceCount; i++) {
      _audioDevice->RecordingDeviceName(i, name, guid);

      OnAudioCaptureDeviceFound(ref new MediaDevice(ToCx(guid), ToCx(name)));
    }
    return true;
  });
  return asyncOp;
}

void Media::SelectVideoDevice(MediaDevice^ device) {
  std::string id = FromCx(device->Id);
  _selectedVideoDevice.id = "";
  _selectedVideoDevice.name = "";
  webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
  for (auto videoDev : g_videoDevices) {
    if (videoDev.id == id) {
      _selectedVideoDevice = videoDev;
      break;
    }
  }
}

void Media::SelectAudioDevice(MediaDevice^ device) {
  std::string id = FromCx(device->Id);
  _selectedAudioDevice = 0;
  char name[webrtc::kAdmMaxDeviceNameSize];
  char guid[webrtc::kAdmMaxGuidSize];

  int16_t recordingDeviceCount = _audioDevice->RecordingDevices();
  for (int i = 0; i < recordingDeviceCount; i++) {
    _audioDevice->RecordingDeviceName(i, name, guid);
    if (strcmp(guid, id.c_str()) == 0) {
      _selectedAudioDevice = i;
      return;
    }
  }
}

IAsyncOperation<IVector<CaptureCapability^>^>^
  MediaDevice::GetVideoCaptureCapabilities() {
  auto op = concurrency::create_async([this]() -> IVector<CaptureCapability^>^ {
    auto mediaCapture =
      webrtc::videocapturemodule::MediaCaptureDevicesWinRT::Instance()->
      GetMediaCapture(_id);
    if (mediaCapture == nullptr) {
      return nullptr;
    }
    auto streamProperties =
      mediaCapture->VideoDeviceController->GetAvailableMediaStreamProperties(
      MediaStreamType::VideoRecord);
    if (streamProperties == nullptr) {
      return nullptr;
    }
    auto ret = ref new Vector<CaptureCapability^>();
    std::set<std::wstring> descSet;
    for (auto prop : streamProperties) {
      if (prop->Type != L"Video") {
        continue;
      }
      auto videoProp =
        static_cast<Windows::Media::MediaProperties::
          IVideoEncodingProperties^>(prop);
      if ((videoProp->FrameRate == nullptr) ||
        (videoProp->FrameRate->Numerator == 0) ||
        (videoProp->FrameRate->Denominator != 1) ||
        (videoProp->Width == 0) || (videoProp->Height == 0)) {
        continue;
      }
      auto cap = ref new CaptureCapability(videoProp->Width, videoProp->Height,
        videoProp->FrameRate->Numerator, videoProp->PixelAspectRatio);
      if (descSet.find(cap->FullDescription->Data()) == descSet.end()) {
        ret->Append(cap);
        descSet.insert(cap->FullDescription->Data());
      }
    }
    return ret;
  });
  return op;
}

}  // namespace webrtc_winrt_api
