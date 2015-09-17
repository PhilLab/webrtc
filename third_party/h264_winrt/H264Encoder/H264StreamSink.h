/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef _H264STREAMSINK_H_INCLUDED_
#define _H264STREAMSINK_H_INCLUDED_

#include "../Utils/Async.h"
#include "../Utils/CritSec.h"
#include "../Utils/ComPtrList.h"
#include "IH264EncodingCallback.h"

#include <mfidl.h>
#include <Mferror.h>

using Microsoft::WRL::ComPtr;

namespace webrtc {

enum State {
    State_TypeNotSet = 0,    // No media type is set
    State_Ready,             // Media type is set, Start has never been called.
    State_Started,
    State_Stopped,
    State_Count              // Number of states
};

enum StreamOperation {
    OpSetMediaType = 0,
    OpStart,
    OpStop,
    OpProcessSample,
    Op_Count
};

class H264MediaSink;

class H264StreamSink : public Microsoft::WRL::RuntimeClass<
  Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::RuntimeClassType::WinRtClassicComMix>,
  IMFStreamSink,
  IMFMediaEventGenerator,
  IMFMediaTypeHandler>
{
  InspectableClass(L"H264StreamSink", BaseTrust)
  class AsyncOperation : public IUnknown
  {
  public:
    AsyncOperation(StreamOperation op);

    StreamOperation m_op;

    // IUnknown methods.
    STDMETHODIMP QueryInterface(REFIID iid, void **ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

  private:
    long    cRef_;
    virtual ~AsyncOperation();
  };

public:
  HRESULT RuntimeClassInitialize(DWORD dwIdentifier, H264MediaSink *pParent);

  // IMFMediaEventGenerator
  IFACEMETHOD(BeginGetEvent)(IMFAsyncCallback *pCallback, IUnknown *punkState);
  IFACEMETHOD(EndGetEvent) (IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
  IFACEMETHOD(GetEvent) (DWORD dwFlags, IMFMediaEvent **ppEvent);
  IFACEMETHOD(QueueEvent) (MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, PROPVARIANT const *pvValue);

  // IMFStreamSink
  IFACEMETHOD(GetMediaSink) (IMFMediaSink **ppMediaSink);
  IFACEMETHOD(GetIdentifier) (DWORD *pdwIdentifier);
  IFACEMETHOD(GetMediaTypeHandler) (IMFMediaTypeHandler **ppHandler);
  IFACEMETHOD(ProcessSample) (IMFSample *pSample);

  IFACEMETHOD(PlaceMarker) (
    /* [in] */ MFSTREAMSINK_MARKER_TYPE eMarkerType,
    /* [in] */ PROPVARIANT const *pvarMarkerValue,
    /* [in] */ PROPVARIANT const *pvarContextValue);

  IFACEMETHOD(Flush)();

  // IMFMediaTypeHandler
  IFACEMETHOD(IsMediaTypeSupported) (IMFMediaType *pMediaType, IMFMediaType **ppMediaType);
  IFACEMETHOD(GetMediaTypeCount) (DWORD *pdwTypeCount);
  IFACEMETHOD(GetMediaTypeByIndex) (DWORD dwIndex, IMFMediaType **ppType);
  IFACEMETHOD(SetCurrentMediaType) (IMFMediaType *pMediaType);
  IFACEMETHOD(GetCurrentMediaType) (IMFMediaType **ppMediaType);
  IFACEMETHOD(GetMajorType) (GUID *pguidMajorType);

  // ValidStateMatrix: Defines a look-up table that says which operations
  // are valid from which states.
  static BOOL ValidStateMatrix[State_Count][Op_Count];

  HRESULT RegisterEncodingCallback(IH264EncodingCallback *callback);

  H264StreamSink();
  virtual ~H264StreamSink();

  HRESULT CheckShutdown() const {
    if (isShutdown_) {
      return MF_E_SHUTDOWN;
    }
    else {
      return S_OK;
    }
  }


  HRESULT     Start(MFTIME start);
  HRESULT     Stop();
  HRESULT     Shutdown();

private:
  HRESULT     ValidateOperation(StreamOperation op);

  HRESULT     QueueAsyncOperation(StreamOperation op);

  HRESULT     OnDispatchWorkItem(IMFAsyncResult *pAsyncResult);

  bool        DropSamplesFromQueue();
  ComPtr<IMFSample> ProcessSamplesFromQueue();
  void        ProcessFormatChange(IMFMediaType *pMediaType);

  void        HandleError(HRESULT hr);

private:
  CritSec                     critSec_;
  CritSec                     cbCritSec_;

  DWORD                       dwIdentifier_;
  State                       state_;
  bool                        isShutdown_;
  GUID                        guidCurrentSubtype_;

  DWORD                       workQueueId_;

  ComPtr<IMFMediaSink>        spSink_;
  ComPtr<IMFMediaEventQueue>  spEventQueue_;
  ComPtr<IMFMediaType>        spCurrentType_;

  ComPtrList<IUnknown>                        sampleQueue_;

  AsyncCallback<H264StreamSink>               workQueueCB_;

  IH264EncodingCallback*                      encodingCallback_;
};

}  // namespace webrtc

#endif  // _H264STREAMSINK_H_INCLUDED_
