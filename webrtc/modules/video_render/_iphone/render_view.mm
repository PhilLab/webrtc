/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#pragma mark #imports

#pragma mark #includes
#include "render_view.h"
#include "trace.h"
//#include "GIPSVPLIB.h"

#include "video_render_delegate_gles20.h"
// Objective-C variables
NSAutoreleasePool* _poolRenderView;

// OpenGLES variables
webrtc::VideoRenderDelegeteGLES_2_0*    _glesRenderer20;
// The pixel dimensions of the CAEAGLLayer.
GLint _framebufferWidth, _framebufferHeight;
// buffers
GLuint _defaultFramebuffer, _colorRenderbuffer;

#pragma mark RenderView (hidden) category
// hidden category for RenderView
@interface RenderView (hidden)

// hidden functions
- (bool)checkForCompleteBuffer;
- (bool)createFramebuffer;
- (void)deleteFramebuffer;
- (bool)setFramebuffer;
- (bool)initializeVariables;
@end // RenderView

@implementation RenderView

#pragma mark @synthesize

@synthesize context = _context;

#pragma mark Begin super class method overrides
// The class type must be overridden in order
// for OpenGLES to render to this UIView
+ (Class)layerClass {
    return [CAEAGLLayer class];
}

// This will be called if the client uses interface builder
// to create an instance
// If the client uses this to initialize, we have no know frame.
// What should we do? Make a frame? Try to use self?
- (id)initWithCoder:(NSCoder*)coder {

    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%s:%d initWithCoder",
                         __FILE__, __FUNCTION__, __LINE__);

    // init super class
    self = [super initWithCoder:coder];
    if(self == nil){
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d Failed to init super class",
                             __FUNCTION__, __LINE__);
        return nil;
    }

    [self initializeVariables];

    return self;
}

// This is the default init.
// If the client uses this to initialize, we have no know frame.
// What should we do? Make a frame? Try to use self?
-(id)init{
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%s:%d init", __FILE__, __FUNCTION__, __LINE__);

    // init super class
    self = [super init];
    if(self == nil){
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d Failed to init super class",
                             __FUNCTION__, __LINE__);
        return nil;
    }

    [self initializeVariables];

    return self;
}

// This is the initializer that decides which OpenGLES version to use.
// Initializes with the OpenGL framework, creates a context, and sets it current
-(id)initWithFrame:(CGRect)frame{

    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%s:%d initWithFrame",
                         __FILE__, __FUNCTION__, __LINE__);

    // init super class
    self = [super initWithFrame:frame];
    if(self == nil){
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d Failed to init super class",
                             __FUNCTION__, __LINE__);
        return nil;
    }

    [self initializeVariables];

    return self;

}

// initializes the member variables
-(bool)initializeVariables{
    //    _poolRenderView = [[NSAutoreleasePool alloc]init];
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer,
                         0, "%s:%d initializing member variables",
                         __FUNCTION__, __LINE__);

    _context = nil;
    _glesRenderer20 = new webrtc::VideoRenderDelegeteGLES_2_0(0);
    _framebufferWidth = 0;
    _framebufferHeight = 0;
    _defaultFramebuffer = 0;
    _colorRenderbuffer = 0;

    return YES;
}

-(bool)createContext{
    // create OpenGLES context from self layer class
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)self.layer;
    eaglLayer.opaque = YES;
    eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:
                                    [NSNumber numberWithBool:NO],
                                    kEAGLDrawablePropertyRetainedBacking,
                                    kEAGLColorFormatRGBA8,
                                    kEAGLDrawablePropertyColorFormat, nil];

    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%s:%d self=0x%x eaglLayer=0x%x _context",
                         __FILE__, __FUNCTION__, __LINE__,
                         self, eaglLayer, _context);

    _context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    if(_context){
        webrtc::WEBRTC_TRACE(webrtc::kTraceInfo, webrtc::kTraceVideoRenderer, 0,
                             "%s:%d Successfully initialized RenderView "\
                             "with kEAGLRenderingAPIOpenGLES2 API",
                             __FUNCTION__, __LINE__);
    }
    else{
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d  Could not create gles 2.0 context",
                             __FUNCTION__, __LINE__);
        return NO;
    }


    // set current EAGLContext to self _context
    if (![self makeCurrentContext]){
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d Failed to set context immediately"\
                             " after creating it", __FUNCTION__, __LINE__);
        return NO;
    }

    if([self setFramebuffer] == NO){
        return NO;
    }

    [self setupWidth:[self frame].size.width
           AndHeight:[self frame].size.height];

    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%d initWithFrame was successful. Returning.",
                         __FUNCTION__, __LINE__);

    return YES;
}

- (NSString *)description{
    return [NSString stringWithFormat:
            @"A WebRTC implemented subclass of UIView. +Class method is overwritten, along with custom methods"];
}

- (void)dealloc {
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%d", __FUNCTION__, __LINE__);

    if(_context){
        if (![self makeCurrentContext]){
        }
        _context = nil;
    }

    [super dealloc];
}

#pragma mark Original method implementations

