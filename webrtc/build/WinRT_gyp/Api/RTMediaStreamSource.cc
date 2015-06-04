
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
    MediaEncodingSubtypes::Bgra8, width, height);
  auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
  videoDesc->EncodingProperties->FrameRate->Numerator = frameRate;
  videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  videoDesc->EncodingProperties->Bitrate =
    static_cast<uint32>(videoDesc->EncodingProperties->FrameRate->Numerator *
    videoDesc->EncodingProperties->FrameRate->Denominator * width * height * 4);
  videoDesc->EncodingProperties->Width = width;
  videoDesc->EncodingProperties->Height = height;
  videoDesc->EncodingProperties->PixelAspectRatio->Numerator = 1;
  videoDesc->EncodingProperties->PixelAspectRatio->Denominator = 1;
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
  if (_rtcRenderer != nullptr) {
    _videoTrack->UnsetRenderer(_rtcRenderer.get());
  }
  DeleteCriticalSection(&_lock);
}

RTMediaStreamSource::RTCRenderer::RTCRenderer(
  RTMediaStreamSource^ streamSource) : _streamSource(streamSource) {
}

RTMediaStreamSource::RTCRenderer::~RTCRenderer() {
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
  hr = MFCreate2DMediaBuffer(_sourceWidth, _sourceHeight, 21, FALSE,
    mediaBuffer.GetAddressOf());
  if (FAILED(hr)) {
    return;
  }
  spSample->AddBuffer(mediaBuffer.Get());
  spSample->SetSampleTime(0);
  ConvertFrame(mediaBuffer.Get());
  spRequest->SetSample(spSample.Get());
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
    &destRawData, &pitch, &buffer, &destMediaBufferSize))) {
    return false;
  }
  EnterCriticalSection(&_lock);
  if (_frame.get() == nullptr) {
    _stride = static_cast<uint32>(pitch);
    LeaveCriticalSection(&_lock);
    imageBuffer->Unlock2D();
    return false;
  }
  _frame->ConvertToRgbBuffer(
    libyuv::FOURCC_ARGB, destRawData, destMediaBufferSize, _stride);
  LeaveCriticalSection(&_lock);
  imageBuffer->Unlock2D();
  return true;
}

void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height) {
}

void RTMediaStreamSource::OnClosed(
  Windows::Media::Core::MediaStreamSource ^sender,
  Windows::Media::Core::MediaStreamSourceClosedEventArgs ^args) {
  webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
  for (unsigned int i = 0; i < gMediaStreamList->Size; i++) {
    if (gMediaStreamList->GetAt(i)->Equals(sender)) {
      gMediaStreamList->RemoveAt(i);
      break;
    }
  }
}
}  // namespace webrtc_winrt_api_internal

