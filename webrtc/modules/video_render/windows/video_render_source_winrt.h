/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#ifndef WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_
#define WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_

#include <wrl/implements.h>

#include <mfidl.h>

#include <windows.media.h>
#include <windows.media.core.h>

#include <queue>
#include <deque>

#include "webrtc/modules/video_render/include/video_render_defines.h"

namespace webrtc {
class CriticalSectionWrapper;
class VideoRenderMediaSourceWinRT;

template <class TBase = IMFAttributes>
class BaseAttributes : public TBase
{
protected:
  // This version of the constructor does not initialize the 
  // attribute store. The derived class must call Initialize() in 
  // its own constructor.
  BaseAttributes()
  {
  }

  // This version of the constructor initializes the attribute 
  // store, but the derived class must pass an HRESULT parameter 
  // to the constructor.

  BaseAttributes(HRESULT &hr, UINT32 cInitialSize = 0)
  {
    hr = Initialize(cInitialSize);
  }

  // The next version of the constructor uses a caller-provided 
  // implementation of IMFAttributes.

  // (Sometimes you want to delegate IMFAttributes calls to some 
  // other object that implements IMFAttributes, rather than using 
  // MFCreateAttributes.)

  BaseAttributes(HRESULT &hr, IUnknown *pUnk)
  {
    hr = Initialize(pUnk);
  }

  virtual ~BaseAttributes()
  {
  }

  // Initializes the object by creating the standard Media Foundation attribute store.
  HRESULT Initialize(UINT32 cInitialSize = 0)
  {
    if (_spAttributes.Get() == nullptr)
    {
      return MFCreateAttributes(&_spAttributes, cInitialSize);
    }
    else
    {
      return S_OK;
    }
  }

  // Initializes this object from a caller-provided attribute store.
  // pUnk: Pointer to an object that exposes IMFAttributes.
  HRESULT Initialize(IUnknown *pUnk)
  {
    if (_spAttributes)
    {
      _spAttributes->Release();
      _spAttributes = NULL;
    }


    return pUnk->QueryInterface(IID_PPV_ARGS(&_spAttributes));
  }

public:

  // IMFAttributes methods

  STDMETHODIMP GetItem(REFGUID guidKey, PROPVARIANT *pValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetItem(guidKey, pValue);
  }

  STDMETHODIMP GetItemType(REFGUID guidKey, MF_ATTRIBUTE_TYPE *pType)
  {
    assert(_spAttributes);
    return _spAttributes->GetItemType(guidKey, pType);
  }

  STDMETHODIMP CompareItem(REFGUID guidKey, REFPROPVARIANT Value, BOOL *pbResult)
  {
    assert(_spAttributes);
    return _spAttributes->CompareItem(guidKey, Value, pbResult);
  }

  STDMETHODIMP Compare(
    IMFAttributes *pTheirs,
    MF_ATTRIBUTES_MATCH_TYPE MatchType,
    BOOL *pbResult
    )
  {
    assert(_spAttributes);
    return _spAttributes->Compare(pTheirs, MatchType, pbResult);
  }

  STDMETHODIMP GetUINT32(REFGUID guidKey, UINT32 *punValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetUINT32(guidKey, punValue);
  }

  STDMETHODIMP GetUINT64(REFGUID guidKey, UINT64 *punValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetUINT64(guidKey, punValue);
  }

  STDMETHODIMP GetDouble(REFGUID guidKey, double *pfValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetDouble(guidKey, pfValue);
  }

  STDMETHODIMP GetGUID(REFGUID guidKey, GUID *pguidValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetGUID(guidKey, pguidValue);
  }

  STDMETHODIMP GetStringLength(REFGUID guidKey, UINT32 *pcchLength)
  {
    assert(_spAttributes);
    return _spAttributes->GetStringLength(guidKey, pcchLength);
  }

  STDMETHODIMP GetString(REFGUID guidKey, LPWSTR pwszValue, UINT32 cchBufSize, UINT32 *pcchLength)
  {
    assert(_spAttributes);
    return _spAttributes->GetString(guidKey, pwszValue, cchBufSize, pcchLength);
  }

