/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include <wrl.h>
#include <Mferror.h>
#include <mfidl.h>

#include "../Utils/CritSec.h"
#include "../Utils/OpQueue.h"
#include "webrtc/modules/video_render/windows/video_render_source_winrt.h"

using Microsoft::WRL::ComPtr;

namespace webrtc {

class H264MediaSource;

class H264MediaStream : public Microsoft::WRL::RuntimeClass<
  Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
  IMFMediaStream,
  IMFMediaEventGenerator>
{
  InspectableClass(L"H264MediaStream", BaseTrust)
public:
  HRESULT RuntimeClassInitialize(IMFMediaType *pMediaType, H264MediaSource *pSource);

  // IMFMediaEventGenerator
  IFACEMETHOD(BeginGetEvent) (IMFAsyncCallback *pCallback, IUnknown *punkState);
  IFACEMETHOD(EndGetEvent) (IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
  IFACEMETHOD(GetEvent) (DWORD dwFlags, IMFMediaEvent **ppEvent);
  IFACEMETHOD(QueueEvent) (MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

  // IMFMediaStream
  IFACEMETHOD(GetMediaSource) (IMFMediaSource **ppMediaSource);
  IFACEMETHOD(GetStreamDescriptor) (IMFStreamDescriptor **ppStreamDescriptor);
  IFACEMETHOD(RequestSample) (IUnknown *pToken);

  // Other public methods
  HRESULT Start();
  HRESULT Stop();
  HRESULT Flush();
  HRESULT Shutdown();

  void ProcessSample(IMFSample *pSample);
  HRESULT SetActive(bool fActive);
  bool IsActive() const { return _fActive; }
  SourceState GetState() const { return _eSourceState; }

  DWORD GetId() const { return _dwId; }

  H264MediaStream();
  ~H264MediaStream(void);

private:
  class SourceLock;

private:
  void DeliverSamples();
  void HandleError(HRESULT hErrorCode);

  HRESULT CheckShutdown() const {
    if (_eSourceState == SourceState_Shutdown) {
      return MF_E_SHUTDOWN;
    }
    else {
      return S_OK;
    }
  }

  void CleanSampleQueue();

private:
  SourceState                 _eSourceState;
  ComPtr<H264MediaSource>     _spSource;
  ComPtr<IMFMediaEventQueue>  _spEventQueue;
  ComPtr<IMFStreamDescriptor> _spStreamDescriptor;

  ComPtrList<IUnknown>        _samples;
  ComPtrList<IUnknown, true>  _tokens;

  DWORD                       _dwId;
  bool                        _fActive;
};

}  // namespace webrtc
