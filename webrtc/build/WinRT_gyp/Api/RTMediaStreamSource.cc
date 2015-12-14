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
#include "libyuv/convert.h"
#include "webrtc/system_wrappers/include/critical_section_wrapper.h"

using Microsoft::WRL::ComPtr;
using Platform::Collections::Vector;
using Windows::Media::Core::VideoStreamDescriptor;
using Windows::Media::Core::MediaStreamSourceSampleRequestedEventArgs;
using Windows::Media::Core::MediaStreamSourceSampleRequest;
using Windows::Media::Core::MediaStreamSourceStartingEventArgs;
using Windows::Media::Core::MediaStreamSource;
using Windows::Media::MediaProperties::VideoEncodingProperties;
using Windows::Media::MediaProperties::MediaEncodingSubtypes;
using Windows::System::Threading::TimerElapsedHandler;
using Windows::System::Threading::ThreadPoolTimer;

namespace webrtc_winrt_api_internal {

MediaStreamSource^ RTMediaStreamSource::CreateMediaSource(
  MediaVideoTrack^ track, uint32 frameRate, String^ id) {

  bool isH264 = track->GetImpl()->GetSource()->IsH264Source();

  auto streamState = ref new RTMediaStreamSource(track, isH264);
  streamState->_id = id;
  streamState->_rtcRenderer = rtc::scoped_ptr<RTCRenderer>(
    new RTCRenderer(streamState));
  track->SetRenderer(streamState->_rtcRenderer.get());
  VideoEncodingProperties^ videoProperties;
  if (isH264) {
    videoProperties = VideoEncodingProperties::CreateH264();
    //videoProperties->ProfileId = Windows::Media::MediaProperties::H264ProfileIds::Baseline;
  }
  else {
    videoProperties =
      VideoEncodingProperties::CreateUncompressed(
        MediaEncodingSubtypes::Nv12, 10, 10);
  }
  streamState->_videoDesc = ref new VideoStreamDescriptor(videoProperties);

  // initial value, this will be override by incoming frame from webrtc.
  // this is needed since the UI element might request sample before webrtc has
  // incoming frame ready(ex.: remote stream), in this case, this initial value
  // will make sure we will at least create a small dummy frame.
  streamState->_videoDesc->EncodingProperties->Width = 720;
  streamState->_videoDesc->EncodingProperties->Height = 1280;

  webrtc_winrt_api::ResolutionHelper::FireEvent(id,
    streamState->_videoDesc->EncodingProperties->Width,
    streamState->_videoDesc->EncodingProperties->Height);

  streamState->_videoDesc->EncodingProperties->FrameRate->Numerator =
                                                                    frameRate;
  streamState->_videoDesc->EncodingProperties->FrameRate->Denominator = 1;
  auto streamSource = ref new MediaStreamSource(streamState->_videoDesc);

  streamState->_startTime = webrtc::TickTime::Now();

  auto startingCookie = streamSource->Starting +=
    ref new Windows::Foundation::TypedEventHandler<
    MediaStreamSource ^,
    MediaStreamSourceStartingEventArgs ^>([streamState](
      MediaStreamSource^ sender,
      MediaStreamSourceStartingEventArgs^ args) {
    // Get a deferall on the starting event so we can trigger it
    // when the first frame arrives.
    streamState->_startingDeferral = args->Request->GetDeferral();
  });

  streamState->_mediaStreamSource = streamSource;

  // Use a lambda to capture a strong reference to RTMediaStreamSource.
  // This is the only way to tie the lifetime of the RTMediaStreamSource
  // to that of the MediaStreamSource.
  auto sampleRequestedCookie = streamSource->SampleRequested +=
    ref new Windows::Foundation::TypedEventHandler<
    MediaStreamSource^,
    MediaStreamSourceSampleRequestedEventArgs^>([streamState](
        MediaStreamSource^ sender,
        MediaStreamSourceSampleRequestedEventArgs^ args) {
    streamState->OnSampleRequested(sender, args);
  });
  streamSource->Closed +=
    ref new Windows::Foundation::TypedEventHandler<
      Windows::Media::Core::MediaStreamSource^,
      Windows::Media::Core::MediaStreamSourceClosedEventArgs ^>(
        [streamState, startingCookie, sampleRequestedCookie](
        Windows::Media::Core::MediaStreamSource^ sender,
        Windows::Media::Core::MediaStreamSourceClosedEventArgs^ args) {
    LOG(LS_INFO) << "RTMediaStreamSource::OnClosed";
    streamState->Teardown();
    sender->Starting -= startingCookie;
    sender->SampleRequested -= sampleRequestedCookie;
  });

  // Create a timer which sends request progress periodically.
  {
    auto handler = ref new TimerElapsedHandler(streamState, &RTMediaStreamSource::ProgressTimerElapsedExecute);
    auto timespan = Windows::Foundation::TimeSpan();
    timespan.Duration = 500 * 1000 * 10;  // 500 ms in hns
    streamState->_progressTimer = ThreadPoolTimer::CreatePeriodicTimer(handler, timespan);
  }

  // Create a timer which ensures we don't display frames faster that expected.
  // Required because Media Foundation sometimes requests samples in burst mode
  // but we use the wall clock to drive timestamps.
  {
    auto handler = ref new TimerElapsedHandler(streamState, &RTMediaStreamSource::FPSTimerElapsedExecute);
    auto timespan = Windows::Foundation::TimeSpan();
    timespan.Duration = 15 * 1000 * 10;
    streamState->_fpsTimer = ThreadPoolTimer::CreatePeriodicTimer(handler, timespan);
  }

  return streamSource;
}

RTMediaStreamSource::RTMediaStreamSource(MediaVideoTrack^ videoTrack, bool isH264) :
    _videoTrack(videoTrack), _stride(0),
    _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection()),
    _frameCounter(0), _isH264(isH264), _futureOffsetMs(150), _lastSampleTime(0),
    _frameSentThisTime(false), _isFirstFrame(true),
    _lastTimeFPSCalculated(webrtc::TickTime::Now()) {
  LOG(LS_INFO) << "RTMediaStreamSource::RTMediaStreamSource";
}