  STDMETHODIMP GetAllocatedString(REFGUID guidKey, LPWSTR *ppwszValue, UINT32 *pcchLength)
  {
    assert(_spAttributes);
    return _spAttributes->GetAllocatedString(guidKey, ppwszValue, pcchLength);
  }

  STDMETHODIMP GetBlobSize(REFGUID guidKey, UINT32 *pcbBlobSize)
  {
    assert(_spAttributes);
    return _spAttributes->GetBlobSize(guidKey, pcbBlobSize);
  }

  STDMETHODIMP GetBlob(REFGUID guidKey, UINT8 *pBuf, UINT32 cbBufSize, UINT32 *pcbBlobSize)
  {
    assert(_spAttributes);
    return _spAttributes->GetBlob(guidKey, pBuf, cbBufSize, pcbBlobSize);
  }

  STDMETHODIMP GetAllocatedBlob(REFGUID guidKey, UINT8 **ppBuf, UINT32 *pcbSize)
  {
    assert(_spAttributes);
    return _spAttributes->GetAllocatedBlob(guidKey, ppBuf, pcbSize);
  }

  STDMETHODIMP GetUnknown(REFGUID guidKey, REFIID riid, LPVOID *ppv)
  {
    assert(_spAttributes);
    return _spAttributes->GetUnknown(guidKey, riid, ppv);
  }

  STDMETHODIMP SetItem(REFGUID guidKey, REFPROPVARIANT Value)
  {
    assert(_spAttributes);
    return _spAttributes->SetItem(guidKey, Value);
  }

  STDMETHODIMP DeleteItem(REFGUID guidKey)
  {
    assert(_spAttributes);
    return _spAttributes->DeleteItem(guidKey);
  }

  STDMETHODIMP DeleteAllItems()
  {
    assert(_spAttributes);
    return _spAttributes->DeleteAllItems();
  }

  STDMETHODIMP SetUINT32(REFGUID guidKey, UINT32 unValue)
  {
    assert(_spAttributes);
    return _spAttributes->SetUINT32(guidKey, unValue);
  }

  STDMETHODIMP SetUINT64(REFGUID guidKey, UINT64 unValue)
  {
    assert(_spAttributes);
    return _spAttributes->SetUINT64(guidKey, unValue);
  }

  STDMETHODIMP SetDouble(REFGUID guidKey, double fValue)
  {
    assert(_spAttributes);
    return _spAttributes->SetDouble(guidKey, fValue);
  }

  STDMETHODIMP SetGUID(REFGUID guidKey, REFGUID guidValue)
  {
    assert(_spAttributes);
    return _spAttributes->SetGUID(guidKey, guidValue);
  }

  STDMETHODIMP SetString(REFGUID guidKey, LPCWSTR wszValue)
  {
    assert(_spAttributes);
    return _spAttributes->SetString(guidKey, wszValue);
  }

  STDMETHODIMP SetBlob(REFGUID guidKey, const UINT8 *pBuf, UINT32 cbBufSize)
  {
    assert(_spAttributes);
    return _spAttributes->SetBlob(guidKey, pBuf, cbBufSize);
  }

  STDMETHODIMP SetUnknown(REFGUID guidKey, IUnknown *pUnknown)
  {
    assert(_spAttributes);
    return _spAttributes->SetUnknown(guidKey, pUnknown);
  }

  STDMETHODIMP LockStore()
  {
    assert(_spAttributes);
    return _spAttributes->LockStore();
  }

  STDMETHODIMP UnlockStore()
  {
    assert(_spAttributes);
    return _spAttributes->UnlockStore();
  }

  STDMETHODIMP GetCount(UINT32 *pcItems)
  {
    assert(_spAttributes);
    return _spAttributes->GetCount(pcItems);
  }

  STDMETHODIMP GetItemByIndex(UINT32 unIndex, GUID *pguidKey, PROPVARIANT *pValue)
  {
    assert(_spAttributes);
    return _spAttributes->GetItemByIndex(unIndex, pguidKey, pValue);
  }

  STDMETHODIMP CopyAllItems(IMFAttributes *pDest)
  {
    assert(_spAttributes);
    return _spAttributes->CopyAllItems(pDest);
  }

  // Helper functions

