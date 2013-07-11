/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#pragma mark -
#pragma mark * imports *

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#import <OpenGLES/EAGLDrawable.h>

#pragma mark -
#pragma mark * includes *
#include "video_render_gles20.h"
#include "trace.h"

namespace webrtc {

#pragma mark -
#pragma mark * VideoRenderEAGL class definition *

VideoRenderGLES_2_0::VideoRenderGLES_2_0(RenderView* view, bool fullscreen, WebRtc_Word32 iId) :
_glesCritSec(        *CriticalSectionWrapper::CreateCriticalSection()),
_screenUpdateThread( 0),
_screenUpdateEvent(  0),
_view(          view),
_windowRect(         ),
_windowWidth(        0),
_windowHeight(       0),
_fullScreen(         fullscreen),
_aglChannels(        ),
_zOrderToChannel(    ),
_glesContext(         [view context]),
_isRendering(        true),
_id(iId)
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d Constructor. Creating Thread and Event. _gles=%x", __FUNCTION__, __LINE__, _glesContext);
    _screenUpdateThread = ThreadWrapper::CreateThread(ScreenUpdateThreadProc, this, kRealtimePriority);
    _screenUpdateEvent = EventWrapper::Create();
    GetWindowRect(_windowRect);
}

VideoRenderGLES_2_0::~VideoRenderGLES_2_0()
{

    // Signal event to exit thread, then delete it
   ThreadWrapper* tmpPtr = _screenUpdateThread;
    _screenUpdateThread = NULL;

    if (tmpPtr)
    {
        tmpPtr->SetNotAlive();
        _screenUpdateEvent->Set();
        _screenUpdateEvent->StopTimer();

        if (tmpPtr->Stop())
        {
            delete tmpPtr;
        }
        delete _screenUpdateEvent;
        _screenUpdateEvent = NULL;
        _isRendering = FALSE;
    }

    // Delete all channels
    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin();
    while (it!= _aglChannels.end())
    {
        delete it->second;
        _aglChannels.erase(it);
        it = _aglChannels.begin();
    }
    _aglChannels.clear();

    // Clean the zOrder map
    std::multimap<int, int>::iterator zIt = _zOrderToChannel.begin();
    while(zIt != _zOrderToChannel.end())
    {
        _zOrderToChannel.erase(zIt);
        zIt = _zOrderToChannel.begin();
    }
    _zOrderToChannel.clear();

}

int VideoRenderGLES_2_0::Init()
{
    LockCritSec();
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d", __FUNCTION__, __LINE__);

    // Start rendering thread...
    if (!_screenUpdateThread)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d _screenUpdateThread doesn't exist", __FUNCTION__, __LINE__);
        UnlockCritSec();
        return -1;
    }

    if(!_view){
        _view = [[RenderView alloc]init];
    }


    // make new context
    if([_view createContext] == NO)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, 0, "%s:%s:%d Could not create GLES context", __FILE__, __FUNCTION__, __LINE__);
    }

    if([_view makeCurrentContext] == NO)
    {
        UnlockCritSec();
        return -1;
    }

    unsigned int threadId;
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d Starting _screenUpdateThread and _screenUpdateEvent", __FUNCTION__, __LINE__);
    _screenUpdateThread->Start(threadId);

    // Start the event triggering the render process
    unsigned int monitorFreq = 60;
    _screenUpdateEvent->StartTimer(true, 1000/monitorFreq);

    _windowWidth = _windowRect.right - _windowRect.left;
    _windowHeight = _windowRect.bottom - _windowRect.top;


    UnlockCritSec();
    return 0;
}