RTMediaStreamSource::~RTMediaStreamSource() {
  LOG(LS_INFO) << "RTMediaStreamSource::~RTMediaStreamSource : " << rtc::ToUtf8(_id->Data()).c_str();
  Teardown();
}

void RTMediaStreamSource::Teardown() {
  LOG(LS_INFO) << "RTMediaStreamSource::Teardown()";
  webrtc::CriticalSectionScoped csLock(_lock.get());
  if (_progressTimer != nullptr) {
    _progressTimer->Cancel();
    _progressTimer = nullptr;
  }
  if (_fpsTimer != nullptr) {
    _fpsTimer->Cancel();
    _fpsTimer = nullptr;
  }
  if (_rtcRenderer != nullptr && _videoTrack != nullptr) {
    _videoTrack->UnsetRenderer(_rtcRenderer.get());
  }
  _videoTrack = nullptr;
  _rtcRenderer.reset();

  _request = nullptr;
  if (_deferral != nullptr) {
    _deferral->Complete();
    _deferral = nullptr;
  }
  if (_startingDeferral != nullptr) {
    _startingDeferral->Complete();
    _startingDeferral = nullptr;
  }

  // Clear the buffered frames.
  while (!_frames.empty()) {
    rtc::scoped_ptr<cricket::VideoFrame> frame(_frames.front());
    _frames.pop_front();
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

  auto frameCopy = frame->Copy();
  // Do the processing async because there's a risk of a deadlock otherwise.
  Concurrency::create_async([this, frameCopy] {
    auto stream = _streamSource.Resolve<RTMediaStreamSource>();
    if (stream != nullptr) {
      stream->ProcessReceivedFrame(frameCopy);
    }
  });

}

// Guid to cache the IDR check result in the sample attributes.
const GUID GUID_IS_IDR = { 0x588e319a, 0x218c, 0x4d0d, { 0x99, 0x6e, 0x77, 0x96, 0xb1, 0x46, 0x3e, 0x7e } };

bool IsSampleIDR(IMFSample* sample) {
  ComPtr<IMFAttributes> sampleAttributes;
  sample->QueryInterface<IMFAttributes>(&sampleAttributes);

  UINT32 isIdr;
  if (SUCCEEDED(sampleAttributes->GetUINT32(GUID_IS_IDR, &isIdr))) {
    return isIdr > 0;
  }

  ComPtr<IMFMediaBuffer> pBuffer;
  sample->GetBufferByIndex(0, &pBuffer);
  BYTE* pBytes;
  DWORD maxLength, curLength;
  if (FAILED(pBuffer->Lock(&pBytes, &maxLength, &curLength))) {
    return false;
  }

  // Search for the beginnings of nal units.
  for (DWORD i = 0; i < curLength - 5; ++i) {
    BYTE* ptr = pBytes + i;
    int prefixLengthFound = 0;
    if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01) {
      prefixLengthFound = 4;
    }
    else if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01) {
      prefixLengthFound = 3;
    }

    if (prefixLengthFound > 0 && (ptr[prefixLengthFound] & 0x1f) == 0x05) {
      // Found IDR NAL unit
      pBuffer->Unlock();
      sampleAttributes->SetUINT32(GUID_IS_IDR, 1);  // Cache result
      return true;
    }
  }
  pBuffer->Unlock();
  sampleAttributes->SetUINT32(GUID_IS_IDR, 0);  // Cache result
  return false;
}

