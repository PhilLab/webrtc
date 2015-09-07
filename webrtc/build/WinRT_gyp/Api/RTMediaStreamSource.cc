// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/RTMediaStreamSource.h"
#include <mfapi.h>
#include <ppltasks.h>
#include <mfidl.h>
#include "talk/media/base/videoframe.h"
#include "talk/app/webrtc/videosourceinterface.h"
#include "libyuv/video_common.h"
#include "libyuv/convert.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

using Microsoft::WRL::ComPtr;
using Platform::Collections::Vector;
using Windows::Media::Core::VideoStreamDescriptor;
using Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs;
using Windows::Media::Core::MediaStreamSource;
using Windows::Media::MediaProperties::VideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;

namespace webrtc_winrt_api_internal {

MediaStreamSource^ RTMediaStreamSource::CreateMediaSource(
  MediaVideoTrack^ track, uint32 frameRate, String^ id) {
  auto streamState = ref new RTMediaStreamSource(track);
  streamState->_id = id;
  streamState->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(
    new RTCRenderer(streamState));
  track->SetRenderer(streamState->_rtcRenderer.get());
  auto videoProperties =
    VideoEncodingProperties::CreateUncompressed(
    MediaEncodingSubtypes::Nv12, 10, 10);
  streamState->_videoDesc = ref new VideoStreamDescriptor(videoProperties);

  // initial value, this will be override by incoming frame from webrtc.
  // this is needed since the UI element might request sample before webrtc has
  // incoming frame ready(ex.: remote stream), in this case, this initial value
  // will make sure we will at least create a small dummy frame.
  streamState->_videoDesc->EncodingProperties->Width = 320;
  streamState->_videoDesc->EncodingProperties->Height = 240;

  webrtc_winrt_api::ResolutionHelper::FireEvent(id,
    streamState->_videoDesc->EncodingProperties->Width,
    streamState->_videoDesc->EncodingProperties->Height);

  streamState->_videoDesc->EncodingProperties->FrameRate->Numerator =
                                                                    frameRate;
  streamState->_videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  auto streamSource = ref new MediaStreamSource(streamState->_videoDesc);
  streamState->_mediaStreamSource = streamSource;
  // Use a lambda to capture a strong reference to RTMediaStreamSource.
  // This is the only way to tie the lifetime of the RTMediaStreamSource
  // to that of the MediaStreamSource.
  streamSource->SampleRequested +=
    ref new Windows::Foundation::TypedEventHandler<MediaStreamSource ^,
    MediaStreamSourceSampleRequestedEventArgs ^>([streamState](
        MediaStreamSource^ sender,
        MediaStreamSourceSampleRequestedEventArgs^ args) {
    streamState->OnSampleRequested(sender, args);
  });
  streamSource->Closed +=
    ref new Windows::Foundation::TypedEventHandler<
      Windows::Media::Core::MediaStreamSource ^,
      Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(
        &webrtc_winrt_api_internal::RTMediaStreamSource::OnClosed);
  streamState->_frameRate = frameRate;
  track->SetRenderer(streamState->_rtcRenderer.get());
  return streamSource;
}

RTMediaStreamSource::RTMediaStreamSource(MediaVideoTrack^ videoTrack) :
    _videoTrack(videoTrack), _stride(0),
    _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection()),
    _timeStamp(0), _isNewFrame(true), _frameRate(0), _frameCounter(0),
    _lastTimeFPSCalculated(webrtc::TickTime::Now()) {
}

RTMediaStreamSource::~RTMediaStreamSource() {
  LOG(LS_INFO) << "RTMediaStreamSource::~RTMediaStreamSource";
  if (_rtcRenderer != nullptr && _videoTrack != nullptr) {
    _videoTrack->UnsetRenderer(_rtcRenderer.get());
  }
}

RTMediaStreamSource::RTCRenderer::RTCRenderer(
  RTMediaStreamSource^ streamSource) : _streamSource(streamSource) {
}

RTMediaStreamSource::RTCRenderer::~RTCRenderer() {
  LOG(LS_INFO) << "RTMediaStreamSource::RTCRenderer::~RTCRenderer";
}

void RTMediaStreamSource::RTCRenderer::SetSize(
  uint32 width, uint32 height, uint32 reserved) {
  auto stream = _streamSource.Resolve<RTMediaStreamSource>();
  if (stream != nullptr) {
    stream->ResizeSource(width, height);
  }
}

void RTMediaStreamSource::RTCRenderer::RenderFrame(
  const cricket::VideoFrame *frame) {
  auto stream = _streamSource.Resolve<RTMediaStreamSource>();
  if (stream != nullptr) {
    stream->ProcessReceivedFrame(frame);
  }
}

