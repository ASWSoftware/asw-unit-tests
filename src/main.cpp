/* **************************************************************************
Main.cpp
Author: Anthony S. West - ASW Software

Console app that runs AswUnitTests framework.

Copyright 2025 Anthony S. West

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
#include <cstdlib>
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
using namespace ASWUnitTests;
//---------------------------------------------------------------------------

namespace
{

std::optional<TColorMode> ParseColorMode(std::string const& text);
std::optional<unsigned int> ParseUnsignedInt(std::string const& text);
void PrintUsage();
std::vector<TJUnitTestCase> ToJUnitTestCases(std::vector<TTestCaseRecord> const& records);

//---------------------------------------------------------------------------
std::optional<TColorMode> ParseColorMode(std::string const& text)
{
    if (text == "auto")
        return TColorMode::Auto;

    if (text == "always")
        return TColorMode::Always;

    if (text == "never")
        return TColorMode::Never;

    return std::nullopt;
}

//---------------------------------------------------------------------------
std::optional<unsigned int> ParseUnsignedInt(std::string const& text)
{
    if (text.empty())
        return std::nullopt;

    try
    {
        size_t charsConsumed = 0;
        unsigned long const value = std::stoul(text, &charsConsumed);

        if (charsConsumed != text.size() || value > std::numeric_limits<unsigned int>::max())
            return std::nullopt;

        return static_cast<unsigned int>(value);
    }
    catch (std::exception const&)
    {
        return std::nullopt;
    }
}

//---------------------------------------------------------------------------
void PrintUsage()
{
    std::cout
    << "Usage: ASWUnitTests [options]\n"
    "\n"
    "Options:\n"
    "  --filter <pattern>  Run only tests whose \"GroupName.TestName\" full name\n"
    "                      matches <pattern>. '*' matches any sequence of\n"
    "                      characters (including none); '?' matches exactly\n"
    "                      one character. The whole name must match, e.g.\n"
    "                      \"*String*\" for a substring search.\n"
    "  --filter-ignore-case\n"
    "                      Match --filter's <pattern> case-insensitively.\n"
    "                      Has no effect without --filter.\n"
    "  --shuffle           Run groups, and each group's tests, in a randomized\n"
    "                      order instead of the default deterministic order.\n"
    "                      The seed used is logged so a failure caused by\n"
    "                      order can be reproduced via --shuffle-seed.\n"
    "  --shuffle-seed <N>  Shuffle (implies --shuffle) using an explicit\n"
    "                      unsigned integer seed, to reproduce a previous\n"
    "                      --shuffle run's order.\n"
    "  --partition-index <N>\n"
    "                      1-based index of this run's partition, from 1 to\n"
    "                      --partition-count. Requires --partition-count.\n"
    "  --partition-count <N>\n"
    "                      Splits the full test suite into <N> roughly-equal\n"
    "                      partitions by each test's position in the canonical\n"
    "                      registration order (the same order --list shows),\n"
    "                      so every test runs in exactly one partition\n"
    "                      regardless of which group it's in. Run <N> separate\n"
    "                      invocations (e.g. one per CI job), each with its\n"
    "                      own --partition-index, to run the suite in parallel\n"
    "                      with no coordination between processes. Requires\n"
    "                      --partition-index.\n"
    "  --color <mode>      One of \"auto\" (default; color only on an\n"
    "                      interactive terminal that supports it, and only\n"
    "                      if the NO_COLOR environment variable isn't set),\n"
    "                      \"always\", or \"never\".\n"
    "  --no-color          Shorthand for --color never.\n"
    "  --color-pass/--color-fail/--color-skip <color>\n"
    "                      Set the color used for passed/failed/skipped\n"
    "                      status text. <color> is one of: default, black,\n"
    "                      red, green, yellow, blue, magenta, cyan, white,\n"
    "                      or bright-<name> for the bright variant (e.g.\n"
    "                      bright-red). Defaults: pass=green, fail=red,\n"
    "                      skip=yellow.\n"
    "  --report-junit <path>\n"
    "                      Write a JUnit-style XML test report to <path>, in\n"
    "                      addition to the normal console output. Recognized\n"
    "                      by most CI systems (GitHub Actions, GitLab CI,\n"
    "                      Jenkins, Azure DevOps, CircleCI) for native test\n"
    "                      result reporting.\n"
    "  --project-name <name>\n"
    "                      Set the name this run is identified by: shown in\n"
    "                      the console's \"Initializing...\" line and, if\n"
    "                      --report-junit is also given, used as the report's\n"
    "                      <testsuites name=\"...\"> attribute (default:\n"
    "                      \"ASWUnitTests\").\n"
    "  --list              List all registered tests as \"GroupName.TestName\"\n"
    "                      and exit, without running anything.\n"
    "  --pause             Prompt \"press enter to continue\" before exiting\n"
    "                      after a --list command or test run.\n"
    "  --version           Print the framework version and exit.\n"
    "  --help              Show this message and exit.\n";
}

//---------------------------------------------------------------------------
std::vector<TJUnitTestCase> ToJUnitTestCases(std::vector<TTestCaseRecord> const& records)
{
    std::vector<TJUnitTestCase> testCases;
    testCases.reserve(records.size());

    for (TTestCaseRecord const& record : records)
    {
        TJUnitOutcome outcome = TJUnitOutcome::Pass;
        if (record.Outcome == TTestOutcome::Fail)
            outcome = TJUnitOutcome::Fail;
        else if (record.Outcome == TTestOutcome::Skip)
            outcome = TJUnitOutcome::Skip;

        testCases.push_back(TJUnitTestCase{ record.GroupName, record.TestName, record.DurationSeconds, outcome,
                                            record.Message });
    }

    return testCases;
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    bool listOnly = false;
    bool hasFilter = false;
    bool filterIgnoreCase = false;
    std::string filterPattern;
    bool shuffle = false;
    std::optional<unsigned int> shuffleSeed;
    bool hasPartitionIndex = false;
    bool hasPartitionCount = false;
    unsigned int partitionIndex = 0;
    unsigned int partitionCount = 0;
    TColorMode colorMode = TColorMode::Auto;
    std::optional<TConsoleColor> colorPass;
    std::optional<TConsoleColor> colorFail;
    std::optional<TConsoleColor> colorSkip;
    std::string junitReportPath;
    std::string projectName = "ASWUnitTests";
    bool pauseOnExit = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--version")
        {
            std::cout << TTestHandler::GetVersionFullStr() << std::endl;
            return 0;
        }
        else if (arg == "--help" || arg == "-h")
        {
            PrintUsage();
            return 0;
        }
        else if (arg == "--list")
        {
            listOnly = true;
        }
        else if (arg == "--filter")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --filter requires a pattern argument.\n";
                return 4;
            }

            filterPattern = argv[++i];
            hasFilter = true;
        }
        else if (arg.rfind("--filter=", 0) == 0)
        {
            filterPattern = arg.substr(9);
            hasFilter = true;
        }
        else if (arg == "--filter-ignore-case")
        {
            filterIgnoreCase = true;
        }
        else if (arg == "--shuffle")
        {
            shuffle = true;
        }
        else if (arg == "--shuffle-seed")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --shuffle-seed requires a numeric argument.\n";
                return 4;
            }

            shuffleSeed = ParseUnsignedInt(argv[++i]);
            if (!shuffleSeed.has_value())
            {
                std::cout << "Error: --shuffle-seed requires a non-negative integer argument.\n";
                return 4;
            }

            shuffle = true;
        }
        else if (arg.rfind("--shuffle-seed=", 0) == 0)
        {
            shuffleSeed = ParseUnsignedInt(std::string(arg.substr(15)));
            if (!shuffleSeed.has_value())
            {
                std::cout << "Error: --shuffle-seed requires a non-negative integer argument.\n";
                return 4;
            }

            shuffle = true;
        }
        else if (arg == "--partition-index")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --partition-index requires a numeric argument.\n";
                return 4;
            }

            std::optional<unsigned int> const parsed = ParseUnsignedInt(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-index requires a positive integer argument.\n";
                return 4;
            }

            partitionIndex = *parsed;
            hasPartitionIndex = true;
        }
        else if (arg.rfind("--partition-index=", 0) == 0)
        {
            std::optional<unsigned int> const parsed = ParseUnsignedInt(std::string(arg.substr(18)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-index requires a positive integer argument.\n";
                return 4;
            }

            partitionIndex = *parsed;
            hasPartitionIndex = true;
        }
        else if (arg == "--partition-count")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --partition-count requires a numeric argument.\n";
                return 4;
            }

            std::optional<unsigned int> const parsed = ParseUnsignedInt(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-count requires a positive integer argument.\n";
                return 4;
            }

            partitionCount = *parsed;
            hasPartitionCount = true;
        }
        else if (arg.rfind("--partition-count=", 0) == 0)
        {
            std::optional<unsigned int> const parsed = ParseUnsignedInt(std::string(arg.substr(18)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-count requires a positive integer argument.\n";
                return 4;
            }

            partitionCount = *parsed;
            hasPartitionCount = true;
        }
        else if (arg == "--no-color")
        {
            colorMode = TColorMode::Never;
        }
        else if (arg == "--color")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return 4;
            }

            std::optional<TColorMode> const parsed = ParseColorMode(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return 4;
            }

            colorMode = *parsed;
        }
        else if (arg.rfind("--color=", 0) == 0)
        {
            std::optional<TColorMode> const parsed = ParseColorMode(std::string(arg.substr(8)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return 4;
            }

            colorMode = *parsed;
        }
        else if (arg == "--color-pass" || arg == "--color-fail" || arg == "--color-skip")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: " << arg << " requires a color name argument.\n";
                return 4;
            }

            std::optional<TConsoleColor> const parsed = TConsole::ParseColorName(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: " << arg << " was given an unrecognized color name.\n";
                return 4;
            }

            if (arg == "--color-pass")
                colorPass = parsed;
            else if (arg == "--color-fail")
                colorFail = parsed;
            else
                colorSkip = parsed;
        }
        else if (arg.rfind("--color-pass=", 0) == 0 || arg.rfind("--color-fail=", 0) == 0 ||
                 arg.rfind("--color-skip=", 0) == 0)
        {
            size_t const equalsPos = arg.find('=');
            std::string const flagName(arg.substr(0, equalsPos));
            std::optional<TConsoleColor> const parsed = TConsole::ParseColorName(std::string(arg.substr(equalsPos + 1)));

            if (!parsed.has_value())
            {
                std::cout << "Error: " << flagName << " was given an unrecognized color name.\n";
                return 4;
            }

            if (flagName == "--color-pass")
                colorPass = parsed;
            else if (flagName == "--color-fail")
                colorFail = parsed;
            else
                colorSkip = parsed;
        }
        else if (arg == "--report-junit")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --report-junit requires a file path argument.\n";
                return 4;
            }

            junitReportPath = argv[++i];
        }
        else if (arg.rfind("--report-junit=", 0) == 0)
        {
            junitReportPath = arg.substr(15);
        }
        else if (arg == "--project-name")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --project-name requires a name argument.\n";
                return 4;
            }

            projectName = argv[++i];
        }
        else if (arg.rfind("--project-name=", 0) == 0)
        {
            projectName = arg.substr(15);
        }
        else if (arg == "--pause")
        {
            pauseOnExit = true;
        }
        else
        {
            std::cout << "Error: unrecognized option \"" << arg << "\".\n\n";
            PrintUsage();
            return 4;
        }
    }

    if (hasPartitionIndex != hasPartitionCount)
    {
        std::cout << "Error: --partition-index and --partition-count must be given together.\n";
        return 4;
    }

    if (hasPartitionCount)
    {
        if (partitionCount == 0)
        {
            std::cout << "Error: --partition-count must be at least 1.\n";
            return 4;
        }

        if (partitionIndex == 0 || partitionIndex > partitionCount)
        {
            std::cout << "Error: --partition-index must be between 1 and --partition-count ("
            << partitionCount << ").\n";
            return 4;
        }
    }

    TConsole::SetColorMode(colorMode);

    if (colorPass.has_value())
        TConsole::SetColor(TLogKind::Pass, *colorPass);

    if (colorFail.has_value())
        TConsole::SetColor(TLogKind::Fail, *colorFail);

    if (colorSkip.has_value())
        TConsole::SetColor(TLogKind::Skip, *colorSkip);

    int returnCode = 0; // no error

    try
    {
        TTestHandler tester;

        tester.Initialize(projectName);

        TestFilter filter;
        std::string filterDescription;

        if (hasFilter)
        {
            filter = [filterPattern, filterIgnoreCase](std::string const& fullTestName)
                {
                    return TTestHandler::WildcardMatch(filterPattern, fullTestName, filterIgnoreCase);
                };

            filterDescription = "\"" + filterPattern + "\"" + (filterIgnoreCase ? " (case-insensitive)" : "");
        }

        if (hasPartitionCount)
        {
            // Membership is assigned by each test's position in the canonical (unshuffled) registration order (same
            // order "--list" shows). Using the canonical order means a given test's partition never changes based on
            // whether "--shuffle" is requested. The presence of "--shuffle" reorders execution within a partition. This
            // allows for the suite to be split across separate process invocations (e.g. one per CI job) with no
            // coordination between them and no merge step needed afterward when "--report-junit" output is requested
            // (it's just another JUnit XML file for the CI system's own native multi-file merging).
            std::vector<std::string> const allTestNames = tester.GetAllTestFullNames();
            unsigned int const zeroBasedIndex = partitionIndex - 1;
            std::unordered_set<std::string> partitionMembers;

            for (size_t testPos = 0; testPos < allTestNames.size(); ++testPos)
            {
                if ((testPos % partitionCount) == zeroBasedIndex)
                    partitionMembers.insert(allTestNames[testPos]);
            }

            TestFilter const previousFilter = filter;

            filter = [previousFilter, partitionMembers = std::move(partitionMembers)](std::string const& fullTestName)
                {
                    if (previousFilter != nullptr && !previousFilter(fullTestName))
                        return false;

                    return partitionMembers.count(fullTestName) > 0;
                };

            if (!filterDescription.empty())
                filterDescription += ", ";

            filterDescription += "partition " + std::to_string(partitionIndex) + " of " +
                std::to_string(partitionCount);
        }

        if (listOnly)
        {
            // Applying the filter here lets the caller verify a pattern's matches before running it.
            tester.ListTests(filter, filterDescription);
        }
        else
        {
            TTestResults testResults = tester.Run(filter, filterDescription, shuffle, shuffleSeed);
            unsigned int nTestsFailed = testResults.FailedCount;
            if (nTestsFailed > 0)
                returnCode = 1;

            if (!junitReportPath.empty())
            {
                std::vector<TJUnitTestCase> const junitTestCases = ToJUnitTestCases(testResults.CaseRecords);
                if (TJUnitReportWriter::Write(junitReportPath, tester.GetProjectName(), junitTestCases))
                    tester.Log("JUnit report written to: " + junitReportPath);
                else
                    tester.Log("Error: could not write JUnit report to: " + junitReportPath);
            }
        }
    }
    catch (std::exception const& ex)
    {
        std::cout << "\nTerminating app. Unhandled exception: " << ex.what() << std::endl;
        returnCode = 2;
    }
    catch (...)
    {
        std::cout << "\nTerminating app. Unhandled exception: Unknown" << std::endl;
        returnCode = 3;
    }

    if (pauseOnExit)
    {
#if defined(_WIN32)
        system("pause");
#else
        std::cout << "Press enter to continue..." << std::endl;
        std::cin.get();
#endif
    }

    return returnCode;
}
