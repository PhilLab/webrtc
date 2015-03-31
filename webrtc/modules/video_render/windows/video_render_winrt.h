#ifndef WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_
#define WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_

#include <Map>

// WebRtc includes
#include "webrtc/modules/video_render/windows/i_video_render_win.h"

// Added
#include "webrtc/modules/video_render/include/video_render_defines.h"

namespace webrtc {
class CriticalSectionWrapper;
class EventWrapper;
class Trace;
class ThreadWrapper;

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
  int InitDevice();
  int CloseDevice();

  CriticalSectionWrapper& _refCritsect;
  Trace* _trace;
  ThreadWrapper* _screenUpdateThread;
  EventWrapper* _screenUpdateEvent;

  void* _hWnd;
  bool _fullScreen;
  int _winWidth;
  int _winHeight;
};

}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_WINRT_H_
