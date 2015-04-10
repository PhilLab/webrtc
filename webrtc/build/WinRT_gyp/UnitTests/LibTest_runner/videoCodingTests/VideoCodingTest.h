/*
*  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "helpers/TestInserter.h"
#pragma once

namespace videoCodingTests {
// TODO(winrt) Add more test with different arguments when/if needed

// class: CVideoCodingNormalTest
// executes video_coding_test project with parameter test_number=1
class CVideoCodingNormalTest :
  public LibTest_runner::CTestBase {
 private:
  AUTO_ADD_TEST(libSrtpTests::SingleInstanceTestSolutionProvider,
    CVideoCodingNormalTest);
 protected:
  int InterchangeableExecute();
 public:
  virtual ~CVideoCodingNormalTest() {}
  TEST_NAME_IMPL(VideoCodingNormalTest);
  TEST_PROJECT_IMPL(video_coding_test);
  TEST_LIBRARY_IMPL(libVideoCoding);
};

// class: CVideoCodingCodecDataBaseTest
// executes video_coding_test project with parameter test_number==4
class CVideoCodingCodecDataBaseTest :
  public LibTest_runner::CTestBase {
 private:
    AUTO_ADD_TEST(libSrtpTests::SingleInstanceTestSolutionProvider,
      CVideoCodingCodecDataBaseTest);
 protected:
    int InterchangeableExecute();
 public:
    virtual ~CVideoCodingCodecDataBaseTest() {}
    TEST_NAME_IMPL(VideoCodingCodecDataBaseTest);
    TEST_PROJECT_IMPL(video_coding_test);
    TEST_LIBRARY_IMPL(libVideoCoding);
};

// class: CVideoCodingGenericCodecTest
// executes video_coding_test project with parameter test_number==7
class CVideoCodingRtpPlayTest :
  public LibTest_runner::CTestBase {
 private:
    AUTO_ADD_TEST(libSrtpTests::SingleInstanceTestSolutionProvider,
      CVideoCodingRtpPlayTest);
 protected:
    int InterchangeableExecute();
 public:
    virtual ~CVideoCodingRtpPlayTest() {}
    TEST_NAME_IMPL(VideoCodingRtpPlayTest);
    TEST_PROJECT_IMPL(video_coding_test);
    TEST_LIBRARY_IMPL(libVideoCoding);
};

// class: CVideoCodingGenericCodecTest
// executes video_coding_test project with parameter test_number==8
class CVideoCodingRtpPlayMTTest :
  public LibTest_runner::CTestBase {
 private:
    AUTO_ADD_TEST(libSrtpTests::SingleInstanceTestSolutionProvider,
      CVideoCodingRtpPlayMTTest);
 protected:
    int InterchangeableExecute();
 public:
    virtual ~CVideoCodingRtpPlayMTTest() {}
    TEST_NAME_IMPL(VideoCodingRtpPlayMTTest);
    TEST_PROJECT_IMPL(video_coding_test);
    TEST_LIBRARY_IMPL(libVideoCoding);
};

// class: CVideoCodingQualityModeTest
// executes video_coding_test project with parameter test_number==9
class CVideoCodingQualityModeTest :
  public LibTest_runner::CTestBase {
 private:
    AUTO_ADD_TEST(libSrtpTests::SingleInstanceTestSolutionProvider,
      CVideoCodingQualityModeTest);
 protected:
    int InterchangeableExecute();
 public:
    virtual ~CVideoCodingQualityModeTest() {}
    TEST_NAME_IMPL(VideoCodingQualityModeTest);
    TEST_PROJECT_IMPL(video_coding_test);
    TEST_LIBRARY_IMPL(libVideoCoding);
};
}  // namespace videoCodingTests
