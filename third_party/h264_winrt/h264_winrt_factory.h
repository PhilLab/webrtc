
#ifndef WEBRTC_THIRD_PARTY_H264_WINRT_FACTORY_H_
#define WEBRTC_THIRD_PARTY_H264_WINRT_FACTORY_H_

#include <talk/media/webrtc/webrtcvideoencoderfactory.h>
#include <talk/media/webrtc/webrtcvideodecoderfactory.h>

namespace webrtc {

    class H264WinRTEncoderFactory : public cricket::WebRtcVideoEncoderFactory {

    public:

        H264WinRTEncoderFactory();

        webrtc::VideoEncoder* CreateVideoEncoder(webrtc::VideoCodecType type) override;

        const std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec>& codecs() const override;

        virtual void DestroyVideoEncoder(webrtc::VideoEncoder* encoder) override;

    private:
        std::vector<cricket::WebRtcVideoEncoderFactory::VideoCodec> codecList_;
    };

    class H264WinRTDecoderFactory : public cricket::WebRtcVideoDecoderFactory {

        webrtc::VideoDecoder* CreateVideoDecoder(webrtc::VideoCodecType type) override;

        virtual void DestroyVideoDecoder(webrtc::VideoDecoder* decoder) override;

    };
}

#endif
