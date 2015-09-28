/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_THIRD_PARTY_H264_DECODER_H_
#define WEBRTC_THIRD_PARTY_H264_DECODER_H_

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include "H264MediaSource.h"
#include "H264MediaStream.h"
#include "IH264DecodingCallback.h"
#include "SourceReaderCB.h"
#include <webrtc/video_decoder.h>
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

using Microsoft::WRL::ComPtr;

namespace webrtc {

class H264WinRTDecoderImpl : public VideoDecoder, public IH264DecodingCallback {
public:
  H264WinRTDecoderImpl();

  virtual ~H264WinRTDecoderImpl();

  int InitDecode(const VideoCodec* inst, int number_of_cores) override;

  int Decode(const EncodedImage& input_image,
    bool missing_frames,
    const RTPFragmentationHeader* fragmentation,
    const CodecSpecificInfo* codec_specific_info,
    int64_t /*render_time_ms*/) override;

  int RegisterDecodeCompleteCallback(DecodedImageCallback* callback) override;

  int Release() override;
  int Reset() override;

  VideoDecoder* Copy();

  // === IH264DecodingCallback overrides ===
  void OnH264Decoded(ComPtr<IMFSample> pSample, DWORD dwStreamFlags) override;

private:
  rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;
  rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _cbLock;
  DecodedImageCallback* decodeCompleteCallback_;

  ComPtr<IMFMediaType> mediaTypeOut_; // nv12
  ComPtr<IMFMediaType> mediaTypeIn_;  // h264

  // Media stream on which we push received h264 frames.
  ComPtr<H264MediaStream> mediaStream_;
  // The reader from which to read decoded NV12 frames.
  ComPtr<IMFSourceReader> sourceReader_;
  // Receives the event from the source reader that
  // a new decoded frame is available.
  ComPtr<SourceReaderCB> readerCB_;

  // mediaStream_'s index in the source reader.
  DWORD streamIndex_;

  bool firstFrame_;
};  // end of H264WinRTDecoderImpl class

}  // namespace webrtc

#endif  // WEBRTC_THIRD_PARTY_H264_DECODER_H_

