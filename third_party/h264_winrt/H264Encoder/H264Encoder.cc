/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "third_party/h264_winrt/H264Encoder/H264Encoder.h"

#include <Windows.h>
#include <stdlib.h>
#include <ppltasks.h>
#include <mfapi.h>
#include <robuffer.h>
#include <wrl.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl\implements.h>
#include <codecapi.h>
#include <sstream>
#include <vector>
#include <iomanip>

#include "H264StreamSink.h"
#include "H264MediaSink.h"
#include "Utils/Utils.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "libyuv/convert.h"
#include "webrtc/base/logging.h"


#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid.lib")

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
  HRESULT hr = S_OK;

  webrtc::CriticalSectionScoped csLock(_lock.get());

  ON_SUCCEEDED(MFStartup(MF_VERSION));

  // output media type (h264)
  ON_SUCCEEDED(MFCreateMediaType(&mediaTypeOut_));
  ON_SUCCEEDED(mediaTypeOut_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ON_SUCCEEDED(mediaTypeOut_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
  // I find that 300kbit represents a good balance between video quality and
  // the bandwidth that a 620 Windows phone can handle.
  // TODO(winrt): Make bitrate a function of resolution.
  //              Change it when resolution changes.
  ON_SUCCEEDED(mediaTypeOut_->SetUINT32(MF_MT_AVG_BITRATE,
    inst->targetBitrate > 0 ? inst->targetBitrate : 300 * 1024));
  ON_SUCCEEDED(mediaTypeOut_->SetUINT32(MF_MT_INTERLACE_MODE,
    MFVideoInterlace_Progressive));
  ON_SUCCEEDED(MFSetAttributeSize(mediaTypeOut_.Get(),
    MF_MT_FRAME_SIZE, inst->width, inst->height));
  ON_SUCCEEDED(MFSetAttributeRatio(mediaTypeOut_.Get(),
    MF_MT_FRAME_RATE, inst->maxFramerate, 1));

  // input media type (nv12)
  ON_SUCCEEDED(MFCreateMediaType(&mediaTypeIn_));
  ON_SUCCEEDED(mediaTypeIn_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ON_SUCCEEDED(mediaTypeIn_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
  ON_SUCCEEDED(mediaTypeIn_->SetUINT32(
    MF_MT_INTERLACE_MODE, MFVideoInterlace_Progressive));
  ON_SUCCEEDED(mediaTypeIn_->SetUINT32(MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
  ON_SUCCEEDED(MFSetAttributeSize(mediaTypeIn_.Get(),
    MF_MT_FRAME_SIZE, inst->width, inst->height));
  ON_SUCCEEDED(MFSetAttributeRatio(mediaTypeIn_.Get(),
    MF_MT_FRAME_RATE, inst->maxFramerate, 1));

  // Create the media sink
  ON_SUCCEEDED(Microsoft::WRL::MakeAndInitialize<H264MediaSink>(&mediaSink_));

  // SinkWriter creation attributes
  ON_SUCCEEDED(MFCreateAttributes(&sinkWriterCreationAttributes_, 1));
  ON_SUCCEEDED(sinkWriterCreationAttributes_->SetUINT32(
    MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE));
  ON_SUCCEEDED(sinkWriterCreationAttributes_->SetUINT32(
    MF_SINK_WRITER_DISABLE_THROTTLING, FALSE));
  ON_SUCCEEDED(sinkWriterCreationAttributes_->SetUINT32(
    MF_LOW_LATENCY, TRUE));

  // Create the sink writer
  ON_SUCCEEDED(MFCreateSinkWriterFromMediaSink(mediaSink_.Get(),
    sinkWriterCreationAttributes_.Get(), &sinkWriter_));

  // Add the h264 output stream to the writer
  ON_SUCCEEDED(sinkWriter_->AddStream(mediaTypeOut_.Get(), &streamIndex_));

  // SinkWriter encoder properties
  ON_SUCCEEDED(MFCreateAttributes(&sinkWriterEncoderAttributes_, 1));
  ON_SUCCEEDED(sinkWriterEncoderAttributes_->SetUINT32(
    MF_MT_ALL_SAMPLES_INDEPENDENT, TRUE));
  ON_SUCCEEDED(sinkWriter_->SetInputMediaType(streamIndex_, mediaTypeIn_.Get(),
    sinkWriterEncoderAttributes_.Get()));

  // Register this as the callback for encoded samples.
  ON_SUCCEEDED(mediaSink_->RegisterEncodingCallback(this));

  ON_SUCCEEDED(sinkWriter_->BeginWriting());

  if (SUCCEEDED(hr)) {
    inited_ = true;
    return WEBRTC_VIDEO_CODEC_OK;
  } else {
    return hr;
  }
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
  HRESULT hr = S_OK;
  ComPtr<IMFSample> sample;
  ON_SUCCEEDED(MFCreateSample(sample.GetAddressOf()));

  ComPtr<IMFAttributes> sampleAttributes;
  ON_SUCCEEDED(sample.As(&sampleAttributes));

  if (SUCCEEDED(hr)) {
    auto totalSize = frame.allocated_size(PlaneType::kYPlane) +
      frame.allocated_size(PlaneType::kUPlane) +
      frame.allocated_size(PlaneType::kVPlane);

    ComPtr<IMFMediaBuffer> mediaBuffer;
    ON_SUCCEEDED(MFCreateMemoryBuffer(totalSize, mediaBuffer.GetAddressOf()));

    BYTE* destBuffer = nullptr;
    if (SUCCEEDED(hr)) {
      DWORD cbMaxLength;
      DWORD cbCurrentLength;
      ON_SUCCEEDED(mediaBuffer->Lock(
        &destBuffer, &cbMaxLength, &cbCurrentLength));
    }

    if (SUCCEEDED(hr)) {
      // TODO(winrt): Internally, H264 uses dimensions with multiples of 16.
      //              Might be possible to anticipate this and provie a sample
      //              with dimensions that fit that characteristic.
      //              Unsure how the difference in stride vs width would be
      //              specified on the sample.
      BYTE* destUV = destBuffer +
        (frame.stride(PlaneType::kYPlane) * frame.height());
      libyuv::I420ToNV12(
        frame.buffer(PlaneType::kYPlane), frame.stride(PlaneType::kYPlane),
        frame.buffer(PlaneType::kUPlane), frame.stride(PlaneType::kUPlane),
        frame.buffer(PlaneType::kVPlane), frame.stride(PlaneType::kVPlane),
        destBuffer, frame.stride(PlaneType::kYPlane),
        destUV, frame.stride(PlaneType::kYPlane),
        frame.width(),
        frame.height());
    }

    ON_SUCCEEDED(mediaBuffer->SetCurrentLength(
      frame.width() * frame.height() * 3 / 2));

    if (destBuffer != nullptr) {
      mediaBuffer->Unlock();
    }

    ON_SUCCEEDED(sample->AddBuffer(mediaBuffer.Get()));
  }

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
      startTime_ = frame.timestamp();
    }

    auto timestampHns = ((frame.timestamp() - startTime_) / 90) * 1000 * 10;
    ON_SUCCEEDED(sample->SetSampleTime(timestampHns));

    if (SUCCEEDED(hr)) {
      auto durationHns = timestampHns - lastTimestampHns_;
      hr = sample->SetSampleDuration(durationHns);
    }

    if (SUCCEEDED(hr)) {
      lastTimestampHns_ = timestampHns;

      // Cache the frame attributes to get them back after the encoding.
      CachedFrameAttributes frameAttributes;
      frameAttributes.timestamp = frame.timestamp();
      frameAttributes.ntpTime = frame.ntp_time_ms();
      frameAttributes.captureRenderTime = frame.render_time_ms();
      _sampleAttributeQueue.push(timestampHns, frameAttributes);
    }

    UINT32 oldWidth = 0, oldHeight = 0;
    ON_SUCCEEDED(MFGetAttributeSize(mediaTypeIn_.Get(),
      MF_MT_FRAME_SIZE, &oldWidth, &oldHeight));

    if (SUCCEEDED(hr) && (static_cast<int>(oldWidth) != frame.width() ||
      static_cast<int>(oldHeight) != frame.height())) {
      ON_SUCCEEDED(MFSetAttributeSize(mediaTypeIn_.Get(),
        MF_MT_FRAME_SIZE, frame.width(), frame.height()));
      ON_SUCCEEDED(MFSetAttributeSize(mediaTypeOut_.Get(),
        MF_MT_FRAME_SIZE, frame.width(), frame.height()));

      ComPtr<IMFSinkWriterEncoderConfig> encoderConfig;
      ON_SUCCEEDED(sinkWriter_.As(&encoderConfig));
      ON_SUCCEEDED(encoderConfig->SetTargetMediaType(streamIndex_,
        mediaTypeOut_.Get(), sinkWriterEncoderAttributes_.Get()));
      ON_SUCCEEDED(sinkWriter_->SetInputMediaType(streamIndex_,
        mediaTypeIn_.Get(), sinkWriterEncoderAttributes_.Get()));
    }

    if (SUCCEEDED(hr) && framePendingCount_ > 30) {
      OutputDebugString(L"!");
      hr = sinkWriter_->Flush((DWORD)MF_SINK_WRITER_ALL_STREAMS);
      framePendingCount_ = 0;
    }

    ON_SUCCEEDED(sinkWriter_->WriteSample(streamIndex_, sample.Get()));

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
  HRESULT hr = S_OK;
  ON_SUCCEEDED(sample->GetTotalLength(&totalLength));

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

    UINT32 outWidth = 0, outHeight = 0;
    ON_SUCCEEDED(MFGetAttributeSize(mediaTypeOut_.Get(),
      MF_MT_FRAME_SIZE, &outWidth, &outHeight));
    if (FAILED(hr)) {
      return;
    }

    // sendBuffer is not copied here.
    EncodedImage encodedImage(sendBuffer.data(), curLength, curLength);
    encodedImage._encodedHeight = outHeight;
    encodedImage._encodedWidth = outWidth;

    ComPtr<IMFAttributes> sampleAttributes;
    hr = sample.As(&sampleAttributes);
    if (SUCCEEDED(hr)) {
      UINT32 cleanPoint;
      hr = sampleAttributes->GetUINT32(
        MFSampleExtension_CleanPoint, &cleanPoint);
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
      int prefixLengthFound = 0;
      if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01
        && ((ptr[4] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
        prefixLengthFound = 4;
      } else if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01
        && ((ptr[3] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
        prefixLengthFound = 3;
      }
      if (prefixLengthFound > 0) {
        fragmentationHeader.VerifyAndAllocateFragmentationHeader(fragIdx + 1);
        fragmentationHeader.fragmentationOffset[fragIdx] = i + prefixLengthFound;
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
        sendBuffer.size() -
        fragmentationHeader.fragmentationOffset[fragIdx - 1];
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
  hr = MFGetAttributeRatio(mediaTypeOut_.Get(),
    MF_MT_FRAME_RATE, &old_framerate, &one);

  if (old_bitrate_kbit != new_bitrate_kbit) {
    LOG(LS_INFO) << "H264WinRTEncoderImpl::SetRates("
      << new_bitrate_kbit << "kbit " << new_framerate << "fps)";
    hr = mediaTypeOut_->SetUINT32(MF_MT_AVG_BITRATE, new_bitrate_kbit * 1024);
#if 0  // TODO(winrt): Changing FPS causes significant hiccup in encoding.  Ignore it for now.
    hr = MFSetAttributeRatio(mediaTypeOut_.Get(), MF_MT_FRAME_RATE, new_framerate, 1);
    hr = MFSetAttributeRatio(mediaTypeIn_.Get(), MF_MT_FRAME_RATE, new_framerate, 1);
#endif

    ComPtr<IMFSinkWriterEncoderConfig> encoderConfig;
    ON_SUCCEEDED(sinkWriter_.As(&encoderConfig));

    ON_SUCCEEDED(encoderConfig->SetTargetMediaType(
      streamIndex_, mediaTypeOut_.Get(), sinkWriterEncoderAttributes_.Get()));
    if (FAILED(hr)) {
      LOG(LS_ERROR) << "SetTargetMediaType failed: " << hr;
    }

#if 0  // TODO(winrt): Changing FPS causes significant hiccup in encoding.  Ignore it for now.
    ON_SUCCEEDED(sinkWriter_->SetInputMediaType(
      streamIndex_, mediaTypeIn_.Get(), sinkWriterEncoderAttributes_.Get()));
#endif
  }

  return WEBRTC_VIDEO_CODEC_OK;
}

}  // namespace webrtc
