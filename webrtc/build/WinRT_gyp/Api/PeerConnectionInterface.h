
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.


#ifndef WEBRTC_BUILD_WINRT_GYP_API_PEERCONNECTIONINTERFACE_H_
#define WEBRTC_BUILD_WINRT_GYP_API_PEERCONNECTIONINTERFACE_H_

#include <collection.h>
#include <vector>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "webrtc/base/scopedptrcollection.h"
#include "webrtc/base/logging.h"
#include "GlobalObserver.h"
#include "DataChannel.h"

using Platform::String;
using Platform::IBox;
using Windows::Foundation::Collections::IVector;
using Windows::Foundation::IAsyncOperation;
using Windows::Foundation::IAsyncAction;
using webrtc_winrt_api_internal::CreateSdpObserver;
using webrtc_winrt_api_internal::SetSdpObserver;
using webrtc_winrt_api_internal::DataChannelObserver;
using webrtc_winrt_api_internal::GlobalObserver;

namespace webrtc_winrt_api {

ref class MediaStream;
ref class MediaStreamTrack;

public enum class LogLevel {
  LOGLVL_SENSITIVE = rtc::LS_SENSITIVE,
  LOGLVL_VERBOSE = rtc::LS_VERBOSE,
  LOGLVL_INFO = rtc::LS_INFO,
  LOGLVL_WARNING = rtc::LS_WARNING,
  LOGLVL_ERROR = rtc::LS_ERROR
};

public ref class CodecInfo sealed {
private:
    int _id;
    int _clockrate;
    int _channels;
    String^ _name;
public:
    CodecInfo(int id, int clockrate, String^ name) {
        _id = id;
        _clockrate = clockrate;
        _name = name;
    }

    property int Id {
        int get() {
            return _id;
        }
        void set(int value) {
            _id = value;
        }
    }

    property int Clockrate {
        int get() {
            return _clockrate;
        }
        void set(int value) {
            _clockrate = value;
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
};

public ref class WinJSHooks sealed {
public:
  static void initialize();
  static IAsyncOperation<bool>^ requestAccessForMediaCapture();
  static bool IsTracing();
  static void StartTracing();
  static void StopTracing();
  static bool SaveTrace(Platform::String^ filename);
  static bool SaveTrace(Platform::String^ host, int port);
};

[Windows::Foundation::Metadata::WebHostHidden]
public ref class WebRTC sealed {
public:
  static IAsyncOperation<bool>^ RequestAccessForMediaCapture();
  static void Initialize(Windows::UI::Core::CoreDispatcher^ dispatcher);

  static bool IsTracing();
  static void StartTracing();
  static void StopTracing();
  static bool SaveTrace(Platform::String^ filename);
  static bool SaveTrace(Platform::String^ host, int port);

  static void EnableLogging(LogLevel level);
  static void DisableLogging();

  // retrieve current folder where the app save logging
  static Windows::Storage::StorageFolder^ LogFolder();

  static String^ LogFileName();

  static IVector<CodecInfo^>^ GetAudioCodecs();
  static IVector<CodecInfo^>^ GetVideoCodecs();
  static void SetPreferredVideoCaptureFormat(int frame_width,
                                             int frame_height,
                                             int fps);
  static void SynNTPTime(int64 current_ntp_time);

private:
  // This type is not meant to be created.
  WebRTC();

  static const unsigned char* GetCategoryGroupEnabled(const char*
                                                        category_group);
  static void __cdecl AddTraceEvent(char phase,
    const unsigned char* category_group_enabled,
    const char* name,
    uint64 id,
    int num_args,
    const char** arg_names,
    const unsigned char* arg_types,
    const uint64* arg_values,
    unsigned char flags);
};

public enum class RTCBundlePolicy {
  Balanced,
  MaxBundle,
  MaxCompat,
};

public enum class RTCIceTransportPolicy {
  None,
  Relay,
  NoHost,
  All
};

public enum class RTCIceGatheringState {
  New,
  Gathering,
  Complete
};

public enum class RTCIceConnectionState {
  New,
  Checking,
  Connected,
  Completed,
  Failed,
  Disconnected,
  Closed
};

/// <summary>
/// Describes the type of an SDP blob.
/// </summary>
public enum class RTCSdpType {
  Offer,
  Pranswer,
  Answer,
};

public enum class RTCSignalingState {
  Stable,
  HaveLocalOffer,
  HaveRemoteOffer,
  HaveLocalPranswer,
  HaveRemotePranswer,
  Closed
};

public ref class RTCIceServer sealed {
public:
  property String^ Url;
  property String^ Username;
  property String^ Credential;
};

public ref class RTCConfiguration sealed {
public:
  property IVector<RTCIceServer^>^ IceServers;
  property IBox<RTCIceTransportPolicy>^ IceTransportPolicy;
  property IBox<RTCBundlePolicy>^ BundlePolicy;
  // TODO(WINRT): DOMString PeerIdentity
};

public ref class RTCIceCandidate sealed {
public:
  RTCIceCandidate();
  RTCIceCandidate(String^ candidate, String^ sdpMid,
    uint16 sdpMLineIndex);
  property String^ Candidate;
  property String^ SdpMid;
  property uint16 SdpMLineIndex;
};

/// <summary>
/// An SDP blob and an associated <see cref="RTCSdpType"/>.
/// </summary>
public ref class RTCSessionDescription sealed {
public:
  RTCSessionDescription();
  RTCSessionDescription(RTCSdpType type, String^ sdp);
  property IBox<RTCSdpType>^ Type;
  property String^ Sdp;
};

// Events and delegates
// ------------------
public ref class RTCPeerConnectionIceEvent sealed {
public:
  property RTCIceCandidate^ Candidate;
};

public ref class RTCPeerConnectionIceStateChangeEvent sealed {
public:
  property RTCIceConnectionState State;
};

// ------------------
public ref class MediaStreamEvent sealed {
public:
  property MediaStream^ Stream;
};

/// <summary>
/// An RTCPeerConnection allows two users to communicate directly.
/// Communications are coordinated via a signaling channel which is provided
/// by unspecified means.
/// </summary>
/// <remarks>
/// http://www.w3.org/TR/webrtc/#peer-to-peer-connections
/// </remarks>
public ref class RTCPeerConnection sealed {
public:
  // Required so the observer can raise events in this class.
  // By default event raising is protected.
  friend class GlobalObserver;

  /// <summary>
  /// Creates an RTCPeerConnection object.
  /// </summary>
  /// <remarks>
  /// Refer to http://www.w3.org/TR/webrtc for the RTCPeerConnection
  /// construction algorithm
  /// </remarks>
  /// <param name="configuration">
  /// The configuration has the information to find and access the
  /// servers used by ICE.
  /// </param>
  RTCPeerConnection(RTCConfiguration^ configuration);

  event RTCPeerConnectionIceEventDelegate^     OnIceCandidate;
  event RTCPeerConnectionIceStateChangeEventDelegate^  OnIceConnectionChange;
  event MediaStreamEventEventDelegate^ OnAddStream;
  event MediaStreamEventEventDelegate^ OnRemoveStream;
  event EventDelegate^ OnNegotiationNeeded;
  event EventDelegate^ OnSignalingStateChange;
  event RTCDataChannelEventDelegate^ OnDataChannel;

  /// <summary>
  /// Generates a blob of SDP that contains an RFC 3264 offer with the
  /// supported configurations for the session, including descriptions
  /// of the local MediaStreams attached to this
  /// <see cref="RTCPeerConnection"/>,
  /// the codec/RTP/RTCP options supported by this implementation, and
  /// any candidates that have been gathered by the ICE Agent.
  /// </summary>
  /// <returns></returns>
  IAsyncOperation<RTCSessionDescription^>^ CreateOffer();

  /// <summary>
  /// Generates an SDP answer with the supported configuration for the
  /// session that is compatible with the parameters in the remote
  /// configuration. Like createOffer, the returned blob contains descriptions
  /// of the local MediaStreams attached to this
  /// <see cref="RTCPeerConnection"/>, the codec/RTP/RTCP options negotiated
  /// for this session, and any candidates that have been gathered by the ICE
  /// Agent.
  /// </summary>
  /// <returns>An action which completes asynchronously</returns>
  IAsyncOperation<RTCSessionDescription^>^ CreateAnswer();

  /// <summary>
  /// Instructs the <see cref="RTCPeerConnection"/> to apply the supplied
  /// <see cref="RTCSessionDescription"/> as the local description.
  /// This API changes the local media state.
  /// </summary>
  /// <param name="description">RTCSessionDescription to apply as the
  /// local description</param>
  /// <returns>An action which completes asynchronously</returns>
  IAsyncAction^ SetLocalDescription(RTCSessionDescription^ description);

  /// <summary>
  /// Instructs the <see cref="RTCPeerConnection"/> to apply the supplied
  /// <see cref="RTCSessionDescription"/> as the remote offer or answer.
  /// This API changes the local media state.
  /// </summary>
  /// <param name="description"><see cref="RTCSessionDescription"/> to
  /// apply as the local description</param>
  /// <returns>An action which completes asynchronously</returns>
  IAsyncAction^ SetRemoteDescription(RTCSessionDescription^ description);

  RTCConfiguration^ GetConfiguration();
  IVector<MediaStream^>^ GetLocalStreams();
  IVector<MediaStream^>^ GetRemoteStreams();
  MediaStream^ GetStreamById(String^ streamId);
  void AddStream(MediaStream^ stream);
  void RemoveStream(MediaStream^ stream);

  /// <summary>
  /// Creates a new <see cref="RTCDataChannel"/> object with the given <paramref name="label"/>.
  /// </summary>
  /// <param name="label">Used as the descriptive name for the new data channel.</param>
  /// <param name="init">Can be used to configure properties of the underlying channel such as data reliability.</param>
  /// <returns>The newly created <see cref="RTCDataChannel"/>.</returns>
  RTCDataChannel^ CreateDataChannel(String^ label, RTCDataChannelInit^ init);

  /// <summary>
  /// Provides a remote candidate to the ICE Agent.
  /// The candidate is added to the remote description.
  /// This call will result in a change to the connection state of the ICE
  /// Agent, and may lead to a change to media state if it results in
  /// different connectivity being established.
  /// </summary>
  /// <param name="candidate">candidate to be added to the remote description
  /// </param>
  /// <returns>An action which completes asynchronously</returns>
  IAsyncAction^ AddIceCandidate(RTCIceCandidate^ candidate);
  void Close();

  /// <summary>
  /// The last <see cref="RTCSessionDescription"/> that was successfully set
  /// using <see cref="SetLocalDescription"/>, plus any local candidates that
  /// have been generated by the ICE Agent since then.
  /// A nullptr handle will be returned if the local description has not yet
  /// been set.
  /// </summary>
  property RTCSessionDescription^ LocalDescription {
    RTCSessionDescription^ get();
  }

  /// <summary>
  /// The last <see cref="RTCSessionDescription"/> that was successfully set
  /// using <see cref="SetRemoteDescription"/>,
  /// plus any remote candidates that have been supplied via
  /// <see cref="AddIceCandidate"/> since then.
  /// A nullptr handle will be returned if the local description has not
  /// yet been set.
  /// </summary>
  property RTCSessionDescription^ RemoteDescription {
    RTCSessionDescription^ get();
  }
  property RTCSignalingState SignalingState {
    RTCSignalingState get();
  }
  property RTCIceGatheringState IceGatheringState {
    RTCIceGatheringState get();
  }
  property RTCIceConnectionState IceConnectionState {
    RTCIceConnectionState get();
  }

private:
  ~RTCPeerConnection();
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> _impl;
  // This lock protects _impl.
  rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;

  rtc::scoped_ptr<GlobalObserver> _observer;

  typedef std::vector<rtc::scoped_refptr<CreateSdpObserver>> CreateSdpObservers;
  typedef std::vector<rtc::scoped_refptr<SetSdpObserver>> SetSdpObservers;
  typedef rtc::ScopedPtrCollection<DataChannelObserver> DataChannelObservers;
  CreateSdpObservers _createSdpObservers;
  SetSdpObservers _setSdpObservers;
  DataChannelObservers _dataChannelObservers;
};

namespace globals {
extern rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
  gPeerConnectionFactory;
// The worker thread for webrtc.
extern rtc::Thread gThread;

template <typename T>
T RunOnGlobalThread(std::function<T()> fn) {
  return gThread.Invoke<T, std::function<T()>>(fn);
}

}  // namespace globals

}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_PEERCONNECTIONINTERFACE_H_
