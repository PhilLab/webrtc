/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "engine_configurations.h"

#import "render_view.h"

#include "video_render_iphone_impl.h"
#include "critical_section_wrapper.h"
#include "video_render_gles20.h"
#include "trace.h"
#include "video_render_defines.h"

namespace webrtc {

VideoRenderIPhoneImpl::VideoRenderIPhoneImpl(
                                        const WebRtc_Word32 id,
                                        const VideoRenderType videoRenderType,
                                        void* window,
                                        const bool fullscreen) :
    _id(id),
    _renderIPhoneCritsect(*CriticalSectionWrapper::CreateCriticalSection()),
    _fullScreen(fullscreen),
    _ptrWindow(window)
{
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "Constructor %s:%d",
                 __FUNCTION__, __LINE__);
}

VideoRenderIPhoneImpl::~VideoRenderIPhoneImpl()
{
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
                 "Destructor %s:%d", __FUNCTION__, __LINE__);
    delete &_renderIPhoneCritsect;
    if (_ptrIPhoneRender)
    {
        delete _ptrIPhoneRender;
        _ptrIPhoneRender = NULL;
    }
}

WebRtc_Word32
VideoRenderIPhoneImpl::Init()
{
    CriticalSectionScoped cs(&_renderIPhoneCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
                 "%s:%d", __FUNCTION__, __LINE__);

    _ptrIPhoneRender =
        new VideoRenderGLES_2_0((RenderView*)_ptrWindow, _fullScreen, _id);
    if (!_ptrWindow)
    {
        WEBRTC_TRACE(kTraceWarning, kTraceVideoRenderer, _id,
                     "Constructor %s:%d", __FUNCTION__, __LINE__);
        return -1;
    }
    int retVal = _ptrIPhoneRender->Init();
    if (retVal == -1)
    {
        WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
                     "Failed to init %s:%d", __FUNCTION__, __LINE__);
        return -1;
    }
    return 0;
}

WebRtc_Word32
VideoRenderIPhoneImpl::ChangeUniqueId(const WebRtc_Word32 id)
{
    CriticalSectionScoped cs(&_renderIPhoneCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id, "%s", __FUNCTION__);
    _id = id;

    if(_ptrIPhoneRender)
    {
        _ptrIPhoneRender->ChangeUniqueID(_id);
    }

    return 0;
}

WebRtc_Word32
VideoRenderIPhoneImpl::ChangeWindow(void* window)
{
    CriticalSectionScoped cs(&_renderIPhoneCritsect);
    WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, _id,
                 "%s changing ID to ", __FUNCTION__, window);

    if (window == NULL)
    {
        return -1;
    }
    _ptrWindow = window;

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id,
                 "%s:%d", __FUNCTION__, __LINE__);

    _ptrWindow = window;
    _ptrIPhoneRender->ChangeWindow((RenderView*)_ptrWindow);

    return 0;
}

VideoRenderCallback*
VideoRenderIPhoneImpl::AddIncomingRenderStream(const WebRtc_UWord32 streamId,
                                               const WebRtc_UWord32 zOrder,
                                               const float left,
                                               const float top,
                                               const float right,
                                               const float bottom)
{
    CriticalSectionScoped cs(&_renderIPhoneCritsect);
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s", __FUNCTION__);
    if(!_ptrWindow)
    {
        return NULL;
    }

    VideoChannelGLES_2_0* nsOpenGLChannel =
        _ptrIPhoneRender->CreateEAGLChannel(streamId, zOrder, left,
                                            top, right, bottom);

    if(!nsOpenGLChannel)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                     "%s Failed to create NSGL channel", __FUNCTION__);
        return NULL;
    }

    return nsOpenGLChannel;
}

WebRtc_Word32
VideoRenderIPhoneImpl::DeleteIncomingRenderStream(const WebRtc_UWord32 streamId)
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id,
                 "Constructor %s:%d", __FUNCTION__, __LINE__);
    CriticalSectionScoped cs(&_renderIPhoneCritsect);
    _ptrIPhoneRender->DeleteEAGLChannel(streamId);

    return 0;
}

