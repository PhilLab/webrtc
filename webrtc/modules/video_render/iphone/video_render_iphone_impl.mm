/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "engine_configurations.h"

#include "video_render_iphone_impl.h"
#include "critical_section_wrapper.h"
#include "video_render_uiview.h"
#include "trace.h"

namespace webrtc {

VideoRenderIPhoneImpl::VideoRenderIPhoneImpl(const int32_t id,
        const VideoRenderType videoRenderType,
        void* window,
        const bool fullscreen) :
_id(id),
_renderUIViewCritsect(*CriticalSectionWrapper::CreateCriticalSection()),
_fullScreen(fullscreen),
_ptrWindow(window)
{

    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "Constructor %s:%d", __FUNCTION__, __LINE__);
}

VideoRenderIPhoneImpl::~VideoRenderIPhoneImpl()
{
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "Destructor %s:%d", __FUNCTION__, __LINE__);
    delete &_renderUIViewCritsect;
    if (_ptrUIViewRender)
    {
        delete _ptrUIViewRender;
        _ptrUIViewRender = NULL;
    }
}

int32_t
VideoRenderIPhoneImpl::Init()
{

    CriticalSectionScoped cs(&_renderUIViewCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "%s:%d", __FUNCTION__, __LINE__);

    // cast ptrWindow from void* to CocoaRenderer. Void* was once NSOpenGLView, and CocoaRenderer is NSOpenGLView.
    _ptrUIViewRender = new VideoRenderUIView((UIImageView*)_ptrWindow, _fullScreen, _id);
    if (!_ptrWindow)
    {
        WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id, "Constructor %s:%d", __FUNCTION__, __LINE__);
        return -1;
    }
    int retVal = _ptrUIViewRender->Init();
    if (retVal == -1)
    {
        WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "Failed to init %s:%d", __FUNCTION__, __LINE__);
        return -1;
    }

    return 0;
}

int32_t
VideoRenderIPhoneImpl::ChangeUniqueId(const int32_t id)
{
    CriticalSectionScoped cs(&_renderUIViewCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "%s", __FUNCTION__);
    _id = id;

    if(_ptrUIViewRender)
    {
        _ptrUIViewRender->ChangeUniqueID(_id);
    }

    return 0;
}

int32_t
VideoRenderIPhoneImpl::ChangeWindow(void* window)
{

    CriticalSectionScoped cs(&_renderUIViewCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "%s changing ID to ", __FUNCTION__, window);

    if (window == NULL)
    {
        return -1;
    }
    _ptrWindow = window;

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s:%d", __FUNCTION__, __LINE__);

    _ptrWindow = window;
    _ptrUIViewRender->ChangeWindow((UIImageView*)_ptrWindow);

    return 0;
}

VideoRenderCallback*
VideoRenderIPhoneImpl::AddIncomingRenderStream(const uint32_t streamId,
        const uint32_t zOrder,
        const float left,
        const float top,
        const float right,
        const float bottom)
{
    CriticalSectionScoped cs(&_renderUIViewCritsect);
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s", __FUNCTION__);
    VideoChannelUIView* nsUIViewChannel = NULL;

    if(!_ptrWindow)
    {
        WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s, no window", __FUNCTION__);
    }

    if(!nsUIViewChannel)
    {
        nsUIViewChannel = _ptrUIViewRender->CreateNSGLChannel(streamId, zOrder, left, top, right, bottom);
    }

    return nsUIViewChannel;

}

int32_t
VideoRenderIPhoneImpl::DeleteIncomingRenderStream(const uint32_t streamId)
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "Constructor %s:%d", __FUNCTION__, __LINE__);
    CriticalSectionScoped cs(&_renderUIViewCritsect);
    _ptrUIViewRender->DeleteNSGLChannel(streamId);

    return 0;
}

int32_t
VideoRenderIPhoneImpl::GetIncomingRenderStreamProperties(const uint32_t streamId,
        uint32_t& zOrder,
        float& left,
        float& top,
        float& right,
        float& bottom) const
{
    return _ptrUIViewRender->GetChannelProperties(streamId, zOrder, left, top, right, bottom);
}

int32_t
VideoRenderIPhoneImpl::StartRender()
{
    return _ptrUIViewRender->StartRender();
}

int32_t
VideoRenderIPhoneImpl::StopRender()
{
    return _ptrUIViewRender->StopRender();
}

VideoRenderType
VideoRenderIPhoneImpl::RenderType()
{
    return kRenderCocoa;
}

RawVideoType
VideoRenderIPhoneImpl::PerferedVideoType()
{
    return kVideoI420;
}

bool
VideoRenderIPhoneImpl::FullScreen()
{
    return false;
}

int32_t
VideoRenderIPhoneImpl::GetGraphicsMemory(uint64_t& totalGraphicsMemory,
        uint64_t& availableGraphicsMemory) const
{
    totalGraphicsMemory = 0;
    availableGraphicsMemory = 0;
    return 0;
}

int32_t
VideoRenderIPhoneImpl::GetScreenResolution(uint32_t& screenWidth,
        uint32_t& screenHeight) const
{
    CriticalSectionScoped cs(&_renderUIViewCritsect);

    return 0;
}

uint32_t
VideoRenderIPhoneImpl::RenderFrameRate(const uint32_t streamId)
{
    CriticalSectionScoped cs(&_renderUIViewCritsect);
    return 0;
}

int32_t
VideoRenderIPhoneImpl::SetStreamCropping(const uint32_t streamId,
        const float left,
        const float top,
        const float right,
        const float bottom)
{
    return 0;
}

int32_t VideoRenderIPhoneImpl::ConfigureRenderer(const uint32_t streamId,
        const unsigned int zOrder,
        const float left,
        const float top,
        const float right,
        const float bottom)
{
    return 0;
}

int32_t
VideoRenderIPhoneImpl::SetTransparentBackground(const bool enable)
{
    return 0;
}

int32_t VideoRenderIPhoneImpl::SetText(const uint8_t textId,
        const uint8_t* text,
        const int32_t textLength,
        const uint32_t textColorRef,
        const uint32_t backgroundColorRef,
        const float left,
        const float top,
        const float right,
        const float bottom)
{
    return _ptrUIViewRender->SetText(textId, text, textLength, textColorRef, backgroundColorRef, left, top, right, bottom);
}

int32_t VideoRenderIPhoneImpl::SetBitmap(const void* bitMap,
        const uint8_t pictureId,
        const void* colorKey,
        const float left,
        const float top,
        const float right,
        const float bottom)
{
    return 0;
}

int32_t VideoRenderIPhoneImpl::FullScreenRender(void* window, const bool enable)
{
    return -1;
}

} //namespace webrtc

