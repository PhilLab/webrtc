/*
 *  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "third_party/h264_winrt/h264_winrt_impl.h"

#include <stdlib.h>

#include "webrtc/common_types.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "webrtc/system_wrappers/interface/trace_event.h"

namespace webrtc {

    //////////////////////////////////////////
    // H264 WinRT Encoder Implementation
    //////////////////////////////////////////

    H264WinRTEncoderImpl::H264WinRTEncoderImpl() {
    }

    H264WinRTEncoderImpl::~H264WinRTEncoderImpl() {
        Release();
    }

    int H264WinRTEncoderImpl::InitEncode(const VideoCodec* inst,
        int number_of_cores,
        size_t /*maxPayloadSize */) {
        if (inst == nullptr) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (inst->maxFramerate < 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        // allow zero to represent an unspecified maxBitRate
        if (inst->maxBitrate > 0 && inst->startBitrate > inst->maxBitrate) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (inst->width <= 1 || inst->height <= 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (number_of_cores < 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (inst->codecSpecific.VP8.feedbackModeOn &&
            inst->numberOfSimulcastStreams > 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (inst->codecSpecific.VP8.automaticResizeOn &&
            inst->numberOfSimulcastStreams > 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        int retVal = Release();
        if (retVal < 0) {
            return retVal;
        }

        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTEncoderImpl::RegisterEncodeCompleteCallback(
        EncodedImageCallback* callback) {
        encoded_complete_callback_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTEncoderImpl::Release() {
        int ret_val = WEBRTC_VIDEO_CODEC_OK;

        return ret_val;
    }

    int H264WinRTEncoderImpl::Encode(
        const VideoFrame& frame,
        const CodecSpecificInfo* codec_specific_info,
        const std::vector<VideoFrameType>* frame_types) {
        TRACE_EVENT1("webrtc", "h264_winrt::Encode", "timestamp",
          frame.timestamp());

        if (!inited_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (frame.IsZeroSize()) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }
        if (encoded_complete_callback_ == nullptr) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }

        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTEncoderImpl::SetChannelParameters(uint32_t packetLoss,
      int64_t rtt) {
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTEncoderImpl::SetRates(uint32_t new_bitrate_kbit,
        uint32_t new_framerate) {
        if (!inited_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (new_framerate < 1) {
            return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
        }

        return WEBRTC_VIDEO_CODEC_OK;
    }

    //////////////////////////////////////////
    // H264 WinRT Decoder Implementation
    //////////////////////////////////////////

    H264WinRTDecoderImpl::H264WinRTDecoderImpl()
        : decode_complete_callback_(nullptr) {
    }

    H264WinRTDecoderImpl::~H264WinRTDecoderImpl() {
        inited_ = true;  // in order to do the actual release
        Release();
    }

    int H264WinRTDecoderImpl::InitDecode(const VideoCodec* inst,
        int number_of_cores) {
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTDecoderImpl::Decode(const EncodedImage& input_image,
        bool missing_frames,
        const RTPFragmentationHeader* fragmentation,
        const CodecSpecificInfo* codec_specific_info,
        int64_t /*render_time_ms*/) {
        if (!inited_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }
        if (decode_complete_callback_ == nullptr) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }

        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTDecoderImpl::RegisterDecodeCompleteCallback(
        DecodedImageCallback* callback) {
        decode_complete_callback_ = callback;
        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTDecoderImpl::Release() {
        inited_ = false;

        return WEBRTC_VIDEO_CODEC_OK;
    }

    int H264WinRTDecoderImpl::Reset() {
        if (!inited_) {
            return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
        }

        return WEBRTC_VIDEO_CODEC_OK;
    }


    VideoDecoder* H264WinRTDecoderImpl::Copy() {
        // Sanity checks.
        if (!inited_) {
            // Not initialized.
            assert(false);
            return nullptr;
        }

        return nullptr;
    }
}  // namespace webrtc