  HRESULT SerializeToStream(DWORD dwOptions, IStream *pStm)
    // dwOptions: Flags from MF_ATTRIBUTE_SERIALIZE_OPTIONS
  {
    assert(_spAttributes);
    return MFSerializeAttributesToStream(_spAttributes.Get(), dwOptions, pStm);
  }

  HRESULT DeserializeFromStream(DWORD dwOptions, IStream *pStm)
  {
    assert(_spAttributes);
    return MFDeserializeAttributesFromStream(_spAttributes.Get(), dwOptions, pStm);
  }

  // SerializeToBlob: Stores the attributes in a byte array. 
  // 
  // ppBuf: Receives a pointer to the byte array. 
  // pcbSize: Receives the size of the byte array.
  //
  // The caller must free the array using CoTaskMemFree.
  HRESULT SerializeToBlob(UINT8 **ppBuffer, UINT *pcbSize)
  {
    assert(_spAttributes);

    if (ppBuffer == NULL)
    {
      return E_POINTER;
    }
    if (pcbSize == NULL)
    {
      return E_POINTER;
    }

    HRESULT hr = S_OK;
    UINT32 cbSize = 0;
    BYTE *pBuffer = NULL;

    CHECK_HR(hr = MFGetAttributesAsBlobSize(_spAttributes.Get(), &cbSize));

    pBuffer = (BYTE*)CoTaskMemAlloc(cbSize);
    if (pBuffer == NULL)
    {
      CHECK_HR(hr = E_OUTOFMEMORY);
    }

    CHECK_HR(hr = MFGetAttributesAsBlob(_spAttributes.Get(), pBuffer, cbSize));

    *ppBuffer = pBuffer;
    *pcbSize = cbSize;

  done:
    if (FAILED(hr))
    {
      *ppBuffer = NULL;
      *pcbSize = 0;
      CoTaskMemFree(pBuffer);
    }
    return hr;
  }

  HRESULT DeserializeFromBlob(const UINT8 *pBuffer, UINT cbSize)
  {
    assert(_spAttributes);
    return MFInitAttributesFromBlob(_spAttributes.Get(), pBuffer, cbSize);
  }

  HRESULT GetRatio(REFGUID guidKey, UINT32 *pnNumerator, UINT32 *punDenominator)
  {
    assert(_spAttributes);
    return MFGetAttributeRatio(_spAttributes.Get(), guidKey, pnNumerator, punDenominator);
  }

  HRESULT SetRatio(REFGUID guidKey, UINT32 unNumerator, UINT32 unDenominator)
  {
    assert(_spAttributes);
    return MFSetAttributeRatio(_spAttributes.Get(), guidKey, unNumerator, unDenominator);
  }

  // Gets an attribute whose value represents the size of something (eg a video frame).
  HRESULT GetSize(REFGUID guidKey, UINT32 *punWidth, UINT32 *punHeight)
  {
    assert(_spAttributes);
    return MFGetAttributeSize(_spAttributes.Get(), guidKey, punWidth, punHeight);
  }