WebRtc_Word32
VideoRenderIPhoneImpl::GetIncomingRenderStreamProperties(
                                                const WebRtc_UWord32 streamId,
                                                WebRtc_UWord32& zOrder,
                                                float& left,
                                                float& top,
                                                float& right,
                                                float& bottom) const
{
    return _ptrIPhoneRender->GetChannelProperties(streamId, zOrder, left,
                                                  top, right, bottom);
}

WebRtc_Word32
VideoRenderIPhoneImpl::StartRender()
{
    return _ptrIPhoneRender->StartRender();
}

WebRtc_Word32
VideoRenderIPhoneImpl::StopRender()
{
    return _ptrIPhoneRender->StopRender();
}

VideoRenderType
VideoRenderIPhoneImpl::RenderType()
{
    return kRenderiPhone;
}

RawVideoType
VideoRenderIPhoneImpl::PerferedVideoType()
{
    return kVideoI420;
}

bool
VideoRenderIPhoneImpl::FullScreen()
{
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s - not supported on iPhone", __FUNCTION__);
    return -1;
}


WebRtc_Word32
VideoRenderIPhoneImpl::GetGraphicsMemory(
                                WebRtc_UWord64& totalGraphicsMemory,
                                WebRtc_UWord64& availableGraphicsMemory) const
{
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s - not supported on iPhone", __FUNCTION__);
    return -1;
}

WebRtc_Word32
VideoRenderIPhoneImpl::GetScreenResolution(WebRtc_UWord32& screenWidth,
                                            WebRtc_UWord32& screenHeight) const
{
    return _ptrIPhoneRender->GetScreenResolution(screenWidth, screenHeight);
}

WebRtc_UWord32
VideoRenderIPhoneImpl::RenderFrameRate(const WebRtc_UWord32 streamId)
{
    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id,
                 "%s - not supported on iPhone", __FUNCTION__);
    return -1;
}

WebRtc_Word32
VideoRenderIPhoneImpl::SetStreamCropping(
                                         const WebRtc_UWord32 streamId,
                                         const float left,
                                         const float top,
                                         const float right,
                                         const float bottom)
{
    return _ptrIPhoneRender->SetStreamCropping(streamId, left, top,
                                               right, bottom);
}


WebRtc_Word32 VideoRenderIPhoneImpl::ConfigureRenderer(
                                        const WebRtc_UWord32 streamId,
                                        const unsigned int zOrder,
                                        const float left,
                                        const float top,
                                        const float right,
                                        const float bottom)
{
    // TODO
    return 0;
}


WebRtc_Word32
VideoRenderIPhoneImpl::SetTransparentBackground(const bool enable)
{
    // TODO
    return 0;
}

WebRtc_Word32 VideoRenderIPhoneImpl::SetText(
                                        const WebRtc_UWord8   textId,
                                        const WebRtc_UWord8*  text,
                                        const WebRtc_Word32   textLength,
                                        const WebRtc_UWord32  textColorRef,
                                        const WebRtc_UWord32  backgroundColorRef,
                                        const float left,
                                        const float top,
                                        const float right,
                                        const float bottom)
{
//    return _ptrIPhoneRender->SetText(textId, text, textLength,
//                                     textColorRef,backgroundColorRef,
//                                     left, top, right, bottom);
    return -1;
}

WebRtc_Word32 VideoRenderIPhoneImpl::SetBitmap(
                                        const void* bitMap,
                                        const WebRtc_UWord8 pictureId,
                                        const void* colorKey,
                                        const float left,
                                        const float top,
                                        const float right,
                                        const float bottom)
{
    // TODO
    return -1;
}

WebRtc_Word32 VideoRenderIPhoneImpl::FullScreenRender(void* window,
                                                      const bool enable)
{
    // TODO
    return -1;
}
} // namspace webrtc
