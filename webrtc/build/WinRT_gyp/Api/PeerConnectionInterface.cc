
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "webrtc/build/WinRT_gyp/Api/PeerConnectionInterface.h"

#include <ppltasks.h>
#include <map>
#include <string>
#include <functional>
#include <codecvt>

#include "GlobalObserver.h"
#include "Marshalling.h"
#include "DataChannel.h"
#include "Media.h"
#include "webrtc/base/ssladapter.h"
#include "webrtc/base/win32socketinit.h"
#include "webrtc/base/thread.h"
#include "webrtc/base/bind.h"
#include "webrtc/base/event_tracer.h"
#include "webrtc/base/loggingserver.h"
#include "webrtc/base/tracelog.h"
#include "webrtc/base/stream.h"
#include "webrtc/test/field_trial.h"
#include "talk/app/webrtc/test/fakeconstraints.h"
#include "talk/session/media/channelmanager.h"
#include "webrtc/system_wrappers/interface/utf_util_win.h"
#include "webrtc/system_wrappers/interface/tick_util.h"
#include "third_party/h264_winrt/h264_winrt_factory.h"
#include "webrtc/system_wrappers/interface/trace_event.h"


using webrtc_winrt_api_internal::FromCx;
using webrtc_winrt_api_internal::ToCx;
using Platform::Collections::Vector;
using Windows::Media::Capture::MediaCapture;
using Windows::Media::Capture::MediaCaptureInitializationSettings;
using rtc::FileStream;

Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