VideoChannelGLES_2_0* VideoRenderGLES_2_0::CreateEAGLChannel(int channel, int zOrder, float startWidth, float startHeight, float stopWidth, float stopHeight)
{
    LockCritSec();

    WEBRTC_TRACE(kTraceModuleCall, kTraceVideoRenderer, _id, "%s:%d Creating channel %d", __FUNCTION__, __LINE__, channel);

    if (HasChannel(channel))
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Channel already exists", __FUNCTION__, __LINE__);
        UnlockCritSec();
        return NULL;
    }

    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d Creating new VideoChannelGLES_2_0 _glesContext=%x", __FUNCTION__, __LINE__, _glesContext);
    VideoChannelGLES_2_0* newEAGLChannel = new VideoChannelGLES_2_0(this, _glesContext, _view, _id);
    //VideoChannelGLES_2_0* newEAGLChannel = NULL;

    if (newEAGLChannel->SetStreamSettings(0, startWidth, startHeight, stopWidth, stopHeight) == -1)
    {
        if (newEAGLChannel)
        {
            delete newEAGLChannel;
            newEAGLChannel = NULL;
        }
        WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d Could not create channel", __FUNCTION__, __LINE__);
        UnlockCritSec();
        return NULL;
    }

    _aglChannels[channel] = newEAGLChannel;
    _zOrderToChannel.insert(std::pair<int, int>(zOrder, channel));

    UnlockCritSec();
    return reinterpret_cast<VideoChannelGLES_2_0*>(newEAGLChannel);
}


int VideoRenderGLES_2_0::DeleteAllEAGLChannels()
{
    LockCritSec();

    std::map<int, VideoChannelGLES_2_0*>::iterator it;
    it = _aglChannels.begin();

    while (it != _aglChannels.end())
    {
        VideoChannelGLES_2_0* channel = it->second;
        delete channel;
        it++;
    }
    _aglChannels.clear();

    UnlockCritSec();
    return 0;
}


int VideoRenderGLES_2_0::DeleteEAGLChannel(int channel)
{
    LockCritSec();

    std::map<int, VideoChannelGLES_2_0*>::iterator it;
    it = _aglChannels.find(channel);
    if (it != _aglChannels.end())
    {
        delete it->second;
        _aglChannels.erase(it);
    }
    else
    {
        UnlockCritSec();
        return -1;
    }

    std::multimap<int, int>::iterator zIt = _zOrderToChannel.begin();
    while( zIt != _zOrderToChannel.end())
    {
        if (zIt->second == channel)
        {
            _zOrderToChannel.erase(zIt);
            break;
        }
        zIt++;
    }

    UnlockCritSec();
    return 0;
}

int VideoRenderGLES_2_0::StopThread()
{
    LockCritSec();

    ThreadWrapper* tmpPtr = _screenUpdateThread;
    _screenUpdateThread = NULL;

    if (tmpPtr)
    {
        tmpPtr->SetNotAlive();
        _screenUpdateEvent->Set();
        if (tmpPtr->Stop())
        {
            delete tmpPtr;
        }
    }

    delete _screenUpdateEvent;
    _screenUpdateEvent = NULL;

    UnlockCritSec();
    return 0;
}


bool VideoRenderGLES_2_0::IsFullScreen()
{
    return _fullScreen;
}


bool VideoRenderGLES_2_0::HasChannels()
{
    LockCritSec();
    if (_aglChannels.begin() != _aglChannels.end())
    {
        UnlockCritSec();
        return true;
    }

    UnlockCritSec();
    return false;
}


bool VideoRenderGLES_2_0::HasChannel(int channel)
{

    LockCritSec();

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.find(channel);

    if (it != _aglChannels.end())
    {
        UnlockCritSec();
        return true;
    }

    UnlockCritSec();
    return false;
}


int VideoRenderGLES_2_0::GetChannels(std::list<int>& channelList)
{
    LockCritSec();

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin();

    while (it != _aglChannels.end())
    {
        channelList.push_back(it->first);
        it++;
    }

    UnlockCritSec();
    return 0;
}

VideoChannelGLES_2_0* VideoRenderGLES_2_0::ConfigureEAGLChannel(int channel, int zOrder, float startWidth, float startHeight, float stopWidth, float stopHeight)
{

    LockCritSec();

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.find(channel);

    if (it != _aglChannels.end())
    {
        VideoChannelGLES_2_0* aglChannel = it->second;
        if (aglChannel->SetStreamSettings(0, startWidth, startHeight, stopWidth, stopHeight) == -1)
        {
            UnlockCritSec();
            return NULL;
        }

        std::multimap<int, int>::iterator it = _zOrderToChannel.begin();
        while(it != _zOrderToChannel.end())
        {
            if (it->second == channel)
            {
                if (it->first != zOrder)
                {
                    _zOrderToChannel.erase(it);
                    _zOrderToChannel.insert(std::pair<int, int>(zOrder, channel));
                }
                break;
            }
            it++;
        }

        UnlockCritSec();
        return reinterpret_cast<VideoChannelGLES_2_0*>(aglChannel);
    }

    UnlockCritSec();
    return NULL;
}

