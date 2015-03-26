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
        '../../../../../webrtc/modules/modules.gyp:modules_unittests',
        '../../../../../webrtc/common_video/common_video_unittests.gyp:common_video_unittests',
      ],
      'defines': [
        'GTEST_RELATIVE_PATH',
        '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        '../../../../../testing/gtest/include',
      ],
      'sources': [
        'App.cpp',
        'Package.appxmanifest',
        'gtest_runner_TemporaryKey.pfx',
        'Logo.png',
        'SmallLogo.png',
        'SplashScreen.png',
        'StoreLogo.png',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/gtest_runner_package',
          'files':[
            'Generated Manifest\AppxManifest.xml',
            'Logo.png',
            'SmallLogo.png',
            'SplashScreen.png',
            'StoreLogo.png',
            '../../../../../data/',
            '../../../../../resources/',
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
            '../../../../../data/',
            '../../../../../resources/',
          ],
        },
      ],

      'msvs_disabled_warnings': [
        4453,  # A '[WebHostHidden]' type should not be used on the published surface of a public type that is not '[WebHostHidden]'
      ],
      'msvs_package_certificate': {
        'KeyFile': 'gtest_runner_TemporaryKey.pfx',
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
           'ws2_32.lib',
          ],
          # 2 == /SUBSYSTEM:WINDOWS
          'SubSystem': '2',
          'AdditionalOptions': [
          ],
        },
      },
    },
    {
      'target_name': 'gtest_runner_appx',
      #'product_name': 'gtest_runner',
      #'product_extension': 'appx',
      'type': 'none',
      'dependencies': [
        'gtest_runner',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/gtest_runner_package',
          'files':[
            '<(PRODUCT_DIR)/gtest_runner.exe',
          ],
        },
      ],
      'appx': {
        'dep': '<(PRODUCT_DIR)/gtest_runner.exe',
        'dir': '<(PRODUCT_DIR)/gtest_runner_package',
        'out': '<(PRODUCT_DIR)/gtest_runner.appx',
      },
    },
  ],
}
