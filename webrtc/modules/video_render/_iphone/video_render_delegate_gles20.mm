/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#import <Foundation/Foundation.h>

#include <stdio.h>
#include <stdlib.h>

#include "video_render_delegate_gles20.h"
#include "trace.h"

namespace webrtc {

const char VideoRenderDelegeteGLES_2_0::g_indices[] = { 0, 3, 2, 0, 2, 1};

const char VideoRenderDelegeteGLES_2_0::g_vertextShader[] =
    {
    "attribute vec4 aPosition;\n"
    "attribute vec2 aTextureCoord;\n"
    "varying vec2 vTextureCoord;\n"
    "void main() {\n"
    "  gl_Position = aPosition;\n"
    "  vTextureCoord = aTextureCoord;\n"
    "}\n"
    };

// The fragment shader.
// Do YUV to RGB565 conversion.
const char VideoRenderDelegeteGLES_2_0::g_fragmentShader[] =
    {
            "precision mediump float;\n"
            "uniform sampler2D Ytex;\n"
            "uniform sampler2D Utex,Vtex;\n"
            "varying vec2 vTextureCoord;\n"
            "void main(void) {\n"
            "  float nx,ny,r,g,b,y,u,v;\n"
            "  mediump vec4 txl,ux,vx;"
            "  nx=vTextureCoord[0];\n"
            "  ny=vTextureCoord[1];\n"
            "  y=texture2D(Ytex,vec2(nx,ny)).r;\n"
            "  u=texture2D(Utex,vec2(nx,ny)).r;\n"
            "  v=texture2D(Vtex,vec2(nx,ny)).r;\n"

            //"  y = v;\n"+
            "  y=1.1643*(y-0.0625);\n"
            "  u=u-0.5;\n"
            "  v=v-0.5;\n"

            "  r=y+1.5958*v;\n"
            "  g=y-0.39173*u-0.81290*v;\n"
            "  b=y+2.017*u;\n"
            "  gl_FragColor=vec4(r,g,b,1.0);\n"
        "}\n"
    };

VideoRenderDelegeteGLES_2_0::VideoRenderDelegeteGLES_2_0(WebRtc_Word32 id)
:_id(id),
_textureWidth(-1),
_textureHeight(-1),
_textureCreated(false)

{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: id %d",
                       __FUNCTION__,(int)_id);

    const GLfloat vertices [20]={
      // X, Y, Z, U, V
        -1, -1, 0, 0, 1, // Bottom Left
        1, -1, 0, 1, 1, //Bottom Right
        1,  1, 0, 1, 0, //Top Right
        -1,  1, 0, 0, 0 }; //Top Left

    memcpy(_vertices,vertices,sizeof(_vertices));
}

VideoRenderDelegeteGLES_2_0::~VideoRenderDelegeteGLES_2_0()
{
    glDeleteTextures(3, _textureIds);
}

WebRtc_Word32 VideoRenderDelegeteGLES_2_0::Setup(WebRtc_Word32 width, WebRtc_Word32 height)
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: width %d, height %d",
                   __FUNCTION__,(int) width,(int) height);

    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    int maxTextureImageUnits[2];
    int maxTextureSize[2];
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, maxTextureSize);

    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: number of textures %d, size %d",
                           __FUNCTION__,(int) maxTextureImageUnits[0],(int) maxTextureSize[0]);


    _program = createProgram(g_vertextShader, g_fragmentShader);
    if (!_program)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Could not create program",
                __FUNCTION__);
        return -1;
    }

    int positionHandle = glGetAttribLocation(_program, "aPosition");
    checkGlError("glGetAttribLocation aPosition");
    if (positionHandle == -1)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Could not get aPosition handle",
                    __FUNCTION__);
        return -1;
    }
    int textureHandle = glGetAttribLocation(_program, "aTextureCoord");
    checkGlError("glGetAttribLocation aTextureCoord");
    if (textureHandle == -1)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Could not get aTextureCoord handle",
                            __FUNCTION__);
        return -1;
    }

    // set the vertices array in the shader
    // _vertices contains 4 vertices with 5 coordinates. 3 for (xyz) for the vertices and 2 for the texture
    glVertexAttribPointer(positionHandle,3,GL_FLOAT,false,5*sizeof(GLfloat),_vertices);
    checkGlError("glVertexAttribPointer aPosition");

    glEnableVertexAttribArray(positionHandle);
    checkGlError("glEnableVertexAttribArray positionHandle");

    // set the texture coordinate array in the shader
    // _vertices contains 4 vertices with 5 coordinates. 3 for (xyz) for the vertices and 2 for the texture
    glVertexAttribPointer(textureHandle, 2,GL_FLOAT, false,
            5*sizeof(GLfloat), &_vertices[3]);
    checkGlError("glVertexAttribPointer maTextureHandle");
    glEnableVertexAttribArray(textureHandle);
    checkGlError("glEnableVertexAttribArray textureHandle");

    glUseProgram(_program);
    int i=glGetUniformLocation(_program, "Ytex");
    checkGlError("glGetUniformLocation");
    glUniform1i(i,0);  /* Bind Ytex to texture unit 0 */
    checkGlError("glUniform1i Ytex");

    i=glGetUniformLocation(_program, "Utex");
    checkGlError("glGetUniformLocation Utex");
    glUniform1i(i,1);  /* Bind Utex to texture unit 1 */
    checkGlError("glUniform1i Utex");

    i=glGetUniformLocation(_program, "Vtex");
    checkGlError("glGetUniformLocation");
    glUniform1i(i,2);  /* Bind Vtex to texture unit 2 */
    checkGlError("glUniform1i");


    glGenTextures(3, _textureIds); //Generate  the Y, U and V texture


    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    checkGlError("glViewport");
    return 0;

}
/*
 * SetCoordinates
 * Sets the coordinates where the stream shall be rendered. Values must be between 0 and 1.
 */
