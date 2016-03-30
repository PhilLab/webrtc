# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
#
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file in the root of the source
# tree. An additional intellectual property rights grant can be found
# in the file PATENTS.  All contributing project authors may
# be found in the AUTHORS file in the root of the source tree.

{
  'includes': ['../../common.gypi',],
  'targets': [
    {
      'target_name': 'webrtc_stats_observer',
      'type': 'static_library',
      'dependencies': [
        '../../../../webrtc/api/api.gyp:libjingle_peerconnection',
      ],
      'sources': [
        'webrtc_stats_observer.cpp',
        'webrtc_stats_observer.h',
        'etw_providers.man',
      ],
     'rules': [{
       # Rule to run the message compiler.
       'rule_name': 'message_compiler',
       'extension': 'man',
       'outputs': [
         '<(RULE_INPUT_ROOT)/etw_providers.h',
         '<(RULE_INPUT_ROOT)/etw_providers.rc',
         '<(RULE_INPUT_ROOT)/etw_providersTemp.bin',
       ],
       'action': [
         'mc.exe',
         '-um',
         '<(RULE_INPUT_PATH)',
       ],
       'message': 'Running message compiler on <(RULE_INPUT_PATH)',
     }],
    },
  ],
}
