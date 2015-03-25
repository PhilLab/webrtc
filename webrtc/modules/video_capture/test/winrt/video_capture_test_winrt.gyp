# Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
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
      'target_name': 'video_capture_test_winrt',
      'type': 'executable',
      'dependencies': [
        '<(webrtc_root)/modules/modules.gyp:video_capture_module_internal_impl',
      ],
      'defines': [
        '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        '../../../../../..',
        'Generated Files',
      ],
      'sources': [
		'MainPage.xaml',
		'MainPage.xaml.h',
		'MainPage.xaml.cpp',
		'Package.appxmanifest',
		'video_capture_test_winrt_TemporaryKey.pfx',
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
	  'msvs_package_certificate': {
		'KeyFile': 'video_capture_test_winrt_TemporaryKey.pfx',
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
		   'mfplat.lib',
		   'mfuuid.lib',
		  ],
		},
	  },
    },
  ],
}
