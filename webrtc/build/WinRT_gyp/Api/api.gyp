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
      'target_name': 'webrtc_winrt_api',
      'type': 'shared_library',
      'dependencies': [
        '../../../../webrtc/system_wrappers/system_wrappers.gyp:metrics_default',
        '../../../../talk/libjingle.gyp:libjingle_peerconnection',
        '../../../../webrtc/test/test.gyp:field_trial',
        # '../../../../webrtc/system_wrappers/system_wrappers.gyp:field_trial_default',
      ],
      'defines': [
         '_HAS_EXCEPTIONS=1',
         '_WINRT_DLL',
      ],
      'include_dirs': [
        '.',
      ],
      'sources': [
        'peerconnectioninterface.h',
        'peerconnectioninterface.cc',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalOptions': [
            '/WINMD',
          ],
        },
      },
    },
  ],
}
