/*
*  Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#pragma once

namespace libSrtpTests {
//=============================================================================
//         class: CSrtpKernelDriverTest
//   Description: class executes replay_driver test project, see
//            chromium\src\third_party\libsrtp\srtp_test_kernel_driver.vcxproj
// History:
// 2015/02/27 TP: created
//=============================================================================
class CSrtpKernelDriverTest :
    public CLibSrtpTestBase {
 private:
    AUTO_ADD_TEST(SingleInstanceTestSolutionProvider, CSrtpKernelDriverTest);

 protected:
    int InterchangeableExecute();

 public:
    virtual ~CSrtpKernelDriverTest() {}
    TEST_NAME_IMPL(CSrtpKernelDriverTest);
    TEST_PROJECT_IMPL(srtp_test_kernel_driver);
    TEST_LIBRARY_IMPL(libSrtp);
};

  typedef std::shared_ptr<CSrtpKernelDriverTest> SpSrtpKernelDriverTest_t;
}  // namespace libSrtpTests