  // Sets an attribute whose value represents the size of something (eg a video frame).
  HRESULT SetSize(REFGUID guidKey, UINT32 unWidth, UINT32 unHeight)
  {
    assert(_spAttributes);
    return MFSetAttributeSize(_spAttributes.Get(), guidKey, unWidth, unHeight);
  }

protected:
  Microsoft::WRL::ComPtr<IMFAttributes> _spAttributes;
};

// Possible states of the stsp source object
enum SourceState
{
  // Invalid state, source cannot be used 
  SourceState_Invalid,
  // Streaming started
  SourceState_Starting,
  // Streaming started
  SourceState_Started,
  // Streanung stopped
  SourceState_Stopped,
  // Source is shut down
  SourceState_Shutdown,
};

struct StreamDescription
{
  GUID guiMajorType;
  GUID guiSubType;
  DWORD dwStreamId;
  DWORD dwFrameWidth;
  DWORD dwFrameHeight;
  DWORD dwFrameRateNumerator;
  DWORD dwFrameRateDenominator;
};

struct SampleHeader
{
  DWORD dwStreamId;
  LONGLONG ullTimestamp;
  LONGLONG ullDuration;
};

class VideoRenderMediaStreamWinRT :
    public IMFMediaStream,
    public IMFQualityAdvise2,
    public IMFGetService {
public:
  static HRESULT CreateInstance(StreamDescription *pStreamDescription, VideoRenderMediaSourceWinRT *pSource, VideoRenderMediaStreamWinRT **ppStream);

  // IUnknown
  IFACEMETHOD(QueryInterface) (REFIID iid, void **ppv);
  IFACEMETHOD_(ULONG, AddRef) ();
  IFACEMETHOD_(ULONG, Release) ();

  // IMFMediaEventGenerator
  IFACEMETHOD(BeginGetEvent) (IMFAsyncCallback *pCallback, IUnknown *punkState);
  IFACEMETHOD(EndGetEvent) (IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
  IFACEMETHOD(GetEvent) (DWORD dwFlags, IMFMediaEvent **ppEvent);
  IFACEMETHOD(QueueEvent) (MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

  // IMFMediaStream
  IFACEMETHOD(GetMediaSource) (IMFMediaSource **ppMediaSource);
  IFACEMETHOD(GetStreamDescriptor) (IMFStreamDescriptor **ppStreamDescriptor);
  IFACEMETHOD(RequestSample) (IUnknown *pToken);

  // IMFQualityAdvise
  IFACEMETHOD(SetDropMode) (_In_ MF_QUALITY_DROP_MODE eDropMode);
  IFACEMETHOD(SetQualityLevel) (_In_ MF_QUALITY_LEVEL eQualityLevel);
  IFACEMETHOD(GetDropMode) (_Out_ MF_QUALITY_DROP_MODE *peDropMode);
  IFACEMETHOD(GetQualityLevel)(_Out_ MF_QUALITY_LEVEL *peQualityLevel);
  IFACEMETHOD(DropTime) (_In_ LONGLONG hnsAmountToDrop);

  // IMFQualityAdvise2
  IFACEMETHOD(NotifyQualityEvent) (_In_opt_ IMFMediaEvent *pEvent, _Out_ DWORD *pdwFlags);

  // IMFGetService
  IFACEMETHOD(GetService) (_In_ REFGUID guidService,
    _In_ REFIID riid,
    _Out_ LPVOID *ppvObject);

  // Other public methods
  HRESULT Start();
  HRESULT Stop();
  HRESULT SetRate(float flRate);
  HRESULT Flush();
  HRESULT Shutdown();
  void ProcessSample(SampleHeader *pSampleHeader, IMFSample *pSample);
  void ProcessFormatChange(StreamDescription *pStreamDescription);
  HRESULT SetActive(bool fActive);
  bool IsActive() const { return _fActive; }
  SourceState GetState() const { return _eSourceState; }

  DWORD GetId() const { return _dwId; }

protected:
  VideoRenderMediaStreamWinRT(VideoRenderMediaSourceWinRT *pSource);
  ~VideoRenderMediaStreamWinRT(void);

private:
  class SourceLock;

private:
  void Initialize(StreamDescription *pStreamDescription);
  void DeliverSamples();
  void SetMediaTypeAttributes(StreamDescription *pStreamDescription, IMFMediaType *pMediaType);
  void SetSampleAttributes(SampleHeader *pSampleHeader, IMFSample *pSample);
  void HandleError(HRESULT hErrorCode);

  bool ShouldDropSample(IMFSample *pSample);
  void CleanSampleQueue();
  void ResetDropTime();

private:
  ULONG _cRef;
  SourceState _eSourceState;
  Microsoft::WRL::ComPtr<VideoRenderMediaSourceWinRT>_spSource;
  Microsoft::WRL::ComPtr<IMFMediaEventQueue> _spEventQueue;
  Microsoft::WRL::ComPtr<IMFStreamDescriptor> _spStreamDescriptor;

  std::deque<IUnknown*> _samples;
  std::deque<IUnknown*> _tokens;

  DWORD _dwId;
  bool _fActive;
  float _flRate;

  MF_QUALITY_DROP_MODE _eDropMode;
  bool _fDiscontinuity;
  bool _fDropTime;
  bool _fInitDropTime;
  bool _fWaitingForCleanPoint;
  LONGLONG _hnsStartDroppingAt;
  LONGLONG _hnsAmountToDrop;
};

// Base class representing asyncronous source operation
class VideoRenderSourceOperation : public IUnknown
{
public:
  enum Type
  {
    // Start the source
    Operation_Start,
    // Stop the source
    Operation_Stop,
    // Set rate
    Operation_SetRate,
  };

public:
  VideoRenderSourceOperation(Type opType);
  virtual ~VideoRenderSourceOperation();
  // IUnknown
  IFACEMETHOD(QueryInterface) (REFIID riid, void **ppv);
  IFACEMETHOD_(ULONG, AddRef) ();
  IFACEMETHOD_(ULONG, Release) ();

  Type GetOperationType() const { return _opType; }
  const PROPVARIANT &GetData() const { return _data; }
  HRESULT SetData(const PROPVARIANT &varData);

private:
  ULONG _cRef;
  Type _opType;
  PROPVARIANT _data;
};

// Start operation
class VideoRenderSourceStartOperation : public VideoRenderSourceOperation
{
public:
  VideoRenderSourceStartOperation(IMFPresentationDescriptor *pPD);
  ~VideoRenderSourceStartOperation();

  IMFPresentationDescriptor *GetPresentationDescriptor() { return _spPD.Get(); }

private:
  Microsoft::WRL::ComPtr<IMFPresentationDescriptor> _spPD;
};

// SetRate operation
class VideoRenderSourceSetRateOperation : public VideoRenderSourceOperation
{
public:
  VideoRenderSourceSetRateOperation(BOOL fThin, float flRate);
  ~VideoRenderSourceSetRateOperation();

  BOOL IsThin() const { return _fThin; }
  float GetRate() const { return _flRate; }

private:
  BOOL _fThin;
  float _flRate;
};

template<class T>
class AsyncCallback : public IMFAsyncCallback {
public:
  typedef HRESULT(T::*InvokeFn)(IMFAsyncResult *pAsyncResult);

  AsyncCallback(T *pParent, InvokeFn fn) : _pParent(pParent), _pInvokeFn(fn) {
  }

  // IUnknown
  STDMETHODIMP_(ULONG) AddRef() {
    // Delegate to parent class.
    return _pParent->AddRef();
  }
  STDMETHODIMP_(ULONG) Release() {
    // Delegate to parent class.
    return _pParent->Release();
  }
  STDMETHODIMP QueryInterface(REFIID iid, void** ppv) {
    if (!ppv) {
      return E_POINTER;
    }
    if (iid == __uuidof(IUnknown)) {
      *ppv = static_cast<IUnknown*>(static_cast<IMFAsyncCallback*>(this));
    }
    else if (iid == __uuidof(IMFAsyncCallback)) {
      *ppv = static_cast<IMFAsyncCallback*>(this);
    }
    else {
      *ppv = NULL;
      return E_NOINTERFACE;
    }
    AddRef();
    return S_OK;
  }

  // IMFAsyncCallback methods
  STDMETHODIMP GetParameters(DWORD*, DWORD*) {
    // Implementation of this method is optional.
    return E_NOTIMPL;
  }

  STDMETHODIMP Invoke(IMFAsyncResult* pAsyncResult) {
    return (_pParent->*_pInvokeFn)(pAsyncResult);
  }

  T *_pParent;
  InvokeFn _pInvokeFn;
};

template <class T, class TOperation>
class OpQueue
{
 public:

  HRESULT QueueOperation(TOperation *pOp);

 protected:

  HRESULT ProcessQueue();
  HRESULT ProcessQueueAsync(IMFAsyncResult *pResult);

  virtual HRESULT DispatchOperation(TOperation *pOp) = 0;
  virtual HRESULT ValidateOperation(TOperation *pOp) = 0;

  OpQueue()
    : m_OnProcessQueue(static_cast<T *>(this), &OpQueue::ProcessQueueAsync),
      m_critSec(NULL)
  {
  }

  OpQueue(CriticalSectionWrapper* critsec)
    : m_OnProcessQueue(static_cast<T *>(this), &OpQueue::ProcessQueueAsync),
      m_critSec(critsec)
  {
  }

  virtual ~OpQueue()
  {
  }

 protected:
  std::queue<TOperation*> m_OpQueue;
  CriticalSectionWrapper* m_critSec;
  AsyncCallback<T> m_OnProcessQueue;
};

class VideoRenderMediaSourceWinRT :
    public Microsoft::WRL::RuntimeClass<
        Microsoft::WRL::RuntimeClassFlags<
            Microsoft::WRL::RuntimeClassType::WinRtClassicComMix >,
        ABI::Windows::Media::Core::IMediaSource,
        IMFMediaSource,
        IMFGetService,
        IMFRateControl,
        BaseAttributes< > > ,
    public OpQueue<VideoRenderMediaSourceWinRT, VideoRenderSourceOperation> {

 InspectableClass(L"webrtc::VideoRenderMediaSourceWinRT", BaseTrust)

 public:
  static HRESULT CreateInstance(VideoRenderMediaSourceWinRT **ppMediaSource);
 
  // IMFMediaEventGenerator
  IFACEMETHOD(BeginGetEvent) (IMFAsyncCallback *pCallback, IUnknown *punkState);
  IFACEMETHOD(EndGetEvent) (IMFAsyncResult *pResult, IMFMediaEvent **ppEvent);
  IFACEMETHOD(GetEvent) (DWORD dwFlags, IMFMediaEvent **ppEvent);
  IFACEMETHOD(QueueEvent) (MediaEventType met, REFGUID guidExtendedType, HRESULT hrStatus, const PROPVARIANT *pvValue);

  // IMFMediaSource
  IFACEMETHOD(CreatePresentationDescriptor) (
      IMFPresentationDescriptor **ppPresentationDescriptor);
  IFACEMETHOD(GetCharacteristics) (DWORD *pdwCharacteristics);
  IFACEMETHOD(Pause) ();
  IFACEMETHOD(Shutdown) ();
  IFACEMETHOD(Start) (
      IMFPresentationDescriptor *pPresentationDescriptor,
      const GUID *pguidTimeFormat,
      const PROPVARIANT *pvarStartPosition);
  IFACEMETHOD(Stop)();

  // IMFGetService
  IFACEMETHOD(GetService) (
      _In_ REFGUID guidService,
      _In_ REFIID riid,
      _Out_opt_ LPVOID *ppvObject);

  // IMFRateControl
  IFACEMETHOD(SetRate) (BOOL fThin, float flRate);
  IFACEMETHOD(GetRate) (_Inout_opt_ BOOL *pfThin, _Inout_opt_ float *pflRate);

  // OpQueue
  __override HRESULT DispatchOperation(_In_ VideoRenderSourceOperation *pOp);
  __override HRESULT ValidateOperation(_In_ VideoRenderSourceOperation *pOp);

  void ProcessVideoFrame(const I420VideoFrame& videoFrame);
  void FrameSizeChange(int width, int height);

  _Acquires_lock_(_critSec)
  HRESULT Lock();
  _Releases_lock_(_critSec)
  HRESULT Unlock();

 public:
  VideoRenderMediaSourceWinRT();
  ~VideoRenderMediaSourceWinRT();

 private:
  void Initialize();

  void HandleError(HRESULT hResult);
  HRESULT GetStreamById(DWORD dwId, VideoRenderMediaStreamWinRT **ppStream);
  void InitPresentationDescription();
  HRESULT ValidatePresentationDescriptor(IMFPresentationDescriptor *pPD);
  void SelectStream(IMFPresentationDescriptor *pPD);

  void DoStart(VideoRenderSourceStartOperation *pOp);
  void DoStop(VideoRenderSourceOperation *pOp);
  void DoSetRate(VideoRenderSourceSetRateOperation *pOp);

  bool IsRateSupported(float flRate, float *pflAdjustedRate);

 private:
  ULONG _cRef;
  CriticalSectionWrapper* _critSec;
  SourceState _eSourceState;

  Microsoft::WRL::ComPtr<IMFMediaEventQueue> _spEventQueue;
  Microsoft::WRL::ComPtr<IMFPresentationDescriptor> _spPresentationDescriptor;
  Microsoft::WRL::ComPtr<IMFMediaStream> _spStream;

  float _flRate;
  LONGLONG _hnsCurrentSampleTime;
};
}  // namespace webrtc

#endif  // WEBRTC_MODULES_VIDEO_RENDER_WINDOWS_VIDEO_RENDER_SOURCE_WINRT_H_
