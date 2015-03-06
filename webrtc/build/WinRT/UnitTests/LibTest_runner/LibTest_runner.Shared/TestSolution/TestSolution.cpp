#include "pch.h"
#include "TestSolution.h"


//=======================================================================
//         Method: InternalTestExecute
//    Description: Runs specified test
//       Argument: const SpTestBase_t & pTest test to run
//         return: void
//
//       History:
// 2015/03/03 TP: created
//======================================================================
void LibTest_runner::CTestSolution::InternalTestExecute(const SpTestBase_t& pTest)
{
  try
  {
    printf("\n--- Executing %s ------\n", pTest->Name().c_str());
    pTest->Execute();
  }
  catch (int status)
  {
    printf("--- %s test terminated with status %d ------\n", pTest->Name().c_str(), status);
  }
}

//=======================================================================
//         Method: CTestSolution::Execute
//    Description: executes test solution
//         return: void
//
//       History:
// 2015/02/27 TP: created
//======================================================================
void LibTest_runner::CTestSolution::Execute()
{
  if (!IsEmpty())
  {
    for (auto it = m_spTests->cbegin(); it != m_spTests->cend(); ++it)
    {
      InternalTestExecute(*it);
    }
  }
}


//=======================================================================
//         Method: Execute
//    Description: Executes test with specified name
//       Argument: const wchar_t * testName - specified test name
//         return: void
//
//       History:
// 2015/03/03 TP: created
//======================================================================
void LibTest_runner::CTestSolution::Execute(const wchar_t* testName)
{
  if (!IsEmpty())
  {
    for (auto it = m_spTests->cbegin(); it != m_spTests->cend(); ++it)
    {
      if ((*it)->Name().compare(testName) == 0)
      {
        InternalTestExecute(*it);
      }
    }
  }
}

//=======================================================================
//         Method: LibTest_runner::CTestSolution::AddTest
//    Description: Adds test to solution
//       Argument: const SpTestBase_t & ptrTest - test to add
//         return: 
//
//       History:
// 2015/02/27 TP: created
//======================================================================
void LibTest_runner::CTestSolution::AddTest(const SpTestBase_t& ptrTest)
{
  if (m_spTests == NULL)
  {
    m_spTests.reset(new TestCollection_t());
  }

  m_spTests->push_back(ptrTest);
}

//=======================================================================
//         Method: LibTest_runner::CTestSolution::GetTestCount
//    Description: Gets test count
//         return: size_t test count
//
//       History:
// 2015/02/27 TP: created
//======================================================================
size_t LibTest_runner::CTestSolution::GetTestCount() const
{
  size_t ret = 0;
  if (!IsEmpty())
  {
    ret = m_spTests->size();
  }

  return ret;
}
