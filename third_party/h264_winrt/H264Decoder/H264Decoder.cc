/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "H264Decoder.h"

#include <Windows.h>
#include <stdlib.h>
#include <ppltasks.h>
#include <mfapi.h>
#include <robuffer.h>
#include <wrl.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <wrl\implements.h>
#include "../Utils/Utils.h"
#include "libyuv/convert.h"
#include "webrtc/base/logging.h"

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
// H264 WinRT Decoder Implementation
//////////////////////////////////////////

H264WinRTDecoderImpl::H264WinRTDecoderImpl()
  : _lock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , _cbLock(webrtc::CriticalSectionWrapper::CreateCriticalSection())
  , decodeCompleteCallback_(nullptr) {
  MakeAndInitialize<SourceReaderCB>(&readerCB_);
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
  hr = MFCreateMediaType(&mediaTypeOut_);
  ThrowIfError(hr);

  hr = mediaTypeOut_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  ThrowIfError(hr);

  hr = mediaTypeOut_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
  ThrowIfError(hr);

  // input media type (h264)
  hr = MFCreateMediaType(&mediaTypeIn_);
  ThrowIfError(hr);

  hr = mediaTypeIn_->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
  ThrowIfError(hr);

  hr = mediaTypeIn_->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_H264);
  ThrowIfError(hr);

  ComPtr<H264MediaSource> mediaSource;
  ThrowIfError(MakeAndInitialize<H264MediaSource>(&mediaSource, mediaTypeIn_.Get(), &mediaStream_));

  ComPtr<IMFAttributes> readerAttributes;
  hr = MFCreateAttributes(&readerAttributes, 1);
  ThrowIfError(hr);

  ComPtr<IUnknown> readerCBAsUnknown;
  readerCB_.As(&readerCBAsUnknown);
  hr = readerAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, readerCBAsUnknown.Get());
  ThrowIfError(hr);

  hr = readerAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);
  ThrowIfError(hr);

  hr = MFCreateSourceReaderFromMediaSource(mediaSource.Get(), readerAttributes.Get(), &sourceReader_);
  ThrowIfError(hr);

  hr = sourceReader_->SetCurrentMediaType(0, nullptr, mediaTypeOut_.Get());
  ThrowIfError(hr);

  hr = sourceReader_->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
  ThrowIfError(hr);

  return 0;
}

const GUID WEBRTC_DECODER_TIMESTAMP           = { 0x11111111, 0xd6b2, 0x4012,{ 0xb8, 0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8 } };
const GUID WEBRTC_DECODER_NTP_TIME            = { 0x22222222, 0xd6b2, 0x4012,{ 0xb8, 0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8 } };
const GUID WEBRTC_DECODER_CAPTURE_RENDER_TIME = { 0x33333333, 0xd6b2, 0x4012,{ 0xb8, 0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8,  0xb8 } };

ComPtr<IMFSample> FromEncodedImage(const EncodedImage& input_image) {
  HRESULT hr;

  ComPtr<IMFSample> sample;
  hr = MFCreateSample(&sample);
  ThrowIfError(hr);


  ComPtr<IMFMediaBuffer> mediaBuffer;
  hr = MFCreateMemoryBuffer(input_image._length, &mediaBuffer);
  ThrowIfError(hr);

  BYTE* destBuffer = NULL;
  if (SUCCEEDED(hr)) {
    DWORD cbMaxLength;
    DWORD cbCurrentLength;
    hr = mediaBuffer->Lock(&destBuffer, &cbMaxLength, &cbCurrentLength);
  }
  ThrowIfError(hr);

  memcpy(destBuffer, input_image._buffer, input_image._length);

  hr = mediaBuffer->SetCurrentLength(input_image._length);
  ThrowIfError(hr);

  hr = mediaBuffer->Unlock();
  ThrowIfError(hr);

  hr = sample->AddBuffer(mediaBuffer.Get());
  ThrowIfError(hr);

  hr = sample->SetSampleTime((input_image._timeStamp / 90) * 10 * 1000);
  ThrowIfError(hr);

  auto duration = (LONGLONG)((1.0f / (float)30) * 1000 * 1000 * 10);
  hr = sample->SetSampleDuration(duration);
  ThrowIfError(hr);

  ComPtr<IMFAttributes> sampleAttributes;
  hr = sample.As(&sampleAttributes);
  ThrowIfError(hr);

  hr = sampleAttributes->SetUINT32(WEBRTC_DECODER_TIMESTAMP, input_image._timeStamp);
  ThrowIfError(hr);

  hr = sampleAttributes->SetUINT64(WEBRTC_DECODER_NTP_TIME, input_image.ntp_time_ms_);
  ThrowIfError(hr);

  hr = sampleAttributes->SetUINT64(WEBRTC_DECODER_CAPTURE_RENDER_TIME, input_image.capture_time_ms_);
  ThrowIfError(hr);

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
    if (mediaStream_ != nullptr) {
      mediaStream_->ProcessSample(sample.Get());
    }
  }

  return hr;
}

