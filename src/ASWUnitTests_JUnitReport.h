/* **************************************************************************
ASWUnitTests_JUnitReport.h
Author: Anthony S. West - ASW Software

Self-contained JUnit-style XML test report writer, for --report-junit.

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

#ifndef ASWUnitTests_JUnitReportH
#define ASWUnitTests_JUnitReportH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TJUnitOutcome
/////////////////////////////////////////////////////////////////////////////
enum class TJUnitOutcome
{
    Pass,
    Fail,
    Skip
};


/////////////////////////////////////////////////////////////////////////////
// TJUnitTestCase
//
// One <testcase> entry. 'Message' is the failure/skip detail text; ignored
// (and may be empty) when Outcome is Pass.
/////////////////////////////////////////////////////////////////////////////
struct TJUnitTestCase
{
    std::string GroupName;
    std::string TestName;
    double DurationSeconds;
    TJUnitOutcome Outcome;
    std::string Message;
};


/////////////////////////////////////////////////////////////////////////////
// TJUnitReportWriter
//
// Writes a JUnit-style XML report (<testsuites>/<testsuite>/<testcase>),
// the de facto standard most CI systems (GitHub Actions, GitLab CI,
// Jenkins, Azure DevOps, CircleCI) natively recognize for test results.
// Self-contained: depends only on the standard library, including its own
// XML escaping, so it can be dropped in without an XML parser/writer
// library or any other third-party dependency.
/////////////////////////////////////////////////////////////////////////////
class TJUnitReportWriter
{
public:
    // Escapes '&', '<', '>', '"', and '\'' for safe inclusion in XML text or attribute values.
    static std::string EscapeXml(std::string const& text);

    // Writes the report to 'filePath'. 'testCases' must already be grouped contiguously by
    // GroupName (as a full test run naturally produces them); each contiguous run of matching
    // GroupName becomes one <testsuite>, in the order first encountered. Returns false if
    // 'filePath' could not be opened for writing.
    static bool Write(std::string const& filePath, std::string const& suitesName,
        std::vector<TJUnitTestCase> const& testCases);
};

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_JUnitReportH