// Any globals we need to keep around.
namespace webrtc_winrt_api {

namespace globals {

bool certificateVerifyCallBack(void* cert) {
  return true;
}

static const std::string logFileName = "_webrtc_logging.log";
double gCurrentCPUUsage = 0.0;
uint64 gCurrentMEMUsage = 0;

// helper function to get default output path for the app
std::string OutputPath() {
  auto folder = Windows::Storage::ApplicationData::Current->LocalFolder;
  wchar_t buffer[255];
  wcsncpy_s(buffer, 255, folder->Path->Data(), _TRUNCATE);
  return webrtc::ToUtf8(buffer) + "\\";
}

// helper function to convert a std string to Platform string
String^ toPlatformString(std::string aString) {
  std::wstring wide_str = std::wstring(aString.begin(), aString.end());
  Platform::String^ p_string = ref new Platform::String(wide_str.c_str());
  return p_string;
}

/**
 * a private class only used in this file, which implements LogSink for logging to file
 */
class FileLogSink
  : public rtc::LogSink{
 public:
  explicit FileLogSink(rtc::FileStream* fStream)  {
      fileStream_.reset(fStream);
  }
  rtc::FileStream* file() { return fileStream_.get(); }
 private:
  void OnLogMessage(const std::string& message) override {
    fileStream_->WriteAll(
      message.data(), message.size(), nullptr, nullptr);
  }

  rtc::scoped_ptr<rtc::FileStream> fileStream_;
};

static bool isInitialized = false;

rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface>
  gPeerConnectionFactory;
  // The worker thread for webrtc.
  rtc::Thread gThread;
  rtc::TraceLog gTraceLog;
  rtc::scoped_ptr<rtc::LoggingServer> gLoggingServer;
  rtc::scoped_ptr<FileLogSink> gLoggingFile;
  cricket::VideoFormat gPreferredVideoCaptureFormat;
}  // namespace globals

RTCIceCandidate::RTCIceCandidate() {
}

RTCIceCandidate::RTCIceCandidate(
  String^ candidate, String^ sdpMid, uint16 sdpMLineIndex) {
  Candidate = candidate;
  SdpMid = sdpMid;
  SdpMLineIndex = sdpMLineIndex;
}

RTCSessionDescription::RTCSessionDescription() {
}

RTCSessionDescription::RTCSessionDescription(RTCSdpType type, String^ sdp) {
  Type = type;
  Sdp = sdp;
}

RTCPeerConnection::RTCPeerConnection(RTCConfiguration^ configuration)
  : _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , _observer(new GlobalObserver()) {
  webrtc::PeerConnectionInterface::RTCConfiguration cc_configuration;
  FromCx(configuration, &cc_configuration);
  globals::RunOnGlobalThread<void>([this, cc_configuration] {
    webrtc::FakeConstraints constraints;
    constraints.SetAllowDtlsSctpDataChannels();
    constraints.AddOptional(
      webrtc::MediaConstraintsInterface::kCombinedAudioVideoBwe, "true");
    _observer->SetPeerConnection(this);
    LOG(LS_INFO) << "Creating PeerConnection native.";
    _impl = globals::gPeerConnectionFactory->CreatePeerConnection(
      cc_configuration, &constraints, nullptr, nullptr, _observer.get());
  });
}

RTCPeerConnection::~RTCPeerConnection() {
  LOG(LS_INFO) << "RTCPeerConnection::~RTCPeerConnection";
}

// Utility function to create an async operation
// which wraps a callback based async function.
// Use std::tuple<> for callbacks with more than one argument.
// Different types T1 and T2 where additional processing
// needs to be done in the callback.
template <typename T1, typename T2>
IAsyncOperation<T2>^ CreateCallbackBridge(
  std::function<void(Concurrency::task_completion_event<T1>)> init,
  std::function<T2(T1)> onCallback) {
  Concurrency::task_completion_event<T1> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init] {
    globals::RunOnGlobalThread<void>([tce, init] {
      init(tce);
    });
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<T1>(tce)
    .then([onCallback](T1 arg) {
    // Then calls the callback with the return value.
    return onCallback(arg);
  });

  // Return an async operation that waits on the return value
  // of the callback and returns it.
  return Concurrency::create_async([tceTask] {
    return tceTask.get();
  });
}

// Specialized version for void callbacks.
IAsyncAction^ CreateCallbackBridge(
  std::function<void(Concurrency::task_completion_event<void>)> init) {
  Concurrency::task_completion_event<void> tce;

  // Start the initial async operation
  Concurrency::create_async([tce, init] {
    globals::RunOnGlobalThread<void>([tce, init] {
      init(tce);
    });
  });

  // Create the task that waits on the completion event.
  auto tceTask = Concurrency::task<void>(tce);

  // Return an async operation that waits on the
  // task completetion event.
  return Concurrency::create_async([tceTask] {
    return tceTask.get();
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateOffer() {
  return CreateCallbackBridge
    <webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
      [this](Concurrency::task_completion_event
        <webrtc::SessionDescriptionInterface*> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      tce.set(nullptr);
      return;
    }

    rtc::scoped_refptr<CreateSdpObserver> observer(
      new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateOffer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi) {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncOperation<RTCSessionDescription^>^ RTCPeerConnection::CreateAnswer() {
  return CreateCallbackBridge
    <webrtc::SessionDescriptionInterface*, RTCSessionDescription^>(
      [this](Concurrency::task_completion_event
      <webrtc::SessionDescriptionInterface*> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      tce.set(nullptr);
      return;
    }

    rtc::scoped_refptr<CreateSdpObserver> observer(
      new rtc::RefCountedObject<CreateSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _createSdpObservers.push_back(observer);

    _impl->CreateAnswer(observer, nullptr);
  }, [](webrtc::SessionDescriptionInterface* sdi) {
    RTCSessionDescription^ ret = nullptr;
    ToCx(sdi, &ret);
    return ret;
  });
}

IAsyncAction^ RTCPeerConnection::SetLocalDescription(
  RTCSessionDescription^ description) {
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      tce.set();
      return;
    }

    rtc::scoped_refptr<SetSdpObserver> observer(
      new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    rtc::scoped_ptr<webrtc::SessionDescriptionInterface> nativeDescription;
    FromCx(description, &nativeDescription);

    _impl->SetLocalDescription(observer, nativeDescription.release());
  });
}

IAsyncAction^ RTCPeerConnection::SetRemoteDescription(
  RTCSessionDescription^ description) {
  return CreateCallbackBridge(
    [this, description](Concurrency::task_completion_event<void> tce) {
    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      tce.set();
      return;
    }

    rtc::scoped_refptr<SetSdpObserver> observer(
      new rtc::RefCountedObject<SetSdpObserver>(tce));
    // TODO(WINRT): Remove it once the callback has been received.
    _setSdpObservers.push_back(observer);

    rtc::scoped_ptr<webrtc::SessionDescriptionInterface> nativeDescription;
    FromCx(description, &nativeDescription);

    _impl->SetRemoteDescription(observer, nativeDescription.release());
  });
}

RTCConfiguration^ RTCPeerConnection::GetConfiguration() {
  RTCConfiguration^ ret = nullptr;
  globals::RunOnGlobalThread<void>([this, ret] {
    // TODO(WINRT): Figure out how to rebuild a configuration
    // object.  There doesn't seem to be a simple function
    // on the webrtc::PeerConnectionInterface.
  });
  return ret;
}

IVector<MediaStream^>^ RTCPeerConnection::GetLocalStreams() {
  auto ret = ref new Vector<MediaStream^>();
  globals::RunOnGlobalThread<void>([this, ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    auto streams = _impl->local_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      ret->Append(ref new MediaStream(streams->at(i)));
    }
  });
  return ret;
}

IVector<MediaStream^>^ RTCPeerConnection::GetRemoteStreams() {
  auto ret = ref new Vector<MediaStream^>();
  globals::RunOnGlobalThread<void>([this, ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    auto streams = _impl->remote_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      ret->Append(ref new MediaStream(streams->at(i)));
    }
  });
  return ret;
}

MediaStream^ RTCPeerConnection::GetStreamById(String^ streamId) {
  MediaStream^ ret = nullptr;
  globals::RunOnGlobalThread<void>([this, streamId, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    std::string streamIdStr = FromCx(streamId);
    // Look through the local streams.
    auto streams = _impl->local_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      auto stream = streams->at(i);
      // TODO(WINRT): Is the label the stream id?
      if (stream->label() == streamIdStr) {
        ret = ref new MediaStream(stream);
        return;
      }
    }
    // Look through the remote streams.
    streams = _impl->remote_streams();
    for (size_t i = 0; i < streams->count(); ++i) {
      auto stream = streams->at(i);
      // TODO(WINRT): Is the label the stream id?
      if (stream->label() == streamIdStr) {
        ret = ref new MediaStream(stream);
        return;
      }
    }
  });
  return ret;
}

