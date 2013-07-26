/*

 Copyright (c) 2013, SMB Phone Inc.
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.

 */

#ifndef WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_BLACKBERRY_VIDEO_CAPTURE_BB_H_
#define WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_BLACKBERRY_VIDEO_CAPTURE_BB_H_

#include "common_types.h"
#include "../video_capture_impl.h"
#include <camera/camera_api.h>

namespace webrtc
{
class CriticalSectionWrapper;
class ThreadWrapper;
namespace videocapturemodule
{
class VideoCaptureModuleBB : public VideoCaptureImpl
{
public:
	VideoCaptureModuleBB(int32_t id);
    virtual ~VideoCaptureModuleBB();
    virtual int32_t Init(const char* deviceUniqueId);
    virtual int32_t StartCapture(const VideoCaptureCapability& capability);
    virtual int32_t StopCapture();
    virtual bool CaptureStarted();
    virtual int32_t CaptureSettings(VideoCaptureCapability& settings);

    int32_t IncomingFrameBB(uint8_t* videoFrame,
    		int32_t videoFrameLength,
            const VideoCaptureCapability& frameInfo,
            int64_t captureTime = 0,
            bool faceDetected = false);

private:

    camera_handle_t _cameraHandle;

    int32_t _currentWidth;
    int32_t _currentHeight;
    int32_t _currentFrameRate;
    bool _captureStarted;
    RawVideoType _captureVideoType;
};
} // namespace videocapturemodule
} // namespace webrtc

#endif // WEBRTC_MODULES_VIDEO_CAPTURE_MAIN_SOURCE_BLACKBERRY_VIDEO_CAPTURE_BB_H_
