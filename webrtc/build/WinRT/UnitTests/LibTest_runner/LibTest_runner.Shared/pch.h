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

// Test Solution 
#include "TestSolution/TestBase.h"
#include "TestSolution/TestSolution.h"

// Tests
#include "Tests\ReplayDriverTest.h"
#include "Tests\RocDriverTest.h"
#include "Tests\RtpwTest.h"
#include "Tests\RdbxDriverTest.h"
#include "Tests\SrtpDriverTest.h"
#include "Tests\SrtpAesCalcTest.h"
#include "Tests\SrtpCipherDriverTest.h"
#include "Tests\SrtpDatatypesDriverTest.h"
#include "Tests\SrtpEnvTest.h"
#include "Tests\SrtpKernelDriverTest.h"
#include "Tests\SrtpRandGenTest.h"
#include "Tests\SrtpSha1DriverTest.h"
#include "Tests\SrtpStatDriverTest.h"
