/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "H264MediaStream.h"

#include <Windows.h>
#include <mfapi.h>

#include "H264MediaSource.h"
#include "../Utils/Utils.h"

using namespace Platform;
using namespace Microsoft::WRL;

namespace webrtc {

class H264MediaStream::SourceLock {
public:
  SourceLock(H264MediaSource *pSource)
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

HRESULT H264MediaStream::RuntimeClassInitialize(IMFMediaType *pMediaType, H264MediaSource *pSource) {
  // Create the media event queue.
  Exception ^error = nullptr;

  try {
    ThrowIfError(MFCreateEventQueue(&_spEventQueue));
    ComPtr<IMFStreamDescriptor> spSD;
    ComPtr<IMFMediaTypeHandler> spMediaTypeHandler;
    _spSource = pSource;

    ThrowIfError(MFCreateStreamDescriptor(0, 1, &pMediaType, &spSD));
    ThrowIfError(spSD->GetMediaTypeHandler(&spMediaTypeHandler));

    ThrowIfError(spMediaTypeHandler->SetCurrentMediaType(pMediaType));

    _spStreamDescriptor = spSD;
    _dwId = 0;
    _eSourceState = SourceState_Stopped;
  }
  catch (Exception ^exc) {
    error = exc;
  }

  if (error != nullptr) {
    throw error;
  }
  return S_OK;
}

H264MediaStream::H264MediaStream()
    : _eSourceState(SourceState_Invalid)
    , _fActive(false) {
}


H264MediaStream::~H264MediaStream() {
  OutputDebugString(L"H264MediaStream::~H264MediaStream()\n");
}

IFACEMETHODIMP H264MediaStream::BeginGetEvent(IMFAsyncCallback *pCallback, IUnknown *punkState) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->BeginGetEvent(pCallback, punkState);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::EndGetEvent(IMFAsyncResult *pResult, IMFMediaEvent **ppEvent) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->EndGetEvent(pResult, ppEvent);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetEvent(DWORD dwFlags, IMFMediaEvent **ppEvent) {
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

IFACEMETHODIMP H264MediaStream::QueueEvent(MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, PROPVARIANT const *pvValue) {
  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    hr = _spEventQueue->QueueEventParamVar(met, guidExtendedType, hrStatus, pvValue);
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetMediaSource(IMFMediaSource **ppMediaSource) {
  if (ppMediaSource == nullptr) {
    return E_INVALIDARG;
  }

  HRESULT hr = S_OK;

  SourceLock lock(_spSource.Get());

  hr = CheckShutdown();

  if (SUCCEEDED(hr)) {
    *ppMediaSource = (IMFMediaSource*)_spSource.Get();
    (*ppMediaSource)->AddRef();
  }

  return hr;
}

IFACEMETHODIMP H264MediaStream::GetStreamDescriptor(IMFStreamDescriptor **ppStreamDescriptor) {
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
  SourceLock lock(_spSource.Get());

  try {
    ThrowIfError(CheckShutdown());

    if (_eSourceState != SourceState_Started) {
      Throw(MF_E_INVALIDREQUEST);
    }

    ThrowIfError(_tokens.InsertBack(pToken));

    DeliverSamples();
  }
  catch (Exception ^exc) {
    HandleError(exc->HResult);
  }

  return S_OK;
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
    }
    else {
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
      _tokens.Clear();
      _samples.Clear();
      // Inform the client that we've stopped.
      hr = QueueEvent(MEStreamStopped, GUID_NULL, S_OK, nullptr);
    }
    else {
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

  _tokens.Clear();
  _samples.Clear();

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
  try {
    ThrowIfError(CheckShutdown());
    // todo Set sample attributes

    if (_eSourceState == SourceState_Started) {
      ThrowIfError(_samples.InsertBack(pSample));
      DeliverSamples();
    }
  }
  catch (Exception ^exc) {
    HandleError(exc->HResult);
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

void H264MediaStream::DeliverSamples() {
  while (!_samples.IsEmpty() && !_tokens.IsEmpty()) {
    ComPtr<IUnknown> spEntry;
    ComPtr<IMFSample> spSample;
    ComPtr<IUnknown> spToken;
    ThrowIfError(_samples.RemoveFront(&spEntry));

    if (SUCCEEDED(spEntry.As(&spSample))) {
      ThrowIfError(_tokens.RemoveFront(&spToken));

      if (spToken) {
        ThrowIfError(spSample->SetUnknown(MFSampleExtension_Token, spToken.Get()));
      }

      ThrowIfError(_spEventQueue->QueueEventParamUnk(MEMediaSample, GUID_NULL, S_OK, spSample.Get()));
    }
    else {
      ComPtr<IMFMediaType> spMediaType;
      ThrowIfError(spEntry.As(&spMediaType));
      ThrowIfError(_spEventQueue->QueueEventParamUnk(MEStreamFormatChanged, GUID_NULL, S_OK, spMediaType.Get()));
    }
  }
}

void H264MediaStream::CleanSampleQueue() {
  auto pos = _samples.FrontPosition();
  ComPtr<IUnknown> spEntry;
  ComPtr<IMFSample> spSample;

  // For video streams leave first key frame.
  for (; SUCCEEDED(_samples.GetItemPos(pos, spEntry.ReleaseAndGetAddressOf())); pos = _samples.Next(pos)) {
    if (SUCCEEDED(spEntry.As(&spSample)) && MFGetAttributeUINT32(spSample.Get(), MFSampleExtension_CleanPoint, 0)) {
      break;
    }
  }

  _samples.Clear();

  if (spSample != nullptr) {
    ThrowIfError(_samples.InsertFront(spSample.Get()));
  }
}

void H264MediaStream::HandleError(HRESULT hErrorCode) {
  if (hErrorCode != MF_E_SHUTDOWN) {
    // Send MEError to the client
    hErrorCode = QueueEvent(MEError, GUID_NULL, hErrorCode, nullptr);
  }
}

}  // namespace webrtc
