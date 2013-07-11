/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef __VIDEO_CHANNEL_GLES_2_0_H
#define __VIDEO_CHANNEL_GLES_2_0_H

#pragma mark * Standard includes
#include <list>
#include <map>

#define COLOR_DEPTH_32  1

#pragma mark * Standard imports
#import <OpenGLES/EAGL.h>

#pragma mark * WebRTC includes
#include "engine_configurations.h"
#include "critical_section_wrapper.h"
#include "event_wrapper.h"
#include "trace.h"
#include "thread_wrapper.h"
#include "video_render_defines.h"
//#include "vp8.h"

@class RenderView;

namespace webrtc {

#pragma mark * Forward declares
    class Trace;
    class CriticalSectionWrapper;
    class ThreadWrapper;
    class Event;
    class VideoRenderGLES_2_0;

#pragma mark * VideoChannelGLES_2_0 class definition
class VideoChannelGLES_2_0 : public VideoRenderCallback
{
public:

    VideoChannelGLES_2_0(VideoRenderGLES_2_0* owner, EAGLContext* eaglContext,
                         RenderView* view, WebRtc_Word32 iId);
    virtual ~VideoChannelGLES_2_0();

    virtual int FrameSizeChange(int width,
                                int height,
                                int numberOfStreams);
    virtual int UpdateSize(int width, int height);
    virtual int UpdateStretchSize(int stretchHeight, int stretchWidth);
    virtual int SetStreamSettings(int streamId,
                                  float startWidth,
                                  float startHeight,
                                  float stopWidth,
                                  float stopHeight);
    virtual int RenderOffScreenBuffer();
    virtual int IsUpdated(bool& isUpdated);
    virtual int PauseRendering();
    virtual int ResumeRendering();
    WebRtc_Word32 GetChannelProperties(const WebRtc_Word16 streamId,
                                     WebRtc_UWord32& zOrder,
                                     float& left,
                                     float& top,
                                     float& right,
                                     float& bottom);

    // implements VideoChannelGLESInterface (VideoRenderCallback)
    virtual WebRtc_Word32 RenderFrame(const WebRtc_UWord32 streamId,
                                      VideoFrame &videoFrame);

private:
    VideoRenderGLES_2_0*        _owner;
    CriticalSectionWrapper*            _critPtr;
    Trace*                        _trace;
    int                                _width;
    int                                _height;
    int                                _stretchedWidth;
    int                                _stretchedHeight;
    unsigned char*                    _buffer;
    int                                _bufferSize;
    int                                _incommingBufferSize;
    bool                            _bufferIsUpdated;
    bool                            _sizeInitialized;
    int                                _numberOfStreams;
    int                                _xOldWidth;
    int                                _yOldHeight;
    bool                            _bVideoSizeStartedChanging;
    int                                _oldStretchedHeight;
    int                                _oldStretchedWidth;
    RenderView*                    _view;
    int                                _widthPOT;
    int                                _heightPOT;
    bool                            _texHasRunOnce;
    int                                _rgbLength;
    StretchMode                _stretchMode;
    bool                            _isRendering;
    WebRtc_Word32                        _id;
    float                            _startWidth, _startHeight;
    float                            _stopWidth, _stopHeight;
    VideoFrame*                    _currentFrame;
};

} // namespace webrtc
#endif