bool RTMediaStreamSource::DropFramesToIDR() {
  cricket::VideoFrame* idrFrame = nullptr;
  // Go through the frames in reverse order (from newest to oldest) and look
  // for an IDR frame.
  for (auto it = _frames.rbegin(); it != _frames.rend(); ++it) {
    IMFSample* pSample = (IMFSample*)(*it)->GetNativeHandle();
    if (pSample == nullptr) {
      continue; // I don't expect this will ever happen.
    }

    if (IsSampleIDR(pSample)) {
      idrFrame = *it;
      break;
    }
  }

  // If we have an IDR frame, drop all older frames.
  if (idrFrame != nullptr) {
    OutputDebugString(L"IDR found, dropping all other samples.\n");
    while (!_frames.empty()) {
      if (_frames.front() == idrFrame) {
        break;
      }
      auto frame = _frames.front();
      _frames.pop_front();
      delete frame;
    }
  }
  return idrFrame != nullptr;
}

LONGLONG RTMediaStreamSource::GetNextSampleTimeHns() {
  webrtc::TickTime now = webrtc::TickTime::Now();

  LONGLONG frameTime = ((now - _startTime).Milliseconds() + _futureOffsetMs) * 1000 * 10;

  // Sometimes we get requests so fast they have identical timestamp.
  // Add a bit to the timetamp so it's different from the last sample.
  if (_lastSampleTime >= frameTime) {
    LOG(LS_INFO) << "!!!!! Bad sample time " << _lastSampleTime << "->" << frameTime;
    frameTime = _lastSampleTime + 500;  // Make the timestamp slightly after the last one.
  }

  _lastSampleTime = frameTime;

  return frameTime;
}

void RTMediaStreamSource::ProgressTimerElapsedExecute(ThreadPoolTimer^ source) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  if (_request != nullptr) {
    _request->ReportSampleProgress(1);
  }
}

void RTMediaStreamSource::FPSTimerElapsedExecute(ThreadPoolTimer^ source) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  _frameSentThisTime = false;
  if (_frames.size() > 0 && _request != nullptr) {
    if (_isH264) {
      ReplyToRequestH264();
    }
    else {
      ReplyToRequestI420();
    }
  }
}

