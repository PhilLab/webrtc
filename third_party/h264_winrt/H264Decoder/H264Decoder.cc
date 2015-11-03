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
#include "Utils/Utils.h"
#include "libyuv/convert.h"
#include "webrtc/base/logging.h"

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid.lib")

namespace webrtc {

//////////////////////////////////////////
// H264 WinRT Decoder Implementation
//////////////////////////////////////////

H264WinRTDecoderImpl::H264WinRTDecoderImpl()
  : _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , _cbLock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , decodeCompleteCallback_(nullptr) {
  Microsoft::WRL::MakeAndInitialize<SourceReaderCB>(&readerCB_);
  readerCB_->RegisterDecodingCallback(this);
}

H264WinRTDecoderImpl::~H264WinRTDecoderImpl() {
  OutputDebugString(L"H264WinRTDecoderImpl::~H264WinRTDecoderImpl()\n");
  Release();
}

int H264WinRTDecoderImpl::InitDecode(const VideoCodec* inst,
  int number_of_cores) {
  webrtc::CriticalSectionScoped csLock(_lock.get());
  HRESULT hr = S_OK;

  // output media type (nv12)
  ON_SUCCEEDED(MFCreateMediaType(&mediaTypeOut_));
  ON_SUCCEEDED(mediaTypeOut_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ON_SUCCEEDED(mediaTypeOut_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12));
  // input media type (h264)
  ON_SUCCEEDED(MFCreateMediaType(&mediaTypeIn_));
  ON_SUCCEEDED(mediaTypeIn_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video));
  ON_SUCCEEDED(mediaTypeIn_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264));
  ComPtr<H264MediaSource> mediaSource;
  ON_SUCCEEDED(Microsoft::WRL::MakeAndInitialize<H264MediaSource>(
    &mediaSource, mediaTypeIn_.Get(), &mediaStream_));

  ComPtr<IMFAttributes> readerAttributes;
  ON_SUCCEEDED(MFCreateAttributes(&readerAttributes, 1));

  ComPtr<IUnknown> readerCBAsUnknown;
  ON_SUCCEEDED(readerCB_.As(&readerCBAsUnknown));
  ON_SUCCEEDED(readerAttributes->SetUnknown(
    MF_SOURCE_READER_ASYNC_CALLBACK, readerCBAsUnknown.Get()));
  ON_SUCCEEDED(readerAttributes->SetUINT32(MF_LOW_LATENCY, TRUE));

  ON_SUCCEEDED(MFCreateSourceReaderFromMediaSource(
    mediaSource.Get(), readerAttributes.Get(), &sourceReader_));

  ON_SUCCEEDED(sourceReader_->SetCurrentMediaType(
    0, nullptr, mediaTypeOut_.Get()));

  ON_SUCCEEDED(sourceReader_->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM,
    0, nullptr, nullptr, nullptr, nullptr));

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

  ON_SUCCEEDED(sample->SetSampleTime(
    (input_image._timeStamp / 90) * 10 * 1000));

  if (SUCCEEDED(hr)) {
    // We don't get duration information from webrtc.
    // Hardcode duration based on 30fps.  The exact value doesn't
    // seem to have any impact.
    auto duration = static_cast<LONGLONG>(
      (1.0f / static_cast<float>(30)) * 1000 * 1000 * 10);
    hr = sample->SetSampleDuration(duration);
  }

  return sample;
}

int H264WinRTDecoderImpl::Decode(const EncodedImage& input_image,
  bool missing_frames,
  const RTPFragmentationHeader* fragmentation,
  const CodecSpecificInfo* codec_specific_info,
  int64_t /*render_time_ms*/) {
  HRESULT hr = S_OK;

  auto sample = FromEncodedImage(input_image);

  {
    webrtc::CriticalSectionScoped csLock(_lock.get());

    CachedFrameAttributes frameAttributes;
    LONGLONG sampleTimestamp;
    sample->GetSampleTime(&sampleTimestamp);

    frameAttributes.timestamp = input_image._timeStamp;
    frameAttributes.ntpTime = input_image.ntp_time_ms_;
    frameAttributes.captureRenderTime = input_image.capture_time_ms_;
    _sampleAttributeQueue.push(sampleTimestamp, frameAttributes);

    if (mediaStream_ != nullptr) {
      mediaStream_->ProcessSample(sample.Get());
    }
  }

  return hr;
}

