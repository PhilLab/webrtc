#include "Media.h"
#include "peerconnectioninterface.h"
#include "Marshalling.h"
#include "webrtc/base/logging.h"
#include "talk/media/devices/devicemanager.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "talk/media/base/videoframe.h"
#include "libyuv/video_common.h"
#include <ppltasks.h>
#include <mfapi.h>

using namespace webrtc_winrt_api;
using namespace webrtc_winrt_api_internal;
using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::Media::MediaProperties;

FrameBuffer::FrameBuffer() : _width(0), _height(0), _stride(0), _buffer(0), _bufferSize(0), _timeStamp(0)
{
  InitializeCriticalSection(&_critical);
}

FrameBuffer::~FrameBuffer()
{
  DeleteCriticalSection(&_critical);
  if (_buffer)
  {
    delete[] _buffer;
  }
}

void FrameBuffer::Initialize(int width, int height)
{
  _width = width;
  _height = height;
  MFCreateVideoSampleAllocatorEx(IID_PPV_ARGS(_spSampleAllocator.ReleaseAndGetAddressOf()));
}

void FrameBuffer::SetFrame(const cricket::VideoFrame *frame)
{
  EnterCriticalSection(&_critical);
  if (!_buffer)
  {
    LeaveCriticalSection(&_critical);
    return;
  }
  cricket::VideoFrame* scaledFrame = frame->Stretch(_width, _height, false, true);
  scaledFrame->ConvertToRgbBuffer(libyuv::FOURCC_ARGB, _buffer, _bufferSize, _stride);
  delete scaledFrame;
  LeaveCriticalSection(&_critical);
}

void FrameBuffer::GenerateFrame(MediaStreamSourceSampleRequest ^ request)
{
  if (request == nullptr)
  {
    return;
  }
  ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
  HRESULT hr = reinterpret_cast<IInspectable*>(request)->QueryInterface(spRequest.ReleaseAndGetAddressOf());
  if (FAILED(hr))
  {
    return;
  }
  ComPtr<IMFSample> spSample;
  hr = MFCreateSample(spSample.GetAddressOf());
  if (FAILED(hr))
  {
    return;
  }
  ComPtr<IMFMediaBuffer> mediaBuffer;
  hr = MFCreate2DMediaBuffer(_width, _height, 20, FALSE, mediaBuffer.GetAddressOf());
  if (FAILED(hr))
  {
    return;
  }
  spSample->AddBuffer(mediaBuffer.Get());
  auto spVideoStreamDescriptor = (VideoStreamDescriptor^)request->StreamDescriptor;
  auto spEncodingProperties = spVideoStreamDescriptor->EncodingProperties;
  auto spRatio = spEncodingProperties->FrameRate;
  UINT32 ui32Numerator = spRatio->Numerator;
  UINT32 ui32Denominator = spRatio->Denominator;
  ULONGLONG ulTimeSpan = ((ULONGLONG)ui32Denominator) * 10000000 / ui32Numerator;

  spSample->SetSampleDuration(ulTimeSpan);
  spSample->SetSampleTime((LONGLONG)_timeStamp);
  if (ConvertFrame(mediaBuffer.Get()))
  {
    _timeStamp += ulTimeSpan;
  }
  spRequest->SetSample(spSample.Get());
}

bool FrameBuffer::ConvertFrame(IMFMediaBuffer* mediaBuffer)
{
  ComPtr<IMF2DBuffer> imageBuffer;
  if (FAILED(mediaBuffer->QueryInterface(imageBuffer.GetAddressOf())))
  {
    return false;
  }
  BYTE* destRawData;
  LONG pitch;
  if (FAILED(imageBuffer->Lock2D(&destRawData, &pitch)))
  {
    return false;
  }
  EnterCriticalSection(&_critical);
  if (!_buffer)
  {
    _stride = (int)pitch;
    _bufferSize = _height*_stride;
    _buffer = new BYTE[_bufferSize];
    LeaveCriticalSection(&_critical);
    imageBuffer->Unlock2D();
    return false;
  }
  memcpy(destRawData, _buffer, _bufferSize);
  LeaveCriticalSection(&_critical);
  imageBuffer->Unlock2D();
  return true;
}

MediaVideoTrack::MediaVideoTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> impl) :
  _impl(impl), _videoRenderer(new RTCRenderer(_frameBuffer))
{
}

MediaVideoTrack::~MediaVideoTrack()
{
  if ((_impl.get()) && (_videoRenderer.get()))
  {
    _impl->RemoveRenderer(_videoRenderer.get());
  }
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

void MediaVideoTrack::SetRenderer(int width, int height, MediaStreamSource^ mediaSource)
{
  _videoRenderer->SetMediaSource(mediaSource);
  _frameBuffer.Initialize(width, height);
  _impl->AddRenderer(_videoRenderer.get());
}

void webrtc_winrt_api::MediaVideoTrack::OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender,
  Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args)
{
  _frameBuffer.GenerateFrame(args->Request);
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

MediaStreamSource^ Media::CreateMediaStreamSource(MediaVideoTrack^ track, int width, int height)
{
  //int aWidth = std::min(width, ((width * 9 / 16) >> 1) * 2);
  //int aHeight = std::min(height, ((height * 16 / 9) >> 1) * 2);
  int aWidth = 1280;
  int aHeight = 720;
  auto videoProperties =
    Windows::Media::MediaProperties::VideoEncodingProperties::CreateUncompressed(
    Windows::Media::MediaProperties::MediaEncodingSubtypes::Bgra8, (uint32)aWidth, (uint32)aHeight);
  auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
  videoDesc->EncodingProperties->FrameRate->Numerator = 30; // TODO: remove magic number
  videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  videoDesc->EncodingProperties->Bitrate = (uint32)(videoDesc->EncodingProperties->FrameRate->Numerator*
    videoDesc->EncodingProperties->FrameRate->Denominator * width * height * 4);
  auto ret = ref new MediaStreamSource(videoDesc);
  ret->SampleRequested += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^,
    Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^>(
    track, &webrtc_winrt_api::MediaVideoTrack::OnSampleRequested);
  ret->Starting += ref new Windows::Foundation::TypedEventHandler<Windows::Media::Core::MediaStreamSource ^, 
    Windows::Media::Core::MediaStreamSourceStartingEventArgs ^>(this, &webrtc_winrt_api::Media::OnStarting);
  TimeSpan spanBuffer;
  spanBuffer.Duration = 10000LL * 250LL;
  ret->BufferTime = spanBuffer;
  ret->CanSeek = false;
  track->SetRenderer(width, height, ret);
  return ret;
}

RTCRenderer::RTCRenderer(FrameBuffer& frameBuffer) : _frameBuffer(frameBuffer)
{
}

RTCRenderer::~RTCRenderer()
{
}

void RTCRenderer::SetSize(int width, int height, int reserved)
{
}

void RTCRenderer::RenderFrame(const cricket::VideoFrame *frame)
{
  _frameBuffer.SetFrame(frame);
}

void RTCRenderer::SetMediaSource(MediaStreamSource^ mediaSource)
{
  _mediaSource = mediaSource;
}


void webrtc_winrt_api::Media::OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args)
{
}
