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
      'target_name': 'webrtc_winjs_api',
      'type': 'shared_library',
      'dependencies': [
        '../api/api.gyp:webrtc_winrt_api',
        '../../../../webrtc/voice_engine/voice_engine.gyp:voice_engine',
        '../../../../webrtc/video_engine/video_engine.gyp:video_engine_core',
        '../../../../webrtc/modules/modules.gyp:video_capture_module_internal_impl',
        '../../../../webrtc/modules/modules.gyp:video_render_module_internal_impl',
        '../../../../webrtc/system_wrappers/system_wrappers.gyp:metrics_default',
        '../../../../webrtc/test/test.gyp:channel_transport',
        '../../../../webrtc/test/test.gyp:field_trial',
        '../../../../webrtc/system_wrappers/system_wrappers.gyp:field_trial_default',
      ],
      'defines': [
         '_HAS_EXCEPTIONS=1',
         '_WINRT_DLL',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        "mediaengine.h",
        "mediaengine.cc",
        "mediastreamsource.h",
        "mediastreamsource.cc",

      ],
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
  ],
}
