
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

namespace webrtc_winrt_api {

  /// <summary>
  /// An IMediaStreamTrack object represents media of a single type that
  /// originates from one media source, e.g. video produced by a web camera.
  /// </summary>
  /// <remarks>
  /// http://www.w3.org/TR/mediacapture-streams
  /// </remarks>
  public interface class IMediaStreamTrack {
    /// <summary>
    /// Type of media, "audio" or "video".
    /// </summary>
    property String^ Kind { String^ get(); }

    /// <summary>
    /// Identifier.
    /// </summary>
    property String^ Id { String^ get(); }

    /// <summary>
    /// Controls the enabled state for the track.
    /// </summary>
    property bool Enabled { bool get(); void set(bool value); }

    /// <summary>
    /// TODO(WINRT): check implementations, they may not be
    /// conforming to the spec:
    /// http://www.w3.org/TR/mediacapture-streams/#widl-MediaStreamTrack-stop-void
    /// </summary>
    void Stop();
  };

  /// <summary>
  /// Represents video media that originates from one video source.
  /// </summary>
  /// <seealso cref="IMediaStreamTrack"/>
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

  /// <summary>
  /// Represents audio media that originates from one audio source.
  /// </summary>
  /// <seealso cref="IMediaStreamTrack"/>
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

  /// <summary>
  /// A MediaStream is used to group several <see cref="IMediaStreamTrack"/>
  /// objects into one unit that can be recorded or rendered in a media
  /// element. Each MediaStream can contain zero or more
  ///  <see cref="IMediaStreamTrack"/> objects.
  /// </summary>
  /// <remarks>
  /// http://www.w3.org/TR/mediacapture-streams/
  /// </remarks>
  public ref class MediaStream sealed {
  internal:

    /// <summary>
    /// Composes a new stream.
    /// </summary>
    /// <param name="impl"></param>
    MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl);

    rtc::scoped_refptr<webrtc::MediaStreamInterface> GetImpl();
  public:
    /// <summary>
    /// Returns a Vector that represents a snapshot of all the
    ///  <see cref="IMediaStreamTrack"/> objects
    /// in this stream's track set whose kind is equal to "audio".
    /// </summary>
    /// <returns>A Vector of <see cref="IMediaStreamTrack"/> objects
    /// representing the audio tracks in this stream</returns>
    IVector<MediaAudioTrack^>^ GetAudioTracks();

    /// <summary>
    /// Returns a Vector that represents a snapshot of all the
    /// <see cref="IMediaStreamTrack"/> objects
    /// in this stream's track set whose kind is equal to "video".
    /// </summary>
    /// <returns>A Vector of <see cref="IMediaStreamTrack"/> objects
    /// representing the video tracks in this stream</returns>
    IVector<MediaVideoTrack^>^ GetVideoTracks();

    /// <summary>
    /// Returns a Vector that represents a snapshot of all the
    /// <see cref="IMediaStreamTrack"/> objects
    /// in this stream's track set, regardless of kind.
    /// </summary>
    /// <returns>A Vector of <see cref="IMediaStreamTrack"/> objects
    /// representing all the tracks in this stream.</returns>
    /// <seealso cref="GetAudioTracks"/>
    /// <seealso cref="GetVideoTracks"/>
    IVector<IMediaStreamTrack^>^ GetTracks();

    /// <summary>
    /// Return either a <see cref="IMediaStreamTrack"/> object from this
    /// stream's track set whose id is equal to trackId, or nullptr,
    /// if no such track exists.
    /// </summary>
    /// <param name="trackId">The identifier of the track to return</param>
    /// <returns>A <see cref="IMediaStreamTrack"/> object from this stream's
    /// track set whose id is
    /// equal to trackId, or nullptr, if no such track exists.</returns>
    IMediaStreamTrack^ GetTrackById(String^ trackId);

    /// <summary>
    /// Adds the given <see cref="IMediaStreamTrack"/> to this
    /// <see cref="MediaStream"/>.
    /// </summary>
    /// <param name="track">Track to be added to the <see cref="MediaStream"/>
    /// </param>
    void AddTrack(IMediaStreamTrack^ track);

