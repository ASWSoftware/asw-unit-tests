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
#include <exception>
#include <iostream>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
using namespace ASWUnitTests;
//---------------------------------------------------------------------------

namespace
{

std::optional<unsigned int> ParseUnsignedInt(std::string const& text);
void PrintUsage();

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
    "  --list              List all registered tests as \"GroupName.TestName\"\n"
    "                      and exit, without running anything.\n"
    "  --version           Print the framework version and exit.\n"
    "  --help              Show this message and exit.\n";
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
        else
        {
            std::cout << "Error: unrecognized option \"" << arg << "\".\n\n";
            PrintUsage();
            return 4;
        }
    }

    int returnCode = 0; // no error

    try
    {
        TTestHandler tester;

        tester.Initialize();

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

#if !defined(NDEBUG)
#  if defined(_WIN32)
    system("pause");
#  else
    std::cout << "Press enter to continue..." << std::endl;
    std::cin.get();
#  endif
#endif

    return returnCode;
}