HRESULT FromSample(ComPtr<IMFSample> pSample,
  UINT32 sampleWidth, UINT32 sampleHeight,
  UINT32 width, UINT32 height, VideoFrame* decodedFrame) {
  HRESULT hr = S_OK;

  if (decodedFrame == nullptr) {
    return E_INVALIDARG;
  }

  decodedFrame->CreateEmptyFrame(width, height, width,
    (width + 1) / 2, (width + 1) / 2);

  ComPtr<IMFMediaBuffer> buffer;
  hr = pSample->GetBufferByIndex(0, &buffer);
  if (SUCCEEDED(hr)) {
    BYTE* bufferBytes;
    DWORD maxLength, curLength;
    hr = buffer->Lock(&bufferBytes, &maxLength, &curLength);
    if (SUCCEEDED(hr)) {
      libyuv::NV12ToI420(
        bufferBytes, sampleWidth,
        bufferBytes + (sampleWidth * sampleHeight), sampleWidth,
        decodedFrame->buffer(kYPlane), width,
        decodedFrame->buffer(kUPlane), (width + 1) / 2,
        decodedFrame->buffer(kVPlane), (width + 1) / 2,
        width, height);
      hr = buffer->Unlock();
    }
  }

  return hr;
}

void H264WinRTDecoderImpl::OnH264Decoded(
  ComPtr<IMFSample> pSample, DWORD dwStreamFlags) {
  HRESULT hr = S_OK;

  UINT32 actualWidth = 0, actualHeight = 0;
  UINT32 sampleWidth = 0, sampleHeight = 0;

  // Get the frame's width and height
  {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (sourceReader_.Get() != nullptr) {
      ComPtr<IMFMediaType> mediaType;
      // TODO(winrt): Cache this and only update when
      //              MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED is set.
      hr = sourceReader_->GetCurrentMediaType(0, &mediaType);
      ComPtr<IMFAttributes> attributes;
      hr = mediaType.As(&attributes);
      UINT32 blobSize = 0;
      MFVideoArea videoArea;
      // Aperture contains the area with valid image data.
      hr = attributes->GetBlob(MF_MT_MINIMUM_DISPLAY_APERTURE,
        reinterpret_cast<uint8*>(&videoArea), sizeof(MFVideoArea), &blobSize);
      actualWidth = videoArea.Area.cx;
      actualHeight = videoArea.Area.cy;
      // These dimensions are those of the image buffer.
      // The image dimensions may be smaller.
      hr = MFGetAttributeSize(attributes.Get(),
        MF_MT_FRAME_SIZE, &sampleWidth, &sampleHeight);
    }
  }

  if (pSample != nullptr && actualWidth > 0 && actualHeight > 0) {
    VideoFrame decodedFrame;
    hr = FromSample(pSample, sampleWidth, sampleHeight,
      actualWidth, actualHeight, &decodedFrame);

    if (SUCCEEDED(hr)) {
      webrtc::CriticalSectionScoped csLock(_cbLock.get());

      LONGLONG sampleTimestamp;
      pSample->GetSampleTime(&sampleTimestamp);
      CachedFrameAttributes frameAttributes;
      if (_sampleAttributeQueue.pop(sampleTimestamp, frameAttributes)) {
        decodedFrame.set_timestamp(frameAttributes.timestamp);
        decodedFrame.set_ntp_time_ms(frameAttributes.ntpTime);
        decodedFrame.set_render_time_ms(frameAttributes.captureRenderTime);
      }

      if (decodeCompleteCallback_ != nullptr) {
        decodeCompleteCallback_->Decoded(decodedFrame);
      }
    }
  }

  webrtc::CriticalSectionScoped csLock(_lock.get());
  // Queue another async request to read the next sample.
  if (sourceReader_ != nullptr) {
    sourceReader_->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM,
      0, nullptr, nullptr, nullptr, nullptr);
  }
}

int H264WinRTDecoderImpl::RegisterDecodeCompleteCallback(
  DecodedImageCallback* callback) {
  webrtc::CriticalSectionScoped csLock(_cbLock.get());
  decodeCompleteCallback_ = callback;
  return 0;
}

int H264WinRTDecoderImpl::Release() {
  OutputDebugString(L"H264WinRTDecoderImpl::Release()\n");
  webrtc::CriticalSectionScoped csLock(_lock.get());
  mediaTypeOut_.Reset();
  mediaTypeIn_.Reset();
  mediaStream_.Reset();
  sourceReader_.Reset();
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

}  // namespace webrtc
