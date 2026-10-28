/* **************************************************************************
Test_ASWUnitTests_JUnitReport.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 ASW Software

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
#include <fstream>
#include <iterator>
#include <string>
#include <vector>
#if defined(_WIN32)
#  include <windows.h>
#else
#  include <unistd.h>
#endif
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_JUnitReport.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

unsigned long GetCurrentProcessIdPortable();
std::string PathToUTF8(std::filesystem::path const& path);
std::string WithoutTimestamps(std::string xml);

//---------------------------------------------------------------------------
/*
    GetCurrentProcessIdPortable

    Portable enough for this one use: a per-process-unique suffix for the temp folder name below, so
    two OS processes running this suite concurrently (e.g. via --partition-index/--partition-count)
    never share, and can't race on, the same temp directory.
*/
unsigned long GetCurrentProcessIdPortable()
{
#if defined(_WIN32)
    return ::GetCurrentProcessId();
#else
    return static_cast<unsigned long>(::getpid());
#endif
}
//---------------------------------------------------------------------------
/*
    PathToUTF8

    The path in UTF-8, as TJUnitReportWriter::Write() takes it. On Windows, path::string() gives the ANSI code
    page instead, and MSVC's throws for a character it lacks.
*/
std::string PathToUTF8(std::filesystem::path const& path)
{
#if defined(_WIN32)
    return ASWUnitTests::WideToUTF8(path.wstring());
#else
    return path.string();
#endif
}
//---------------------------------------------------------------------------
/*
    WithoutTimestamps

    'xml' with each timestamp attribute's value removed, so two reports built in different seconds compare equal.
*/
std::string WithoutTimestamps(std::string xml)
{
    std::string const attribute = "timestamp=\"";
    std::string::size_type pos = 0;

    while ((pos = xml.find(attribute, pos)) != std::string::npos)
    {
        pos += attribute.size();
        std::string::size_type const end = xml.find('"', pos);
        if (end == std::string::npos)
            break;

        xml.erase(pos, end - pos);
    }

    return xml;
}

} // namespace

