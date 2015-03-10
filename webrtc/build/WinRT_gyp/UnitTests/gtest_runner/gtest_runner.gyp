# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': ['../../../common.gypi',],
  'variables': {

  },
  'targets': [
    {
      'target_name': 'gtest_runner',
      'type': 'executable',
      'dependencies': [
        '../../../../../webrtc/base/base_tests.gyp:rtc_base_unittests',
        '../../../../../webrtc/system_wrappers/system_wrappers_tests.gyp:system_wrappers_unittests',
        '../../../../../webrtc/common_audio/common_audio.gyp:common_audio_unittests',
        '../../../../../webrtc/test/test.gyp:test_support',
        '../../../../../webrtc/test/test.gyp:test_support_main',
      ],
      'defines': [
        'GTEST_RELATIVE_PATH',
        '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        '../../../../../testing/gtest/include',
        'Generated Files',
      ],
      'sources': [
        'MainPage.xaml',
        'MainPage.xaml.h',
        'MainPage.xaml.cpp',
        'Package.appxmanifest',
        'gtest_runner_TemporaryKey.pfx',
        'Assets/Logo.scale-100.png',
        'Assets/SmallLogo.scale-100.png',
        'Assets/SplashScreen.scale-100.png',
        'Assets/StoreLogo.scale-100.png',
        'App.xaml',
        'App.xaml.h',
        'App.xaml.cpp',
        'pch.h',
        'pch.cpp',
      ],
      'msvs_disabled_warnings': [
        4453,  # A '[WebHostHidden]' type should not be used on the published surface of a public type that is not '[WebHostHidden]'
      ],
      'msvs_settings': {
        'VCCLCompilerTool': {
          # ExceptionHandling must match _HAS_EXCEPTIONS above.
          'ExceptionHandling': '1',
        },
        'VCLinkerTool': {
          'UseLibraryDependencyInputs': "true",
          'AdditionalDependencies': [
           'ws2_32.lib',
          ],
        },
      },
    },
  ],
}
