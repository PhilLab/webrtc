/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "third_party/h264_winrt/H264Decoder/H264MediaStream.h"

#include <Windows.h>
#include <mfapi.h>

#include "H264MediaSource.h"
#include "Utils/Utils.h"

using Platform::Exception;
using Microsoft::WRL::ComPtr;

namespace webrtc {

class H264MediaStream::SourceLock {
 public:
  explicit SourceLock(H264MediaSource *pSource)
    : _spSource(pSource) {
    if (_spSource) {
      _spSource->Lock();
    }
  }

  ~SourceLock() {
    if (_spSource) {
      _spSource->Unlock();
    }
  }

 private:
  ComPtr<H264MediaSource> _spSource;
};

HRESULT H264MediaStream::RuntimeClassInitialize(
  IMFMediaType *pMediaType, H264MediaSource *pSource) {
  HRESULT hr = S_OK;
  // Create the media event queue.

  ON_SUCCEEDED(MFCreateEventQueue(&_spEventQueue));
  ComPtr<IMFStreamDescriptor> spSD;
  ComPtr<IMFMediaTypeHandler> spMediaTypeHandler;
  _spSource = pSource;

  ON_SUCCEEDED(MFCreateStreamDescriptor(0, 1, &pMediaType, &spSD));
  ON_SUCCEEDED(spSD->GetMediaTypeHandler(&spMediaTypeHandler));
  ON_SUCCEEDED(spMediaTypeHandler->SetCurrentMediaType(pMediaType));

  if (SUCCEEDED(hr)) {
    _spStreamDescriptor = spSD;
    _dwId = 0;
    _eSourceState = SourceState_Stopped;
  }

  return hr;
}

H264MediaStream::H264MediaStream()
    : _eSourceState(SourceState_Invalid)
    , _fActive(false) {
}


H264MediaStream::~H264MediaStream() {
  OutputDebugString(L"H264MediaStream::~H264MediaStream()\n");
}

IFACEMETHODIMP H264MediaStream::BeginGetEvent(
  IMFAsyncCallback *pCallback, IUnknown *punkState) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->BeginGetEvent(pCallback, punkState);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::EndGetEvent(
  IMFAsyncResult *pResult, IMFMediaEvent **ppEvent) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->EndGetEvent(pResult, ppEvent);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetEvent(
  DWORD dwFlags, IMFMediaEvent **ppEvent) {
  // NOTE:
  // GetEvent can block indefinitely, so we don't hold the lock.
  // This requires some juggling with the event queue pointer.

  HRESULT hr = S_OK;

  ComPtr<IMFMediaEventQueue> spQueue;

  {
    SourceLock lock(_spSource.Get());

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

IFACEMETHODIMP H264MediaStream::QueueEvent(
  MediaEventType met, REFGUID guidExtendedType,
  HRESULT hrStatus, PROPVARIANT const *pvValue) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->QueueEventParamVar(
      met, guidExtendedType, hrStatus, pvValue);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetMediaSource(
  IMFMediaSource **ppMediaSource) {
  if (ppMediaSource == nullptr) {
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    *ppMediaSource = reinterpret_cast<IMFMediaSource*>(_spSource.Get());
    (*ppMediaSource)->AddRef();
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetStreamDescriptor(
  IMFStreamDescriptor **ppStreamDescriptor) {
  if (ppStreamDescriptor == nullptr) {
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    *ppStreamDescriptor = _spStreamDescriptor.Get();
    (*ppStreamDescriptor)->AddRef();
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::RequestSample(IUnknown *pToken) {
  HRESULT hr = S_OK;
  SourceLock lock(_spSource.Get());

  ON_SUCCEEDED(CheckShutdown());

  if (SUCCEEDED(hr) && _eSourceState != SourceState_Started) {
    hr = MF_E_INVALIDREQUEST;
  }

  if (SUCCEEDED(hr)) {
    _tokens.push_back(pToken);
    hr = DeliverSamples();
  }

  return hr;
}

HRESULT H264MediaStream::Start() {
  OutputDebugString(L"H264MediaStream::Start()\n");
  HRESULT hr = S_OK;
  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    if (_eSourceState == SourceState_Stopped ||
      _eSourceState == SourceState_Started) {
      _eSourceState = SourceState_Started;
      // Inform the client that we've started
      hr = QueueEvent(MEStreamStarted, GUID_NULL, S_OK, nullptr);
    } else {
      hr = MF_E_INVALID_STATE_TRANSITION;
    }
  }

  if (FAILED(hr)) {
    HandleError(hr);
  }

  return S_OK;
}

HRESULT H264MediaStream::Stop() {
  OutputDebugString(L"H264MediaStream::Stop()\n");
  HRESULT hr = S_OK;
  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    if (_eSourceState == SourceState_Started) {
      _eSourceState = SourceState_Stopped;
      _tokens.clear();
      _samples.clear();
      // Inform the client that we've stopped.
      hr = QueueEvent(MEStreamStopped, GUID_NULL, S_OK, nullptr);
    } else {
      hr = MF_E_INVALID_STATE_TRANSITION;
    }
  }

  if (FAILED(hr)) {
    HandleError(hr);
  }

  return S_OK;
}

HRESULT H264MediaStream::Flush() {
  SourceLock lock(_spSource.Get());

  _tokens.clear();
  _samples.clear();

  return S_OK;
}

HRESULT H264MediaStream::Shutdown() {
  OutputDebugString(L"H264MediaStream::Shutdown()\n");
  SourceLock lock(_spSource.Get());

  HRESULT hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    Flush();

    if (_spEventQueue) {
      _spEventQueue->Shutdown();
      _spEventQueue.Reset();
    }

    _spStreamDescriptor.Reset();
    _spSource.Reset();
    _eSourceState = SourceState_Shutdown;
  }

  return hr;
}

void H264MediaStream::ProcessSample(IMFSample *pSample) {
  SourceLock lock(_spSource.Get());
  HRESULT hr = CheckShutdown();
  // todo Set sample attributes

  if (SUCCEEDED(hr) && _eSourceState == SourceState_Started) {
    _samples.push_back(pSample);
    hr = DeliverSamples();
  }
  if (FAILED(hr)) {
    HandleError(hr);
  }
}

HRESULT H264MediaStream::SetActive(bool fActive) {
  SourceLock lock(_spSource.Get());

  HRESULT hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    if (_eSourceState != SourceState_Stopped &&
      _eSourceState != SourceState_Started) {
      hr = MF_E_INVALIDREQUEST;
    }
  }

  if (SUCCEEDED(hr)) {
    _fActive = fActive;
  }

  return hr;
}

HRESULT H264MediaStream::DeliverSamples() {
  HRESULT hr = S_OK;
  while (!_samples.empty() && !_tokens.empty()) {
    ComPtr<IUnknown> spEntry;
    ComPtr<IMFSample> spSample;
    ComPtr<IUnknown> spToken;

    spEntry = _samples.front();
    _samples.pop_front();

    if (SUCCEEDED(spEntry.As(&spSample))) {
      spToken = _tokens.front();
      _tokens.pop_front();

      if (spToken) {
        ON_SUCCEEDED(spSample->SetUnknown(
          MFSampleExtension_Token, spToken.Get()));
      }

      ON_SUCCEEDED(_spEventQueue->QueueEventParamUnk(MEMediaSample,
        GUID_NULL, S_OK, spSample.Get()));
    } else {
      ComPtr<IMFMediaType> spMediaType;
      hr = spEntry.As(&spMediaType);
      ON_SUCCEEDED(_spEventQueue->QueueEventParamUnk(MEStreamFormatChanged,
        GUID_NULL, S_OK, spMediaType.Get()));
    }
  }
  return hr;
}

void H264MediaStream::HandleError(HRESULT hErrorCode) {
  if (hErrorCode != MF_E_SHUTDOWN) {
    // Send MEError to the client
    hErrorCode = QueueEvent(MEError, GUID_NULL, hErrorCode, nullptr);
  }
}

}  // namespace webrtc