void RTCPeerConnection::AddStream(MediaStream^ stream) {
  globals::RunOnGlobalThread<void>([this, stream] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    _impl->AddStream(stream->GetImpl());
  });
}

void RTCPeerConnection::RemoveStream(MediaStream^ stream) {
  globals::RunOnGlobalThread<void>([this, stream] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    _impl->RemoveStream(stream->GetImpl());
  });
}

RTCDataChannel^ RTCPeerConnection::CreateDataChannel(
  String^ label, RTCDataChannelInit^ init) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  if (_impl == nullptr) {
    return nullptr;
  }

  webrtc::DataChannelInit nativeInit;
  if (init != nullptr) {
    FromCx(init, &nativeInit);
  }

  auto channel = _impl->CreateDataChannel(
    FromCx(label), init != nullptr ? &nativeInit : nullptr);
  auto ret = ref new RTCDataChannel(channel);

  auto observer = new webrtc_winrt_api_internal::DataChannelObserver(ret);
  // TODO(WINRT): Figure out when to remove this observer.
  _dataChannelObservers.PushBack(observer);
  channel->RegisterObserver(observer);
  return ret;
}

IAsyncAction^ RTCPeerConnection::AddIceCandidate(RTCIceCandidate^ candidate) {
  return Concurrency::create_async([this, candidate] {
    globals::RunOnGlobalThread<void>([this, candidate] {
      webrtc::CriticalSectionScoped csLock(_lock.get());
      if (_impl == nullptr) {
        return;
      }

      rtc::scoped_ptr<webrtc::IceCandidateInterface> nativeCandidate;
      FromCx(candidate, &nativeCandidate);
      _impl->AddIceCandidate(nativeCandidate.get());
    });
  });
}

