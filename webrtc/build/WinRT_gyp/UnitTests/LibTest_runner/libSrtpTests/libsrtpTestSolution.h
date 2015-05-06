#pragma once

namespace libSrtpTests
{
  using namespace LibTest_runner;
  //Singleton for libsrtp tests
  typedef CSafeSingletonT<CTestSolution> TestSolution;

  //=============================================================================
  //         class: SingleInstanceTestSolutionProvider
  //   Description: Functor providing single instance of CTestSolution class for LibSrtp Tests
  // History: 
  // 2015/03/02 TP: created
  //=============================================================================
  struct SingleInstanceTestSolutionProvider
  {
    CTestSolution& operator()()
    {
      return TestSolution::Instance();
    }
  };
}