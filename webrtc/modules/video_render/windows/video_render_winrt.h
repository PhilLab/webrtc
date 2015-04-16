/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_
#define WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_

#include <Map>

#include <windows.ui.xaml.controls.h>

#include <wrl/implements.h>

#include "webrtc/modules/video_render/windows/i_video_render_win.h"
#include "webrtc/modules/video_render/include/video_render_defines.h"
#include "webrtc/modules/video_render/windows/video_render_source_winrt.h"

namespace webrtc {
class CriticalSectionWrapper;
class EventWrapper;
class Trace;
class ThreadWrapper;

class VideoChannelWinRT : public VideoRenderCallback
{
public:
  VideoChannelWinRT(Windows::UI::Xaml::Controls::MediaElement^ mediaElement, CriticalSectionWrapper* critSect, Trace* trace);

  virtual ~VideoChannelWinRT();

  // Inherited from VideoRencerCallback, called from VideoAPI class.
  // Called when the incomming frame size and/or number of streams in mix changes
  virtual int FrameSizeChange(int width, int height, int numberOfStreams);

  // A new frame is delivered.
  virtual int DeliverFrame(const I420VideoFrame& videoFrame);
  virtual int32_t RenderFrame(const uint32_t streamId,
    I420VideoFrame& videoFrame);

  // Called to check if the video frame is updated.
  int IsUpdated(bool& isUpdated);
  // Called after the video frame has been render to the screen
  int RenderOffFrame();

  Microsoft::WRL::ComPtr<VideoRenderMediaSourceWinRT> GetMediaSource();
  void Lock();
  void Unlock();
  webrtc::I420VideoFrame& GetVideoFrame();
  int GetWidth();
  int GetHeight();

  void SetStreamSettings(uint16_t streamId,
    uint32_t zOrder,
    float startWidth,
    float startHeight,
    float stopWidth,
    float stopHeight);
  int GetStreamSettings(uint16_t streamId,
    uint32_t& zOrder,
    float& startWidth,
    float& startHeight,
    float& stopWidth,
    float& stopHeight);

private:
  //critical section passed from the owner
  CriticalSectionWrapper* _critSect;

  Windows::UI::Xaml::Controls::MediaElement^ _mediaElement;
  Microsoft::WRL::ComPtr<VideoRenderMediaSourceWinRT> _renderMediaSource;

  webrtc::I420VideoFrame _videoFrame;

  bool _bufferIsUpdated;
  // the frame size
  int _width;
  int _height;
  //sream settings
  //TODO support multiple streams in one channel
  uint16_t _streamId;
  uint32_t _zOrder;
  float _startWidth;
  float _startHeight;
  float _stopWidth;
  float _stopHeight;
};

class VideoRenderWinRT : IVideoRenderWin {
 public:
  VideoRenderWinRT(Trace* trace, void* hWnd, bool fullScreen);
  ~VideoRenderWinRT();

 public:
  // IVideoRenderWin

  /**************************************************************************
  *
  *   Init
  *
  ***************************************************************************/
  virtual int32_t Init();

  /**************************************************************************
  *
  *   Incoming Streams
  *
  ***************************************************************************/
  virtual VideoRenderCallback* CreateChannel(
      const uint32_t streamId,
      const uint32_t zOrder,
      const float left,
      const float top,
      const float right,
      const float bottom);

  virtual int32_t DeleteChannel(const uint32_t streamId);

  virtual int32_t GetStreamSettings(
    const uint32_t channel,
    const uint16_t streamId,
    uint32_t& zOrder,
    float& left,
    float& top,
    float& right,
    float& bottom) {
    // TODO (WinRT) This is a dummy body to be fixed
    return 1;
  }

  /**************************************************************************
  *
  *   Start/Stop
  *
  ***************************************************************************/

  virtual int32_t StartRender();
  virtual int32_t StopRender();
  /**************************************************************************
  *
  *   Properties
  *
  ***************************************************************************/

  virtual bool IsFullScreen();

  virtual int32_t SetCropping(
      const uint32_t channel,
      const uint16_t streamId,
      const float left,
      const float top,
      const float right,
      const float bottom);

  virtual int32_t ConfigureRenderer(
      const uint32_t channel,
      const uint16_t streamId,
      const unsigned int zOrder,
      const float left,
      const float top,
      const float right,
      const float bottom);

  virtual int32_t SetTransparentBackground(const bool enable);

  virtual int32_t ChangeWindow(void* window);

  virtual int32_t GetGraphicsMemory(uint64_t& totalMemory,
                                    uint64_t& availableMemory);

  virtual int32_t SetText(
      const uint8_t textId,
      const uint8_t* text,
      const int32_t textLength,
      const uint32_t colorText,
      const uint32_t colorBg,
      const float left,
      const float top,
      const float rigth,
      const float bottom);

  virtual int32_t SetBitmap(
      const void* bitMap,
      const uint8_t pictureId,
      const void* colorKey,
      const float left,
      const float top,
      const float right,
      const float bottom);

 public:
  int UpdateRenderSurface();

 protected:
  // The thread rendering the screen
  static bool ScreenUpdateThreadProc(void* obj);
  bool ScreenUpdateProcess();

 private:
  CriticalSectionWrapper& _refCritsect;
  Trace* _trace;
  ThreadWrapper* _screenUpdateThread;
  EventWrapper* _screenUpdateEvent;

  VideoChannelWinRT* _channel;

  void* _hWnd;
  bool _fullScreen;
  int _winWidth;
  int _winHeight;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_