/*
 *
 *    Rendering process
 *
 */

bool VideoRenderGLES_2_0::ScreenUpdateThreadProc(void* obj)
{
    return static_cast<VideoRenderGLES_2_0*>(obj)->ScreenUpdateProcess();
}


bool VideoRenderGLES_2_0::ScreenUpdateProcess()
{
    _screenUpdateEvent->Wait(100);


    LockCritSec();

    if(_isRendering == FALSE)
    {
        WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d _isRendering = FALSE. Returning", __FUNCTION__, __LINE__);
        UnlockCritSec();
        return true;
    }


    if (!_screenUpdateThread)
    {
        UnlockCritSec();
        return false;
    }

    if([_view makeCurrentContext] == NO)
    {
        UnlockCritSec();
        return -1;
    }

    if (GetWindowRect(_windowRect) == -1)
    {
        UnlockCritSec();
        return true;
    }

    if (_windowWidth != (_windowRect.right - _windowRect.left)
        || _windowHeight != (_windowRect.bottom - _windowRect.top))
    {
        _windowWidth = _windowRect.right - _windowRect.left;
        _windowHeight = _windowRect.bottom - _windowRect.top;
    }


    // Check if there are any updated buffers
    bool updated = false;

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin();
    while (it != _aglChannels.end())
    {

        VideoChannelGLES_2_0* aglChannel = it->second;
        aglChannel->UpdateStretchSize(_windowHeight, _windowWidth);
        aglChannel->IsUpdated(updated);
        if (updated)
        {
            break;
        }
        it++;
    }

    if (updated)
    {
        // At least one buffer has been updated, we need to repaint the texture
        if (RenderOffScreenBuffers() != -1)
        {
            WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%s:%dRenderOffScreenBuffers failed", __FILE__, __FUNCTION__, __LINE__);
        }
        [_view presentFramebuffer];
    }


    UnlockCritSec();

    return true;

}

/*
 *
 *    Rendering functions
 *
 */

int VideoRenderGLES_2_0::RenderOffScreenBuffers()
{
    LockCritSec();

    // Get the current window size, it might have changed since last render.
    if (GetWindowRect(_windowRect) == -1)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Could not get window rect", __FUNCTION__, __LINE__);
        UnlockCritSec();
        return -1;
    }


    if([_view makeCurrentContext] == NO)
    {
        UnlockCritSec();
        return -1;
    }

    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s:%d glClear _glesContext=%x", __FUNCTION__, __LINE__, _glesContext);


    // Loop through all channels starting highest zOrder ending with lowest.
    for (std::multimap<int, int>::reverse_iterator rIt = _zOrderToChannel.rbegin();
         rIt != _zOrderToChannel.rend();
         rIt++)
    {
        int channelId = rIt->second;
        std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.find(channelId);

        VideoChannelGLES_2_0* aglChannel = it->second;

        aglChannel->RenderOffScreenBuffer();
    }

    UnlockCritSec();



    return 0;
}


/*
 *
 * Help functions
 *
 * All help functions assumes external protections
 *
 */

int VideoRenderGLES_2_0::GetWindowRect(Rect& rect)
{
    LockCritSec();

    CGRect frame = [_view frame];
    rect.top = frame.origin.x;
    rect.left = frame.origin.y;
    rect.bottom = frame.size.width;
    rect.right = frame.size.height;

    UnlockCritSec();
    return 0;
}


int VideoRenderGLES_2_0::PauseRendering()
{
    LockCritSec();

    if(FALSE == _isRendering)
    {
        UnlockCritSec();
        return 0;
    }



    for(std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin(); it != _aglChannels.end(); it++)
    {
        it->second->PauseRendering();
    }

    _isRendering = FALSE;
    if(_screenUpdateThread)
    {
        if(-1 == _screenUpdateThread->Stop())
        {
            WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Failed to pause screenUpdateProcess", __FUNCTION__, __LINE__);
        }
        if(-1 == _screenUpdateEvent->StopTimer())
        {
            WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Failed to pause screenUpdateEvent", __FUNCTION__, __LINE__);
        }
        WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, 0, "%s:%s:%d Paused rendering thread", __FILE__, __FUNCTION__, __LINE__);
    }


    return 0;
}