void RTMediaStreamSource::OnSampleRequested(
  MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args) {
  try {
    // Check to detect cases where samples are still being requested
    // but the source has ended.
    auto trackState = _videoTrack->GetImpl()->GetSource()->state();
    if (trackState == webrtc::MediaSourceInterface::kEnded) {
      return;
    }
    if (_mediaStreamSource == nullptr)
      return;
    auto request = args->Request;
    if (request == nullptr) {
      return;
    }

    ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
    HRESULT hr = reinterpret_cast<IInspectable*>(request)->QueryInterface(
      spRequest.ReleaseAndGetAddressOf());
    if (FAILED(hr)) {
      return;
    }

    webrtc::TickTime now = webrtc::TickTime::Now();

    auto deferral = request->GetDeferral();

    webrtc::CriticalSectionScoped csLock(_lock.get());

    ComPtr<IMFSample> spSample;
    hr = MFCreateSample(spSample.GetAddressOf());
    if (FAILED(hr)) {
      deferral->Complete();
      return;
    }

    LONGLONG duration = (LONGLONG)((1.0 / _frameRate) * 1000 * 1000 * 10);
    spSample->SetSampleDuration(duration);
    spSample->SetSampleTime((LONGLONG)_timeStamp);
    _timeStamp += duration;

    // Do FPS calculation and notification.
    if (_isNewFrame) {
      _isNewFrame = false;
      spSample->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
      spSample->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
      _frameCounter++;
      // If we have about a second worth of frames
      if ((now - _lastTimeFPSCalculated).Milliseconds() > 1000) {
        webrtc_winrt_api::FrameCounterHelper::FireEvent(_id,
          _frameCounter.ToString());
        _frameCounter = 0;
        _lastTimeFPSCalculated = now;
      }
    }

    ComPtr<IMFMediaBuffer> mediaBuffer;

    if (_frame.get() != nullptr) {
      if ((_videoDesc->EncodingProperties->Width != _frame->GetWidth()) ||
        (_videoDesc->EncodingProperties->Height != _frame->GetHeight())) {
        _videoDesc->EncodingProperties->Width =
                    (unsigned int)_frame->GetWidth();
        _videoDesc->EncodingProperties->Height =
                    (unsigned int)_frame->GetHeight();
        webrtc_winrt_api::ResolutionHelper::FireEvent(_id,
          _videoDesc->EncodingProperties->Width,
          _videoDesc->EncodingProperties->Height);
      }
    }
    hr = MFCreate2DMediaBuffer(_videoDesc->EncodingProperties->Width,
      _videoDesc->EncodingProperties->Height, libyuv::FOURCC_NV12, FALSE,
      mediaBuffer.GetAddressOf());
    if (FAILED(hr)) {
      deferral->Complete();
      return;
    }

    spSample->AddBuffer(mediaBuffer.Get());

    if (_frame.get() != nullptr) {
      ConvertFrame(mediaBuffer.Get());
    }

    hr = spRequest->SetSample(spSample.Get());
    deferral->Complete();
    if (FAILED(hr)) {
      return;
    }
  }
  catch (...) {
    LOG(LS_ERROR) << "Exception in RTMediaStreamSource::OnSampleRequested.";
  }
}

void RTMediaStreamSource::ProcessReceivedFrame(
  const cricket::VideoFrame *frame) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  _frame.reset(frame->Copy());
  _isNewFrame = true;
}

bool RTMediaStreamSource::ConvertFrame(IMFMediaBuffer* mediaBuffer) {
    ComPtr<IMF2DBuffer2> imageBuffer;
  if (FAILED(mediaBuffer->QueryInterface(imageBuffer.GetAddressOf()))) {
    return false;
  }
  BYTE* destRawData;
  BYTE* buffer;
  LONG pitch;
  DWORD destMediaBufferSize;


  if (FAILED(imageBuffer->Lock2DSize(MF2DBuffer_LockFlags_Write,
    &destRawData, &pitch, &buffer, &destMediaBufferSize))) {
    return false;
  }
  try {
    _frame->MakeExclusive();
    // Convert to NV12
    uint8* uvDest = destRawData + (pitch * _frame->GetHeight());
    libyuv::I420ToNV12(_frame->GetYPlane(), _frame->GetYPitch(),
      _frame->GetUPlane(), _frame->GetUPitch(),
      _frame->GetVPlane(), _frame->GetVPitch(),
      reinterpret_cast<uint8*>(destRawData), pitch,
      uvDest, pitch,
      (int)_frame->GetWidth(), (int)_frame->GetHeight());
  }
  catch (...) {
    LOG(LS_ERROR) << "Exception caught in RTMediaStreamSource::ConvertFrame()";
  }
  imageBuffer->Unlock2D();
  return true;
}

void RTMediaStreamSource::BlankFrame(IMFMediaBuffer* mediaBuffer) {
  ComPtr<IMF2DBuffer2> imageBuffer;
  if (FAILED(mediaBuffer->QueryInterface(imageBuffer.GetAddressOf()))) {
    return;
  }
  BYTE* destRawData;
  BYTE* buffer;
  LONG pitch;
  DWORD destMediaBufferSize;

  if (FAILED(imageBuffer->Lock2DSize(MF2DBuffer_LockFlags_Write,
    &destRawData, &pitch, &buffer, &destMediaBufferSize))) {
    return;
  }
  memset(destRawData, 0, destMediaBufferSize);
  imageBuffer->Unlock2D();
}

void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height) {
}

void RTMediaStreamSource::OnClosed(
  Windows::Media::Core::MediaStreamSource ^sender,
  Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args) {
  LOG(LS_INFO) << "RTMediaStreamSource::OnClosed";
}
}  // namespace webrtc_winrt_api_internal

extern Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

void webrtc_winrt_api::FrameCounterHelper::FireEvent(String^ id,
  Platform::String^ str) {
  if (g_windowDispatcher != nullptr) {
    g_windowDispatcher->RunAsync(
      Windows::UI::Core::CoreDispatcherPriority::Normal,
      ref new Windows::UI::Core::DispatchedHandler([id, str] {
      FramesPerSecondChanged(id, str);
    }));
  } else {
    FramesPerSecondChanged(id, str);
  }
}

void webrtc_winrt_api::ResolutionHelper::FireEvent(String^ id,
  unsigned int width, unsigned int heigth) {
  if (g_windowDispatcher != nullptr) {
    g_windowDispatcher->RunAsync(
      Windows::UI::Core::CoreDispatcherPriority::Normal,
      ref new Windows::UI::Core::DispatchedHandler([id, width, heigth] {
      ResolutionChanged(id, width, heigth);
    }));
  } else {
    ResolutionChanged(id, width, heigth);
  }
}
