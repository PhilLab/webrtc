/*
 *  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "webrtc/engine_configurations.h"

#ifndef WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_MAC_VIDEO_RENDER_NSOPENGL_H_
#define WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_MAC_VIDEO_RENDER_NSOPENGL_H_

//#include <Cocoa/Cocoa.h>
//#include <QuickTime/QuickTime.h>
//#include <OpenGL/OpenGL.h>
//#include <OpenGL/glu.h>
//#include <OpenGL/glext.h>
#include <UIKit/UIKit.h>
#include <list>
#include <map>

#include	"webrtc/modules/video_render/include/video_render_defines.h"

#import		"tick_util.h"

class Trace;

namespace webrtc {
class EventWrapper;
class ThreadWrapper;
class VideoRenderUIView;
class CriticalSectionWrapper;

class VideoChannelUIView : public VideoRenderCallback
{

public:

    VideoChannelUIView(UIView *nsglContext, int iId, VideoRenderUIView* owner);
    virtual ~VideoChannelUIView();

    // A new frame is delivered
    virtual int DeliverFrame(unsigned char* buffer, int bufferSize, unsigned int timeStamp90kHz);

    // Called when the incomming frame size and/or number of streams in mix changes
    virtual int FrameSizeChange(int width, int height, int numberOfStreams);

    virtual int UpdateSize(int width, int height);

    // Setup 
    int SetStreamSettings(int streamId, float startWidth, float startHeight, float stopWidth, float stopHeight);
    int SetStreamCropSettings(int streamId, float startWidth, float startHeight, float stopWidth, float stopHeight);

    // Called when it's time to render the last frame for the channel
    int RenderOffScreenBuffer();

    // Returns true if a new buffer has been delivered to the texture
    int IsUpdated(bool& isUpdated);
    virtual int UpdateStretchSize(int stretchHeight, int stretchWidth);

    // ********** new module functions ************ //
    virtual int32_t RenderFrame(const uint32_t streamId, I420VideoFrame& videoFrame);

    // ********** new module helper functions ***** //
    int ChangeContext(UIView *nsglContext);
    int32_t GetChannelProperties(float& left,
            float& top,
            float& right,
            float& bottom);

private:

    UIView* _nsglContext;
    int _id;
    VideoRenderUIView* _owner;
    int32_t _width;
    int32_t _height;
    float _startWidth;
    float _startHeight;
    float _stopWidth;
    float _stopHeight;
    int _stretchedWidth;
    int _stretchedHeight;
//    int _oldStretchedHeight;
//    int _oldStretchedWidth;
//    int _xOldWidth;
//    int _yOldHeight;
    unsigned char* _buffer;
    int _bufferSize;
    int _incommingBufferSize;
    bool _bufferIsUpdated;
    int _numberOfStreams;
//    int _pixelFormat;
//    int _pixelDataType;
//    bool _bVideoSizeStartedChanging;
    int _framesDelivered;
//    TickTime _lastFramerateReportTime;
//    int _lastFramerateReportFramesDelivered;
};

class VideoRenderUIView
{

public: // methods
    VideoRenderUIView(UIImageView *windowRef, bool fullScreen, int iId);
    ~VideoRenderUIView();

    static int GetOpenGLVersion(int& nsglMajor, int& nsglMinor);

    // Allocates textures
    int Init();
    VideoChannelUIView* CreateNSGLChannel(int streamID, int zOrder, float startWidth, float startHeight, float stopWidth, float stopHeight);
    VideoChannelUIView* ConfigureNSGLChannel(int channel, int zOrder, float startWidth, float startHeight, float stopWidth, float stopHeight);
    int DeleteNSGLChannel(int channel);
    int DeleteAllNSGLChannels();
    int StopThread();
    bool HasChannels();
    bool HasChannel(int channel);
    int GetChannels(std::list<int>& channelList);
    void LockAGLCntx();
    void UnlockAGLCntx();

    // ********** new module functions ************ //
    int ChangeWindow(UIImageView* newWindowRef);
    int32_t ChangeUniqueID(int32_t id);
    int32_t StartRender();
    int32_t StopRender();
    int32_t DeleteNSGLChannel(const uint32_t streamID);
    int32_t GetChannelProperties(const uint16_t streamId,
            uint32_t& zOrder,
            float& left,
            float& top,
            float& right,
            float& bottom);

    int32_t SetText(const uint8_t textId,
            const uint8_t* text,
            const int32_t textLength,
            const uint32_t textColorRef,
            const uint32_t backgroundColorRef,
            const float left,
            const float top,
            const float right,
            const float bottom);

    // ********** new module helper functions ***** //
    int configureNSOpenGLEngine();
    int configureNSOpenGLView();
    int setRenderTargetWindow();
    int setRenderTargetFullScreen();
    UIImageView *getWindowReference (){return _windowRef;};

protected: // methods
    static bool ScreenUpdateThreadProc(void* obj);
    bool ScreenUpdateProcess();
    int GetWindowRect(Rect& rect);

private: // methods

    int CreateMixingContext();
    int RenderOffScreenBuffers();
    int DisplayBuffers();

private: // variables


    UIImageView* _windowRef;
    int _id;
    CriticalSectionWrapper& _nsglContextCritSec;
    ThreadWrapper* _screenUpdateThread;
    EventWrapper* _screenUpdateEvent;
    UIView* _nsglContext;
    UIView* _nsglFullScreenContext;
    Rect _windowRect; // The size of the window
    int _windowWidth;
    int _windowHeight;
    std::map<int, VideoChannelUIView*> _nsglChannels;
    std::multimap<int, int> _zOrderToChannel;
    unsigned int _threadID;
    bool _renderingIsPaused;
};

} //namespace webrtc

#endif   // WEBRTC_MODULES_VIDEO_RENDER_MAIN_SOURCE_MAC_VIDEO_RENDER_NSOPENGL_H_

