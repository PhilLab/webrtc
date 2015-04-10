# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': ['../../common.gypi',],
  'variables': {

  },
  'targets': [
    {
      'target_name': 'standup',
      'type': 'executable',
      'dependencies': [
        '../../../../webrtc/voice_engine/voice_engine.gyp:voice_engine',
        '../../../../webrtc/video_engine/video_engine.gyp:video_engine_core',
        '../../../../webrtc/modules/modules.gyp:video_capture_module_internal_impl',
        '../../../../webrtc/modules/modules.gyp:video_render_module_internal_impl',
      ],
      'defines': [
         '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        'App.cpp',
        '..\UnitTests\gtest_runner\gtest_runner_TemporaryKey.pfx',
      ],
      'conditions': [
        ['OS_RUNTIME=="winrt" and winrt_platform=="win_phone"', {
          'sources': [
            'Package.phone.appxmanifest',
          ],
        }],
          ['OS_RUNTIME=="winrt" and winrt_platform!="win_phone"', {
          'sources': [
            'Package.appxmanifest',
          ],
        }],
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/standup_package',
          'files':[
            'Generated Manifest\AppxManifest.xml',
            'Logo.png',
            'SmallLogo.png',
            'SplashScreen.png',
            'StoreLogo.png',
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
          ],
        },
      ],
      'msvs_disabled_warnings': [
      ],
      'msvs_package_certificate': {
        'KeyFile': '..\UnitTests\gtest_runner\gtest_runner_TemporaryKey.pfx',
        'Thumbprint': 'E3AA95A6CD6D9DF6D0B7C68EBA246B558824F8C1',
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          # ExceptionHandling must match _HAS_EXCEPTIONS above.
          'ExceptionHandling': '1',
        },
        'VCLinkerTool': {
          'AdditionalDependencies': [
           'mfplat.lib',
           'mfuuid.lib',
          ],
        },
      },
    },
    {
      'target_name': 'standup_appx',
      'type': 'none',
      'dependencies': [
        'standup',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/standup_package',
          'files':[
            '<(PRODUCT_DIR)/standup.exe',
          ],
        },
      ],
      'appx': {
        'dep': '<(PRODUCT_DIR)/standup_package/standup.exe',
        'dir': '<(PRODUCT_DIR)/standup_package',
        'out': '<(PRODUCT_DIR)/standup.appx',
        'cert': '..\UnitTests\gtest_runner\gtest_runner_TemporaryKey.pfx'
      },
    },
  ],
}
