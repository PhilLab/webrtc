# Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': ['../../../../build/common.gypi',],
  'targets': [
    {
      'target_name': 'audio_device_test_winrt',
      'type': 'executable',
      'dependencies': [
        '../../../../../webrtc/base/base.gyp:rtc_base',
        '../../../../../webrtc/modules/modules.gyp:audio_device',
        '../../../../../webrtc/test/test.gyp:test_support_main',
      ],
      'defines': [
        '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        '../../../../../testing/gtest/include',
      ],
      'sources': [
        'App.cpp',
        'Package.appxmanifest',
        'Logo.png',
        'SmallLogo.png',
        'SplashScreen.png',
        'StoreLogo.png',
        'WinRTTestManager.h',
        'WinRTTestManager.cpp',
      ],
      'forcePackage': [
        '../../../../../resources/audio_device/',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/audio_device_test_winrt_package',
          'files':[
            'Generated Manifest\AppxManifest.xml',
            'Logo.png',
            'SmallLogo.png',
            'SplashScreen.png',
            'StoreLogo.png',
            '../../../../../resources/audio_device/',
          ],
        },
        # Hack for MSVS to copy to the Appx folder
        {
          'destination': '<(PRODUCT_DIR)/AppX',
          'files':[
            'Logo.png',
            'SmallLogo.png',
            'SplashScreen.png',
            'StoreLogo.png',
            '../../../../../resources/audio_device/',
          ],
        },
      ],
      'msvs_package_certificate': {
        'KeyFile': '<(webrtc_root)/build/WinRT_gyp/UnitTests/gtest_runner/gtest_runner_TemporaryKey.pfx',
        'Thumbprint': 'E3AA95A6CD6D9DF6D0B7C68EBA246B558824F8C1',
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          # ExceptionHandling must match _HAS_EXCEPTIONS above.
          'ExceptionHandling': '1',
        },
        'VCLinkerTool': {
          'UseLibraryDependencyInputs': "true",
          'AdditionalDependencies': [
           
          ],
          # 2 == /SUBSYSTEM:WINDOWS
          'SubSystem': '2',
          'AdditionalOptions': [
          ],
        },
      },
    },
    {
      'target_name': 'audio_device_test_winrt_appx',
      #'product_name': 'audio_device_test_winrt',
      #'product_extension': 'appx',
      'type': 'none',
      'dependencies': [
        'audio_device_test_winrt',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/audio_device_test_winrt_package',
          'files':[
            '<(PRODUCT_DIR)/audio_device_test_winrt.exe',
          ],
        },
      ],
      'appx': {
        'dep': '<(PRODUCT_DIR)/audio_device_test_winrt_package/audio_device_test_winrt.exe',
        'dir': '<(PRODUCT_DIR)/audio_device_test_winrt_package',
        'cert': '<(webrtc_root)/build/WinRT_gyp/UnitTests/gtest_runner/gtest_runner_TemporaryKey.pfx',
        'out': '<(PRODUCT_DIR)/audio_device_test_winrt.appx',
      },
    },
  ],
}