WebRtc_Word32 VideoRenderDelegeteGLES_2_0::SetCoordinates(
                           WebRtc_Word32 zOrder,
                           const float left,
                           const float top,
                           const float right,
                           const float bottom)
{
    if((top>1 || top<0) || (right>1 || right<0) || (bottom>1 || bottom<0) || (left>1 || left<0))
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Wrong coordinates",
                               __FUNCTION__);

        return -1;
    }


    //Bottom Right
    _vertices[0]=(right*2)-1;
    _vertices[1]=-1*(2*bottom)+1;
    _vertices[2]=zOrder;

    // Bottom Left
    _vertices[5]=(left*2)-1;
    _vertices[6]=-1*(2*bottom)+1;
    _vertices[7]=zOrder;

    //Top Left
    _vertices[10]=(left*2)-1;
    _vertices[11]=-1*(2*top)+1;
    _vertices[12]=zOrder;

    //Top Right
    _vertices[15]=(right*2)-1;
    _vertices[16]=-1*(2*top)+1;
    _vertices[17]=zOrder;

    return 0;

}

WebRtc_Word32 VideoRenderDelegeteGLES_2_0::Render(const VideoFrame& frameToRender)
{
//    glClearColor(1.0, 0.0, 0.0, 1.0);
//    glClear(GL_COLOR_BUFFER_BIT);
//    return 0;

    if(frameToRender.Length()==0)
    {
        WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, 0, "%s:%s:%d frameLength = 0", __FILE__, __FUNCTION__, __LINE__);
        return -1;
    }



    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: id %d",
                       __FUNCTION__,(int)_id);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    if(_textureWidth != (GLsizei)frameToRender.Width() ||  _textureHeight != (GLsizei)frameToRender.Height())
    {
        setupTextures(frameToRender);
    }
    else
    {
        updateTextures(frameToRender);
    }

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, g_indices);
    checkGlError("glDrawArrays");

    return 0;
}



GLuint VideoRenderDelegeteGLES_2_0::loadShader(GLenum shaderType, const char* pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char* buf = (char*) malloc(infoLen);
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Could not compile shader %d: %s",
                            __FUNCTION__,shaderType, buf);
                    free(buf);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint VideoRenderDelegeteGLES_2_0::createProgram(const char* pVertexSource, const char* pFragmentSource) {
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader) {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader) {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program) {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE) {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength) {
                char* buf = (char*) malloc(bufLength);
                if (buf) {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    WEBRTC_TRACE(kTraceError, kTraceVideoRenderer, _id, "%s: Could not link program: %s",
                            __FUNCTION__, buf);
                    free(buf);
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}


void VideoRenderDelegeteGLES_2_0::printGLString(const char *name, GLenum s)
{
    const char *v = (const char *) glGetString(s);
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "GL %s = %s\n", name, v);
}

void VideoRenderDelegeteGLES_2_0::checkGlError(const char* op)
{
    for (GLint error = glGetError(); error; error = glGetError())
    {
    }
}


void VideoRenderDelegeteGLES_2_0::setupTextures(const VideoFrame& frameToRender)
{
    WEBRTC_TRACE(kTraceDebug, kTraceVideoRenderer, _id, "%s: width %d, height %d length %u",
                   __FUNCTION__,frameToRender.Width(), frameToRender.Height(),frameToRender.Length());

    const GLsizei width=frameToRender.Width();
    const GLsizei height=frameToRender.Height();

//    glDeleteTextures(3, _textureIds);
//    glGenTextures(3, _textureIds); //Generate  the Y, U and V texture
    GLuint currentTextureId = _textureIds[0]; // Y
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE, width,height,0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (const GLvoid*) frameToRender.Buffer());


    currentTextureId = _textureIds[1]; // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const WebRtc_UWord8* uComponent=frameToRender.Buffer()+width*height;
    glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,width/2,height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE,(const GLvoid*) uComponent);


    currentTextureId = _textureIds[2]; // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    const WebRtc_UWord8* vComponent=uComponent+(width*height)/4;
    glTexImage2D(GL_TEXTURE_2D,0,GL_LUMINANCE,width/2,height/2,0,GL_LUMINANCE,GL_UNSIGNED_BYTE, (const GLvoid*) vComponent);
    checkGlError("setupTextures");

    _textureWidth=width;
    _textureHeight=height;
    _textureCreated = true;

}

void VideoRenderDelegeteGLES_2_0::updateTextures(const VideoFrame& frameToRender)
{
    if(_textureCreated == false) return;


    const GLsizei width=frameToRender.Width();
    const GLsizei height=frameToRender.Height();

    if(width != _textureWidth ||
       height != _textureHeight){
        return;
    }

    GLuint currentTextureId = _textureIds[0]; ; // Y
    glActiveTexture(GL_TEXTURE0); ;
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,GL_LUMINANCE,GL_UNSIGNED_BYTE, (const GLvoid*) frameToRender.Buffer());

    currentTextureId = _textureIds[1]; // U
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    const WebRtc_UWord8* uComponent=frameToRender.Buffer()+width*height;
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_LUMINANCE,GL_UNSIGNED_BYTE, (const GLvoid*)uComponent);

    currentTextureId = _textureIds[2]; // V
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, currentTextureId);
    const WebRtc_UWord8* vComponent=uComponent+(width*height)/4;
    glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width/2,height/2,GL_LUMINANCE,GL_UNSIGNED_BYTE, (const GLvoid*)vComponent);
    checkGlError("updateTextures");
}
}
