/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "video_channel_gles20.h"


#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>
#import "render_view.h"

#pragma mark -
#pragma mark * includes *
#include "trace.h"
#include "video_render_gles20.h"

namespace webrtc {

#pragma mark -
#pragma mark * GIPSEAGLChannel class definition *

VideoChannelGLES_2_0::VideoChannelGLES_2_0(VideoRenderGLES_2_0* owner, EAGLContext* eaglContext, RenderView* view, WebRtc_Word32 iId) :
_owner                (owner),
_width(              0),
_height(             0),
_stretchedWidth(    0),
_stretchedHeight(    0),
_buffer(             0),
_bufferSize(         0),
_incommingBufferSize(0),
_bufferIsUpdated(    false),
_numberOfStreams(    0),
_view            (view),
_heightPOT            (512),
_texHasRunOnce        (false),
_stretchMode(        kStretchNone),
_isRendering        (true),
_id(                iId),
_startWidth(        0.0),
_startHeight(        0.0),
_stopWidth(            1.0),
_stopHeight(        1.0),
_currentFrame(new VideoFrame())
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%s:%d Constructor", __FILE__, __FUNCTION__, __LINE__);
}

VideoChannelGLES_2_0::~VideoChannelGLES_2_0()
{
    _owner->LockCritSec();

    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%s:%d Destructor", __FILE__, __FUNCTION__, __LINE__);

    if (_buffer)
    {
        delete [] _buffer;
        _buffer = NULL;
    }

    _owner->UnlockCritSec();
}


int VideoChannelGLES_2_0::UpdateSize(int width, int height)
{
    _owner->LockCritSec();

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s:%s:%d ", __FILE__, __FUNCTION__, __LINE__);

    _owner->UnlockCritSec();

    return 0;
}

int VideoChannelGLES_2_0::UpdateStretchSize(int stretchHeight, int stretchWidth)
{
    _owner->LockCritSec();

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s:%s:%d ", __FILE__, __FUNCTION__, __LINE__);
    _stretchedHeight = stretchHeight;
    _stretchedWidth = stretchWidth;

    _owner->UnlockCritSec();
    return 0;
}

int VideoChannelGLES_2_0::FrameSizeChange(int width, int height, int numberOfStreams)
{
    _owner->LockCritSec();

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s:%s:%d ", __FILE__, __FUNCTION__, __LINE__);
    if (width == _width && _height == height)
    {
        // We already have a correct buffer size
        _numberOfStreams = numberOfStreams;
        _owner->UnlockCritSec();
        return 0;
    }

    _width = width;
    _height = height;

    [_view setupWidth:_width AndHeight:_height];

    _owner->UnlockCritSec();
    return 0;

}


WebRtc_Word32 VideoChannelGLES_2_0::RenderFrame(const WebRtc_UWord32 streamId, VideoFrame &videoFrame)
{
    _owner->LockCritSec();

    if(_width != (int)videoFrame.Width() ||
       _height != (int)videoFrame.Height())
    {
        _width = videoFrame.Width();
        _height = videoFrame.Height();
        if([_view setupWidth:_width AndHeight:_height] == -1){
            WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, 0, "%s:%s:%d could not setup new texture width", __FILE__, __FUNCTION__, __LINE__);
            _owner->UnlockCritSec();
            return -1;
        }
    }

    videoFrame.SetRenderTime(0);

    // increase size if need be. Store frame for later
    int frameLen = _width * _height * 1.5;
    if(_currentFrame->Length() < (WebRtc_UWord32)frameLen){
        _currentFrame->VerifyAndAllocate(frameLen);
    }
    _currentFrame->CopyFrame(videoFrame);
    _bufferIsUpdated = true;

    _owner->UnlockCritSec();
    return 0;
}



int VideoChannelGLES_2_0::RenderOffScreenBuffer()
{

    _owner->LockCritSec();

    if([_view renderFrame:_currentFrame] == -1){
        _owner->UnlockCritSec();
        return -1;
    }


    _bufferIsUpdated = false;


    _owner->UnlockCritSec();
    return 0;
}

int VideoChannelGLES_2_0::IsUpdated(bool& isUpdated)

{
    _owner->LockCritSec();
    isUpdated = _bufferIsUpdated;
    _owner->UnlockCritSec();

    return 0;
}


int VideoChannelGLES_2_0::SetStreamSettings(int streamId, float startWidth, float startHeight, float stopWidth, float stopHeight)
{

    _owner->LockCritSec();

    _startWidth = startWidth;
    _stopWidth = stopWidth;
    _startHeight = startHeight;
    _stopHeight = stopHeight;

    [_view setCoordinatesForZOrder:0 Left:_startWidth Top:_stopHeight Right:_stopWidth Bottom:_startHeight];

    int oldWidth = _width;
    int oldHeight = _height;
    int oldNumberOfStreams = _numberOfStreams;
    int retVal = FrameSizeChange(oldWidth, oldHeight, oldNumberOfStreams);

    _owner->UnlockCritSec();

    return retVal;
}


int VideoChannelGLES_2_0::PauseRendering()
{
    _owner->LockCritSec();
    _isRendering = FALSE;
    _owner->UnlockCritSec();
    return 0;
}

int VideoChannelGLES_2_0::ResumeRendering()
{
    _owner->LockCritSec();
    _isRendering = TRUE;
    _owner->UnlockCritSec();
    return 0;
}

WebRtc_Word32 VideoChannelGLES_2_0::GetChannelProperties(const WebRtc_Word16 streamId,
                                 WebRtc_UWord32& zOrder,
                                 float& left,
                                 float& top,
                                 float& right,
                                 float& bottom)
{
    zOrder = 0;
    left = _startWidth;
    top = _stopHeight;
    right = _stopWidth;
    bottom = _startHeight;

    return 0;
}

} // namespace webrtc
