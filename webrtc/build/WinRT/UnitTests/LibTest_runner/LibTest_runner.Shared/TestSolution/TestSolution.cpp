#include "pch.h"
#include "TestSolution.h"


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
      (*it)->Execute();
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
