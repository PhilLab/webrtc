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
        '.',
      ],
      'sources': [
        'App.cpp',
        'common.h',
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
        '..\gtest_runner\gtest_runner_TemporaryKey.pfx',
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
          'destination': '<(PRODUCT_DIR)/libTest_runner_package',
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
        'KeyFile': '..\gtest_runner\gtest_runner_TemporaryKey.pfx',
        'Thumbprint': 'E3AA95A6CD6D9DF6D0B7C68EBA246B558824F8C1',
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
    {	
      'target_name': 'libTest_runner_appx',
      'product_name': 'libTest_runner',
      'product_extension': 'appx',
      'type': 'none',
      'dependencies': [
        'libTest_runner',
      ],
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/libTest_runner_package',
          'files':[
            '<(PRODUCT_DIR)/libTest_runner.exe',
          ],
        },
      ],
      'appx': {
        'dep': '<(PRODUCT_DIR)/libTest_runner_package/libTest_runner.exe',
        'dir': '<(PRODUCT_DIR)/libTest_runner_package',
        'out': '<(PRODUCT_DIR)/libTest_runner.appx',
        'cert': '..\gtest_runner\gtest_runner_TemporaryKey.pfx'
      },
    },
  ],
}
