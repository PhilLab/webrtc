/*
 *  Copyright (c) 2012 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#ifndef WEBRTC_MODULES_AUDIO_DEVICE_WIN_AUDIO_DEVICE_WASAPI_WIN_H_
#define WEBRTC_MODULES_AUDIO_DEVICE_WIN_AUDIO_DEVICE_WASAPI_WIN_H_

#if (_MSC_VER >= 1400) && defined(WINRT)  // only include for VS 2005 and higher

#include "webrtc/modules/audio_device/audio_device_generic.h"

#include <wmcodecdsp.h>      // CLSID_CWMAudioAEC
                             // (must be before audioclient.h)
#include <Audioclient.h>     // WASAPI
#include <Audiopolicy.h>
#include <wrl\implements.h>
#include <Mmdeviceapi.h>     // MMDevice
#include <avrt.h>            // Avrt
#include <endpointvolume.h>
#include <mediaobj.h>        // IMediaObject
#include <mfapi.h>
#include <ppltasks.h>


#include "webrtc/system_wrappers/interface/critical_section_wrapper.h"
#include "webrtc/base/scoped_ref_ptr.h"

using Windows::Devices::Enumeration::DeviceClass;
using Windows::Devices::Enumeration::DeviceInformation;
using Windows::Devices::Enumeration::DeviceInformationCollection;
using Windows::Media::Devices::AudioDeviceRole;
using Windows::Media::Devices::MediaDevice;

// Use Multimedia Class Scheduler Service (MMCSS) to boost the thread priority
#pragma comment(lib, "avrt.lib")

namespace webrtc {

const float MAX_CORE_SPEAKER_VOLUME = 255.0f;
const float MIN_CORE_SPEAKER_VOLUME = 0.0f;
const float MAX_CORE_MICROPHONE_VOLUME = 255.0f;
const float MIN_CORE_MICROPHONE_VOLUME = 0.0f;
const uint16_t CORE_SPEAKER_VOLUME_STEP_SIZE = 1;
const uint16_t CORE_MICROPHONE_VOLUME_STEP_SIZE = 1;

class AudioDeviceWindowsWasapi;

// Utility class which initializes COM in the constructor (STA or MTA),
// and uninitializes COM in the destructor.
class ScopedCOMInitializer {
 public:
  // Enum value provided to initialize the thread as an MTA instead of STA.
  enum SelectMTA { kMTA };

  // Constructor for STA initialization.
  ScopedCOMInitializer() {
    Initialize(COINIT_APARTMENTTHREADED);
  }

  // Constructor for MTA initialization.
  explicit ScopedCOMInitializer(SelectMTA mta) {
    Initialize(COINIT_MULTITHREADED);
  }

  ScopedCOMInitializer::~ScopedCOMInitializer() {
    if (SUCCEEDED(hr_))
      CoUninitialize();
  }

  bool succeeded() const { return SUCCEEDED(hr_); }

 private:
  void Initialize(COINIT init) {
    hr_ = CoInitializeEx(NULL, init);
  }

  HRESULT hr_;

  ScopedCOMInitializer(const ScopedCOMInitializer&);
  void operator=(const ScopedCOMInitializer&);
};


class AudioInterfaceActivator :
  public Microsoft::WRL::RuntimeClass < Microsoft::WRL::RuntimeClassFlags<
  Microsoft::WRL::ClassicCom >,
  Microsoft::WRL::FtmBase, IActivateAudioInterfaceCompletionHandler > {
  concurrency::task_completion_event<void> m_ActivateCompleted;
  STDMETHODIMP ActivateCompleted(
    IActivateAudioInterfaceAsyncOperation *pAsyncOp);

 public:
  enum ActivatorDeviceType{
    eNone = 0,
    eInputDevice,
    eOutputDevice
  };

  static ActivatorDeviceType m_DeviceType;

  static AudioDeviceWindowsWasapi* m_AudioDevice;
 public:
  static concurrency::task<Microsoft::WRL::ComPtr<IAudioClient2>>
    AudioInterfaceActivator::ActivateAudioClientAsync(LPCWCHAR deviceId,
      ActivatorDeviceType deviceType);
  static void SetAudioDevice(AudioDeviceWindowsWasapi* device);
};

class AudioDeviceWindowsWasapi : public AudioDeviceGeneric {
 public:
    explicit AudioDeviceWindowsWasapi(const int32_t id);
    ~AudioDeviceWindowsWasapi();

    friend class  AudioInterfaceActivator;

    // Retrieve the currently utilized audio layer
    virtual int32_t ActiveAudioLayer(
      AudioDeviceModule::AudioLayer& audioLayer) const;

    // Main initializaton and termination
    virtual int32_t Init();
    virtual int32_t Terminate();
    virtual bool Initialized() const;

    // Device enumeration
    virtual int16_t PlayoutDevices();
    virtual int16_t RecordingDevices();
    virtual int32_t PlayoutDeviceName(
        uint16_t index,
        char name[kAdmMaxDeviceNameSize],
        char guid[kAdmMaxGuidSize]);
    virtual int32_t RecordingDeviceName(
        uint16_t index,
        char name[kAdmMaxDeviceNameSize],
        char guid[kAdmMaxGuidSize]);

    // Device selection
    virtual int32_t SetPlayoutDevice(uint16_t index);
    virtual int32_t SetPlayoutDevice(
      AudioDeviceModule::WindowsDeviceType device);
    virtual int32_t SetRecordingDevice(uint16_t index);
    virtual int32_t SetRecordingDevice(
      AudioDeviceModule::WindowsDeviceType device);

    // Audio transport initialization
    virtual int32_t PlayoutIsAvailable(bool& available);
    virtual int32_t InitPlayout();
    virtual bool PlayoutIsInitialized() const;
    virtual int32_t RecordingIsAvailable(bool& available);
    virtual int32_t InitRecording();
    virtual bool RecordingIsInitialized() const;

    // Audio transport control
    virtual int32_t StartPlayout();
    virtual int32_t StopPlayout();
    virtual bool Playing() const;
    virtual int32_t StartRecording();
    virtual int32_t StopRecording();
    virtual bool Recording() const;

    // Microphone Automatic Gain Control (AGC)
    virtual int32_t SetAGC(bool enable);
    virtual bool AGC() const;

    // Volume control based on the Windows Wave API (Windows only)
    virtual int32_t SetWaveOutVolume(uint16_t volumeLeft, uint16_t volumeRight);
    virtual int32_t WaveOutVolume(uint16_t& volumeLeft,
                                  uint16_t& volumeRight) const;

    // Audio mixer initialization
    virtual int32_t InitSpeaker();
    virtual bool SpeakerIsInitialized() const;
    virtual int32_t InitMicrophone();
    virtual bool MicrophoneIsInitialized() const;

    // Speaker volume controls
    virtual int32_t SpeakerVolumeIsAvailable(bool& available);
    virtual int32_t SetSpeakerVolume(uint32_t volume);
    virtual int32_t SpeakerVolume(uint32_t& volume) const;
    virtual int32_t MaxSpeakerVolume(uint32_t& maxVolume) const;
    virtual int32_t MinSpeakerVolume(uint32_t& minVolume) const;
    virtual int32_t SpeakerVolumeStepSize(uint16_t& stepSize) const;

    // Microphone volume controls
    virtual int32_t MicrophoneVolumeIsAvailable(bool& available);
    virtual int32_t SetMicrophoneVolume(uint32_t volume);
    virtual int32_t MicrophoneVolume(uint32_t& volume) const;
    virtual int32_t MaxMicrophoneVolume(uint32_t& maxVolume) const;
    virtual int32_t MinMicrophoneVolume(uint32_t& minVolume) const;
    virtual int32_t MicrophoneVolumeStepSize(uint16_t& stepSize) const;

    // Speaker mute control
    virtual int32_t SpeakerMuteIsAvailable(bool& available);
    virtual int32_t SetSpeakerMute(bool enable);
    virtual int32_t SpeakerMute(bool& enabled) const;

    // Microphone mute control
    virtual int32_t MicrophoneMuteIsAvailable(bool& available);
    virtual int32_t SetMicrophoneMute(bool enable);
    virtual int32_t MicrophoneMute(bool& enabled) const;

    // Microphone boost control
    virtual int32_t MicrophoneBoostIsAvailable(bool& available);
    virtual int32_t SetMicrophoneBoost(bool enable);
    virtual int32_t MicrophoneBoost(bool& enabled) const;

    // Stereo support
    virtual int32_t StereoPlayoutIsAvailable(bool& available);
    virtual int32_t SetStereoPlayout(bool enable);
    virtual int32_t StereoPlayout(bool& enabled) const;
    virtual int32_t StereoRecordingIsAvailable(bool& available);
    virtual int32_t SetStereoRecording(bool enable);
    virtual int32_t StereoRecording(bool& enabled) const;

    // Delay information and control
    virtual int32_t SetPlayoutBuffer(const AudioDeviceModule::BufferType type,
      uint16_t sizeMS);
    virtual int32_t PlayoutBuffer(AudioDeviceModule::BufferType& type,
      uint16_t& sizeMS) const;
    virtual int32_t PlayoutDelay(uint16_t& delayMS) const;
    virtual int32_t RecordingDelay(uint16_t& delayMS) const;

    // CPU load
    virtual int32_t CPULoad(uint16_t& load) const;

    virtual int32_t EnableBuiltInAEC(bool enable);
    virtual bool BuiltInAECIsEnabled() const;

 public:
    virtual bool PlayoutWarning() const;
    virtual bool PlayoutError() const;
    virtual bool RecordingWarning() const;
    virtual bool RecordingError() const;
    virtual void ClearPlayoutWarning();
    virtual void ClearPlayoutError();
    virtual void ClearRecordingWarning();
    virtual void ClearRecordingError();

 public:
    virtual void AttachAudioBuffer(AudioDeviceBuffer* audioBuffer);

 public:  // IUnknown interface methods
  virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,
    LPVOID * ppvObj);
  virtual ULONG STDMETHODCALLTYPE AddRef();
  virtual ULONG STDMETHODCALLTYPE Release();


 private:
    bool KeyPressed() const;

 private:  // thread functions
    DWORD InitCaptureThreadPriority();
    void RevertCaptureThreadPriority();
    static DWORD WINAPI WSAPICaptureThread(LPVOID context);
    DWORD DoCaptureThread();

    static DWORD WINAPI WSAPIRenderThread(LPVOID context);
    DWORD DoRenderThread();

    static DWORD WINAPI GetCaptureVolumeThread(LPVOID context);
    DWORD DoGetCaptureVolumeThread();

    static DWORD WINAPI SetCaptureVolumeThread(LPVOID context);
    DWORD DoSetCaptureVolumeThread();

    void _SetThreadName(DWORD dwThreadID, LPCSTR szThreadName);
    void _Lock() { _critSect.Enter(); }
    void _UnLock() { _critSect.Leave(); }

 private:
    int32_t Id() {return _id;}

 private:
    int SetBoolProperty(IPropertyStore* ptrPS,
                        REFPROPERTYKEY key,
                        VARIANT_BOOL value);

    int SetVtI4Property(IPropertyStore* ptrPS,
                        REFPROPERTYKEY key,
                        LONG value);

    int32_t _EnumerateEndpointDevicesAll();
    void _TraceCOMError(HRESULT hr) const;
    void _Get44kHzDrift();

    int32_t _RefreshDeviceList(DeviceClass cls);
    int16_t _DeviceListCount(DeviceClass cls);

    Platform::String^ _GetDefaultDeviceName(DeviceClass cls);
    Platform::String^ _GetListDeviceName(DeviceClass cls, int index);
    Platform::String^ _GetDeviceName(DeviceInformation^ device);

    Platform::String^ _GetListDeviceID(DeviceClass cls, int index);
    Platform::String^ _GetDefaultDeviceID(DeviceClass cls);
    Platform::String^ _GetDeviceID(DeviceInformation^ device);

    DeviceInformation^ _GetDefaultDevice(DeviceClass cls);
    DeviceInformation^ _GetListDevice(DeviceClass cls, int index);

    Windows::Foundation::IAsyncAction^ _InitializeAudioDeviceInAsync();
    Windows::Foundation::IAsyncAction^ _InitializeAudioDeviceOutAsync();

    HRESULT _InitializeAudioDeviceIn();
    HRESULT _InitializeAudioDeviceOut();

    // Surround system support
    bool ShouldUpmix();
    WAVEFORMATEX* GenerateMixFormatForMediaEngine(
      WAVEFORMATEX* actualMixFormat);
    WAVEFORMATPCMEX* GeneratePCMMixFormat(WAVEFORMATEX* actualMixFormat);
    template<typename T>void Upmix(T *inSamples, uint32_t numberOfFrames,
      T *outSamples, uint32_t inChannels, uint32_t outChannels);

    // Converts from wide-char to UTF-8 if UNICODE is defined.
    // Does nothing if UNICODE is undefined.
    char* WideToUTF8(const TCHAR* src) const;


 private:
    ScopedCOMInitializer                    _comInit;
    AudioDeviceBuffer*                      _ptrAudioBuffer;
    CriticalSectionWrapper&                 _critSect;
    CriticalSectionWrapper&                 _volumeMutex;
    int32_t                                 _id;

 private:  // MMDevice
    Platform::String^   _deviceIdStringIn;
    Platform::String^   _deviceIdStringOut;
    DeviceInformation^  _defaultCaptureDevice;
    DeviceInformation^  _defaultRenderDevice;

    WAVEFORMATEX           *_mixFormatIn;
    WAVEFORMATEX           *_mixFormatOut;
    WAVEFORMATPCMEX        *_mixFormatSurroundOut;
    bool                   _enableUpmix;

    DeviceInformationCollection^ _ptrCaptureCollection;
    DeviceInformationCollection^ _ptrRenderCollection;
    DeviceInformationCollection^ _ptrCollection;
    AudioInterfaceActivator*     _ptrActivator;

 private:  // WASAPI
    IAudioClient*                           _ptrClientOut;
    IAudioClient*                           _ptrClientIn;
    IAudioRenderClient*                     _ptrRenderClient;
    IAudioCaptureClient*                    _ptrCaptureClient;
    ISimpleAudioVolume*                     _ptrCaptureVolume;
    ISimpleAudioVolume*                     _ptrRenderSimpleVolume;

    bool                                    _builtInAecEnabled;

    HANDLE                                  _hRenderSamplesReadyEvent;
    HANDLE                                  _hPlayThread;
    HANDLE                                  _hRenderStartedEvent;
    HANDLE                                  _hShutdownRenderEvent;

    HANDLE                                  _hCaptureSamplesReadyEvent;
    HANDLE                                  _hRecThread;
    HANDLE                                  _hCaptureStartedEvent;
    HANDLE                                  _hShutdownCaptureEvent;

    HANDLE                                  _hGetCaptureVolumeThread;
    HANDLE                                  _hSetCaptureVolumeThread;
    HANDLE                                  _hSetCaptureVolumeEvent;

    HANDLE                                  _hMmTask;

    UINT                                    _playAudioFrameSize;
    uint32_t                          _playSampleRate;
    uint32_t                          _devicePlaySampleRate;
    uint32_t                          _playBlockSize;
    uint32_t                          _devicePlayBlockSize;
    uint32_t                          _playChannels;
    uint32_t                          _sndCardPlayDelay;

    float                                   _sampleDriftAt48kHz;
    float                                   _driftAccumulator;

    UINT64                                  _writtenSamples;
    LONGLONG                                _playAcc;

    UINT                                    _recAudioFrameSize;
    uint32_t                          _recSampleRate;
    uint32_t                          _recBlockSize;
    uint32_t                          _recChannels;
    UINT64                                  _readSamples;
    uint32_t                          _sndCardRecDelay;

    uint16_t                          _recChannelsPrioList[2];
    uint16_t                          _playChannelsPrioList[2];

    LARGE_INTEGER                           _perfCounterFreq;
    double                                  _perfCounterFactor;
    float                                   _avgCPULoad;

 private:
    bool                                    _initialized;
    bool                                    _recording;
    bool                                    _playing;
    bool                                    _recIsInitialized;
    bool                                    _playIsInitialized;
    bool                                    _speakerIsInitialized;
    bool                                    _microphoneIsInitialized;

    bool                                    _usingInputDeviceIndex;
    bool                                    _usingOutputDeviceIndex;
    AudioDeviceModule::WindowsDeviceType    _inputDevice;
    AudioDeviceModule::WindowsDeviceType    _outputDevice;
    uint16_t                          _inputDeviceIndex;
    uint16_t                          _outputDeviceIndex;

    bool                                    _AGC;

    uint16_t                          _playWarning;
    uint16_t                          _playError;
    uint16_t                          _recWarning;
    uint16_t                          _recError;

    AudioDeviceModule::BufferType           _playBufType;
    uint16_t                          _playBufDelay;
    uint16_t                          _playBufDelayFixed;

    uint16_t                          _newMicLevel;

    mutable char                            _str[512];
};

#endif    // #if (_MSC_VER >= 1400)

}  // namespace webrtc

#endif  // WEBRTC_MODULES_AUDIO_DEVICE_WIN_AUDIO_DEVICE_WASAPI_WIN_H_
