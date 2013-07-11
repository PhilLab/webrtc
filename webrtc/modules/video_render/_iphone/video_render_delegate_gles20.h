/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef GIPS_VIDEO_RENDER_OPENGL20_HH
#define GIPS_VIDEO_RENDER_OPENGL20_HH

#include "video_render_defines.h"
#include "video_render_frames.h"
#import <Foundation/Foundation.h>
#import <OpenGLES/ES2/gl.h>
#import <OpenGLES/ES2/glext.h>
#import <OpenGLES/EAGL.h>

namespace webrtc {
class VideoRenderDelegeteGLES_2_0
{
public:
    VideoRenderDelegeteGLES_2_0(WebRtc_Word32 id);
    ~VideoRenderDelegeteGLES_2_0();

    WebRtc_Word32 Setup(WebRtc_Word32 width, WebRtc_Word32 height);
    WebRtc_Word32 Render(const VideoFrame& frameToRender);
    WebRtc_Word32 SetCoordinates(WebRtc_Word32 zOrder,
                               const float left,
                               const float top,
                               const float right,
                               const float bottom);

private:
    void printGLString(const char *name, GLenum s);
    void checkGlError(const char* op);
    GLuint loadShader(GLenum shaderType, const char* pSource);
    GLuint createProgram(const char* pVertexSource, const char* pFragmentSource);
    void setupTextures(const VideoFrame& frameToRender);
    void updateTextures(const VideoFrame& frameToRender);

    WebRtc_Word32 _id;
    GLuint _textureIds[3]; // Texture id of Y,U and V texture.
    GLuint _program;
    GLuint _vPositionHandle;
    GLsizei _textureWidth;
    GLsizei _textureHeight;
    bool    _textureCreated;

    GLfloat _vertices[20];
    static const char g_indices[];

    static const char g_vertextShader[];
    static const char g_fragmentShader[];

};
} // namespace webrtc
#endif //GIPS_VIDEO_RENDER_OPENGL20_HH
