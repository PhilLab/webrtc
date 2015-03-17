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
#include <exception>

// Helpers
#include "Helpers/SafeSingleton.h"
#include "Helpers/TestInserter.h"
#include "Helpers/StdOutputRedirector.h"

// Test Solution 
#include "TestSolution/ReportGenerationException.h"
#include "TestSolution/TestsReporterBase.h"
#include "TestSolution/TestBase.h"
#include "TestSolution/TestSolution.h"
#include "TestSolution/WStringReporter.h"
#include "TestSolution/XmlReporter.h"


// libsrtp tests
#include "libSrtpTests\LibSrtpTestBase.h"
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