void RTMediaStreamSource::ReplyToRequestH264() {
  // Scan for IDR.  If we find one, skip directly to it.
  // Scan backwards to get the latest IDR frame.
  OutputDebugString((L"Queue:" + (_frames.size().ToString()) + L"\n")->Data());
  if (_frames.size() > 30) {
    OutputDebugString(L"Frame queue > 30, scanning ahead for IDR.\n");
    LOG(LS_INFO) << "Frame queue > 30, scanning ahead for IDR: " << _frames.size();
    DropFramesToIDR();
  }

  rtc::scoped_ptr<cricket::VideoFrame> frame(_frames.front());
  _frames.pop_front();

  // Get the IMFSample in the frame.
  ComPtr<IMFSample> spSample;
  {
    IMFSample* tmp = (IMFSample*)frame->GetNativeHandle();
    if (tmp != nullptr) {
      tmp->AddRef();
      spSample.Attach(tmp);
    }
  }

  if (IsSampleIDR(spSample.Get())) {
    ComPtr<IMFAttributes> sampleAttributes;
    spSample.As(&sampleAttributes);
    sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
    // TODO(winrt): Can this help in any way?
    // sampleAttributes->SetUINT32(MFSampleExtension_Discontinuity, TRUE);
  }

  // Update rotation property
  {
    auto props = _videoDesc->EncodingProperties->Properties;

    auto lastRotation = props->HasKey(MF_MT_VIDEO_ROTATION)
      ? props->Lookup(MF_MT_VIDEO_ROTATION)
      : nullptr;

    uint32_t currentRotation = (uint32_t)frame->GetRotation();

    if (lastRotation == nullptr || (uint32_t)lastRotation != currentRotation) {
      // Only update the rotation when we have an IDR.
      if (IsSampleIDR(spSample.Get())) {
        OutputDebugString(L"Setting new rotation!!!\n");
        props->Insert(MF_MT_VIDEO_ROTATION, currentRotation);
      }
    }
  }

  LONGLONG duration = (LONGLONG)((1.0 / 30) * 1000 * 1000 * 10);
  spSample->SetSampleDuration(duration);

  LONGLONG frameTime = GetNextSampleTimeHns();
  // Set timestamp
  OutputDebugString((L"frameTime: " + frameTime + L"\n")->Data());
  spSample->SetSampleTime(frameTime);

  if (_isFirstFrame) {
    _isFirstFrame = false;
    spSample->SetSampleTime(0);
  }

  Microsoft::WRL::ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
  HRESULT hr = reinterpret_cast<IInspectable*>(_request)->QueryInterface(
    spRequest.ReleaseAndGetAddressOf());

  hr = spRequest->SetSample(spSample.Get());

  if (_deferral != nullptr) {
    _deferral->Complete();
  }

  _frameSentThisTime = true;

  UpdateFrameRate();

  _request = nullptr;
  _deferral = nullptr;
}

void RTMediaStreamSource::ReplyToRequestI420() {
  rtc::scoped_ptr<cricket::VideoFrame> frame(_frames.front());
  _frames.pop_front();

  // Update rotation property
  {
    auto props = _videoDesc->EncodingProperties->Properties;

    auto lastRotation = props->HasKey(MF_MT_VIDEO_ROTATION)
      ? props->Lookup(MF_MT_VIDEO_ROTATION)
      : nullptr;

    uint32_t currentRotation = (uint32_t)frame->GetRotation();

    if (lastRotation == nullptr || (uint32_t)lastRotation != currentRotation) {
      OutputDebugString(L"Setting new rotation!!!\n");
      props->Insert(MF_MT_VIDEO_ROTATION, currentRotation);
    }
  }

  ComPtr<IMFSample> spSample;

  HRESULT hr = MFCreateSample(spSample.GetAddressOf());
  if (FAILED(hr)) {
    if (_deferral != nullptr) {
      _deferral->Complete();
    }
    return;
  }

  ComPtr<IMFAttributes> sampleAttributes;
  spSample.As(&sampleAttributes);
  sampleAttributes->SetUINT32(MFSampleExtension_CleanPoint, TRUE);
  sampleAttributes->SetUINT32(MFSampleExtension_Discontinuity, TRUE);

  LONGLONG duration = (LONGLONG)((1.0 / 30) * 1000 * 1000 * 10);
  spSample->SetSampleDuration(duration);

  //LONGLONG sampleTime = (LONGLONG)frame->GetTimeStamp() * 10 * 1000 / 90;
  spSample->SetSampleTime(0);

  //LONGLONG frameTime = GetNextSampleTimeHns();
  //spSample->SetSampleTime(frameTime);


  ComPtr<IMFMediaBuffer> mediaBuffer;

  if (frame != nullptr) {
    if ((_videoDesc->EncodingProperties->Width != frame->GetWidth()) ||
      (_videoDesc->EncodingProperties->Height != frame->GetHeight())) {
      _videoDesc->EncodingProperties->Width =
        (unsigned int)frame->GetWidth();
      _videoDesc->EncodingProperties->Height =
        (unsigned int)frame->GetHeight();
      webrtc_winrt_api::ResolutionHelper::FireEvent(_id,
        _videoDesc->EncodingProperties->Width,
        _videoDesc->EncodingProperties->Height);
    }
  }
  hr = MFCreate2DMediaBuffer(_videoDesc->EncodingProperties->Width,
    _videoDesc->EncodingProperties->Height, cricket::FOURCC_NV12, FALSE,
    mediaBuffer.GetAddressOf());
  if (FAILED(hr)) {
    if (_deferral != nullptr) {
      _deferral->Complete();
    }
    return;
  }

  spSample->AddBuffer(mediaBuffer.Get());

  ConvertFrame(mediaBuffer.Get(), frame.get());

  Microsoft::WRL::ComPtr<IMFMediaStreamSourceSampleRequest> spRequest;
  hr = reinterpret_cast<IInspectable*>(_request)->QueryInterface(
    spRequest.ReleaseAndGetAddressOf());

  hr = spRequest->SetSample(spSample.Get());
  if (_deferral != nullptr) {
    _deferral->Complete();
  }

  _frameSentThisTime = true;

  UpdateFrameRate();

  _request = nullptr;
  _deferral = nullptr;
}

