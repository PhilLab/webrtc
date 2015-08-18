
// Copyright (c) 2015 The WebRTC project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "webrtc/build/WinRT_gyp/UnitTests/LibTest_runner/common.h"
#include "webrtc/build/WinRT_gyp/UnitTests/LibTest_runner/TestSolution/XmlReporter.h"


using namespace Windows::Data::Xml::Dom;
using namespace Windows::Data::Xml::Xsl;
using namespace Windows::Storage;
using namespace Platform;
using namespace concurrency;

// XML Elements
#define SOLUTION_ELEMENT_NAME L"TestSolution"
#define PROJECT_ELEMENT_NAME  L"project"
#define LIBRARY_ELEMENT_NAME  L"library"
#define TEST_ELEMENT_NAME     L"test"
// XML attributes
#define ATTRIBUTE_NAME        L"name"
#define ATTRIBUTE_TESTS       L"tests"
#define ATTRIBUTE_EXECUTED    L"executed"
#define ATTRIBUTE_EXECUTION_TIME_MS    L"ExecutionTimeMs"
#define ATTRIBUTE_SUCCEEDED   L"succeeded"
#define ATTRIBUTE_RESULT_MESSAGE L"resultmessage"
#define ATTRIBUTE_EXIT_STATUS L"exitstatus"
#define ATTRIBUTE_VALUE_TRUE  L"true"
#define ATTRIBUTE_VALUE_FALSE L"false"

namespace LibTest_runner {
#pragma warning(push)
#pragma warning(disable : 4290)


  //=======================================================================
  //         Method: GetLibraryElement
  //    Description: Gets Library element. Creates new if not exists
  //       Argument: String ^ libraryName - library name
  //         return: XmlElement^ library element
  //
  //       History:
  // 2015/03/16 TP: created
  //======================================================================
  XmlElement^ CXmlReporter::GetLibraryElement(String^ libraryName) {
    XmlElement^ ret;

    // gets library elements
    XmlNodeList^ libraries = solutionEl_->GetElementsByTagName(
      LIBRARY_ELEMENT_NAME);
    if (libraries != nullptr) {
      // find library in library collections
      for each (XmlElement^ item in libraries) {
        String^ name = item->GetAttribute(ATTRIBUTE_NAME);
        if ((name != nullptr) && (name == libraryName)) {
          ret = item;
          break;
        }
      }
    }

    // project not found create new
    if (ret == nullptr) {
      ret = report_->CreateElement(LIBRARY_ELEMENT_NAME);
      ret->SetAttribute(ATTRIBUTE_NAME, libraryName);
      solutionEl_->AppendChild(ret);
    }
    return ret;
  }


  //=======================================================================
  //         Method: GetProjectElement
  //    Description: Gets specified project element in specified library
  //                 element. Creates new if not exists
  //       Argument: XmlElement ^ library  - library element
  //       Argument: String ^ projectName - project name
  //         return: XmlElement^ project element
  //
  //       History:
  // 2015/03/16 TP: created
  //======================================================================
  XmlElement^ CXmlReporter::GetProjectElement(XmlElement^ library,
    String^ projectName) {
    XmlElement^ ret;

    XmlNodeList^ projects =
      library->GetElementsByTagName(PROJECT_ELEMENT_NAME);
    if (projects != nullptr) {
      // find project in project collections
      for each (XmlElement^ item in projects) {
        String^ name = item->GetAttribute(ATTRIBUTE_NAME);
        if ((name != nullptr) && (name == projectName)) {
          ret = item;
          break;
        }
      }
    }

    // project not found create new
    if (ret == nullptr) {
      ret = report_->CreateElement(PROJECT_ELEMENT_NAME);
      ret->SetAttribute(ATTRIBUTE_NAME, projectName);
      library->AppendChild(ret);
    }

    return ret;
  }

  //=======================================================================
  //         Method: CXmlReporter
  //    Description: ctor
  //       Argument: String^  outputFile - report output file name.
  //                       Relative to root folder in the local app data store.
  //                       See Windows::Storage::ApplicationData::LocalFolder
  //                       for details
  //                 unsigned int flags - reporter flags
  //         return:
  //
  //       History:
  // 2015/03/16 TP: created
  //======================================================================
  CXmlReporter::CXmlReporter(String^ outputFile, unsigned int flags)
    : OutputFile_(outputFile)
    , m_nFlags(flags) {}

  void CXmlReporter::AddTestResult(const CTestBase& test)
    throw(ReportGenerationException) {
    if (test.Executed() || (m_nFlags & kAllTests)) {
      // first find project element
      XmlElement^ projectEl = GetProjectElement(GetLibraryElement(
        ref new String(test.Library().c_str())),
        ref new String(test.Project().c_str()));

      // create test element
      XmlElement^ testEl = report_->CreateElement(TEST_ELEMENT_NAME);
      // set test attributes
      testEl->SetAttribute(ATTRIBUTE_NAME,
        ref new String(test.Name().c_str()));
      testEl->SetAttribute(ATTRIBUTE_EXECUTED, test.Executed() ?
        ATTRIBUTE_VALUE_TRUE : ATTRIBUTE_VALUE_FALSE);
      testEl->SetAttribute(ATTRIBUTE_SUCCEEDED, test.Succeed() ?
        ATTRIBUTE_VALUE_TRUE : ATTRIBUTE_VALUE_FALSE);
      testEl->SetAttribute(ATTRIBUTE_EXIT_STATUS, "" + test.ExitStatus());
      testEl->SetAttribute(ATTRIBUTE_RESULT_MESSAGE,
        ref new String(test.ResultMessage().c_str()));
      testEl->SetAttribute(ATTRIBUTE_EXECUTION_TIME_MS, "" +
        test.GetExecutionTimeMs().count());
      projectEl->AppendChild(testEl);
    }
  }

  void CXmlReporter::Begin() throw(ReportGenerationException) {
    report_ = ref new XmlDocument();
  }

  void CXmlReporter::AddTestSolutionHeader(const CTestSolution& solution)
    throw(ReportGenerationException) {
    // creates element for solution
    solutionEl_ = report_->CreateElement(SOLUTION_ELEMENT_NAME);
    solutionEl_->SetAttribute(ATTRIBUTE_TESTS, "" + solution.GetTestCount());
    report_->AppendChild(solutionEl_);
  }

  void CXmlReporter::End() throw(ReportGenerationException) {
    auto writeTask = create_task(
      ApplicationData::Current->LocalFolder->CreateFileAsync(
        OutputFile_, CreationCollisionOption::ReplaceExisting));

    try {
      writeTask.then([this](StorageFile^ reportFile) {
        this->report_->SaveToFileAsync(reportFile);
      }).get();
    }
    catch (Platform::Exception^ ex) {
      throw new ReportGenerationException(ex);
    }
  }

#pragma warning(pop)
}  // namespace LibTest_runner
