/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "third_party/h264_winrt/H264Decoder/H264MediaSource.h"

#include <mfapi.h>

#include "H264MediaStream.h"
#include "Utils/Utils.h"

namespace webrtc {

H264MediaSource::H264MediaSource(void)
    : _eSourceState(SourceState_Invalid) {
}

H264MediaSource::~H264MediaSource(void) {
  OutputDebugString(L"H264MediaSource::~H264MediaSource()\n");
}

HRESULT H264MediaSource::RuntimeClassInitialize(
  _In_  IMFMediaType *pTargetMediaType,
  _Out_ H264MediaStream** ppStream) {
  HRESULT hr = S_OK;
  ON_SUCCEEDED(MFCreateEventQueue(&_spEventQueue));
  ON_SUCCEEDED(Microsoft::WRL::MakeAndInitialize<H264MediaStream>(
    &_stream, pTargetMediaType, this));

  if (SUCCEEDED(hr)) {
    InitPresentationDescription();

    _eSourceState = SourceState_Stopped;

    *ppStream = _stream.Get();
    (*ppStream)->AddRef();
  }
  return hr;
}

IFACEMETHODIMP H264MediaSource::BeginGetEvent(
  IMFAsyncCallback *pCallback, IUnknown *punkState) {
  HRESULT hr = S_OK;

  AutoLock lock(_critSec);

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->BeginGetEvent(pCallback, punkState);
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::EndGetEvent(
  IMFAsyncResult *pResult, IMFMediaEvent **ppEvent) {
  HRESULT hr = S_OK;

  AutoLock lock(_critSec);

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->EndGetEvent(pResult, ppEvent);
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::GetEvent(
  DWORD dwFlags, IMFMediaEvent **ppEvent) {
  // NOTE:
  // GetEvent can block indefinitely, so we don't hold the lock.
  // This requires some juggling with the event queue pointer.

  HRESULT hr = S_OK;

  ComPtr<IMFMediaEventQueue> spQueue;

  {
    AutoLock lock(_critSec);

    // Check shutdown
    hr = CheckShutdown();

    // Get the pointer to the event queue.
    if (SUCCEEDED(hr)) {
      spQueue = _spEventQueue;
    }
  }

  // Now get the event.
  if (SUCCEEDED(hr)) {
    hr = spQueue->GetEvent(dwFlags, ppEvent);
  }


  return hr;
}

IFACEMETHODIMP H264MediaSource::QueueEvent(
  MediaEventType met, REFGUID guidExtendedType,
  HRESULT hrStatus, PROPVARIANT const *pvValue) {
    HRESULT hr = S_OK;

    AutoLock lock(_critSec);

    hr = CheckShutdown();

    if (SUCCEEDED(hr)) {
        hr = _spEventQueue->QueueEventParamVar(
          met, guidExtendedType, hrStatus, pvValue);
    }

    return hr;
}

IFACEMETHODIMP H264MediaSource::CreatePresentationDescriptor(
  IMFPresentationDescriptor **ppPresentationDescriptor
  ) {
  if (ppPresentationDescriptor == NULL) {
    return E_POINTER;
  }

  AutoLock lock(_critSec);

  HRESULT hr = CheckShutdown();

  if (SUCCEEDED(hr) &&
    (_eSourceState == SourceState::SourceState_Invalid ||
      !_spPresentationDescriptor)) {
    hr = MF_E_NOT_INITIALIZED;
  }

  if (SUCCEEDED(hr)) {
    hr = _spPresentationDescriptor->Clone(ppPresentationDescriptor);
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::GetCharacteristics(
  DWORD *pdwCharacteristics) {
  if (pdwCharacteristics == NULL) {
    return E_POINTER;
  }

  AutoLock lock(_critSec);

  HRESULT hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    *pdwCharacteristics = MFMEDIASOURCE_IS_LIVE;
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::Pause() {
    return MF_E_INVALID_STATE_TRANSITION;
}

IFACEMETHODIMP H264MediaSource::Shutdown() {
  OutputDebugString(L"H264MediaSource::Shutdown()\n");
  AutoLock lock(_critSec);

  HRESULT hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    if (_spEventQueue != nullptr) {
      _spEventQueue->Shutdown();
    }

    _stream->Shutdown();

    _eSourceState = SourceState_Shutdown;

    _stream.Reset();

    _spEventQueue.Reset();
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::Start(
  IMFPresentationDescriptor *pPresentationDescriptor,
  const GUID *pguidTimeFormat,
  const PROPVARIANT *pvarStartPos
  ) {
  OutputDebugString(L"H264MediaSource::Start()\n");
  HRESULT hr = S_OK;

  if (pvarStartPos == nullptr || pPresentationDescriptor == nullptr) {
    return E_INVALIDARG;
  }

  if ((pguidTimeFormat != nullptr) && (*pguidTimeFormat != GUID_NULL)) {
    return MF_E_UNSUPPORTED_TIME_FORMAT;
  }

  if (pvarStartPos->vt != VT_EMPTY && pvarStartPos->vt != VT_I8) {
    return MF_E_UNSUPPORTED_TIME_FORMAT;
  }

  AutoLock lock(_critSec);

  if (_eSourceState != SourceState_Stopped &&
    _eSourceState != SourceState_Started) {
    hr = MF_E_INVALIDREQUEST;
  }

  if (SUCCEEDED(hr)) {
    hr = ValidatePresentationDescriptor(pPresentationDescriptor);
  }

  SelectStreams(pPresentationDescriptor);

  _eSourceState = SourceState_Starting;

  _eSourceState = SourceState_Started;
  ON_SUCCEEDED(_spEventQueue->QueueEventParamVar(
    MESourceStarted, GUID_NULL, S_OK, pvarStartPos));

  if (FAILED(hr)) {
    _spEventQueue->QueueEventParamVar(MESourceStarted, GUID_NULL, hr, nullptr);
  }

  return hr;
}

IFACEMETHODIMP H264MediaSource::Stop() {
  OutputDebugString(L"H264MediaSource::Stop()\n");

  HRESULT hr = S_OK;
  ON_SUCCEEDED(_stream->Flush());
  ON_SUCCEEDED(_stream->Stop());
  hr = _spEventQueue->QueueEventParamVar(
    MESourceStopped, GUID_NULL, hr, nullptr);

  return hr;
}

_Acquires_lock_(_critSec)
HRESULT H264MediaSource::Lock() {
  _critSec.Lock();
  return S_OK;
}

_Releases_lock_(_critSec)
HRESULT H264MediaSource::Unlock() {
  _critSec.Unlock();
  return S_OK;
}

void H264MediaSource::InitPresentationDescription() {
  HRESULT hr = S_OK;
  ComPtr<IMFPresentationDescriptor> spPresentationDescriptor;
  ComPtr<IMFStreamDescriptor> streamDescriptor;

  ON_SUCCEEDED(_stream->GetStreamDescriptor(&streamDescriptor));
  ON_SUCCEEDED(MFCreatePresentationDescriptor(1,
    streamDescriptor.GetAddressOf(), &spPresentationDescriptor));
  ON_SUCCEEDED(spPresentationDescriptor->SelectStream(0));

  if (SUCCEEDED(hr)) {
    _spPresentationDescriptor = spPresentationDescriptor;
  }
}

HRESULT H264MediaSource::ValidatePresentationDescriptor(
  IMFPresentationDescriptor *pPD) {
  HRESULT hr = S_OK;
  BOOL fSelected = FALSE;
  DWORD cStreams = 0;

  if (_stream == nullptr) {
    return E_UNEXPECTED;
  }

  // The caller's PD must have the same just the one stream.
  hr = pPD->GetStreamDescriptorCount(&cStreams);

  if (SUCCEEDED(hr)) {
    if (cStreams != 1) {
      hr = E_INVALIDARG;
    }
  }

  // The caller must select the stream.
  if (SUCCEEDED(hr)) {
    ComPtr<IMFStreamDescriptor> spSD;
    hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &spSD);
    if (FAILED(hr) || !fSelected) {
      hr = E_INVALIDARG;
    }
  }

  return hr;
}

HRESULT H264MediaSource::SelectStreams(IMFPresentationDescriptor *pPD) {
  HRESULT hr = S_OK;
  ComPtr<IMFStreamDescriptor> spSD;
  DWORD nStreamId = 0;
  BOOL fSelected = FALSE;

  ON_SUCCEEDED(pPD->GetStreamDescriptorByIndex(0, &fSelected, &spSD));

  ON_SUCCEEDED(spSD->GetStreamIdentifier(&nStreamId));

  if (SUCCEEDED(hr) && _stream->GetId() != nStreamId) {
    return MF_E_NOT_FOUND;
  }

  if (SUCCEEDED(hr)) {
    bool fWasSelected = _stream->IsActive();
    ON_SUCCEEDED(_stream->SetActive(!!fSelected));

    if (SUCCEEDED(hr) && fSelected) {
      MediaEventType met = (fWasSelected) ? MEUpdatedStream : MENewStream;
      ComPtr<IUnknown> spStreamUnk;

      ON_SUCCEEDED(_stream.As(&spStreamUnk));

      ON_SUCCEEDED(_spEventQueue->QueueEventParamUnk(
        met, GUID_NULL, S_OK, spStreamUnk.Get()));

      ON_SUCCEEDED(_stream->Start());
    }
  }
  return hr;
}

}  // namespace webrtc
