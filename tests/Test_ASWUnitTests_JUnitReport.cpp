/* **************************************************************************
Test_ASWUnitTests_JUnitReport.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

************************************************************************** */

//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <fstream>
#include <iterator>
#include <vector>
#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif
//---------------------------------------------------------------------------
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------

namespace
{

// Portable enough for this one use: a per-process-unique suffix for the temp folder name below, so
// two OS processes running this suite concurrently (e.g. via --partition-index/--partition-count)
// never share, and can't race on, the same temp directory.
unsigned long GetCurrentProcessIdPortable()
{
#if defined(_WIN32)
    return ::GetCurrentProcessId();
#else
    return static_cast<unsigned long>(::getpid());
#endif
}

} // namespace

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_JUnitReport
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_JUnitReport::TTest_ASWUnitTests_JUnitReport()
    : inherited("ASWUnitTests_JUnitReport_Tests"),
      m_GroupBaseTempDir(std::filesystem::temp_directory_path() /
          ("ASWUnitTests_JUnitReport_Tests_" + std::to_string(GetCurrentProcessIdPortable())))
{
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_PlainTextUnchanged, "EscapeXml_PlainTextUnchanged");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_SpecialCharacters, "EscapeXml_SpecialCharacters");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_Write_ProducesReadableReport, "Write_ProducesReadableReport");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_JUnitReport::~TTest_ASWUnitTests_JUnitReport()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::SetUp_Group()
{
    Log("Setting up temp group folder: " + m_GroupBaseTempDir.string());
    std::filesystem::create_directories(m_GroupBaseTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::SetUp_Test(ITestCase& testCase)
{
    m_TestTempDir = m_GroupBaseTempDir / testCase.GetName();
    Log("  Setting up temp folder for " + testCase.GetName() + ": " + m_TestTempDir.string());
    std::filesystem::create_directories(m_TestTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::TearDown_Group()
{
    Log("Cleaning up temp group folder: " + m_GroupBaseTempDir.string());
    std::filesystem::remove_all(m_GroupBaseTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::TearDown_Test(ITestCase& testCase)
{
    Log("  Cleaning up temp folder for " + testCase.GetName() + ": " + m_TestTempDir.string());
    std::filesystem::remove_all(m_TestTempDir);
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_PlainTextUnchanged()
{
    // Arrange
    std::string plain = "Test_HexSingleToByte[A] passed in 1.234 ms";

    // Act
    std::string escaped = TJUnitReportWriter::EscapeXml(plain);

    // Assert
    CheckEquals(plain, escaped, __func__, __LINE__, "plain text passes through unchanged");
    CheckEquals(std::string(""), TJUnitReportWriter::EscapeXml(""), __func__, __LINE__, "empty string");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_SpecialCharacters()
{
    // Arrange & Act & Assert
    CheckEquals(std::string("&amp;"), TJUnitReportWriter::EscapeXml("&"), __func__, __LINE__, "ampersand");
    CheckEquals(std::string("&lt;"), TJUnitReportWriter::EscapeXml("<"), __func__, __LINE__, "less-than");
    CheckEquals(std::string("&gt;"), TJUnitReportWriter::EscapeXml(">"), __func__, __LINE__, "greater-than");
    CheckEquals(std::string("&quot;"), TJUnitReportWriter::EscapeXml("\""), __func__, __LINE__, "double quote");
    CheckEquals(std::string("&apos;"), TJUnitReportWriter::EscapeXml("'"), __func__, __LINE__, "apostrophe");

    // A message realistically produced by a failing Check*/Assert* call, exercising all five together.
    std::string const message = "Expected \"a\" but was <b> & 'c'";
    std::string const expected = "Expected &quot;a&quot; but was &lt;b&gt; &amp; &apos;c&apos;";
    CheckEquals(expected, TJUnitReportWriter::EscapeXml(message), __func__, __LINE__, "combined failure message");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_Write_ProducesReadableReport()
{
    // Arrange
    std::filesystem::path const reportPath = m_TestTempDir / "report.xml";

    std::vector<TJUnitTestCase> const testCases =
    {
        { "GroupA", "TestPass", 0.001, TJUnitOutcome::Pass, "" },
        { "GroupA", "TestFail", 0.002, TJUnitOutcome::Fail, "expected 1 but was 2" },
        { "GroupB", "TestSkip", 0.0, TJUnitOutcome::Skip, "not implemented yet" },
    };

    // Act
    bool const written = TJUnitReportWriter::Write(reportPath.string(), "MyProject", testCases);

    // Assert
    AssertTrue(written, __func__, __LINE__, "Write() reports success");
    AssertTrue(std::filesystem::exists(reportPath), __func__, __LINE__, "report file exists on disk");

    std::ifstream reportFile(reportPath);
    std::string const content((std::istreambuf_iterator<char>(reportFile)), std::istreambuf_iterator<char>());

    CheckTrue(content.find("<testsuites name=\"MyProject\"") != std::string::npos,
        __func__, __LINE__, "root element carries the project name");
    CheckTrue(content.find("<testsuite name=\"GroupA\"") != std::string::npos, __func__, __LINE__, "GroupA testsuite present");
    CheckTrue(content.find("<testsuite name=\"GroupB\"") != std::string::npos, __func__, __LINE__, "GroupB testsuite present");
    CheckTrue(content.find("name=\"TestPass\"") != std::string::npos, __func__, __LINE__, "TestPass present");
    CheckTrue(content.find("<failure") != std::string::npos, __func__, __LINE__, "failure element present for TestFail");
    CheckTrue(content.find("<skipped") != std::string::npos, __func__, __LINE__, "skipped element present for TestSkip");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_JUnitReport)