void RTCPeerConnection::Close() {
  globals::RunOnGlobalThread<void>([this] {
    webrtc::CriticalSectionScoped csLock(_lock.get());

    if (_impl.get()) {
      _impl->Close();
    }

    // Needed to remove the circular references and allow
    // this object to be garbage collected.
    _observer->SetPeerConnection(nullptr);
    _impl = nullptr;
  });
}

void RTCPeerConnection::ToggleETWStats(bool enable) {
  globals::RunOnGlobalThread<void>([this, enable] {
    _observer->ToggleETWStats(enable);
  });
}

void RTCPeerConnection::ToggleConnectionHealthStats(bool enable) {
  globals::RunOnGlobalThread<void>([this, enable] {
    _observer->ToggleConnectionHealthStats(enable);
  });
}

void RTCPeerConnection::ToggleRTCStats(bool enable) {
  globals::RunOnGlobalThread<void>([this, enable] {
    _observer->ToggleRTCStats(enable);
  });
}

RTCSessionDescription^ RTCPeerConnection::LocalDescription::get() {
  RTCSessionDescription^ ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    if (_impl->local_description() != nullptr) {
      ToCx(_impl->local_description(), &ret);
    }
  });
  return ret;
}

RTCSessionDescription^ RTCPeerConnection::RemoteDescription::get() {
  RTCSessionDescription^ ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      return;
    }

    if (_impl->remote_description() != nullptr) {
      ToCx(_impl->remote_description(), &ret);
    }
  });
  return ret;
}

RTCSignalingState RTCPeerConnection::SignalingState::get() {
  RTCSignalingState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      ret = RTCSignalingState::Closed;
      return;
    }

    ToCx(_impl->signaling_state(), &ret);
  });
  return ret;
}

RTCIceGatheringState RTCPeerConnection::IceGatheringState::get() {
  RTCIceGatheringState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      ret = RTCIceGatheringState::Complete;
      return;
    }

    ToCx(_impl->ice_gathering_state(), &ret);
  });
  return ret;
}

RTCIceConnectionState RTCPeerConnection::IceConnectionState::get() {
  RTCIceConnectionState ret;
  globals::RunOnGlobalThread<void>([this, &ret] {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (_impl == nullptr) {
      ret = RTCIceConnectionState::Closed;
      return;
    }

    ToCx(_impl->ice_connection_state(), &ret);
  });
  return ret;
}

IAsyncOperation<bool>^  WebRTC::RequestAccessForMediaCapture() {
  // On some platforms, two calls of InitializeAsync on two diferent
  // instances causes exception to be thrown from the second call to
  // InitializeAsync. The second InitializeAsync
  // is called in MediaCaptureDevicesWinRT::GetMediaCapture
  // Behavior present on Lumia620, OS version 8.10.14219.341.
  Platform::Agile<MediaCapture> mediaAccessRequester(
    ref new MediaCapture());
  MediaCaptureInitializationSettings^ mediaSettings =
    ref new MediaCaptureInitializationSettings();
  mediaSettings->AudioDeviceId = "";
  mediaSettings->VideoDeviceId = "";
  mediaSettings->StreamingCaptureMode =
    Windows::Media::Capture::StreamingCaptureMode::AudioAndVideo;
  mediaSettings->PhotoCaptureSource =
    Windows::Media::Capture::PhotoCaptureSource::VideoPreview;
  Concurrency::task<void> initTask = Concurrency::create_task(
    mediaAccessRequester->InitializeAsync(mediaSettings));
  return Concurrency::create_async([initTask] {
    bool accessRequestAccepted = true;
    try {
      initTask.get();
    } catch (Platform::Exception^ e) {
      LOG(LS_ERROR) << "Failed to obtain media access permission: "
                    << rtc::ToUtf8(e->Message->Data()).c_str();
      accessRequestAccepted = false;
    }
    return accessRequestAccepted;
  });
}

