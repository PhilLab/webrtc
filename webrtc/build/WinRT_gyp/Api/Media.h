
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_API_MEDIA_H_
#define WEBRTC_BUILD_WINRT_GYP_API_MEDIA_H_

#include <mfidl.h>
#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "talk/app/webrtc/mediaconstraintsinterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "GlobalObserver.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/media/devices/winrtdevicemanager.h"
#include "webrtc/modules/audio_device/include/audio_device.h"
#include "Delegates.h"

using Windows::Foundation::IAsyncOperation;
using Platform::String;
using Windows::Foundation::Collections::IVector;
using Windows::Media::Core::IMediaSource;
using namespace cricket;

namespace webrtc_winrt_api {
  public interface class IMediaStreamTrack {
    property String^ Kind { String^ get(); }
    property String^ Id { String^ get(); }
    property bool Enabled { bool get(); void set(bool value); }
    void Stop();
  };

  public ref class MediaVideoTrack sealed : public IMediaStreamTrack {
  internal:
    MediaVideoTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> impl);
    rtc::scoped_refptr<webrtc::VideoTrackInterface> GetImpl() {
      return _impl;
    }
  public:
    virtual ~MediaVideoTrack();
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
    virtual void Stop();
  internal:
    void SetRenderer(webrtc::VideoRendererInterface* renderer);
    void UnsetRenderer(webrtc::VideoRendererInterface* renderer);
  private:
    rtc::scoped_refptr<webrtc::VideoTrackInterface> _impl;
  };

  public ref class MediaAudioTrack sealed : public IMediaStreamTrack {
  internal:
    MediaAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> impl);
    rtc::scoped_refptr<webrtc::AudioTrackInterface> GetImpl() {
      return _impl;
    }
  public:
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
    virtual void Stop();
  private:
    rtc::scoped_refptr<webrtc::AudioTrackInterface> _impl;
  };

  public ref class MediaStream sealed {
  internal:
    MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl);
    rtc::scoped_refptr<webrtc::MediaStreamInterface> GetImpl();
  public:
    IVector<MediaAudioTrack^>^ GetAudioTracks();
    IVector<MediaVideoTrack^>^ GetVideoTracks();
    IVector<IMediaStreamTrack^>^ GetTracks();
    IMediaStreamTrack^ GetTrackById(String^ trackId);
    void AddTrack(IMediaStreamTrack^ track);
    void RemoveTrack(IMediaStreamTrack^ track);

    property bool Active { bool get(); }
  private:
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _impl;
  };

  public ref class MediaDevice sealed {
  private:
    String^ _id;
    String^ _name;
  public:
    MediaDevice(String^ id, String^ name) {
      _id = id;
      _name = name;
    }
    property String^ Id {
      String^ get(){
        return _id;
      }
      void set(String^ value) {
        _id = value;
      }
    }

    property String^ Name {
      String^ get(){
        return _name;
      }
      void set(String^ value) {
        _name = value;
      }
    }
  };

  public ref class RTCMediaStreamConstraints sealed {
  public:
    property bool audio;
    property bool video;
  };

  public ref class Media sealed {
  public:
    Media();

    // TODO(WINRT): Arguments
    IAsyncOperation<MediaStream^>^ GetUserMedia(RTCMediaStreamConstraints^ constraints);
    IMediaSource^ CreateMediaStreamSource(
      MediaVideoTrack^ track, uint32 framerate);

    IAsyncOperation<bool>^ EnumerateAudioVideoCaptureDevices();
    void SelectVideoDevice(MediaDevice^ device);
    void SelectAudioDevice(MediaDevice^ device);

    event OnMediaCaptureDeviceFoundDelegate^ OnVideoCaptureDeviceFound;
    event OnMediaCaptureDeviceFoundDelegate^ OnAudioCaptureDeviceFound;

  private:
    rtc::scoped_ptr<cricket::DeviceManagerInterface> _dev_manager;
    cricket::Device _selectedVideoDevice;
    webrtc::AudioDeviceModule *_audioDevice;
    uint16_t _selectedAudioDevice;
  };

  class MediaConstraints : public webrtc::MediaConstraintsInterface {
  public:
    MediaConstraints() { }
    virtual ~MediaConstraints() { }

    virtual const Constraints& GetMandatory() const {
      return mandatory_;
    }

    virtual const Constraints& GetOptional() const {
      return optional_;
    }

    template <class T>
    void AddMandatory(const std::string& key, const T& value) {
      mandatory_.push_back(Constraint(key, rtc::ToString<T>(value)));
    }

    template <class T>
    void SetMandatory(const std::string& key, const T& value) {
      std::string value_str;
      if (mandatory_.FindFirst(key, &value_str)) {
        for (Constraints::iterator iter = mandatory_.begin();
          iter != mandatory_.end(); ++iter) {
          if (iter->key == key) {
            mandatory_.erase(iter);
            break;
          }
        }
      }
      AddMandatory(key, value);
    }

    void SetMandatoryReceiveAudio(bool enable) {
      SetMandatory(MediaConstraintsInterface::kOfferToReceiveAudio, enable);
    }

    void SetMandatoryReceiveVideo(bool enable) {
      SetMandatory(MediaConstraintsInterface::kOfferToReceiveVideo, enable);
    }

  private:
    Constraints mandatory_;
    Constraints optional_;
  };

}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_MEDIA_H_
