/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_THIRD_PARTY_H264_ENCODER_H_
#define WEBRTC_THIRD_PARTY_H264_ENCODER_H_

#include <mfapi.h>
#include <mfidl.h>
#include <Mfreadwrite.h>
#include <mferror.h>
#include "H264MediaSink.h"
#include "IH264EncodingCallback.h"
#include <webrtc/video_encoder.h>
#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"

#pragma comment(lib, "mfreadwrite")
#pragma comment(lib, "mfplat")
#pragma comment(lib, "mfuuid")

using namespace Microsoft::WRL;

namespace webrtc {

class H264MediaSink;

class H264WinRTEncoderImpl : public VideoEncoder, public IH264EncodingCallback {

public:
  H264WinRTEncoderImpl();

  ~H264WinRTEncoderImpl();

  // === VideoEncoder overrides ===
  int InitEncode(const VideoCodec* codec_settings,
    int number_of_cores, size_t max_payload_size) override;
  int RegisterEncodeCompleteCallback(EncodedImageCallback* callback) override;
  int Release() override;
  int Encode(const VideoFrame& input_image,
    const CodecSpecificInfo* codec_specific_info,
    const std::vector<VideoFrameType>* frame_types) override;
  int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;
  int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;

  // === IH264EncodingCallback overrides ===
  void OnH264Encoded(ComPtr<IMFSample> sample) override;

private:
  rtc::scoped_ptr<webrtc::CriticalSectionWrapper> _lock;
  bool inited_;
  const CodecSpecificInfo* codecSpecificInfo_;
  ComPtr<IMFSinkWriter> sinkWriter_;
  ComPtr<IMFAttributes> sinkWriterCreationAttributes_;
  ComPtr<IMFAttributes> sinkWriterEncoderAttributes_;
  ComPtr<IMFMediaType> mediaTypeOut_; // h264
  ComPtr<IMFMediaType> mediaTypeIn_; // nv12
  ComPtr<H264MediaSink> mediaSink_;
  EncodedImageCallback* encodedCompleteCallback_;
  DWORD streamIndex_;
  LONGLONG startTime_;
  //LONGLONG timeStamp_;
  LONGLONG lastTimestampHns_;
  bool firstFrame_;
  int framePendingCount_;
};  // end of H264WinRTEncoderImpl class

}  // namespace webrtc
#endif  // WEBRTC_THIRD_PARTY_H264_ENCODER_H_