// sets a flag that is checked before issuing GPU commands
// stops the thread
int VideoRenderGLES_2_0::ResumeRendering()
{
    LockCritSec();

    if(TRUE == _isRendering)
    {
        UnlockCritSec();
        return 0;
    }

    _isRendering = TRUE;
    if(_screenUpdateThread)
    {
        unsigned int threadId;
        if(-1 == _screenUpdateThread->Start(threadId))
        {
            // could not start the thread
            WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Failed to resume screenUpdateProcess", __FUNCTION__, __LINE__);
        }
        else{
            WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, 0, "%s:%s:%d Resumed screenUpdateProcess thread", __FILE__, __FUNCTION__, __LINE__);
        }

        unsigned int monitorFreq = 60;
        if(-1 == _screenUpdateEvent->StartTimer(true, 1000/monitorFreq))
        {
            WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s:%d Failed to resume screenUpdateEvent", __FUNCTION__, __LINE__);
        }
        else{
            WEBRTC_TRACE(kTraceInfo, kTraceVideoRenderer, 0, "%s:%s:%d Resumed screenUpdateEvent", __FILE__, __FUNCTION__, __LINE__);
        }
    }

    for(std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin(); it != _aglChannels.end(); it++)
    {
        it->second->ResumeRendering();
    }

    UnlockCritSec();
    return 0;
}





int  VideoRenderGLES_2_0::ChangeWindow(void* newWindowRef)
{
    LockCritSec();

    _view = (RenderView*)newWindowRef;

    UnlockCritSec();
    return -1;
}

WebRtc_Word32  VideoRenderGLES_2_0::ChangeUniqueID(WebRtc_Word32 id)
{
    LockCritSec();

    _id    = id;

    UnlockCritSec();
    return -1;
}

WebRtc_Word32  VideoRenderGLES_2_0::StartRender()
{
    _isRendering = true;
    return 0;
}

WebRtc_Word32  VideoRenderGLES_2_0::StopRender()
{
    _isRendering = false;
    return 0;
}

WebRtc_Word32  VideoRenderGLES_2_0::DeleteAGLChannel(const WebRtc_UWord32 streamID)
{
    return DeleteEAGLChannel(streamID);
}

WebRtc_Word32  VideoRenderGLES_2_0::GetChannelProperties(const WebRtc_UWord16 streamId,
                                 WebRtc_UWord32& zOrder,
                                 float& left,
                                 float& top,
                                 float& right,
                                 float& bottom)
{
    // Check if there are any updated buffers
    int counter = 0;

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin();
    while (it != _aglChannels.end())
    {
        if(counter == streamId)
        {
            VideoChannelGLES_2_0* aglChannel = it->second;
            aglChannel->GetChannelProperties(0, zOrder, left, top, right, bottom);
            return 0;
        }
        counter++;
        it++;
    }
    return -1;
}




WebRtc_Word32 VideoRenderGLES_2_0::GetScreenResolution(WebRtc_UWord32& screenWidth,
                                           WebRtc_UWord32& screenHeight)
{
    screenWidth = [_view frame].size.width;
    screenHeight = [_view frame].size.height;
    return 0;
}


WebRtc_Word32 VideoRenderGLES_2_0::SetStreamCropping(const WebRtc_UWord32 streamId,
                              const float  left,
                              const float  top,
                              const float  right,
                              const float  bottom)
{


    // Check if there are any updated buffers
    //bool updated = false;
    WebRtc_UWord32 counter = 0;

    std::map<int, VideoChannelGLES_2_0*>::iterator it = _aglChannels.begin();
    while (it != _aglChannels.end())
    {
        if(counter == streamId)
        {
            VideoChannelGLES_2_0* aglChannel = it->second;
            aglChannel->SetStreamSettings(0, left, top, right, bottom);
        }
        counter++;
        it++;
    }


    return 0;
}


int VideoRenderGLES_2_0::LockCritSec()
{
    _glesCritSec.Enter();
    return 0;
}



int VideoRenderGLES_2_0::UnlockCritSec()
{
    _glesCritSec.Leave();
    return 0;
}

} // namespace webrtc