// generates and binds the OpenGLES buffers
- (bool)createFramebuffer{
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%d Generating and binding OpenGLES buffers",
                         __FUNCTION__, __LINE__);
    if (_context){
        if (![self makeCurrentContext]){
            return NO;
        }

        // Create default framebuffer object.
        glGenFramebuffers(1, &_defaultFramebuffer);  ;
        glBindFramebuffer(GL_FRAMEBUFFER, _defaultFramebuffer); ;

        // Create color render buffer and allocate backing store.
        glGenRenderbuffers(1, &_colorRenderbuffer); ;
        glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderbuffer); ;
        [_context renderbufferStorage:GL_RENDERBUFFER
                         fromDrawable:(CAEAGLLayer *)self.layer]; ;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER,
                                     GL_RENDERBUFFER_WIDTH,
                                     &_framebufferWidth); ;
        glGetRenderbufferParameteriv(GL_RENDERBUFFER,
                                     GL_RENDERBUFFER_HEIGHT,
                                     &_framebufferHeight); ;
        glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                  GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER,
                                  _colorRenderbuffer);

        [self checkForCompleteBuffer];
    }
    else{
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%s:%d Could not create frame buffers",
                             __FILE__, __FUNCTION__, __LINE__);
        return NO;
    }
    return YES;
}

- (bool)checkForCompleteBuffer{
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
        webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer,
                             0, "%s:%s:%d Failed to completely generate"\
                             " OpenGLES buffer session",
                             __FILE__, __FUNCTION__, __LINE__);
        return NO;
    }
    return YES;
}

// destroys the OpenGLES frame buffers
- (void)deleteFramebuffer{
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%d Deleting OpenGLES buffers",
                         __FUNCTION__, __LINE__);
    if (_context){
        [EAGLContext setCurrentContext:_context];
        if (_defaultFramebuffer)
        {
            glDeleteFramebuffers(1, &_defaultFramebuffer);
            _defaultFramebuffer = 0;
        }
        if (_colorRenderbuffer)
        {
            glDeleteRenderbuffers(1, &_colorRenderbuffer);
            _colorRenderbuffer = 0;
        }
    }
    else{
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%s:%d ERROR: context is NULL",
                             __FILE__, __FUNCTION__, __LINE__);
    }
    ;
}

 // this is the master initialization call for all of our OpenGLES functions
- (bool)setFramebuffer{
    webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer, 0,
                         "%s:%d Updating context and setting glViewport",
                         __FUNCTION__, __LINE__);

    if (_context){
        if([self makeCurrentContext] == NO){
            ;
            return NO;
        }

        [self createFramebuffer];

        glBindFramebuffer(GL_FRAMEBUFFER, _defaultFramebuffer);
        glViewport(0, 0, self.frame.size.width, self.frame.size.height);
    }
    else{
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%s:%d ERROR: context is not valid",
                             __FILE__, __FUNCTION__, __LINE__);
        return NO;
    }

    return YES;
}

// "paint" the UIView
- (bool)presentFramebuffer{

    bool success = NO;
    if (_context){
        [self makeCurrentContext];
        [EAGLContext setCurrentContext:_context];
        [_context presentRenderbuffer:GL_RENDERBUFFER];
//        [self setNeedsDisplay];
        // update UI stuff on the main thread
        [self performSelectorOnMainThread:@selector(setNeedsDisplay)
                               withObject:nil waitUntilDone:NO];
        if(success == NO){
            webrtc::WEBRTC_TRACE(webrtc::kTraceWarning,
                                 webrtc::kTraceVideoRenderer, 0,
                                 "%s:%d [context presentRenderbuffer] "\
                                 "returned false", __FUNCTION__, __LINE__);
        }
    }
    else{
        webrtc::WEBRTC_TRACE(webrtc::kTraceError, webrtc::kTraceVideoRenderer,
                             0, "%s:%d Cannot presentRenderbuffer because"\
                             " of invalid EAGLContext", __FUNCTION__, __LINE__);
    }
    return YES;
}

// Sets "this" as the current canvas for OpenGL to render to
- (bool)makeCurrentContext{
    if(!_context){
        webrtc::WEBRTC_TRACE(webrtc::kTraceDebug, webrtc::kTraceVideoRenderer,
                             0, "%s:%s:%d Could not make context current",
                             __FILE__, __FUNCTION__, __LINE__);
        return NO;
    }
    else{
        if([EAGLContext setCurrentContext:_context] == GL_FALSE){
            return NO;
        }
    }
    return YES;
}

-(WebRtc_Word32)setupWidth:(WebRtc_Word32)width AndHeight:(WebRtc_Word32)height{
    [self makeCurrentContext];
    return _glesRenderer20->Setup(width, height);
}

-(WebRtc_Word32)renderFrame:(webrtc::VideoFrame*)frameToRender{

    [self makeCurrentContext];
    int ret = _glesRenderer20->Render(*frameToRender);

    return ret;

}

-(WebRtc_Word32)setCoordinatesForZOrder:(WebRtc_Word32)zOrder
                                    Left:(const float)left
                                    Top:(const float)top
                                    Right:(const float)right
                                    Bottom:(const float)bottom{
    [self makeCurrentContext];
    return _glesRenderer20->SetCoordinates(zOrder, left, top, right, bottom);
}

@end
