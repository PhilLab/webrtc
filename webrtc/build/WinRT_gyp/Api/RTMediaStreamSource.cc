
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
  MediaVideoTrack^ track, uint32 frameRate) {
  auto streamState = ref new RTMediaStreamSource(track);
  streamState->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(
    new RTCRenderer(streamState));
  track->SetRenderer(streamState->_rtcRenderer.get());
  size_t width;
  size_t height;
  streamState->GetSourceDimensions(width, height);
  auto videoProperties =
    VideoEncodingProperties::CreateUncompressed(
    MediaEncodingSubtypes::Nv12, width, height);
  auto videoDesc = ref new VideoStreamDescriptor(videoProperties);
  videoDesc->EncodingProperties->FrameRate->Numerator = frameRate;
  videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  auto streamSource = ref new MediaStreamSource(videoDesc);
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
  track->SetRenderer(streamState->_rtcRenderer.get());
  {
    webrtc::CriticalSectionScoped cs(&gMediaStreamListLock);
    gMediaStreamList->Append(streamState);
  }
  return streamState->_mediaStreamSource;
}

RTMediaStreamSource::RTMediaStreamSource(MediaVideoTrack^ videoTrack) :
    _videoTrack(videoTrack), _stride(0),
    _sourceWidth(0), _sourceHeight(0), _timeStamp(0), _frameRate(0) {
  InitializeCriticalSection(&_lock);
  _firstFrameEvent = CreateEvent(nullptr, true, false, nullptr);
}

RTMediaStreamSource::~RTMediaStreamSource() {
  if (_rtcRenderer != nullptr) {
    _videoTrack->UnsetRenderer(_rtcRenderer.get());
  }
  DeleteCriticalSection(&_lock);
  CloseHandle(_firstFrameEvent);
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
  EnterCriticalSection(&_lock);
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
  if (_sourceWidth == 0)
  {
    _sourceWidth = frame->GetWidth();
    _sourceHeight = frame->GetHeight();
    SetEvent(_firstFrameEvent);
  }
  if ((frame->GetWidth() == _sourceWidth) && (frame->GetHeight() == _sourceHeight))
  {
    _frame.reset(frame->Copy());
  }
  else
  {
    _frame.reset(frame->Stretch(_sourceWidth, _sourceHeight, true, false));
  }
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
  imageBuffer->Unlock2D();
  return true;
}

void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height) {
}

void RTMediaStreamSource::GetSourceDimensions(size_t& width, size_t& height)
{
  WaitForSingleObject(_firstFrameEvent, INFINITE);
  width = _sourceWidth;
  height = _sourceHeight;
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

