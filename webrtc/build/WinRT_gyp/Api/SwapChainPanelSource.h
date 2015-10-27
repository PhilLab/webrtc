// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef WEBRTC_BUILD_WINRT_GYP_API_SWAPCHAINPANELSOURCE_H_
#define WEBRTC_BUILD_WINRT_GYP_API_SWAPCHAINPANELSOURCE_H_

#include <Windows.h>
#include <collection.h>
#include <ppltasks.h>
#include <Windows.ui.xaml.media.dxinterop.h>

namespace webrtc_winrt_api {
  public delegate void FormatChangeDelegate(
    uintptr_t swapChain, uint32 frameRate, uint32 width, uint32 height);
  public delegate void ErrorDelegate(uint32 hr);

  [Windows::Foundation::Metadata::WebHostHidden]
  public ref class SwapChainPanelSource sealed {
  public:
    SwapChainPanelSource();
    virtual ~SwapChainPanelSource();
    void StartSource(Windows::UI::Xaml::Controls::SwapChainPanel^
      swapChainPanel);
    void StopSource();
    event FormatChangeDelegate^ FormatChange {
      Windows::Foundation::EventRegistrationToken add(
        FormatChangeDelegate^ format);
      void remove(Windows::Foundation::EventRegistrationToken token);
    }
    event ErrorDelegate^ Error;
  private:
    void OnRegistrationCompleted(
      Windows::ApplicationModel::Background::BackgroundTaskRegistration^
      sender,
      Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs^
      args);
    void OnUnloaded(Platform::Object ^sender,
      Windows::UI::Xaml::RoutedEventArgs ^e);
    void OnFormatChange(uintptr_t swapChain, unsigned int frameRate,
      unsigned int width, unsigned int height);

    Windows::UI::Xaml::Controls::SwapChainPanel^ _swapChain;
    Microsoft::WRL::ComPtr<ISwapChainPanelNative2> _nativeSwapChain;
    HANDLE _currentSwapChainHandle;
    bool _formatChangeEventSubscribed;
    event FormatChangeDelegate^ _formatChangeEvent;
  };
}  // namespace webrtc_winrt_api

#endif  // WEBRTC_BUILD_WINRT_GYP_API_SWAPCHAINPANELSOURCE_H_
