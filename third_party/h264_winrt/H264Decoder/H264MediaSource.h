/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include <Mferror.h>
#include <mfidl.h>
#include <wrl.h>
#include "../Utils/OpQueue.h"

#include "../Utils/CritSec.h"
#include "webrtc/modules/video_render/windows/video_render_source_winrt.h"

using Microsoft::WRL::ComPtr;

namespace webrtc {

class H264MediaStream;

class H264MediaSource : public Microsoft::WRL::RuntimeClass<
  Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
  IMFMediaSource,
  IMFMediaEventGenerator>
{
  InspectableClass(L"H264MediaSource", BaseTrust)
public:
  HRESULT RuntimeClassInitialize(
    _In_  IMFMediaType *pTargetMediaType,
    _Out_ H264MediaStream** ppStream);

  // IMFMediaEventGenerator
  IFACEMETHOD(BeginGetEvent) (IMFAsyncCallback *pCallback, IUnknown *punkState);
  IFACEMETHOD(EndGetEvent) (IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
  IFACEMETHOD(GetEvent) (DWORD dwFlags, IMFMediaEvent **ppEvent);
  IFACEMETHOD(QueueEvent) (MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

  // IMFMediaSource
  IFACEMETHOD(CreatePresentationDescriptor) (IMFPresentationDescriptor **ppPresentationDescriptor);
  IFACEMETHOD(GetCharacteristics) (DWORD *pdwCharacteristics);
  IFACEMETHOD(Pause) ();
  IFACEMETHOD(Shutdown) ();
  IFACEMETHOD(Start) (
    IMFPresentationDescriptor *pPresentationDescriptor,
    const GUID *pguidTimeFormat,
    const PROPVARIANT *pvarStartPosition
    );
  IFACEMETHOD(Stop)();

  _Acquires_lock_(_critSec)
    HRESULT Lock();
  _Releases_lock_(_critSec)
    HRESULT Unlock();

  H264MediaSource();
  virtual ~H264MediaSource();

private:

  void InitPresentationDescription();
  HRESULT ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
  void SelectStreams(IMFPresentationDescriptor *pPD);

  HRESULT CheckShutdown() const {
    if (_eSourceState == SourceState::SourceState_Shutdown) {
      return MF_E_SHUTDOWN;
    }
    else {
      return S_OK;
    }
  }

private:
  CritSec                     _critSec;
  SourceState                 _eSourceState;
  ComPtr<IMFMediaEventQueue>  _spEventQueue;

  ComPtr<IMFPresentationDescriptor> _spPresentationDescriptor;

  ComPtr<H264MediaStream> _stream;
};

}  // namespace webrtc
