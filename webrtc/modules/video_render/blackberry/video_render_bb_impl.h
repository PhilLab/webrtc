/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_BB_VIDEO_RENDER_BB_IMPL_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_BB_VIDEO_RENDER_BB_IMPL_H_

#include "i_video_render.h"
#include "common_types.h"
#include "map_wrapper.h"
#include <bps/bps.h>
#include <bps/screen.h>

struct _screen_context;
struct _screen_window;
struct _screen_display;
typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLContext;
typedef void *EGLSurface;

namespace webrtc {

class CriticalSectionWrapper;
class EventWrapper;
class ThreadWrapper;
class BlackberryWindowWrapper;
class VideoRenderBlackBerry;
class VideoRenderOpenGles20;

// The object a module user uses to send new frames to the Blackberry OpenGL ES window

class BlackberryRenderCallback : public VideoRenderCallback {
 public:
  BlackberryRenderCallback(VideoRenderBlackBerry* parentRenderer,
                           uint32_t streamId);

  virtual int32_t RenderFrame(const uint32_t streamId,
		  	  	  	  	  	  	  	I420VideoFrame& videoFrame);

  virtual ~BlackberryRenderCallback() {};

  void RenderToGL();

  VideoRenderOpenGles20* GetRenderer() { return _ptrOpenGLRenderer; }

 private:
  VideoRenderBlackBerry* _ptrParentRenderer;
  I420VideoFrame _videoFrame;
  bool _hasFrame;
  bool _frameIsRendered;
  bool _isSetup;
  VideoRenderOpenGles20* _ptrOpenGLRenderer;
  CriticalSectionWrapper& _critSect;
};

class VideoRenderBlackBerry : IVideoRender
{
 public:
  static int32_t SetAndroidEnvVariables(void* javaVM);

  VideoRenderBlackBerry(const int32_t id,
                     const VideoRenderType videoRenderType,
                     void* window,
                     const bool fullscreen);

  virtual ~VideoRenderBlackBerry();

  virtual int32_t Init();

  virtual int32_t ChangeUniqueId(const int32_t id);

  virtual int32_t ChangeWindow(void* window);

  virtual VideoRenderCallback* AddIncomingRenderStream(
      const uint32_t streamId,
      const uint32_t zOrder,
      const float left, const float top,
      const float right, const float bottom);

  virtual int32_t DeleteIncomingRenderStream(
      const uint32_t streamId);

  virtual int32_t GetIncomingRenderStreamProperties(
      const uint32_t streamId,
      uint32_t& zOrder,
      float& left, float& top,
      float& right, float& bottom) const;

  virtual int32_t StartRender();

  virtual int32_t StopRender();

  virtual void ReDraw();

  virtual void OnBBRenderEvent();

  // Properties

  virtual VideoRenderType RenderType();

  virtual RawVideoType PerferedVideoType();

  virtual bool FullScreen();

  virtual int32_t GetGraphicsMemory(
      uint64_t& totalGraphicsMemory,
      uint64_t& availableGraphicsMemory) const;

  virtual int32_t GetScreenResolution(
      uint32_t& screenWidth,
      uint32_t& screenHeight) const;

  virtual uint32_t RenderFrameRate(const uint32_t streamId);

  virtual int32_t SetStreamCropping(const uint32_t streamId,
                                          const float left, const float top,
                                          const float right,
                                          const float bottom);

  virtual int32_t SetTransparentBackground(const bool enable);

  virtual int32_t ConfigureRenderer(const uint32_t streamId,
                                          const unsigned int zOrder,
                                          const float left, const float top,
                                          const float right,
                                          const float bottom);

  virtual int32_t SetText(const uint8_t textId,
                                const uint8_t* text,
                                const int32_t textLength,
                                const uint32_t textColorRef,
                                const uint32_t backgroundColorRef,
                                const float left, const float top,
                                const float rigth, const float bottom);

  virtual int32_t SetBitmap(const void* bitMap,
                                  const uint8_t pictureId,
                                  const void* colorKey, const float left,
                                  const float top, const float right,
                                  const float bottom);

 protected:

  bool CreateGLThread();
  static bool GLThread(void* pThis);
  bool GLThreadRun();

  bool CreateGLWindow();
  bool CleanUpGLWindow();

  virtual BlackberryRenderCallback* CreateRenderChannel(
      int32_t streamId,
      int32_t zOrder,
      const float left,
      const float top,
      const float right,
      const float bottom,
      VideoRenderBlackBerry& renderer);

//  screen_window_t

  int32_t _id;
  CriticalSectionWrapper& _critSect;
  VideoRenderType _renderType;
  BlackberryWindowWrapper* _ptrWindowWrapper;
  screen_context_t _screen_ctx;
  screen_window_t _ptrGLWindow;
  screen_display_t _ptrDisplay;
  EGLDisplay _eglDisplay;
  EGLConfig _eglConfig;
  EGLContext _eglContext;
  EGLSurface _eglSurface;
  ThreadWrapper* _GLRenderThreadPtr;

  int _windowWidth;
  int _windowHeight;
  bool _glInitialized;
  bool _started;
  bool _stopped;


 private:

  // Map with streams to render.
  MapWrapper _streamsMap;
};

} //namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_BB_VIDEO_RENDER_BB_IMPL_H_
