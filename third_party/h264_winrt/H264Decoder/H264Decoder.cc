/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "third_party/h264_winrt/H264Decoder/H264Decoder.h"

#include <Windows.h>
#include <stdlib.h>
#include <ppltasks.h>
#include <mfapi.h>
#include <robuffer.h>
#include <wrl.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl\implements.h>
#include <iomanip>
#include "Utils/Utils.h"
#include "libyuv/convert.h"
#include "webrtc/base/logging.h"
#include "webrtc/base/checks.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid.lib")

namespace webrtc {

//////////////////////////////////////////
// H264 WinRT Decoder Implementation
//////////////////////////////////////////

H264WinRTDecoderImpl::H264WinRTDecoderImpl()
  : width_(0),
  height_(0),
  _cbLock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , decodeCompleteCallback_(nullptr) {
}

H264WinRTDecoderImpl::~H264WinRTDecoderImpl() {
  OutputDebugString(L"H264WinRTDecoderImpl::~H264WinRTDecoderImpl()\n");
  Release();
}

int H264WinRTDecoderImpl::InitDecode(const VideoCodec* inst,
  int number_of_cores) {
  // Nothing to do here, decoder acts as a passthrough
  return 0;
}

ComPtr<IMFSample> FromEncodedImage(const EncodedImage& input_image) {
  HRESULT hr = S_OK;

  ComPtr<IMFSample> sample;
  hr = MFCreateSample(&sample);

  ComPtr<IMFMediaBuffer> mediaBuffer;
  ON_SUCCEEDED(MFCreateMemoryBuffer((DWORD)input_image._length, &mediaBuffer));

  BYTE* destBuffer = NULL;
  if (SUCCEEDED(hr)) {
    DWORD cbMaxLength;
    DWORD cbCurrentLength;
    hr = mediaBuffer->Lock(&destBuffer, &cbMaxLength, &cbCurrentLength);
  }

  if (SUCCEEDED(hr)) {
    memcpy(destBuffer, input_image._buffer, input_image._length);
  }

  ON_SUCCEEDED(mediaBuffer->SetCurrentLength((DWORD)input_image._length));
  ON_SUCCEEDED(mediaBuffer->Unlock());

  ON_SUCCEEDED(sample->AddBuffer(mediaBuffer.Get()));

  return sample;
}

// Used to store an encoded H264 sample in a VideoFrame
class H264NativeHandleBuffer : public NativeHandleBuffer {
public:
  H264NativeHandleBuffer(ComPtr<IMFSample> sample, int width, int height)
    : NativeHandleBuffer(sample.Get(), width, height)
    , _sample(sample) {
  }

  virtual ~H264NativeHandleBuffer() {
  }

  rtc::scoped_refptr<VideoFrameBuffer> NativeToI420Buffer() override {
    RTC_NOTREACHED();  // Should not be called.
    return nullptr;
  }

private:
  ComPtr<IMFSample> _sample;
};

int H264WinRTDecoderImpl::Decode(const EncodedImage& input_image,
  bool missing_frames,
  const RTPFragmentationHeader* fragmentation,
  const CodecSpecificInfo* codec_specific_info,
  int64_t render_time_ms) {

  UpdateVideoFrameDimensions(input_image);
  auto sample = FromEncodedImage(input_image);

  if (sample != nullptr) {
    rtc::scoped_refptr<VideoFrameBuffer> buffer(new rtc::RefCountedObject<H264NativeHandleBuffer>(
      sample, width_, height_));
    VideoFrame decodedFrame(buffer, input_image._timeStamp, render_time_ms, kVideoRotation_0);

    webrtc::CriticalSectionScoped csLock(_cbLock.get());

    if (decodeCompleteCallback_ != nullptr) {
      decodeCompleteCallback_->Decoded(decodedFrame);
    }
  }
  return 0;
}

int H264WinRTDecoderImpl::RegisterDecodeCompleteCallback(
  DecodedImageCallback* callback) {
  webrtc::CriticalSectionScoped csLock(_cbLock.get());
  decodeCompleteCallback_ = callback;
  return 0;
}

int H264WinRTDecoderImpl::Release() {
  OutputDebugString(L"H264WinRTDecoderImpl::Release()\n");
  return 0;
}

int H264WinRTDecoderImpl::Reset() {
  // TODO(winrt): Figure out when this happens.
  return 0;
}


VideoDecoder* H264WinRTDecoderImpl::Copy() {
  // TODO(winrt): Implement?
  return nullptr;
}

void H264WinRTDecoderImpl::UpdateVideoFrameDimensions(const EncodedImage& input_image)
{
  if (input_image._frameType == VideoFrameType::kKeyFrame)
  {
    width_ = input_image._encodedWidth;
    height_ = input_image._encodedHeight;
  }
}

}  // namespace webrtc
