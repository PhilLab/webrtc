
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
#include "libyuv/video_common.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

using Microsoft::WRL::ComPtr;
using Platform::Collections::Vector;
using Windows::Media::Core::VideoStreamDescriptor;
using Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs;
using Windows::Media::Core::MediaStreamSource;
using Windows::Media::MediaProperties::VideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;

namespace {
webrtc::CriticalSectionWrapper& gMediaStreamListLock(
  *webrtc::CriticalSectionWrapper::CreateCriticalSection());
Vector<webrtc_winrt_api_internal::RTMediaStreamSource^>^ gMediaStreamList =
  ref new Vector<webrtc_winrt_api_internal::RTMediaStreamSource^>();
}  // namespace

namespace webrtc_winrt_api_internal {

MediaStreamSource^ RTMediaStreamSource::CreateMediaSource(
  MediaVideoTrack^ track, uint32 width, uint32 height, uint32 frameRate) {
  auto videoProperties =
    VideoEncodingProperties::CreateUncompressed(
    MediaEncodingSubtypes::Nv12, width, height);
  auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
  videoDesc->EncodingProperties->FrameRate->Numerator = frameRate;
  videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  auto streamSource = ref new MediaStreamSource(videoDesc);
  auto streamState = ref new RTMediaStreamSource(track);
  streamState->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(
    new RTCRenderer(streamState));
  streamState->_mediaStreamSource = streamSource;
  streamState->_mediaStreamSource->SampleRequested +=
    ref new Windows::Foundation::TypedEventHandler<MediaStreamSource ^,
      MediaStreamSourceSampleRequestedEventArgs ^>(
        streamState, &RTMediaStreamSource::OnSampleRequested);
  streamState->_mediaStreamSource->Closed +=
    ref new Windows::Foundation::TypedEventHandler<
      Windows::Media::Core::MediaStreamSource ^,
      Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(
        &webrtc_winrt_api_internal::RTMediaStreamSource::OnClosed);
  streamState->_frameRate = frameRate;
  streamState->_sourceWidth = width;
  streamState->_sourceHeight = height;
  track->SetRenderer(streamState->_rtcRenderer.get()); {
    webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
    gMediaStreamList->Append(streamState);
  }
  return streamState->_mediaStreamSource;
}

RTMediaStreamSource::RTMediaStreamSource(MediaVideoTrack^ videoTrack) :
    _videoTrack(videoTrack), _stride(0),
    _sourceWidth(0), _sourceHeight(0), _timeStamp(0), _frameRate(0) {
  InitializeCriticalSection(&_lock);
}

RTMediaStreamSource::~RTMediaStreamSource() {
  LOG(LS_INFO) << "RTMediaStreamSource::~RTMediaStreamSource";
  if (_rtcRenderer != nullptr) {
    _videoTrack->UnsetRenderer(_rtcRenderer.get());
  }
  DeleteCriticalSection(&_lock);
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
  ComPtr<IMFSample> spSample;
  hr = MFCreateSample(spSample.GetAddressOf());
  if (FAILED(hr)) {
    return;
  }
  ComPtr<IMFMediaBuffer> mediaBuffer;
  EnterCriticalSection(&_lock);
  // TODO: fix size issue
  //if (_frame.get() != nullptr)
  //{
  //  _sourceWidth = _frame->GetWidth();
  //  _sourceHeight = _frame->GetHeight();
  //}
  hr = MFCreate2DMediaBuffer(_sourceWidth, _sourceHeight, libyuv::FOURCC_NV12, FALSE,
    mediaBuffer.GetAddressOf());
  if (FAILED(hr)) {
    LeaveCriticalSection(&_lock);
    return;
  }
  spSample->AddBuffer(mediaBuffer.Get());
  spSample->SetSampleTime(0);
  if (_frame.get() != nullptr)
  {
    ConvertFrame(mediaBuffer.Get());
  }

  hr = spRequest->SetSample(spSample.Get());
  if (FAILED(hr))
  {
    LeaveCriticalSection(&_lock);
    return;
  }
  LeaveCriticalSection(&_lock);
}

void RTMediaStreamSource::ProcessReceivedFrame(
  const cricket::VideoFrame *frame) {
  EnterCriticalSection(&_lock);
    _frame.reset(frame->Copy());
  LeaveCriticalSection(&_lock);
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
    &destRawData, &pitch, &buffer, &destMediaBufferSize)))
  {
    return false;
  }
  try {
    _frame->MakeExclusive();
    // Convert to NV12
    uint8* y_buffer = _frame->GetYPlane();
    const size_t width = _frame->GetWidth();
    const int32 yPitch = _frame->GetYPitch();
    uint8* originalBuf = destRawData;
    for (size_t i = 0; i < _frame->GetHeight(); i++)
    {
      memcpy(destRawData, y_buffer, width);
      destRawData += pitch;
      y_buffer += yPitch;
    }
    destRawData = originalBuf + (pitch*_sourceHeight);
    uint8* u_buffer = _frame->GetUPlane();
    uint8* v_buffer = _frame->GetVPlane();
    const int32 uPitch = _frame->GetUPitch();
    const int32 vPitch = _frame->GetVPitch();
    const size_t uvHeight = _frame->GetHeight() / 2;
    const size_t uvWidth = _frame->GetWidth() / 2;
    for (size_t y = 0; y < uvHeight; y++)
    {
      uint8* lineBuffer = destRawData;
      for (size_t x = 0; x < uvWidth; x++)
      {
        *lineBuffer++ = u_buffer[x];
        *lineBuffer++ = v_buffer[x];
      }
      destRawData += pitch;
      u_buffer += uPitch;
      v_buffer += vPitch;
    }
  }
  catch (...) {
    LOG(LS_ERROR) << "Exception caught in RTMediaStreamSource::ConvertFrame()";
  }
  imageBuffer->Unlock2D();
  return true;
}

void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height) {
}

void RTMediaStreamSource::OnClosed(
  Windows::Media::Core::MediaStreamSource ^sender,
  Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args) {
  LOG(LS_INFO) << "RTMediaStreamSource::OnClosed";
  webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
  for (unsigned int i = 0; i < gMediaStreamList->Size; i++) {
    auto obj = gMediaStreamList->GetAt(i);
    if (obj->_mediaStreamSource == sender) {
      gMediaStreamList->RemoveAt(i);
      obj->_mediaStreamSource = nullptr;
      break;
    }
  }
}
}  // namespace webrtc_winrt_api_internal

