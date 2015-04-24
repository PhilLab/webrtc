/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "webrtc/modules/video_render/windows/video_render_winrt.h"

// System include files
#include <windows.h>

#include <windows.media.core.h>

#include <ppltasks.h>

// WebRtc include files
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/event_wrapper.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
#include "webrtc/system_wrappers/interface/trace.h"

using Microsoft::WRL::ComPtr;
using Windows::Media::Core::IMediaSource;
using Windows::UI::Xaml::Controls::MediaElement;

extern Windows::UI::Core::CoreDispatcher^ g_windowDispatcher;

namespace webrtc {

/*
 *
 *    VideoChannelWinRT
 *
 */
VideoChannelWinRT::VideoChannelWinRT(
    Windows::UI::Xaml::Controls::MediaElement^ mediaElement,
    CriticalSectionWrapper* critSect,
    Trace* trace) :
    _mediaElement(mediaElement),
    _width(0),
    _height(0),
    _bufferIsUpdated(false),
    _critSect(critSect),
    _streamId(0),
    _zOrder(0),
    _startWidth(0),
    _startHeight(0),
    _stopWidth(0),
    _stopHeight(0)
{

  VideoRenderMediaSourceWinRT::CreateInstance(&_renderMediaSource);

  ComPtr<IInspectable> inspectable;
  _renderMediaSource.As(&inspectable);

  IMediaSource^ mediaSource = safe_cast<IMediaSource^>(reinterpret_cast<Platform::Object^>(inspectable.Get()));

  Windows::UI::Core::CoreDispatcher^ dispatcher = g_windowDispatcher;
  Windows::UI::Core::CoreDispatcherPriority priority = Windows::UI::Core::CoreDispatcherPriority::Normal;
  Windows::UI::Core::DispatchedHandler^ handler = ref new Windows::UI::Core::DispatchedHandler([this, mediaSource]() {
    _mediaElement->SetMediaStreamSource(mediaSource);
  });
  Windows::Foundation::IAsyncAction^ action = dispatcher->RunAsync(priority, handler);
  Concurrency::create_task(action).wait();
}

  VideoChannelWinRT::~VideoChannelWinRT()
{
}

void VideoChannelWinRT::SetStreamSettings(uint16_t streamId,
    uint32_t zOrder,
    float startWidth,
    float startHeight,
    float stopWidth,
    float stopHeight)
{
  _streamId = streamId;
  _zOrder = zOrder;
  _startWidth = startWidth;
  _startHeight = startHeight;
  _stopWidth = stopWidth;
  _stopHeight = stopHeight;
}

int VideoChannelWinRT::GetStreamSettings(uint16_t streamId,
    uint32_t& zOrder,
    float& startWidth,
    float& startHeight,
    float& stopWidth,
    float& stopHeight)
{
  streamId = _streamId;
  zOrder = _zOrder;
  startWidth = _startWidth;
  startHeight = _startHeight;
  stopWidth = _stopWidth;
  stopHeight = _stopHeight;
  return 0;
}

Microsoft::WRL::ComPtr<VideoRenderMediaSourceWinRT> VideoChannelWinRT::GetMediaSource()
{
  return _renderMediaSource;
}

void VideoChannelWinRT::Lock()
{
  _critSect->Enter();
}

void VideoChannelWinRT::Unlock()
{
  _critSect->Leave();
}

webrtc::I420VideoFrame& VideoChannelWinRT::GetVideoFrame()
{
  return _videoFrame;
}

int VideoChannelWinRT::GetWidth()
{
  return _width;
}

int VideoChannelWinRT::GetHeight()
{
  return _height;
}

// Called from video engine when a the frame size changed
int VideoChannelWinRT::FrameSizeChange(int width, int height, int numberOfStreams)
{
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, -1,
    "FrameSizeChange, width: %d, height: %d, streams: %d", width,
    height, numberOfStreams);

  CriticalSectionScoped cs(_critSect);
  _width = width;
  _height = height;

  _renderMediaSource->FrameSizeChange(width, height);

