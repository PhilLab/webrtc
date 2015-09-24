#pragma once

//=============================================================================
//         class: TestInserter
//      template: TestSolutionProvider - functor, provides TestSolution instance 
//                                       to add test
//                Test - test class to add
//   Description: Class for insertion specified test class to specified test solution
// History: 
// 2015/03/02 TP: created
//=============================================================================
template <class TestSolutionProvider, class Test>
struct TestInserter
{
  TestInserter()
  {
    //add new test so solution
    TestSolutionProvider()().AddTest(std::shared_ptr<Test>(new Test()));
  }
};


// Macro provides automatic registration for Test class.
// See TestInserter class for parameters description
// !!! DO NOT FORGET declare AUTO_ADD_TEST_IMPL macro in some cpp file !!!
// Usage:
//    class CSomeTest: public CTestBase {
//      AUTO_ADD_TEST(TestSolutionProvider, CSomeTest);
#define AUTO_ADD_TEST(TestSolutionProvider, Test) \
    typedef TestSolutionProvider __TestSolutionProvider; \
    typedef Test __Test; \
    static TestInserter<__TestSolutionProvider, __Test>  __inserter; \


// Implementation part of AUTO_ADD_TEST macro. See AUTO_ADD_TEST for details
// Usage, Must be placed in cpp file:
//   AUTO_ADD_TEST_IMPL(CSomeTest);
//TODO: there should be a compilation error when AUTO_ADD_TEST_IMPL not defined
#define AUTO_ADD_TEST_IMPL(clazz) \
    TestInserter<clazz::__TestSolutionProvider, clazz::__Test> clazz::__inserter; \

#endif  // WEBRTC_BUILD_WINRT_GYP_UNITTESTS_LIBTEST_RUNNER_HELPERS_TESTINSERTER_H_
