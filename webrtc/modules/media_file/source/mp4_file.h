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

#ifndef WEBRTC_MODULES_MEDIA_FILE_SOURCE_MP4_FILE_H_
#define WEBRTC_MODULES_MEDIA_FILE_SOURCE_MP4_FILE_H_

#include <stdio.h>

#include "webrtc/typedefs.h"
#include "mp4_file_objc.h"

namespace webrtc {
class CriticalSectionWrapper;
class ListWrapper;

class MP4File
{
public:
    
    MP4File();
    ~MP4File();
    
    int32_t CreateVideoStream(uint16_t width, uint16_t height, uint32_t bitrate);
    int32_t CreateAudioStream(int32_t samplerate, uint16_t bitrate);
    int32_t Create(const char* fileName, bool saveVideoToLibrary);
    
    int32_t WriteAudio(const uint8_t* data, int32_t length,
                             uint32_t timeStamp);
    int32_t WriteVideo(const uint8_t* data, int32_t length,
                             uint32_t timeStamp);
    
    int32_t Close();
    
private:
    enum MP4FileMode
    {
        NotSet,
        Write
    };

    void CloseWrite();
    
    void ResetMembers();
    
private:
    
    CriticalSectionWrapper* _crit;
    
    MP4FileObjC* _mp4File;
    NSURL* _movieURL;

    int32_t _videoFrames;
    int32_t _audioFrames;
    
    size_t _bytesWritten;
    
    bool _writeAudioStream;
    bool _writeVideoStream;
    
    MP4FileMode _mp4Mode;
    bool _created;
};
} // namespace webrtc

#endif // WEBRTC_MODULES_MEDIA_FILE_SOURCE_MP4_FILE_H_
