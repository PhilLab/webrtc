//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <collection.h>
#include <ppltasks.h>

#include "App.xaml.h"

// STL includes
#include <string>
#include <vector>

// Helpers
#include "Helpers/SafeSingleton.h"
#include "Helpers/TestInserter.h"
#include "Helpers/StdOutputRedirector.h"

// Test Solution 
#include "TestSolution/TestBase.h"
#include "TestSolution/TestSolution.h"

// libsrtp tests
#include "libSrtpTests\libsrtpTestSolution.h"
#include "libSrtpTests\ReplayDriverTest.h"
#include "libSrtpTests\RocDriverTest.h"
#include "libSrtpTests\RtpwTest.h"
#include "libSrtpTests\RdbxDriverTest.h"
#include "libSrtpTests\SrtpDriverTest.h"
#include "libSrtpTests\SrtpAesCalcTest.h"
#include "libSrtpTests\SrtpCipherDriverTest.h"
#include "libSrtpTests\SrtpDatatypesDriverTest.h"
#include "libSrtpTests\SrtpEnvTest.h"
#include "libSrtpTests\SrtpKernelDriverTest.h"
#include "libSrtpTests\SrtpRandGenTest.h"
#include "libSrtpTests\SrtpSha1DriverTest.h"
#include "libSrtpTests\SrtpStatDriverTest.h"
