/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef _RENDER_VIEW_H_
#define _RENDER_VIEW_H_

// iOS framework imports
#import <UIKit/UIKit.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <QuartzCore/QuartzCore.h>

#include "typedefs.h"
#include "common_types.h"

#include "video_render_frames.h"

@class EAGLContext;

@interface RenderView : UIView {
    EAGLContext*        _context;
}

- (bool)createContext;
- (bool)makeCurrentContext;
- (bool)presentFramebuffer;

-(WebRtc_Word32)setupWidth:(WebRtc_Word32)width AndHeight:(WebRtc_Word32)height;
-(WebRtc_Word32)renderFrame:(webrtc::VideoFrame*)frameToRender;
-(WebRtc_Word32)setCoordinatesForZOrder:(WebRtc_Word32)zOrder
                                 Left:(const float)left
                                  Top:(const float)top
                                Right:(const float)right
                               Bottom:(const float)bottom;

@property (nonatomic, retain) EAGLContext*        context;

@end

#endif // _RENDER_VIEW_H_
