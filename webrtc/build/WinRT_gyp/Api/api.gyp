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
        '../../../../talk/libjingle.gyp:libjingle_peerconnection',
      ],
      'defines': [
         '_HAS_EXCEPTIONS=1',
         '_WINRT_DLL',
      ],
      'include_dirs': [
        '../../../../third_party/libyuv/include',
        '.',
      ],
      'sources': [
        'PeerConnectionInterface.h',
        'PeerConnectionInterface.cc',
        'GlobalObserver.h',
        'GlobalObserver.cc',
        'Marshalling.h',
        'Marshalling.cc',
        'Media.h',
        'Media.cc',
        'DataChannel.h',
        'DataChannel.cc',
        'Delegates.h',
        'RTMediaStreamSource.h',
        'RTMediaStreamSource.cc',
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'AdditionalOptions': [
            '/WINMD',
          ],
          'WindowsMetadataFile':'$(OutDir)webrtc_winrt_api.winmd',
          'conditions': [
            ['OS_RUNTIME=="winrt" and (winrt_platform=="win10" or winrt_platform=="win10_arm")', {
              'AdditionalDependencies': [
                'WindowsApp.lib',
              ],
            }],
          ],
        },
      },
    },
  ],
}