HRESULT FromSample(ComPtr<IMFSample> pSample, UINT32 width, UINT32 height, VideoFrame& decodedFrame) {
  HRESULT hr = S_OK;

  decodedFrame.CreateEmptyFrame(width, height, width,
    (width + 1) / 2, (width + 1) / 2);

  ComPtr<IMFMediaBuffer> buffer;
  hr = pSample->GetBufferByIndex(0, &buffer);
  if (SUCCEEDED(hr)) {
    BYTE* bufferBytes;
    DWORD maxLength, curLength;
    hr = buffer->Lock(&bufferBytes, &maxLength, &curLength);
    if (SUCCEEDED(hr)) {
      libyuv::NV12ToI420(
        bufferBytes, width,
        bufferBytes + (width * height), width,
        decodedFrame.buffer(kYPlane), width,
        decodedFrame.buffer(kUPlane), (width + 1) / 2,
        decodedFrame.buffer(kVPlane), (width + 1) / 2,
        width, height);
      hr = buffer->Unlock();
    }
  }

  // Get all the timestamp attributes out of the sample.
  ComPtr<IMFAttributes> sampleAttributes;
  hr = pSample.As(&sampleAttributes);
  if (SUCCEEDED(hr)) {
    UINT32 timestamp;
    UINT64 ntpTime, captureRenderTime;

    if (SUCCEEDED(sampleAttributes->GetUINT32(WEBRTC_DECODER_TIMESTAMP, &timestamp))) {
      decodedFrame.set_timestamp(timestamp);
    }

    if (SUCCEEDED(sampleAttributes->GetUINT64(WEBRTC_DECODER_NTP_TIME, &ntpTime))) {
      decodedFrame.set_ntp_time_ms((int64_t)ntpTime);
    }

    if (SUCCEEDED(sampleAttributes->GetUINT64(WEBRTC_DECODER_CAPTURE_RENDER_TIME, &captureRenderTime))) {
      decodedFrame.set_render_time_ms((int64_t)captureRenderTime);
    }
  }

  return hr;
}

void H264WinRTDecoderImpl::OnH264Decoded(ComPtr<IMFSample> pSample, DWORD dwStreamFlags) {

  HRESULT hr = S_OK;

  UINT32 width = 0, height = 0;

  // Get the frame's width and height
  {
    webrtc::CriticalSectionScoped csLock(_lock.get());
    if (sourceReader_.Get() != nullptr) {
      ComPtr<IMFMediaType> mediaType;
      // TODO: Cache this and only update when MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED is set.
      hr = sourceReader_->GetCurrentMediaType(0, &mediaType);
      ComPtr<IMFAttributes> attributes;
      hr = mediaType.As(&attributes);
      hr = MFGetAttributeSize(attributes.Get(), MF_MT_FRAME_SIZE, &width, &height);
    }
  }

  if (pSample != nullptr && width > 0 && height > 0) {

    VideoFrame decodedFrame;
    hr = FromSample(pSample, width, height, decodedFrame);

    if (SUCCEEDED(hr)) {
      webrtc::CriticalSectionScoped csLock(_cbLock.get());

      if (decodeCompleteCallback_ != nullptr) {
        decodeCompleteCallback_->Decoded(decodedFrame);
      }
    }
  }

  webrtc::CriticalSectionScoped csLock(_lock.get());
  // Queue another async request to read the next sample.
  if (sourceReader_ != nullptr) {
    sourceReader_->ReadSample((DWORD)MF_SOURCE_READER_ANY_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
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
  // TODO: Figure out when this happens.
  return 0;
}


VideoDecoder* H264WinRTDecoderImpl::Copy() {
  // TODO: Implement?
  return nullptr;
}

}  // namespace webrtc