//---------------------------------------------------------------------------

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
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_BuildXML_CountsAndEscapes, "BuildXML_CountsAndEscapes");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_PlainTextUnchanged, "EscapeXml_PlainTextUnchanged");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_EscapeXml_SpecialCharacters, "EscapeXml_SpecialCharacters");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_Write_ProducesReadableReport, "Write_ProducesReadableReport");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_Write_TakesAUTF8Path, "Write_TakesAUTF8Path");
    RegisterTest(&TTest_ASWUnitTests_JUnitReport::Test_Write_WritesBuildXMLUnchanged, "Write_WritesBuildXMLUnchanged");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_JUnitReport::~TTest_ASWUnitTests_JUnitReport()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::SetUp_Group()
{
    Log("Setting up temp group folder: " + PathToUTF8(m_GroupBaseTempDir));
    std::filesystem::create_directories(m_GroupBaseTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::SetUp_Test(ITestCase& testCase)
{
    m_TestTempDir = m_GroupBaseTempDir / testCase.GetName();
    Log("  Setting up temp folder for " + testCase.GetName() + ": " + PathToUTF8(m_TestTempDir));
    std::filesystem::create_directories(m_TestTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::TearDown_Group()
{
    Log("Cleaning up temp group folder: " + PathToUTF8(m_GroupBaseTempDir));
    std::filesystem::remove_all(m_GroupBaseTempDir);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::TearDown_Test(ITestCase& testCase)
{
    Log("  Cleaning up temp folder for " + testCase.GetName() + ": " + PathToUTF8(m_TestTempDir));
    std::filesystem::remove_all(m_TestTempDir);
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_BuildXML_CountsAndEscapes()
{
    // Arrange
    std::vector<TJUnitTestCase> const testCases =
    {
        { "GroupA", "TestPass", 0.25, TJUnitOutcome::Pass, "" },
        { "GroupA", "TestFail", 0.5, TJUnitOutcome::Fail, "expected <1> & \"2\"" },
        { "GroupB", "TestSkip", 0.0, TJUnitOutcome::Skip, "not implemented yet" },
    };

    // Act
    std::string const xml = TJUnitReportWriter::BuildXML("My<Project>", testCases);

    // Assert
    CheckStartsWith(xml, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n", __func__, __LINE__,
        "starts with the XML declaration");
    CheckContains(xml, "<testsuites name=\"My&lt;Project&gt;\" tests=\"3\" failures=\"1\" skipped=\"1\" "
        "time=\"0.750000\">", __func__, __LINE__, "root element's name and totals");
    CheckContains(xml, "<testsuite name=\"GroupA\" tests=\"2\" failures=\"1\" skipped=\"0\" time=\"0.750000\"",
        __func__, __LINE__, "GroupA's counts");
    CheckContains(xml, "<testsuite name=\"GroupB\" tests=\"1\" failures=\"0\" skipped=\"1\" time=\"0.000000\"",
        __func__, __LINE__, "GroupB's counts");
    CheckContains(xml, "<failure message=\"expected &lt;1&gt; &amp; &quot;2&quot;\">", __func__, __LINE__,
        "failure message escaped");
    CheckEndsWith(xml, "</testsuites>\n", __func__, __LINE__, "ends with the closing root element");
}
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
    bool const written = TJUnitReportWriter::Write(PathToUTF8(reportPath), "MyProject", testCases);

    // Assert
    AssertTrue(written, __func__, __LINE__, "Write() reports success");
    AssertTrue(std::filesystem::exists(reportPath), __func__, __LINE__, "report file exists on disk");

    std::ifstream reportFile(reportPath);
    std::string const content((std::istreambuf_iterator<char>(reportFile)), std::istreambuf_iterator<char>());

    CheckContains(content, "<testsuites name=\"MyProject\"", __func__, __LINE__,
        "root element carries the project name");
    CheckContains(content, "<testsuite name=\"GroupA\"", __func__, __LINE__, "GroupA testsuite present");
    CheckContains(content, "<testsuite name=\"GroupB\"", __func__, __LINE__, "GroupB testsuite present");
    CheckContains(content, "name=\"TestPass\"", __func__, __LINE__, "TestPass present");
    CheckContains(content, "<failure", __func__, __LINE__, "failure element present for TestFail");
    CheckContains(content, "<skipped", __func__, __LINE__, "skipped element present for TestSkip");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_Write_TakesAUTF8Path()
{
    // Arrange: a file name with U+03A9, which the Windows-1252 code page lacks.
#if defined(_WIN32)
    std::filesystem::path const reportPath = m_TestTempDir / L"report-\x03A9.xml";
#else
    std::filesystem::path const reportPath = m_TestTempDir / "report-\xCE\xA9.xml";
#endif
    std::vector<TJUnitTestCase> const testCases = { { "GroupA", "TestPass", 0.0, TJUnitOutcome::Pass, "" } };

    // Act
    bool const written = TJUnitReportWriter::Write(PathToUTF8(reportPath), "MyProject", testCases);

    // Assert
    CheckTrue(written, __func__, __LINE__, "Write() reports success");
    CheckTrue(std::filesystem::exists(reportPath), __func__, __LINE__, "the report has the name it was given");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_JUnitReport::Test_Write_WritesBuildXMLUnchanged()
{
    // Arrange
    std::filesystem::path const reportPath = m_TestTempDir / "report.xml";

    std::vector<TJUnitTestCase> const testCases =
    {
        { "GroupA", "TestPass", 0.0, TJUnitOutcome::Pass, "" },
        { "GroupA", "TestFail", 0.0, TJUnitOutcome::Fail, "line 1\nline 2" },
    };

    // Act
    bool const written = TJUnitReportWriter::Write(PathToUTF8(reportPath), "MyProject", testCases);
    std::string const expected = TJUnitReportWriter::BuildXML("MyProject", testCases);

    // Assert
    AssertTrue(written, __func__, __LINE__, "Write() reports success");

    // Read in binary, so a CRLF the file holds isn't turned back into LF on Windows.
    std::ifstream reportFile(reportPath, std::ios::in | std::ios::binary);
    std::string const content((std::istreambuf_iterator<char>(reportFile)), std::istreambuf_iterator<char>());

    CheckEquals(WithoutTimestamps(expected), WithoutTimestamps(content), __func__, __LINE__,
        "the file holds BuildXML()'s bytes (timestamps aside, which can differ by a second)");
    CheckTrue(content.find('\r') == std::string::npos, __func__, __LINE__, "no CR added to the line ends");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_JUnitReport)
