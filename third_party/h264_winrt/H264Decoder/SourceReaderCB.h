/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef THIRD_PARTY_H264_WINRT_H264DECODER_SOURCEREADERCB_H_
#define THIRD_PARTY_H264_WINRT_H264DECODER_SOURCEREADERCB_H_

#include <wrl.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include "IH264DecodingCallback.h"

using Microsoft::WRL::ComPtr;

namespace webrtc {

class SourceReaderCB : public Microsoft::WRL::RuntimeClass<
  Microsoft::WRL::RuntimeClassFlags<
  Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
  IMFSourceReaderCallback> {
  InspectableClass(L"SourceReaderCB", BaseTrust)
 public:
  SourceReaderCB();
  virtual ~SourceReaderCB();

  // IMFSourceReaderCallback methods
  STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex,
    DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample);
  STDMETHODIMP OnEvent(DWORD, IMFMediaEvent *);
  STDMETHODIMP OnFlush(DWORD);

  void RegisterDecodingCallback(IH264DecodingCallback* callback);

 private:
  IH264DecodingCallback* callback_;
};

}  // namespace webrtc

#endif  // THIRD_PARTY_H264_WINRT_H264DECODER_SOURCEREADERCB_H_
