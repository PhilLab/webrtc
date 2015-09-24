#include "common.h"
#include "LibSrtpTestBase.h"

extern "C" void getopt_reset();

namespace libSrtpTests
{
  void CLibSrtpTestBase::InterchangeablePrepareForExecution()
  {
    //reset global variables for getopt
    getopt_reset();
  }
}
