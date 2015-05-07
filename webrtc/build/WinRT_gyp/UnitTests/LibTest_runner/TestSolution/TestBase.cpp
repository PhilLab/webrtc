#include "common.h"
#include "TestBase.h"

using namespace std::chrono;


//=======================================================================
//         Method: CTestBase
//    Description: ctor
//         return: 
//
//       History:
// 2015/03/06 TP: created
//======================================================================
LibTest_runner::CTestBase::CTestBase()
  : m_nExitStatus(0)
  , m_bSucceed(false)
  , m_bExecuted(false)
  , m_uExecutionTimeMs(0)
{
}


//=======================================================================
//         Method: PrepareForExecution
//    Description: Prepares test for execution
//         return: void
//
//       History:
// 2015/03/06 TP: created
//======================================================================
void LibTest_runner::CTestBase::PrepareForExecution()
{
  Reset();
  m_wsOutput.resize(OutputBufferSize());
  InterchangeablePrepareForExecution();
}


//=======================================================================
//         Method: VerifyResult
//    Description: Verifies test results and sets success accordingly 
//         return: void
//
//       History:
// 2015/03/06 TP: created
//======================================================================
void LibTest_runner::CTestBase::VerifyResult()
{
  m_bSucceed = false;

  if (m_bExecuted)
  {
    //this is pretty much we can do here, rest of the 
    //verification has to be test specific
    m_bSucceed = m_nExitStatus == 0;
    InterchangeableVerifyResult();
  }
}

//=======================================================================
//         Method: Execute
//    Description: Executes test
//         return: void
//
//       History:
// 2015/03/06 TP: created
//======================================================================
int LibTest_runner::CTestBase::Execute()
{
  PrepareForExecution();
  m_bExecuted = true;
  high_resolution_clock::time_point startTime;
  high_resolution_clock::time_point endTime;
  try
  {
    LibTest_runner::CStdOutputRedirector<true> redirector(m_wsOutput); //grab test output
    //store time
    startTime = high_resolution_clock::now();
    m_nExitStatus = InterchangeableExecute();
    endTime = high_resolution_clock::now();
  }
  catch (int Status)
  {
    m_nExitStatus = Status;
  }

  m_uExecutionTimeMs = duration_cast<milliseconds>(endTime - startTime);

  VerifyResult();
  InterchangeableTestCleanup();

  return m_nExitStatus;
}

//=======================================================================
//         Method: Reset
//    Description: Resets test
//         return: void
//
//       History:
// 2015/03/06 TP: created
//======================================================================
void LibTest_runner::CTestBase::Reset()
{
  m_nExitStatus = 0;
  m_bSucceed = false;
  m_bExecuted = false;
  m_wsOutput.clear();
}
