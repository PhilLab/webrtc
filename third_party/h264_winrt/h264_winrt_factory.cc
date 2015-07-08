
#include "h264_winrt_factory.h"
#include "h264_winrt_impl.h"
#include <talk/media/webrtc/webrtcvideoencoderfactory.h>
#include <talk/media/webrtc/webrtcvideodecoderfactory.h>

namespace webrtc {

    H264WinRTEncoderFactory::H264WinRTEncoderFactory()
    {
        codecList_ = std::vector < cricket::WebRtcVideoEncoderFactory::VideoCodec >
        {
            cricket::WebRtcVideoEncoderFactory::VideoCodec(
            webrtc::VideoCodecType::kVideoCodecH264,
            "h264winrt",
            1920,
            1080,
            60)
        };
    }

    webrtc::VideoEncoder* H264WinRTEncoderFactory::CreateVideoEncoder(webrtc::VideoCodecType type)
    {
        return new H264WinRTEncoderImpl();
    }

    const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec>& H264WinRTEncoderFactory::codecs() const
    {
        return codecList_;
    }

    void H264WinRTEncoderFactory::DestroyVideoEncoder(webrtc::VideoEncoder* encoder)
    {
        encoder->Release();
    }


    webrtc::VideoDecoder* H264WinRTDecoderFactory::CreateVideoDecoder(webrtc::VideoCodecType type)
    {
        return new H264WinRTDecoderImpl();
    }

    void H264WinRTDecoderFactory::DestroyVideoDecoder(webrtc::VideoDecoder* decoder)
    {
        decoder->Release();
    }

}  // namespace webrtc