void WinJSHooks::initialize() {
  g_windowDispatcher =
    Windows::UI::Core::CoreWindow::GetForCurrentThread()->Dispatcher;

  webrtc_winrt_api::WebRTC::Initialize(g_windowDispatcher);
}

IAsyncOperation<bool>^ WinJSHooks::requestAccessForMediaCapture() {
    return  webrtc_winrt_api::WebRTC::RequestAccessForMediaCapture();
}

bool WinJSHooks::IsTracing() {
  return  webrtc_winrt_api::WebRTC::IsTracing();
}

void WinJSHooks::StartTracing() {
  webrtc_winrt_api::WebRTC::StartTracing();
}

void WinJSHooks::StopTracing() {
  webrtc_winrt_api::WebRTC::StopTracing();
}

bool WinJSHooks::SaveTrace(Platform::String^ filename) {
  return  webrtc_winrt_api::WebRTC::SaveTrace(filename);
}
bool WinJSHooks::SaveTrace(Platform::String^ host, int port) {
  return  webrtc_winrt_api::WebRTC::SaveTrace(host, port);
}

void WebRTC::Initialize(Windows::UI::Core::CoreDispatcher^ dispatcher) {
  if (globals::isInitialized)
    return;

  g_windowDispatcher = dispatcher;

  // Create a worker thread
  globals::gThread.SetName("WinRTApiWorker", nullptr);
  globals::gThread.Start();
  globals::RunOnGlobalThread<void>([] {
    rtc::EnsureWinsockInit();
    rtc::InitializeSSL(globals::certificateVerifyCallBack);

    auto encoderFactory = new webrtc::H264WinRTEncoderFactory();
    auto decoderFactory = new webrtc::H264WinRTDecoderFactory();

    LOG(LS_INFO) << "Creating PeerConnectionFactory.";
    globals::gPeerConnectionFactory =
        webrtc::CreatePeerConnectionFactory(encoderFactory, decoderFactory);

    webrtc::SetupEventTracer(&WebRTC::GetCategoryGroupEnabled,
      &WebRTC::AddTraceEvent);
  });
  globals::isInitialized = true;
}

bool WebRTC::IsTracing() {
  return globals::gTraceLog.IsTracing();
}

void WebRTC::StartTracing() {
  globals::gTraceLog.EnableTraceInternalStorage();
  globals::gTraceLog.StartTracing();
}

void WebRTC::StopTracing() {
  globals::gTraceLog.StopTracing();
}

bool WebRTC::SaveTrace(Platform::String^ filename) {
  std::string filenameStr = FromCx(filename);
  return globals::gTraceLog.Save(filenameStr);
}

bool WebRTC::SaveTrace(Platform::String^ host, int port) {
  std::string hostStr = FromCx(host);
  return globals::gTraceLog.Save(hostStr, port);
}

void WebRTC::EnableLogging(LogLevel level) {
  if (globals::gLoggingFile.get() != nullptr ||
      globals::gLoggingServer.get() != nullptr) {
    // already logging
    return;
  }

  // setup logging to network
  rtc::SocketAddress sa(INADDR_ANY, 47003);
  globals::gLoggingServer = rtc::scoped_ptr<rtc::LoggingServer>(
    new rtc::LoggingServer());
  globals::gLoggingServer->Listen(sa, static_cast<rtc::LoggingSeverity>(level));

  // setup logging to a file
  rtc::FileStream* fileStream = new rtc::FileStream();
  fileStream->Open(globals::OutputPath() + globals::logFileName, "wb", NULL);
  fileStream->DisableBuffering();
  globals::gLoggingFile = rtc::scoped_ptr<globals::FileLogSink>(
                           new globals::FileLogSink(fileStream));
  rtc::LogMessage::AddLogToStream(globals::gLoggingFile.get(),
                   static_cast<rtc::LoggingSeverity>(level));

  LOG(LS_INFO) << "WebRTC logging enabled";
}