  ComPtr<IInspectable> inspectable;
  _renderMediaSource.As(&inspectable);

  IMediaSource^ mediaSource = safe_cast<IMediaSource^>(reinterpret_cast<Platform::Object^>(inspectable.Get()));

  Windows::UI::Core::CoreDispatcher^ dispatcher = g_windowDispatcher;
  Windows::UI::Core::CoreDispatcherPriority priority = Windows::UI::Core::CoreDispatcherPriority::Normal;
  Windows::UI::Core::DispatchedHandler^ handler = ref new Windows::UI::Core::DispatchedHandler([this, mediaSource]() {
    _mediaElement->SetMediaStreamSource(mediaSource);
    _mediaElement->Play();
  });
  Windows::Foundation::IAsyncAction^ action = dispatcher->RunAsync(priority, handler);
  Concurrency::create_task(action).wait();

  return 0;
}

int32_t VideoChannelWinRT::RenderFrame(const uint32_t streamId,
  const I420VideoFrame& videoFrame)
{
  if (_width != videoFrame.width() || _height != videoFrame.height())
  {
    if (FrameSizeChange(videoFrame.width(), videoFrame.height(), 1) == -1)
    {
      return -1;
    }
  }
  return DeliverFrame(videoFrame);
}

// Called from video engine when a new frame should be rendered.
int VideoChannelWinRT::DeliverFrame(const I420VideoFrame& videoFrame) {
  WEBRTC_TRACE(kTraceStream, kTraceVideo, -1,
    "DeliverFrame to VideoChannelWinRT");

  CriticalSectionScoped cs(_critSect);

  // FIXME if _bufferIsUpdated is still true (not be renderred), do we want to
  // update the texture? probably not
  if (_bufferIsUpdated) {
    WEBRTC_TRACE(kTraceStream, kTraceVideo, -1,
      "Last frame hasn't been rendered yet. Drop this frame.");
    return -1;
  }

  _videoFrame.CopyFrame(videoFrame);

  _bufferIsUpdated = true;
  return 0;
}

// Called by channel owner to indicate the frame has been rendered off
int VideoChannelWinRT::RenderOffFrame()
{
  WEBRTC_TRACE(kTraceStream, kTraceVideo, -1,
    "Frame has been rendered to the screen.");
  CriticalSectionScoped cs(_critSect);
  _bufferIsUpdated = false;
  return 0;
}

// Called by channel owner to check if the buffer is updated
int VideoChannelWinRT::IsUpdated(bool& isUpdated)
{
  CriticalSectionScoped cs(_critSect);
  isUpdated = _bufferIsUpdated;
  return 0;
}

/*
 *
 *    VideoRenderWinRT
 *
 */
VideoRenderWinRT::VideoRenderWinRT(Trace* trace,
                                   void* hWnd,
                                   bool fullScreen) :
    _refCritsect(*CriticalSectionWrapper::CreateCriticalSection()),
    _trace(trace),
    _hWnd(hWnd),
    _fullScreen(fullScreen),
    _screenUpdateThread(),
    _screenUpdateEvent(NULL),
    _channel(NULL),
    _winWidth(0),
    _winHeight(0) {
  _screenUpdateThread = ThreadWrapper::CreateThread(ScreenUpdateThreadProc,
    this, "VideoRenderWinRT");
  _screenUpdateEvent = EventTimerWrapper::Create();
}

VideoRenderWinRT::~VideoRenderWinRT() {
  rtc::scoped_ptr<ThreadWrapper> tmpPtr(_screenUpdateThread.release());
  if (tmpPtr)
  {
    _screenUpdateEvent->Set();
    _screenUpdateEvent->StopTimer();

    tmpPtr->Stop();
  }
  delete _screenUpdateEvent;

  delete &_refCritsect;
}

