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
      'target_name': 'libTest_runner',
      'type': 'executable',
      'dependencies': [
        '../../../../../third_party/libsrtp/libsrtp.gyp:libsrtp',
        '../../../../../third_party/libsrtp/libsrtp.gyp:rdbx_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:roc_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:replay_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:rtpw',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_cipher_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_datatypes_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_stat_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_kernel_driver',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_aes_calc',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_rand_gen',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_env',
        '../../../../../third_party/libsrtp/libsrtp.gyp:srtp_test_sha1_driver',
      ],
      'defines': [
         '_HAS_EXCEPTIONS=1',
      ],
      'include_dirs': [
        'Generated Files',
      ],
      'sources': [
        'MainPage.xaml',
        'MainPage.xaml.h',
        'MainPage.xaml.cpp',
        'Package.appxmanifest',
        'Assets/Logo.scale-100.png',
        'Assets/SmallLogo.scale-100.png',
        'Assets/SplashScreen.scale-100.png',
        'Assets/StoreLogo.scale-100.png',
        'App.xaml',
        'App.xaml.h',
        'App.xaml.cpp',
        'pch.h',
        'pch.cpp',
        'Helpers/SafeSingleton.h',
        'Helpers/StdOutputRedirector.h',
        'Helpers/TestInserter.h',
        'TestSolution/TestBase.cpp',
        'TestSolution/TestBase.h',
        'TestSolution/TestSolution.cpp',
        'TestSolution/TestSolution.h',
        'TestSolution/TestsReporterBase.h',
        'TestSolution/WStringReporter.cpp',
        'TestSolution/WStringReporter.h',
        'TestSolution/XmlReporter.cpp',
        'TestSolution/XmlReporter.h',
        'libSrtpTests/LibSrtpTestBase.cpp',
        'libSrtpTests/LibSrtpTestBase.h',
        'libSrtpTests/RdbxDriverTest.cpp',
        'libSrtpTests/RdbxDriverTest.h',
        'libSrtpTests/ReplayDriverTest.cpp',
        'libSrtpTests/ReplayDriverTest.h',
        'libSrtpTests/RocDriverTest.cpp',
        'libSrtpTests/RocDriverTest.h',
        'libSrtpTests/RtpwTest.cpp',
        'libSrtpTests/RtpwTest.h',
        'libSrtpTests/SrtpAesCalcTest.cpp',
        'libSrtpTests/SrtpAesCalcTest.h',
        'libSrtpTests/SrtpCipherDriverTest.cpp',
        'libSrtpTests/SrtpCipherDriverTest.h',
        'libSrtpTests/SrtpDatatypesDriverTest.cpp',
        'libSrtpTests/SrtpDatatypesDriverTest.h',
        'libSrtpTests/SrtpDriverTest.cpp',
        'libSrtpTests/SrtpDriverTest.h',
        'libSrtpTests/SrtpEnvTest.cpp',
        'libSrtpTests/SrtpEnvTest.h',
        'libSrtpTests/SrtpKernelDriverTest.cpp',
        'libSrtpTests/SrtpKernelDriverTest.h',
        'libSrtpTests/SrtpRandGenTest.cpp',
        'libSrtpTests/SrtpRandGenTest.h',
        'libSrtpTests/SrtpSha1DriverTest.cpp',
        'libSrtpTests/SrtpSha1DriverTest.h',
        'libSrtpTests/SrtpStatDriverTest.cpp',
        'libSrtpTests/SrtpStatDriverTest.h',
        'libSrtpTests/libsrtpTestSolution.h',
      ],
      'msvs_disabled_warnings': [
      ],
      'msvs_package_certificate': {
      },
      'msvs_settings': {
        'VCCLCompilerTool': {
          # ExceptionHandling must match _HAS_EXCEPTIONS above.
          'ExceptionHandling': '1',
        },
        'VCLinkerTool': {
        },
      },
    },
  ],
}