void WebRTC::DisableLogging() {
  LOG(LS_INFO) << "WebRTC logging disabled";
  rtc::LogMessage::RemoveLogToStream(globals::gLoggingFile.get());
  globals::gLoggingFile.get()->file()->Close();
  globals::gLoggingFile.reset();
  globals::gLoggingServer.reset();
}

Windows::Storage::StorageFolder^  WebRTC::LogFolder() {
  return  Windows::Storage::ApplicationData::Current->LocalFolder;
}

String^  WebRTC::LogFileName() {
  return globals::toPlatformString(globals::logFileName);
}


IVector<CodecInfo^>^ WebRTC::GetAudioCodecs() {
  auto ret = ref new Vector<CodecInfo^>();
  std::vector<cricket::AudioCodec> codecs;
  cricket::ChannelManager* chmng =
      globals::gPeerConnectionFactory->channel_manager();
  chmng->GetSupportedAudioCodecs(&codecs);
    for (auto it = codecs.begin(); it != codecs.end(); ++it) {
      ret->Append(ref new CodecInfo(it->id, it->clockrate, ToCx(it->name)));
    }
  return ret;
}

IVector<CodecInfo^>^ WebRTC::GetVideoCodecs() {
  auto ret = ref new Vector<CodecInfo^>();
  std::vector<cricket::VideoCodec> codecs;
  cricket::ChannelManager* chmng =
      globals::gPeerConnectionFactory->channel_manager();
  chmng->GetSupportedVideoCodecs(&codecs);
    for (auto it = codecs.begin(); it != codecs.end(); ++it) {
      if (it->GetCodecType() == cricket::VideoCodec::CODEC_VIDEO)
      ret->Append(ref new CodecInfo(it->id, it->clockrate, ToCx(it->name)));
    }
  return ret;
}

void WebRTC::SynNTPTime(int64 current_ntp_time) {
  webrtc::TickTime::SyncWithNtp(current_ntp_time);
}


void WebRTC::UpdateCPUUsage(double cpu_usage) {
  globals::gCurrentCPUUsage = cpu_usage;

  //TRACE_COUNTER1 can only log 32bit integer value
  // also, when the app is idle, CPUUsage is very low <1%
  TRACE_COUNTER1("webrtc", "winrtCPUUsage", (int32)(globals::gCurrentCPUUsage * 100));
}

double WebRTC::GetCPUUsage() {
  return globals::gCurrentCPUUsage;
}

void WebRTC::UpdateMemUsage(INT64 mem_usage) {
  globals::gCurrentMEMUsage = mem_usage;

  //TRACE_COUNTER1 can only log 32bit integer value
  TRACE_COUNTER1("webrtc", "winrtMemUsage", (int32)(globals::gCurrentMEMUsage / 1024));
}

INT64 WebRTC::GetMemUsage() {
  return globals::gCurrentMEMUsage;
}


void WebRTC::SetPreferredVideoCaptureFormat(int frame_width,
                                            int frame_height, int fps) {
  globals::gPreferredVideoCaptureFormat.interval =
    cricket::VideoFormat::FpsToInterval(fps);

  globals::gPreferredVideoCaptureFormat.width = frame_width;

  globals::gPreferredVideoCaptureFormat.height = frame_height;

  cricket::ChannelManager* chmng =
    globals::gPeerConnectionFactory->channel_manager();
  chmng->SetPreferredCaptureFormat(globals::gPreferredVideoCaptureFormat);
}

const unsigned char* /*__cdecl*/ WebRTC::GetCategoryGroupEnabled(
  const char* category_group) {
  return reinterpret_cast<const unsigned char*>("webrtc");
}

void __cdecl WebRTC::AddTraceEvent(char phase,
  const unsigned char* category_group_enabled,
  const char* name,
  uint64 id,
  int num_args,
  const char** arg_names,
  const unsigned char* arg_types,
  const uint64* arg_values,
  unsigned char flags) {
  globals::gTraceLog.Add(phase, category_group_enabled, name, id,
    num_args, arg_names, arg_types, arg_values, flags);
}

}  // namespace webrtc_winrt_api
