/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef THIRD_PARTY_H264_WINRT_H264DECODER_IH264DECODINGCALLBACK_H_
#define THIRD_PARTY_H264_WINRT_H264DECODER_IH264DECODINGCALLBACK_H_

#include <wrl\implements.h>

namespace webrtc {

interface IH264DecodingCallback {
  virtual void OnH264Decoded(Microsoft::WRL::ComPtr<IMFSample> sample,
    DWORD dwStreamFlags) = 0;
};

}  // namespace webrtc

#endif  // THIRD_PARTY_H264_WINRT_H264DECODER_IH264DECODINGCALLBACK_H_

