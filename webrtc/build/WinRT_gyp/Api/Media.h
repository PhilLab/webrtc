#pragma once

#include <collection.h>
#include "talk/app/webrtc/peerconnectioninterface.h"
#include "talk/app/webrtc/mediastreaminterface.h"
#include "webrtc/base/scoped_ptr.h"
#include "GlobalObserver.h"
#include <mfidl.h>

using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Platform;
using namespace Platform::Collections;
using namespace Windows::Media::Core;

namespace webrtc_winrt_api
{
  class FrameBuffer
  {
  public:
    FrameBuffer();
    ~FrameBuffer();
    void Initialize(int width, int height);
    void SetFrame(const cricket::VideoFrame *frame);
    void GenerateFrame(MediaStreamSourceSampleRequest ^ request);
  private:
    bool ConvertFrame(IMFMediaBuffer* mediaBuffer);

    int _width;
    int _height;
    Microsoft::WRL::ComPtr<IMFVideoSampleAllocator> _spSampleAllocator;
    Microsoft::WRL::ComPtr<IMFMediaBuffer> _mediaBuffer;
    CRITICAL_SECTION _critical;
    int _stride;
    BYTE* _buffer;
    size_t _bufferSize;
    ULONGLONG _timeStamp;
  };
  class RTCRenderer : public webrtc::VideoRendererInterface
  {
  public:
    RTCRenderer(FrameBuffer& frameBuffer);
    virtual ~RTCRenderer();
    virtual void SetSize(int width, int height, int reserved);
    virtual void RenderFrame(const cricket::VideoFrame *frame);
    void SetMediaSource(MediaStreamSource^ mediaSource);
  private:
    FrameBuffer& _frameBuffer;
    MediaStreamSource^ _mediaSource;
  };

  public interface class IMediaStreamTrack
  {
    property String^ Kind { String^ get(); }
    property String^ Id { String^ get(); }
    property bool Enabled { bool get(); void set(bool value); }
  };

  public ref class MediaVideoTrack sealed : public IMediaStreamTrack
  {
  internal:
    MediaVideoTrack(rtc::scoped_refptr<webrtc::VideoTrackInterface> impl);
    void OnSampleRequested(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs ^args);
  public:
    virtual ~MediaVideoTrack();
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
  internal:
    void SetRenderer(int width, int height, MediaStreamSource^ mediaSource);
  private:
    FrameBuffer _frameBuffer;
    rtc::scoped_refptr<webrtc::VideoTrackInterface> _impl;
    rtc::scoped_ptr<RTCRenderer> _videoRenderer;
  };

  public ref class MediaAudioTrack sealed : public IMediaStreamTrack
  {
  internal:
    MediaAudioTrack(rtc::scoped_refptr<webrtc::AudioTrackInterface> impl);
  public:
    virtual property String^ Kind { String^ get(); }
    virtual property String^ Id { String^ get(); }
    virtual property bool Enabled { bool get(); void set(bool value); }
  private:
    rtc::scoped_refptr<webrtc::AudioTrackInterface> _impl;
  };


  public ref class MediaStream sealed
  {
  internal:
    MediaStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> impl);
    rtc::scoped_refptr<webrtc::MediaStreamInterface> GetImpl();
  public:
    IVector<MediaAudioTrack^>^ GetAudioTracks();
    IVector<MediaVideoTrack^>^ GetVideoTracks();
    IVector<IMediaStreamTrack^>^ GetTracks();
  private:
    rtc::scoped_refptr<webrtc::MediaStreamInterface> _impl;
  };

  public ref class Media sealed
  {
  public:
    // TODO: Arguments
    IAsyncOperation<MediaStream^>^ GetUserMedia();
    MediaStreamSource^ CreateMediaStreamSource(MediaVideoTrack^ track, int width, int height);
    void OnStarting(Windows::Media::Core::MediaStreamSource ^sender, Windows::Media::Core::MediaStreamSourceStartingEventArgs ^args);
  };

}