int32_t VideoRenderWinRT::Init() {

  CriticalSectionScoped cs(&_refCritsect);

  // Start rendering thread...
  if (!_screenUpdateThread)
  {
    WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Thread not created");
    return -1;
  }
  _screenUpdateThread->Start();
  _screenUpdateThread->SetPriority(kRealtimePriority);

  // Start the event triggering the render process
  unsigned int monitorFreq = 60;
  _screenUpdateEvent->StartTimer(true, 1000 / monitorFreq);

  return 0;
}

int32_t VideoRenderWinRT::ChangeWindow(void* window) {
  return -1;
}

int VideoRenderWinRT::UpdateRenderSurface() {
  CriticalSectionScoped cs(&_refCritsect);

  // Check if there are any updated buffers
  bool updated = false;
  if (_channel == NULL)
    return -1;
  _channel->IsUpdated(updated);
  //nothing is updated, return
  if (!updated)
    return -1;

  _channel->Lock();
  const uint8_t* buf = _channel->GetVideoFrame().buffer(kYPlane);
  int siz = _channel->GetVideoFrame().allocated_size(kYPlane);
  WEBRTC_TRACE(kTraceInfo, kTraceVideo, -1, "UpdateRenderSurface - %d, %d", buf, siz);
  _channel->GetMediaSource()->ProcessVideoFrame(_channel->GetVideoFrame());
  _channel->Unlock();

  //Notice channel that this frame as been rendered
  _channel->RenderOffFrame();

  return 0;
}

/*
 *
 *    Rendering process
 *
 */
bool VideoRenderWinRT::ScreenUpdateThreadProc(void* obj) {
  return static_cast<VideoRenderWinRT*> (obj)->ScreenUpdateProcess();
}

bool VideoRenderWinRT::ScreenUpdateProcess() {

  _screenUpdateEvent->Wait(100);

  if (!_screenUpdateThread)
  {
    //stop the thread
    return false;
  }

  HRESULT hr = S_OK;

  if (SUCCEEDED(hr))
  {
    UpdateRenderSurface();
  }

  return true;
}

VideoRenderCallback* VideoRenderWinRT::CreateChannel(
    const uint32_t channel,
    const uint32_t zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom) {

  CriticalSectionScoped cs(&_refCritsect);

  ComPtr<IInspectable> mediaElementPtr((IInspectable*)_hWnd);

  MediaElement^ mediaElement = safe_cast<MediaElement^>(reinterpret_cast<Platform::Object^>(mediaElementPtr.Get()));

  VideoChannelWinRT* pChannel = new VideoChannelWinRT(mediaElement, &_refCritsect, _trace);
  pChannel->SetStreamSettings(0, zOrder, left, top, right, bottom);

  // store channel
  _channel = pChannel;

  return pChannel;
}

int32_t VideoRenderWinRT::DeleteChannel(const uint32_t streamId) {

  CriticalSectionScoped cs(&_refCritsect);

  delete _channel;
  _channel = NULL;

  return 0;
}

int32_t VideoRenderWinRT::StartRender() {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::StopRender() {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

bool VideoRenderWinRT::IsFullScreen() {
  return _fullScreen;
}

int32_t VideoRenderWinRT::SetCropping(
    const uint32_t channel,
    const uint16_t streamId,
    const float left,
    const float top,
    const float right,
    const float bottom) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::SetTransparentBackground(
    const bool enable) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::SetText(const uint8_t textId,
    const uint8_t* text,
    const int32_t textLength,
    const uint32_t colorText,
    const uint32_t colorBg,
    const float left,
    const float top,
    const float rigth,
    const float bottom) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::SetBitmap(
    const void* bitMap,
    const uint8_t pictureId,
    const void* colorKey,
    const float left,
    const float top,
    const float right,
    const float bottom) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::GetGraphicsMemory(uint64_t& totalMemory,
    uint64_t& availableMemory) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

int32_t VideoRenderWinRT::ConfigureRenderer(
    const uint32_t channel,
    const uint16_t streamId,
    const unsigned int zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom) {
  WEBRTC_TRACE(kTraceError, kTraceVideo, -1, "Not supported.");
  return 0;
}

}  // namespace webrtc
