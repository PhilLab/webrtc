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
        '../../../../third_party/h264_winrt/h264_winrt.gyp:webrtc_h264_winrt',
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
      'conditions': [
        ['OS_RUNTIME=="winrt" and (winrt_platform=="win10" or winrt_platform=="win10_arm")', {
          'msvs_disabled_warnings': [
            '4458',  # local members hides previously defined memebers or function members or class members
          ],
        }],
      ],
      'msvs_settings': {
        'VCLinkerTool': {
          'UseLibraryDependencyInputs': "true",
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
            ['OS_RUNTIME=="winrt" and winrt_platform=="win_phone"', {
              'AdditionalOptions': [
                # Fixes linking for assembler opus source files 
                '<(PRODUCT_DIR)/obj/opus/celt_pitch_xcorr_arm.obj',
                '<(SHARED_INTERMEDIATE_DIR)/third_party/libvpx/*.obj',
                '../../../../third_party/libyuv/source/*.obj',
                '<(PRODUCT_DIR)/obj/openmax_dl_armv7/*.obj',
                '<(PRODUCT_DIR)/obj/openmax_dl_neon/*.obj',
              ],
            }],
          ],
        },
      },
    },
  ],
}
