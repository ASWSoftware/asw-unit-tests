/* **************************************************************************
ASWUnitTests_JUnitReport.cpp
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
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <sstream>
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

std::string FormatSeconds(double seconds);
std::string FormatUtcTimestamp();
void WriteTestCase(std::ostringstream& body, TJUnitTestCase const& testCase);

//---------------------------------------------------------------------------
std::string FormatSeconds(double seconds)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << seconds;
    return oss.str();
}
//---------------------------------------------------------------------------
std::string FormatUtcTimestamp()
{
    std::chrono::system_clock::time_point const now = std::chrono::system_clock::now();
    std::time_t const nowTimeT = std::chrono::system_clock::to_time_t(now);
    std::tm const utcTm = *std::gmtime(&nowTimeT);

    std::ostringstream oss;
    oss << std::put_time(&utcTm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}
//---------------------------------------------------------------------------
void WriteTestCase(std::ostringstream& body, TJUnitTestCase const& testCase)
{
    body << "    <testcase name=\"" << TJUnitReportWriter::EscapeXml(testCase.TestName) << "\" classname=\""
    << TJUnitReportWriter::EscapeXml(testCase.GroupName) << "\" time=\""
    << FormatSeconds(testCase.DurationSeconds) << "\"";

    if (testCase.Outcome == TJUnitOutcome::Pass)
    {
        body << "/>\n";
        return;
    }

    body << ">\n";

    if (testCase.Outcome == TJUnitOutcome::Fail)
    {
        body << "      <failure message=\"" << TJUnitReportWriter::EscapeXml(testCase.Message) << "\">"
        << TJUnitReportWriter::EscapeXml(testCase.Message) << "</failure>\n";
    }
    else // Skip
    {
        body << "      <skipped message=\"" << TJUnitReportWriter::EscapeXml(testCase.Message) << "\"/>\n";
    }

    body << "    </testcase>\n";
}

} // namespace

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TJUnitReportWriter
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
std::string TJUnitReportWriter::EscapeXml(std::string const& text)
{
    std::string result;
    result.reserve(text.size());

    for (char ch : text)
    {
        switch (ch)
        {
            case '&':
                result += "&amp;";
                break;
            case '<':
                result += "&lt;";
                break;
            case '>':
                result += "&gt;";
                break;
            case '"':
                result += "&quot;";
                break;
            case '\'':
                result += "&apos;";
                break;
            default:
                result += ch;
                break;
        }
    }

    return result;
}
//---------------------------------------------------------------------------
bool TJUnitReportWriter::Write(std::string const& filePath, std::string const& suitesName,
    std::vector<TJUnitTestCase> const& testCases)
{
    std::string const timestamp = FormatUtcTimestamp();
    std::ostringstream body;

    size_t totalTests = 0;
    size_t totalFailures = 0;
    size_t totalSkipped = 0;
    double totalTime = 0.0;

    size_t index = 0;
    while (index < testCases.size())
    {
        std::string const groupName = testCases[index].GroupName;
        size_t suiteTests = 0;
        size_t suiteFailures = 0;
        size_t suiteSkipped = 0;
        double suiteTime = 0.0;
        std::ostringstream suiteBody;

        while (index < testCases.size() && testCases[index].GroupName == groupName)
        {
            TJUnitTestCase const& testCase = testCases[index];

            ++suiteTests;
            suiteTime += testCase.DurationSeconds;
            if (testCase.Outcome == TJUnitOutcome::Fail)
                ++suiteFailures;
            else if (testCase.Outcome == TJUnitOutcome::Skip)
                ++suiteSkipped;

            WriteTestCase(suiteBody, testCase);
            ++index;
        }

        body << "  <testsuite name=\"" << EscapeXml(groupName) << "\" tests=\"" << suiteTests << "\" failures=\""
        << suiteFailures << "\" skipped=\"" << suiteSkipped << "\" time=\"" << FormatSeconds(suiteTime)
        << "\" timestamp=\"" << timestamp << "\">\n" << suiteBody.str() << "  </testsuite>\n";

        totalTests += suiteTests;
        totalFailures += suiteFailures;
        totalSkipped += suiteSkipped;
        totalTime += suiteTime;
    }

    std::ofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open())
        return false;

    file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
    file << "<testsuites name=\"" << EscapeXml(suitesName) << "\" tests=\"" << totalTests << "\" failures=\""
    << totalFailures << "\" skipped=\"" << totalSkipped << "\" time=\"" << FormatSeconds(totalTime) << "\">\n";
    file << body.str();
    file << "</testsuites>\n";

    return file.good();
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
