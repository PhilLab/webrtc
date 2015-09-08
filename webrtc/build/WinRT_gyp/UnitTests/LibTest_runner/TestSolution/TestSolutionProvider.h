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

namespace LibTest_runner {
  // Singleton for tests
  typedef CSafeSingletonT<CTestSolution> TestSolution;

  //===========================================================================
  //         class: SingleInstanceTestSolutionProvider
  //   Description: Functor providing single instance of CTestSolution
  //   class for LibSrtp Tests
  // History:
  // 2015/03/02 TP: created
  //===========================================================================
struct SingleInstanceTestSolutionProvider {
    CTestSolution& operator()() {
      return TestSolution::Instance();
    }
};
}
