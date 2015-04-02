/*
*  Copyright (c) 2011 The WebRTC project authors. All Rights Reserved.
*
*  Use of this source code is governed by a BSD-style license
*  that can be found in the LICENSE file in the root of the source
*  tree. An additional intellectual property rights grant can be found
*  in the file PATENTS.  All contributing project authors may
*  be found in the AUTHORS file in the root of the source tree.
*/

#include "common.h"
#include "VideoCodingTest.h"

// test entry point declaration
extern int video_coding_tester_main(int argc, char **argv);

AUTO_ADD_TEST_IMPL(videoCodingTests::CVideoCodingNormalTest);
AUTO_ADD_TEST_IMPL(videoCodingTests::CVideoCodingCodecDataBaseTest);
AUTO_ADD_TEST_IMPL(videoCodingTests::CVideoCodingRtpPlayTest);
AUTO_ADD_TEST_IMPL(videoCodingTests::CVideoCodingRtpPlayMTTest);
AUTO_ADD_TEST_IMPL(videoCodingTests::CVideoCodingQualityModeTest);

int videoCodingTests::CVideoCodingNormalTest::InterchangeableExecute() {
  char* argv[] = { ".", "--test_number=1" };
  return video_coding_tester_main(2, argv);
}

int videoCodingTests::CVideoCodingCodecDataBaseTest::InterchangeableExecute() {
  char* argv[] = { ".", "--test_number=4" };
  return video_coding_tester_main(2, argv);
}

int videoCodingTests::CVideoCodingRtpPlayTest::InterchangeableExecute() {
  char* argv[] = { ".", "--test_number=7" };
  return video_coding_tester_main(2, argv);
}

int videoCodingTests::CVideoCodingRtpPlayMTTest::InterchangeableExecute() {
  char* argv[] = { ".", "--test_number=8" };
  return video_coding_tester_main(2, argv);
}

int videoCodingTests::CVideoCodingQualityModeTest::InterchangeableExecute() {
  char* argv[] = { ".", "--test_number=9" };
  return video_coding_tester_main(2, argv);
}
