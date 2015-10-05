/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "H264Encoder.h"
#include "H264StreamSink.h"
#include "H264MediaSink.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "../Utils/Utils.h"
#include "libyuv/convert.h"
#include "webrtc/base/logging.h"

#include <Windows.h>
#include <stdlib.h>
#include <ppltasks.h>
#include <mfapi.h>
#include <robuffer.h>
#include <wrl.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl\implements.h>
#include <sstream>
#include <iomanip>
#include <codecapi.h>

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid.lib")

using namespace concurrency;
using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::Foundation;
using namespace Windows::Media::Core;
using namespace Windows::Media::MediaProperties;
using namespace Windows::Media::Transcoding;
using namespace Windows::Storage::Streams;

namespace webrtc {

//////////////////////////////////////////
// H264 WinRT Encoder Implementation
//////////////////////////////////////////

H264WinRTEncoderImpl::H264WinRTEncoderImpl()
  : _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , firstFrame_(true)
  , startTime_(0)
  , framePendingCount_(0)
  , lastTimestampHns_(0) {
}

H264WinRTEncoderImpl::~H264WinRTEncoderImpl() {
  OutputDebugString(L"H264WinRTEncoderImpl::~H264WinRTEncoderImpl()\n");
  Release();
}

int H264WinRTEncoderImpl::InitEncode(const VideoCodec* inst,
  int number_of_cores,
  size_t /*maxPayloadSize */) {

  webrtc::CriticalSectionScoped csLock(_lock.get());

  ThrowIfError(MFStartup(MF_VERSION));

  // output media type (h264)
  ThrowIfError(MFCreateMediaType(&mediaTypeOut_));
  ThrowIfError(mediaTypeOut_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ThrowIfError(mediaTypeOut_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
  // I find that 300kbit represents a good balance between video quality and
  // the bandwidth that a 620 Windows phone can handle.
  ThrowIfError(mediaTypeOut_->SetUINT32(MF_MT_AVG_BITRATE, inst->targetBitrate > 0 ? inst->targetBitrate : 300 * 1024));
  ThrowIfError(mediaTypeOut_->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
  ThrowIfError(mediaTypeOut_->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
  // These are for increasing the number of keyframes.  It improves time to first frame and
  // recovery when the video freezes.
  ThrowIfError(mediaTypeOut_->SetUINT32(MF_MT_MAX_KEYFRAME_SPACING, 30));
  ThrowIfError(mediaTypeOut_->SetUINT32(CODECAPI_AVEncMPVGOPSize, 10));
  ThrowIfError(mediaTypeOut_->SetUINT32(CODECAPI_AVEncVideoMaxKeyframeDistance, 10));
  ThrowIfError(MFSetAttributeSize(mediaTypeOut_.Get(), MF_MT_FRAME_SIZE, inst->width, inst->height));
  ThrowIfError(MFSetAttributeRatio(mediaTypeOut_.Get(), MF_MT_FRAME_RATE, inst->maxFramerate, 1));

  // input media type (nv12)
  ThrowIfError(MFCreateMediaType(&mediaTypeIn_));
  ThrowIfError(mediaTypeIn_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ThrowIfError(mediaTypeIn_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
  ThrowIfError(mediaTypeIn_->SetUINT32(MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
  ThrowIfError(mediaTypeIn_->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
  ThrowIfError(MFSetAttributeSize(mediaTypeIn_.Get(), MF_MT_FRAME_SIZE, inst->width, inst->height));
  ThrowIfError(MFSetAttributeRatio(mediaTypeIn_.Get(), MF_MT_FRAME_RATE, inst->maxFramerate, 1));

  // Create the media sink
  ThrowIfError(MakeAndInitialize<H264MediaSink>(&mediaSink_));

  // SinkWriter creation attributes
  ThrowIfError(MFCreateAttributes(&sinkWriterCreationAttributes_, 1));
  ThrowIfError(sinkWriterCreationAttributes_->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
  ThrowIfError(sinkWriterCreationAttributes_->SetUINT32(MF_SINK_WRITER_DISABLE_THROTTLING, FALSE));
  ThrowIfError(sinkWriterCreationAttributes_->SetUINT32(MF_LOW_LATENCY, TRUE));

  // Create the sink writer
  ThrowIfError(MFCreateSinkWriterFromMediaSink(mediaSink_.Get(), sinkWriterCreationAttributes_.Get(), &sinkWriter_));

  // Add the h264 output stream to the writer
  ThrowIfError(sinkWriter_->AddStream(mediaTypeOut_.Get(), &streamIndex_));

  // SinkWriter encoder properties
  ThrowIfError(MFCreateAttributes(&sinkWriterEncoderAttributes_, 1));
  ThrowIfError(sinkWriterEncoderAttributes_->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
  ThrowIfError(sinkWriter_->SetInputMediaType(streamIndex_, mediaTypeIn_.Get(), sinkWriterEncoderAttributes_.Get()));

  // Register this as the callback for encoded samples.
  mediaSink_->RegisterEncodingCallback(this);

  ThrowIfError(sinkWriter_->BeginWriting());

  inited_ = true;

  return WEBRTC_VIDEO_CODEC_OK;
}

int H264WinRTEncoderImpl::RegisterEncodeCompleteCallback(
  EncodedImageCallback* callback) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  encodedCompleteCallback_ = callback;
  return WEBRTC_VIDEO_CODEC_OK;
}

int H264WinRTEncoderImpl::Release() {
  OutputDebugString(L"H264WinRTEncoderImpl::Release()\n");
  webrtc::CriticalSectionScoped csLock(_lock.get());
  sinkWriter_.Reset();
  if (mediaSink_ != nullptr) {
    mediaSink_->Shutdown();
  }
  sinkWriterCreationAttributes_.Reset();
  sinkWriterEncoderAttributes_.Reset();
  mediaTypeOut_.Reset();
  mediaTypeIn_.Reset();
  mediaSink_.Reset();
  encodedCompleteCallback_ = nullptr;
  startTime_ = 0;
  lastTimestampHns_ = 0;
  firstFrame_ = true;
  inited_ = false;
  return WEBRTC_VIDEO_CODEC_OK;
}

ComPtr<IMFSample> FromVideoFrame(const VideoFrame& frame) {
  HRESULT hr;
  ComPtr<IMFSample> sample;
  hr = MFCreateSample(sample.GetAddressOf());
  ThrowIfError(hr);

  ComPtr<IMFAttributes> sampleAttributes;
  hr = sample.As(&sampleAttributes);
  ThrowIfError(hr);

  auto totalSize = frame.allocated_size(PlaneType::kYPlane) +
    frame.allocated_size(PlaneType::kUPlane) +
    frame.allocated_size(PlaneType::kVPlane);

  ComPtr<IMFMediaBuffer> mediaBuffer;
  hr = MFCreateMemoryBuffer(totalSize, mediaBuffer.GetAddressOf());
  ThrowIfError(hr);

  BYTE* destBuffer = NULL;
  if (SUCCEEDED(hr)) {
    DWORD cbMaxLength;
    DWORD cbCurrentLength;
    hr = mediaBuffer->Lock(&destBuffer, &cbMaxLength, &cbCurrentLength);
  }

  BYTE* destUV = destBuffer + (frame.stride(PlaneType::kYPlane) * frame.height());
  libyuv::I420ToNV12(
    frame.buffer(PlaneType::kYPlane), frame.stride(PlaneType::kYPlane),
    frame.buffer(PlaneType::kUPlane), frame.stride(PlaneType::kUPlane),
    frame.buffer(PlaneType::kVPlane), frame.stride(PlaneType::kVPlane),
    destBuffer, frame.stride(PlaneType::kYPlane),
    destUV, frame.stride(PlaneType::kYPlane),
    frame.width(),
    frame.height());

  hr = mediaBuffer->SetCurrentLength(frame.width() * frame.height() * 3 / 2);
  ThrowIfError(hr);

  hr = mediaBuffer->Unlock();
  ThrowIfError(hr);

  hr = sample->AddBuffer(mediaBuffer.Get());
  ThrowIfError(hr);

  return sample;
}

int H264WinRTEncoderImpl::Encode(
  const VideoFrame& frame,
  const CodecSpecificInfo* codec_specific_info,
  const std::vector<VideoFrameType>* frame_types) {

  {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (!inited_) {
      return -1;
    }
  }
  HRESULT hr = S_OK;

  codecSpecificInfo_ = codec_specific_info;

  auto sample = FromVideoFrame(frame);

  {
    webrtc::CriticalSectionScoped csLock(_lock.get());

    if (firstFrame_) {
      firstFrame_ = false;
    }

    auto timestampHns = ((frame.timestamp() - startTime_) / 90) * 1000 * 10;
    hr = sample->SetSampleTime(timestampHns);
    ThrowIfError(hr);

    auto durationHns = timestampHns - lastTimestampHns_;
    hr = sample->SetSampleDuration(durationHns);
    ThrowIfError(hr);

    lastTimestampHns_ = timestampHns;

    // Cache the frame attributes to get them back after the encoding.
    CachedFrameAttributes frameAttributes;
    frameAttributes.timestamp = frame.timestamp();
    frameAttributes.ntpTime = frame.ntp_time_ms();
    frameAttributes.captureRenderTime = frame.render_time_ms();
    _sampleAttributeQueue.push(timestampHns, frameAttributes);

    UINT32 oldWidth, oldHeight;
    hr = MFGetAttributeSize(mediaTypeIn_.Get(), MF_MT_FRAME_SIZE, &oldWidth, &oldHeight);
    ThrowIfError(hr);

    if ((int)oldWidth != frame.width() || (int)oldHeight != frame.height()) {
      hr = MFSetAttributeSize(mediaTypeIn_.Get(), MF_MT_FRAME_SIZE, frame.width(), frame.height());
      ThrowIfError(hr);
      hr = MFSetAttributeSize(mediaTypeOut_.Get(), MF_MT_FRAME_SIZE, frame.width(), frame.height());
      ThrowIfError(hr);

      ComPtr<IMFSinkWriterEncoderConfig> encoderConfig;
      hr = sinkWriter_.As(&encoderConfig);
      ThrowIfError(hr);

      hr = encoderConfig->SetTargetMediaType(streamIndex_, mediaTypeOut_.Get(), sinkWriterEncoderAttributes_.Get());
      ThrowIfError(hr);

      hr = sinkWriter_->SetInputMediaType(streamIndex_, mediaTypeIn_.Get(), sinkWriterEncoderAttributes_.Get());
      ThrowIfError(hr);
    }

    if (framePendingCount_ > 30) {
      OutputDebugString(L"!");
      hr = sinkWriter_->Flush((DWORD)MF_SINK_WRITER_ALL_STREAMS);
      ThrowIfError(hr);
      framePendingCount_ = 0;
    }

    hr = sinkWriter_->WriteSample(streamIndex_, sample.Get());
    ThrowIfError(hr);

    ++framePendingCount_;
  }
  return WEBRTC_VIDEO_CODEC_OK;
}

void H264WinRTEncoderImpl::OnH264Encoded(ComPtr<IMFSample> sample) {
  {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (!inited_ || encodedCompleteCallback_ == nullptr) {
      return;
    }
    --framePendingCount_;
  }

  DWORD totalLength;
  HRESULT hr = sample->GetTotalLength(&totalLength);
  ThrowIfError(hr);

  ComPtr<IMFMediaBuffer> buffer;
  hr = sample->GetBufferByIndex(0, &buffer);

  if (SUCCEEDED(hr)) {
    BYTE* byteBuffer;
    DWORD maxLength;
    DWORD curLength;
    hr = buffer->Lock(&byteBuffer, &maxLength, &curLength);
    if (FAILED(hr)) {
      return;
    }
    if (curLength == 0) {
      LOG(LS_WARNING) << "Got empty sample.";
      buffer->Unlock();
      return;
    }
    std::vector<byte> sendBuffer;
    sendBuffer.resize(curLength);
    memcpy(sendBuffer.data(), byteBuffer, curLength);
    hr = buffer->Unlock();
    if (FAILED(hr)) {
      return;
    }

    UINT32 outWidth, outHeight;
    hr = MFGetAttributeSize(mediaTypeOut_.Get(), MF_MT_FRAME_SIZE, &outWidth, &outHeight);
    ThrowIfError(hr);

    // sendBuffer is not copied here.
    EncodedImage encodedImage(sendBuffer.data(), curLength, curLength);
    encodedImage._encodedHeight = outHeight;
    encodedImage._encodedWidth = outWidth;

    ComPtr<IMFAttributes> sampleAttributes;
    hr = sample.As(&sampleAttributes);
    if (SUCCEEDED(hr)) {
      UINT32 cleanPoint;
      hr = sampleAttributes->GetUINT32(MFSampleExtension_CleanPoint, &cleanPoint);
      if (SUCCEEDED(hr) && cleanPoint) {
        encodedImage._completeFrame = true;
        encodedImage._frameType = kKeyFrame;
      }
    }

    RTPFragmentationHeader fragmentationHeader;
    // TODO(winrt): Optimization, calculate number of fragments ahead.

    // Scan for and create mark all fragments.
    uint32_t fragIdx = 0;
    for (uint32_t i = 0; i < sendBuffer.size() - 5; ++i) {
      byte* ptr = sendBuffer.data() + i;
      if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01
        && ((ptr[4] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
        fragmentationHeader.VerifyAndAllocateFragmentationHeader(fragIdx + 1);
        fragmentationHeader.fragmentationOffset[fragIdx] = i + 4;
        fragmentationHeader.fragmentationLength[fragIdx] = 0;  // We'll set that later
        // Set the length of the previous fragment.
        if (fragIdx > 0) {
          fragmentationHeader.fragmentationLength[fragIdx - 1] =
            i - fragmentationHeader.fragmentationOffset[fragIdx - 1];
        }
        fragmentationHeader.fragmentationPlType[fragIdx] = 0;
        fragmentationHeader.fragmentationTimeDiff[fragIdx] = 0;
        ++fragIdx;
        i += 5;
      }
    }
    // Set the length of the last fragment.
    if (fragIdx > 0) {
      fragmentationHeader.fragmentationLength[fragIdx - 1] =
        sendBuffer.size() - fragmentationHeader.fragmentationOffset[fragIdx - 1];
    }

    {
      webrtc::CriticalSectionScoped csLock(_lock.get());

      LONGLONG sampleTimestamp;
      sample->GetSampleTime(&sampleTimestamp);

      CachedFrameAttributes frameAttributes;
      if (_sampleAttributeQueue.pop(sampleTimestamp, frameAttributes)) {
        encodedImage._timeStamp = frameAttributes.timestamp;
        encodedImage.ntp_time_ms_ = frameAttributes.ntpTime;
        encodedImage.capture_time_ms_ = frameAttributes.captureRenderTime;
      }

      if (encodedCompleteCallback_ != nullptr) {
        encodedCompleteCallback_->Encoded(
          encodedImage, codecSpecificInfo_, &fragmentationHeader);
      }
    }
  }
}

int H264WinRTEncoderImpl::SetChannelParameters(
  uint32_t packetLoss, int64_t rtt) {
  return WEBRTC_VIDEO_CODEC_OK;
}

static uint32_t setRatesBuffer = 0;

int H264WinRTEncoderImpl::SetRates(
  uint32_t new_bitrate_kbit, uint32_t new_framerate) {

  // TODO(winrt): Revisit this function once we know how to work around
  //              the crash in the H264 stack that sometimes happens.
  return WEBRTC_VIDEO_CODEC_OK;

  setRatesBuffer++;
  if (setRatesBuffer < 20) {
    return WEBRTC_VIDEO_CODEC_OK;
  }
  setRatesBuffer = 0;

  webrtc::CriticalSectionScoped csLock(_lock.get());
  if (sinkWriter_ == nullptr) {
    return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
  }

  HRESULT hr = S_OK;

  uint32_t old_bitrate_kbit, old_framerate, one;

  hr = mediaTypeOut_->GetUINT32(MF_MT_AVG_BITRATE, &old_bitrate_kbit);
  old_bitrate_kbit /= 1000;
  hr = MFGetAttributeRatio(mediaTypeOut_.Get(), MF_MT_FRAME_RATE, &old_framerate, &one);

  if (old_bitrate_kbit != new_bitrate_kbit) {
    LOG(LS_INFO) << "H264WinRTEncoderImpl::SetRates("
      << new_bitrate_kbit << "kbit " << new_framerate << "fps)";
    hr = mediaTypeOut_->SetUINT32(MF_MT_AVG_BITRATE, new_bitrate_kbit * 1024);
#if 0 // TODO(winrt): Changing FPS causes significant hiccup in encoding.  Ignore it for now.
    hr = MFSetAttributeRatio(mediaTypeOut_.Get(), MF_MT_FRAME_RATE, new_framerate, 1);
    hr = MFSetAttributeRatio(mediaTypeIn_.Get(), MF_MT_FRAME_RATE, new_framerate, 1);
#endif

    ComPtr<IMFSinkWriterEncoderConfig> encoderConfig;
    hr = sinkWriter_.As(&encoderConfig);
    ThrowIfError(hr);

    hr = encoderConfig->SetTargetMediaType(streamIndex_, mediaTypeOut_.Get(), sinkWriterEncoderAttributes_.Get());
    if (FAILED(hr)) {
      LOG(LS_ERROR) << "SetTargetMediaType failed: " << hr;
    }

#if 0 // TODO(winrt): Changing FPS causes significant hiccup in encoding.  Ignore it for now.
    hr = sinkWriter_->SetInputMediaType(streamIndex_, mediaTypeIn_.Get(), sinkWriterEncoderAttributes_.Get());
    ThrowIfError(hr);
#endif
  }

  return WEBRTC_VIDEO_CODEC_OK;
}

}  // namespace webrtc
