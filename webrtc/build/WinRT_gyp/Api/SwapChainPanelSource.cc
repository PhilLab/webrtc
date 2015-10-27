// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include "webrtc/build/WinRT_gyp/Api/SwapChainPanelSource.h"

using webrtc_winrt_api::SwapChainPanelSource;
using webrtc_winrt_api::FormatChangeDelegate;
using Platform::COMException;
using Windows::ApplicationModel::Background::BackgroundTaskRegistration;
using Windows::ApplicationModel::Background::BackgroundTaskCompletedEventArgs;
using Windows::Foundation::EventRegistrationToken;

SwapChainPanelSource::SwapChainPanelSource() : _swapChain(nullptr),
_currentSwapChainHandle(nullptr), _formatChangeEventSubscribed(false) {
}

SwapChainPanelSource::~SwapChainPanelSource() {
  StopSource();
}

void SwapChainPanelSource::StartSource(
  Windows::UI::Xaml::Controls::SwapChainPanel^ swapChainPanel) {
  StopSource();
  _swapChain = swapChainPanel;
  _swapChain->Unloaded += ref new Windows::UI::Xaml::RoutedEventHandler(
    this, &webrtc_winrt_api::SwapChainPanelSource::OnUnloaded);
  HRESULT hr;
  if ((FAILED(hr = reinterpret_cast<IUnknown*>(swapChainPanel)->
    QueryInterface(IID_PPV_ARGS(&_nativeSwapChain)))) ||
    (!_nativeSwapChain)) {
    throw ref new COMException(hr);
  }
  if (!_formatChangeEventSubscribed) {
    FormatChange += ref new webrtc_winrt_api::FormatChangeDelegate(this,
      &webrtc_winrt_api::SwapChainPanelSource::OnFormatChange);
    _formatChangeEventSubscribed = true;
  }
}

void SwapChainPanelSource::StopSource() {
}

EventRegistrationToken SwapChainPanelSource::FormatChange::add(
  FormatChangeDelegate^ format) {
  return _formatChangeEvent += format;
}

void SwapChainPanelSource::FormatChange::remove(EventRegistrationToken token) {
  _formatChangeEvent -= token;
}

void SwapChainPanelSource::OnRegistrationCompleted(
  BackgroundTaskRegistration ^sender, BackgroundTaskCompletedEventArgs ^args) {
  args->CheckResult();
}

void SwapChainPanelSource::OnUnloaded(Platform::Object ^sender,
  Windows::UI::Xaml::RoutedEventArgs ^e) {
  StopSource();
}

void SwapChainPanelSource::OnFormatChange(uintptr_t swapChain,
  unsigned int, unsigned int, unsigned int) {
}