    /// <summary>
    /// Removes the given <see cref="IMediaStreamTrack"/> from this
    /// <see cref="MediaStream"/>.
    /// </summary>
    /// <param name="track">Track to be removed from the
    /// <see cref="MediaStream"/></param>
    void RemoveTrack(IMediaStreamTrack^ track);

    /// <summary>
    /// Identifier.
    /// </summary>
    property String^ Id { String^ get();}

    /// <summary>
    /// TODO(winrt): fix comment after removing the TODO from the method
    /// definition.
    /// </summary>
    void Stop();

    /// <summary>
    /// This attribute is true if the <see cref="MediaStream"/> has at least
    /// one <see cref="IMediaStreamTrack"/>
    /// that has not ended, and false otherwise.
    /// </summary>
    property bool Active { bool get(); }
  private:
    ~MediaStream();
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _impl;
  };

  /// <summary>
  /// Represents video camera capture capabilities.
  /// </summary>
  public ref class CaptureCapability sealed {
  public:
    CaptureCapability(unsigned int width, unsigned int height,
      unsigned int fps,
      Windows::Media::MediaProperties::MediaRatio^ pixelAspect) {
      _width = width;
      _height = height;
      _fps = fps;
      _pixelAspectRatio = pixelAspect;
      wchar_t resolutionDesc[64];
      swprintf_s(resolutionDesc, 64, L"%u x %u",
        _width, _height);
      _resolutionDescription = ref new String(resolutionDesc);
      wchar_t fpsDesc[64];
      swprintf_s(fpsDesc, 64, L"%u fps", _fps);
      _fpsDescription = ref new String(fpsDesc);
      wchar_t desc[128];
      swprintf_s(desc, 128, L"%s %s", resolutionDesc, fpsDesc);
      _description = ref new String(desc);
    }
    property unsigned int Width {
      unsigned int get() {
        return _width;
      }
    }
    property unsigned int Height {
      unsigned int get() {
        return _height;
      }
    }
    property unsigned int FrameRate {
      unsigned int get() {
        return _fps;
      }
    }
    property Windows::Media::MediaProperties::MediaRatio^ PixelAspectRatio {
      Windows::Media::MediaProperties::MediaRatio^ get() {
        return _pixelAspectRatio;
      }
    }
    property String^ FullDescription {
      String^ get() {
        return _description;
      }
    }
    property String^ ResolutionDescription {
      String^ get() {
        return _resolutionDescription;
      }
    }
    property String^ FrameRateDescription {
      String^ get() {
        return _fpsDescription;
      }
    }
  private:
    unsigned int _width;
    unsigned int _height;
    unsigned int _fps;
    Windows::Media::MediaProperties::MediaRatio^ _pixelAspectRatio;
    String^ _resolutionDescription;
    String^ _fpsDescription;
    String^ _description;
  };

  /// <summary>
  /// Represents a local media device, such as a microphone or a camera.
  /// </summary>
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
      String^ get() {
        return _id;
      }
      void set(String^ value) {
        _id = value;
      }
    }

    property String^ Name {
      String^ get() {
        return _name;
      }
      void set(String^ value) {
        _name = value;
      }
    }

    /// <summary>
    /// Retrieves video capabilities for a given device.
    /// TODO(winrt) move to the Media class, video specific method should not
    // be in generic MediaDevice class.
    /// </summary>
    /// <returns>This is an asynchronous method. The result is a vector of the
    /// capabilities supported by the video device.</returns>
    IAsyncOperation<IVector<CaptureCapability^>^>^
      GetVideoCaptureCapabilities();
  };

  /// <summary>
  /// Allows defining constraints to exclude media types from a
  // <see cref="MediaStream"/>.
  /// </summary>
  public ref class RTCMediaStreamConstraints sealed {
  public:
    property bool audioEnabled;
    property bool videoEnabled;
  };

  /// <summary>
  /// Defines methods for accessing local media devices, like microphones and
  /// video cameras, and creating
  /// multimedia streams.
  /// </summary>
  /// <remarks>
  /// http://www.w3.org/TR/mediacapture-streams
  /// </remarks>
  public ref class Media sealed {
  public:
    Media();

    /// <summary>
    /// In order for this method to complete successfully, the user must have
    /// allowed the application permissions to use the devices for the
    /// requested media types (microphone for audio, webcam for video).
    /// Creates a <see cref="MediaStream"/> with both audio and video tracks,
    /// unless the <paramref name="mediaStreamConstraints"/>
    /// is set to exclude either media type.
    /// </summary>
    /// <param name="mediaStreamConstraints">Controls whether video/audio
    /// tracks are included.</param>
    /// <returns>
    /// This is an asynchronous method. The result upon completion is a
    /// <see cref="MediaStream"/> including
    /// audio and/or video tracks, as requested by the
    /// <paramref name="mediaStreamConstraints"/> parameter.
    /// </returns>
    IAsyncOperation<MediaStream^>^ GetUserMedia(
      RTCMediaStreamConstraints^ mediaStreamConstraints);

    /// <summary>
    /// Creates an <see cref="IMediaSource"/> for a video track, with a given
    /// frame rate and identifier to be used for notifications on media
    /// changes.
    /// </summary>
    /// <param name="track">Video track to create a <see cref="IMediaSource"/>
    /// from</param>
    /// <param name="framerate">Target frame rate</param>
    /// <param name="id">Identifier that can be used by applications for
    /// distinguishing between <see cref="MediaStream"/>s
    /// when receiving media change event notifications.
    /// </param>
    /// <returns>A media source.</returns>
    IMediaSource^ CreateMediaStreamSource(
      MediaVideoTrack^ track, uint32 framerate, String^ id);

    /// <summary>
    /// Has to be called after <see cref="EnumerateAudioVideoCaptureDevices"/>.
    /// Retrieves system devices that can be used for audio capturing
    /// (microphones).
    /// </summary>
    /// <returns>Vector of system devices that can be used for audio capturing
    /// (microphones).</returns>
    IVector<MediaDevice^>^ GetAudioCaptureDevices();

    /// <summary>
    /// Has to be called after <see cref="EnumerateAudioVideoCaptureDevices"/>.
    /// Retrieves system devices that can be used for video capturing (webcams).
    /// </summary>
    /// <returns>Vector of system devices that can be used for video capturing
    /// (webcams).</returns>
    IVector<MediaDevice^>^ GetVideoCaptureDevices();

    /// <summary>
    /// Enumerates through all capture devices on the system.
    /// Has to be called before <see cref="GetAudioCaptureDevices"/> and
    /// <see cref="GetVideoCaptureDevices"/>.
    /// Sends events: <see cref="OnVideoCaptureDeviceFound"/>,
    /// <see cref="OnAudioCaptureDeviceFound"/>.
    /// TODO(winrt): we should be able to refactor this so that it returns
    /// a list of devices and get rid of GetAudio/GetVideoCaptureDevices
    /// </summary>
    /// <returns>This is an asynchronous method. The result is true if the
    /// operation completed successfully, and false otherwise.</returns>
    IAsyncOperation<bool>^ EnumerateAudioVideoCaptureDevices();

    /// <summary>
    /// Allows switching between webcams.
    /// </summary>
    /// <param name="device">Webcam to be used for video capturing.</param>
    void SelectVideoDevice(MediaDevice^ device);

    /// <summary>
    /// Allows switching between microphones.
    /// </summary>
    /// <param name="device">Microphone to be used for audio capturing.</param>
    void SelectAudioDevice(MediaDevice^ device);

    /// <summary>
    /// Fired when a video capture device (webcam) is found on the system
    /// while enumerating devices.
    /// </summary>
    event OnMediaCaptureDeviceFoundDelegate^ OnVideoCaptureDeviceFound;

    /// <summary>
    /// Fired when an audio capture device (microphone) is found on the
    /// system while enumerating devices.
    /// </summary>
    event OnMediaCaptureDeviceFoundDelegate^ OnAudioCaptureDeviceFound;

  private:
    rtc::scoped_ptr<cricket::DeviceManagerInterface> _dev_manager;
    cricket::Device _selectedVideoDevice;
    webrtc::AudioDeviceModule *_audioDevice;
    uint16_t _selectedAudioDevice;
  };

}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_MEDIA_H_
