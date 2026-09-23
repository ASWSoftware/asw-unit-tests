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
#include <string>
#include <string_view>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
using namespace ASWUnitTests;
//---------------------------------------------------------------------------

namespace
{

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
    "  --list              List all registered tests as \"GroupName.TestName\"\n"
    "                      and exit, without running anything.\n"
    "  --version           Print the framework version and exit.\n"
    "  --help              Show this message and exit.\n";
}

} // namespace

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    bool listOnly = false;
    bool hasFilter = false;
    bool filterIgnoreCase = false;
    std::string filterPattern;

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
            TTestResults testResults = tester.Run(filter, filterDescription);
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
