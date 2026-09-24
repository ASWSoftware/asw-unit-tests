/* **************************************************************************
ASWUnitTests_CLI.cpp
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
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------
#include <exception>
#include <iostream>
#include <limits>
#include <string_view>
#include <unordered_set>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

void PrintUsage();

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

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TCLIParser
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TestFilter TCLIParser::BuildTestFilter(TTestHandler& tester, TCLIOptions const& options,
    std::string& filterDescription)
{
    TestFilter filter;

    if (options.HasFilter)
    {
        filter = [pattern = options.FilterPattern, ignoreCase = options.FilterIgnoreCase](
            std::string const& fullTestName)
            {
                return TTestHandler::WildcardMatch(pattern, fullTestName, ignoreCase);
            };

        filterDescription = "\"" + options.FilterPattern + "\"" + (options.FilterIgnoreCase ? " (case-insensitive)" : "");
    }

    if (options.HasPartitionCount)
    {
        // Membership is assigned by each test's position in the canonical (unshuffled) registration order (same
        // order "--list" shows). Using the canonical order means a given test's partition never changes based on
        // whether "--shuffle" is requested. The presence of "--shuffle" reorders execution within a partition. This
        // allows for the suite to be split across separate process invocations (e.g. one per CI job) with no
        // coordination between them and no merge step needed afterward when "--report-junit" output is requested
        // (it's just another JUnit XML file for the CI system's own native multi-file merging).
        std::vector<std::string> const allTestNames = tester.GetAllTestFullNames();
        unsigned int const zeroBasedIndex = options.PartitionIndex - 1;
        std::unordered_set<std::string> partitionMembers;

        for (size_t testPos = 0; testPos < allTestNames.size(); ++testPos)
        {
            if ((testPos % options.PartitionCount) == zeroBasedIndex)
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

        filterDescription += "partition " + std::to_string(options.PartitionIndex) + " of " +
            std::to_string(options.PartitionCount);
    }

    return filter;
}
//---------------------------------------------------------------------------
std::optional<int> TCLIParser::ParseArguments(int argc, char* argv[], TCLIOptions& options)
{
    for (int i = 1; i < argc; ++i)
    {
        std::string_view arg = argv[i];

        if (arg == "--version")
        {
            std::cout << TTestHandler::GetVersionFullStr() << std::endl;
            return ExitCode_Success;
        }
        else if (arg == "--help" || arg == "-h")
        {
            PrintUsage();
            return ExitCode_Success;
        }
        else if (arg == "--list")
        {
            options.ListOnly = true;
        }
        else if (arg == "--filter")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --filter requires a pattern argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.FilterPattern = argv[++i];
            options.HasFilter = true;
        }
        else if (arg.rfind("--filter=", 0) == 0)
        {
            options.FilterPattern = arg.substr(9);
            options.HasFilter = true;
        }
        else if (arg == "--filter-ignore-case")
        {
            options.FilterIgnoreCase = true;
        }
        else if (arg == "--shuffle")
        {
            options.Shuffle = true;
        }
        else if (arg == "--shuffle-seed")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --shuffle-seed requires a numeric argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.ShuffleSeed = ParseUnsignedInt(argv[++i]);
            if (!options.ShuffleSeed.has_value())
            {
                std::cout << "Error: --shuffle-seed requires a non-negative integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.Shuffle = true;
        }
        else if (arg.rfind("--shuffle-seed=", 0) == 0)
        {
            options.ShuffleSeed = ParseUnsignedInt(std::string(arg.substr(15)));
            if (!options.ShuffleSeed.has_value())
            {
                std::cout << "Error: --shuffle-seed requires a non-negative integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.Shuffle = true;
        }
        else if (arg == "--partition-index")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --partition-index requires a numeric argument.\n";
                return ExitCode_InvalidArguments;
            }

            std::optional<unsigned int> const parsed = ParseUnsignedInt(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-index requires a positive integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.PartitionIndex = *parsed;
            options.HasPartitionIndex = true;
        }
        else if (arg.rfind("--partition-index=", 0) == 0)
        {
            std::optional<unsigned int> const parsed = ParseUnsignedInt(std::string(arg.substr(18)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-index requires a positive integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.PartitionIndex = *parsed;
            options.HasPartitionIndex = true;
        }
        else if (arg == "--partition-count")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --partition-count requires a numeric argument.\n";
                return ExitCode_InvalidArguments;
            }

            std::optional<unsigned int> const parsed = ParseUnsignedInt(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-count requires a positive integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.PartitionCount = *parsed;
            options.HasPartitionCount = true;
        }
        else if (arg.rfind("--partition-count=", 0) == 0)
        {
            std::optional<unsigned int> const parsed = ParseUnsignedInt(std::string(arg.substr(18)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --partition-count requires a positive integer argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.PartitionCount = *parsed;
            options.HasPartitionCount = true;
        }
        else if (arg == "--no-color")
        {
            options.ColorMode = TColorMode::Never;
        }
        else if (arg == "--color")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return ExitCode_InvalidArguments;
            }

            std::optional<TColorMode> const parsed = ParseColorMode(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return ExitCode_InvalidArguments;
            }

            options.ColorMode = *parsed;
        }
        else if (arg.rfind("--color=", 0) == 0)
        {
            std::optional<TColorMode> const parsed = ParseColorMode(std::string(arg.substr(8)));
            if (!parsed.has_value())
            {
                std::cout << "Error: --color requires \"auto\", \"always\", or \"never\".\n";
                return ExitCode_InvalidArguments;
            }

            options.ColorMode = *parsed;
        }
        else if (arg == "--color-pass" || arg == "--color-fail" || arg == "--color-skip")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: " << arg << " requires a color name argument.\n";
                return ExitCode_InvalidArguments;
            }

            std::optional<TConsoleColor> const parsed = TConsole::ParseColorName(argv[++i]);
            if (!parsed.has_value())
            {
                std::cout << "Error: " << arg << " was given an unrecognized color name.\n";
                return ExitCode_InvalidArguments;
            }

            if (arg == "--color-pass")
                options.ColorPass = parsed;
            else if (arg == "--color-fail")
                options.ColorFail = parsed;
            else
                options.ColorSkip = parsed;
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
                return ExitCode_InvalidArguments;
            }

            if (flagName == "--color-pass")
                options.ColorPass = parsed;
            else if (flagName == "--color-fail")
                options.ColorFail = parsed;
            else
                options.ColorSkip = parsed;
        }
        else if (arg == "--report-junit")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --report-junit requires a file path argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.JunitReportPath = argv[++i];
        }
        else if (arg.rfind("--report-junit=", 0) == 0)
        {
            options.JunitReportPath = arg.substr(15);
        }
        else if (arg == "--project-name")
        {
            if (i + 1 >= argc)
            {
                std::cout << "Error: --project-name requires a name argument.\n";
                return ExitCode_InvalidArguments;
            }

            options.ProjectName = argv[++i];
        }
        else if (arg.rfind("--project-name=", 0) == 0)
        {
            options.ProjectName = arg.substr(15);
        }
        else if (arg == "--pause")
        {
            options.PauseOnExit = true;
        }
        else
        {
            std::cout << "Error: unrecognized option \"" << arg << "\".\n\n";
            PrintUsage();
            return ExitCode_InvalidArguments;
        }
    }

    if (options.HasPartitionIndex != options.HasPartitionCount)
    {
        std::cout << "Error: --partition-index and --partition-count must be given together.\n";
        return ExitCode_InvalidArguments;
    }

    if (options.HasPartitionCount)
    {
        if (options.PartitionCount == 0)
        {
            std::cout << "Error: --partition-count must be at least 1.\n";
            return ExitCode_InvalidArguments;
        }

        if (options.PartitionIndex == 0 || options.PartitionIndex > options.PartitionCount)
        {
            std::cout << "Error: --partition-index must be between 1 and --partition-count ("
            << options.PartitionCount << ").\n";
            return ExitCode_InvalidArguments;
        }
    }

    return std::nullopt;
}
//---------------------------------------------------------------------------
std::optional<TColorMode> TCLIParser::ParseColorMode(std::string const& text)
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
std::optional<unsigned int> TCLIParser::ParseUnsignedInt(std::string const& text)
{
    if (text.empty() || text[0] == '-')
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

} // namespace ASWUnitTests
