/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "SourceReaderCB.h"
#include "IH264DecodingCallback.h"
#include <Windows.h>

namespace webrtc {

SourceReaderCB::SourceReaderCB()
  : callback_(nullptr) {
}

SourceReaderCB::~SourceReaderCB() {
}

STDMETHODIMP SourceReaderCB::OnReadSample(
  HRESULT hrStatus, DWORD dwStreamIndex,
  DWORD dwStreamFlags, LONGLONG llTimestamp, IMFSample *pSample) {
  if (pSample == nullptr) {
    OutputDebugString((L"SourceReaderCB::OnReadSample()  Got NULL sample HRESULT:" + hrStatus.ToString())->Data());
  }

  callback_->OnH264Decoded(pSample, dwStreamFlags);
  return S_OK;
}

STDMETHODIMP SourceReaderCB::OnEvent(DWORD, IMFMediaEvent *) {
  return S_OK;
}

STDMETHODIMP SourceReaderCB::OnFlush(DWORD) {
  return S_OK;
}

void SourceReaderCB::RegisterDecodingCallback(IH264DecodingCallback* callback) {
  callback_ = callback;
}

}  // namespace webrtc
