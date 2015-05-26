#include "Media.h"
#include "peerconnectioninterface.h"
#include "Marshalling.h"
#include "RTMediaStreamSource.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include <ppltasks.h>
#include <mfapi.h>

using namespace webrtc_winrt_api;
using namespace webrtc_winrt_api_internal;
using namespace Platform;
using namespace Microsoft::WRL;
//using namespace Windows::Media::MediaProperties;

MediaVideoTrack::MediaVideoTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> impl) :
  _impl(impl)
{
}

MediaVideoTrack::~MediaVideoTrack()
{
}

String^ MediaVideoTrack::Kind::get()
{
  return ToCx(_impl->kind());
}

String^ MediaVideoTrack::Id::get()
{
  return ToCx(_impl->id());
}

bool MediaVideoTrack::Enabled::get()
{
  return _impl->enabled();
}

void MediaVideoTrack::Enabled::set(bool value)
{
  _impl->set_enabled(value);
}

void MediaVideoTrack::SetRenderer(webrtc::VideoRendererInterface* renderer)
{
  _impl->AddRenderer(renderer);
}

void MediaVideoTrack::UnsetRenderer(webrtc::VideoRendererInterface* renderer)
{
  _impl->RemoveRenderer(renderer);
}

// ===========================================================================

MediaAudioTrack::MediaAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> impl) :
_impl(impl)
{
}

String^ MediaAudioTrack::Kind::get()
{
  return ToCx(_impl->kind());
}

String^ MediaAudioTrack::Id::get()
{
  return ToCx(_impl->id());
}

bool MediaAudioTrack::Enabled::get()
{
  return _impl->enabled();
}

void MediaAudioTrack::Enabled::set(bool value)
{
  _impl->set_enabled(value);
}

MediaStream::MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl)
  : _impl(impl)
{

}

rtc::scoped_refptr<webrtc::MediaStreamInterface> MediaStream::GetImpl()
{
  return _impl;
}

IVector<MediaAudioTrack^>^ MediaStream::GetAudioTracks()
{
  auto ret = ref new Vector<MediaAudioTrack^>();
  for (auto track : _impl->GetAudioTracks())
  {
    ret->Append(ref new MediaAudioTrack(track));
  }
  return ret;
}

IVector<MediaVideoTrack^>^ MediaStream::GetVideoTracks()
{
  auto ret = ref new Vector<MediaVideoTrack^>();
  for (auto track : _impl->GetVideoTracks())
  {
    ret->Append(ref new MediaVideoTrack(track));
  }
  return ret;
}

IVector<IMediaStreamTrack^>^ MediaStream::GetTracks()
{
  auto ret = ref new Vector<IMediaStreamTrack^>();
  for (auto track : _impl->GetAudioTracks())
  {
    ret->Append(ref new MediaAudioTrack(track));
  }
  for (auto track : _impl->GetVideoTracks())
  {
    ret->Append(ref new MediaVideoTrack(track));
  }
  return ret;
}

// ===========================================================================

const char kAudioLabel[] = "audio_label";
const char kVideoLabel[] = "video_label";
const char kStreamLabel[] = "stream_label";

IAsyncOperation<MediaStream^>^ Media::GetUserMedia()
{
  IAsyncOperation<MediaStream^>^ asyncOp = Concurrency::create_async(
    [this]() -> MediaStream^ {

    // TODO: Check if a stream already exists.  Create only once.

    return globals::RunOnGlobalThread<MediaStream^>([this]()->MediaStream^
    {
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
      if (videoCapturer != nullptr)
      {
        rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track(
          globals::gPeerConnectionFactory->CreateVideoTrack(
          kVideoLabel,
          globals::gPeerConnectionFactory->CreateVideoSource(videoCapturer, NULL)));
        stream->AddTrack(video_track);
      }

      auto ret = ref new MediaStream(stream);
      return ret;
    });
  });

  return asyncOp;
}

IMediaSource^ Media::CreateMediaStreamSource(MediaVideoTrack^ track, uint32 width, uint32 height, uint32 framerate)
{
  return RTMediaStreamSource::CreateMediaSource(track, width, height, framerate);
}