void RTMediaStreamSource::UpdateFrameRate() {
  // Do FPS calculation and notification.
  _frameCounter++;
  // If we have about a second worth of frames
  webrtc::TickTime now = webrtc::TickTime::Now();
  if ((now - _lastTimeFPSCalculated).Milliseconds() > 1000) {
    webrtc_winrt_api::FrameCounterHelper::FireEvent(_id,
      _frameCounter.ToString());
    _frameCounter = 0;
    _lastTimeFPSCalculated = now;
  }
}


void RTMediaStreamSource::OnSampleRequested(
  MediaStreamSource ^sender, MediaStreamSourceSampleRequestedEventArgs ^args) {
  // Debugging helper to see when a frame is requested.
  if (_isH264) {
    OutputDebugString(L"?");
  }
  try {
    // Check to detect cases where samples are still being requested
    // but the source has ended.
    auto trackState = _videoTrack->GetImpl()->GetSource()->state();
    if (trackState == webrtc::MediaSourceInterface::kEnded) {
      return;
    }
    if (_mediaStreamSource == nullptr)
      return;

    webrtc::CriticalSectionScoped csLock(_lock.get());

    _request = args->Request;
    if (_request == nullptr) {
      return;
    }

    webrtc::TickTime now = webrtc::TickTime::Now();

    if (_frames.size() > 0 && !_frameSentThisTime) {
      if (_isH264) {
        ReplyToRequestH264();
      }
      else {
        ReplyToRequestI420();
      }
      return;
    }
    else {
      // Save the request and referral for when a sample comes in.
      if (_deferral != nullptr) {
        LOG(LS_ERROR) << "Got referral when another hasn't completed.";
      }
      _deferral = _request->GetDeferral();
      return;
    }
  }
  catch (...) {
    LOG(LS_ERROR) << "Exception in RTMediaStreamSource::OnSampleRequested.";
  }
}

void RTMediaStreamSource::ProcessReceivedFrame(
  cricket::VideoFrame *frame) {
  // Debugging helper to see when a frame is received.
  if (_isH264) {
    OutputDebugString(L"!");
  }
  webrtc::CriticalSectionScoped csLock(_lock.get());

  if (_startingDeferral != nullptr) {
    _startTime = webrtc::TickTime::Now();
    _startingDeferral->Complete();
    _startingDeferral = nullptr;
  }

  if (_isH264) {
    // For H264 we keep all frames since they are encoded.
    _frames.push_back(frame);
  }
  else {
    // For I420 frame, keep only the latest.
    for (auto oldFrame : _frames) {
      delete oldFrame;
    }
    _frames.clear();
    _frames.push_back(frame);
  }

  // If we have a pending request, reply to it now.
  if (_deferral != nullptr && _request != nullptr && !_frameSentThisTime) {
    if (_isH264) {
      ReplyToRequestH264();
    }
    else {
      ReplyToRequestI420();
    }
  }
}

bool RTMediaStreamSource::ConvertFrame(IMFMediaBuffer* mediaBuffer, cricket::VideoFrame* frame) {
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
    frame->MakeExclusive();
    // Convert to NV12
    uint8* uvDest = destRawData + (pitch * frame->GetHeight());
    libyuv::I420ToNV12(frame->GetYPlane(), frame->GetYPitch(),
      frame->GetUPlane(), frame->GetUPitch(),
      frame->GetVPlane(), frame->GetVPitch(),
      reinterpret_cast<uint8*>(destRawData), pitch,
      uvDest, pitch,
      static_cast<int>(frame->GetWidth()), static_cast<int>(frame->GetHeight()));
  }
  catch (...) {
    LOG(LS_ERROR) << "Exception caught in RTMediaStreamSource::ConvertFrame()";
  }
  imageBuffer->Unlock2D();
  return true;
}

void RTMediaStreamSource::ResizeSource(uint32 width, uint32 height) {
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
