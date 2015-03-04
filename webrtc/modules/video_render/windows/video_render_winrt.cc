#include "webrtc/modules/video_render/windows/video_render_winrt.h"

// System include files
#include <windows.h>

// WebRtc include files
#include "webrtc/common_video/libyuv/include/webrtc_libyuv.h"
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/system_wrappers/interface/event_wrapper.h"
#include "webrtc/system_wrappers/interface/thread_wrapper.h"
#include "webrtc/system_wrappers/interface/trace.h"

namespace webrtc {
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
    _screenUpdateThread(NULL),
    _screenUpdateEvent(NULL),
    _winWidth(0),
    _winHeight(0) {
}

VideoRenderWinRT::~VideoRenderWinRT() {
}

int VideoRenderWinRT::InitDevice() {
  return 0;
}

int32_t VideoRenderWinRT::Init() {
  return -1;
}

int32_t VideoRenderWinRT::ChangeWindow(void* window) {
  return -1;
}

int VideoRenderWinRT::UpdateRenderSurface() {
  return 0;
}

/*
 *
 *    Rendering process
 *
 */
bool VideoRenderWinRT::ScreenUpdateThreadProc(void* obj) {
  return false;
}

bool VideoRenderWinRT::ScreenUpdateProcess() {
  return false;
}

int VideoRenderWinRT::CloseDevice() {
  return 0;
}

int32_t VideoRenderWinRT::DeleteChannel(const uint32_t streamId) {
  return -1;
}

VideoRenderCallback* VideoRenderWinRT::CreateChannel(
    const uint32_t channel,
    const uint32_t zOrder,
    const float left,
    const float top,
    const float right,
    const float bottom) {
  return NULL;
}

int32_t VideoRenderWinRT::StartRender() {
  return 0;
}

int32_t VideoRenderWinRT::StopRender() {
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
  return 0;
}

int32_t VideoRenderWinRT::SetTransparentBackground(
    const bool enable) {
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
  return 0;
}

int32_t VideoRenderWinRT::GetGraphicsMemory(uint64_t& totalMemory,
    uint64_t& availableMemory) {
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
  return 0;
}

}  // namespace webrtc
