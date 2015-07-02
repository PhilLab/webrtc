
#ifndef WEBRTC_THIRD_PARTY_H264_WINRT_IMPL_H_
#define WEBRTC_THIRD_PARTY_H264_WINRT_IMPL_H_

#include <vector>

#include "webrtc/common_video/interface/i420_buffer_pool.h"
#include "webrtc/modules/video_coding/codecs/interface/video_codec_interface.h"
#include "webrtc/video_frame.h"

namespace webrtc {

    class H264WinRTEncoderImpl : public VideoEncoder {
    public:
        H264WinRTEncoderImpl();

        ~H264WinRTEncoderImpl();

        int InitEncode(const VideoCodec* codec_settings,
                       int number_of_cores,
                       size_t max_payload_size) override;

        int RegisterEncodeCompleteCallback(EncodedImageCallback* callback) override;

        int Release() override;

        int Encode(const I420VideoFrame& input_image,
                   const CodecSpecificInfo* codec_specific_info,
                   const std::vector<VideoFrameType>* frame_types) override;

        int SetChannelParameters(uint32_t packet_loss, int64_t rtt) override;

        int SetRates(uint32_t new_bitrate_kbit, uint32_t frame_rate) override;

    private:

        bool inited_;
        EncodedImageCallback* encoded_complete_callback_;

    };  // end of H264WinRTEncoderImpl class

    class H264WinRTDecoderImpl : public VideoDecoder {
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

        VideoDecoder* Copy() override;

    private:

        bool inited_;
        DecodedImageCallback* decode_complete_callback_;
    };  // end of H264WinRTDecoderImpl class

}  // namespace webrtc

#endif  // WEBRTC_THIRD_PARTY_H264_WINRT_IMPL_H_

