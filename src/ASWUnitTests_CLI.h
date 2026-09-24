/* **************************************************************************
ASWUnitTests_CLI.h
Author: Anthony S. West - ASW Software

Command-line option parsing and filter/partition composition for main.cpp,
split out into its own unit so it can be linked from a test module without
pulling in main() itself.

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

#ifndef ASWUnitTests_CLIH
#define ASWUnitTests_CLIH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// Exit codes
constexpr int ExitCode_Success = 0;
constexpr int ExitCode_TestsFailed = 1;
constexpr int ExitCode_UnhandledException = 2;
constexpr int ExitCode_UnhandledExceptionUnknown = 3;
constexpr int ExitCode_InvalidArguments = 4;


/////////////////////////////////////////////////////////////////////////////
// TCLIOptions
//
// Holds every option --help documents, parsed by TCLIParser::ParseArguments()
// and consumed by TCLIParser::BuildTestFilter() and main.cpp's RunTests().
/////////////////////////////////////////////////////////////////////////////
struct TCLIOptions
{
    bool ListOnly = false;
    bool HasFilter = false;
    bool FilterIgnoreCase = false;
    std::string FilterPattern;
    bool Shuffle = false;
    std::optional<unsigned int> ShuffleSeed;
    bool HasPartitionIndex = false;
    bool HasPartitionCount = false;
    unsigned int PartitionIndex = 0;
    unsigned int PartitionCount = 0;
    TColorMode ColorMode = TColorMode::Auto;
    std::optional<TConsoleColor> ColorPass;
    std::optional<TConsoleColor> ColorFail;
    std::optional<TConsoleColor> ColorSkip;
    std::string JunitReportPath;
    std::string ProjectName = "ASWUnitTests";
    bool PauseOnExit = false;
};


/////////////////////////////////////////////////////////////////////////////
// TCLIParser
//
// Static helpers for turning argv into a TCLIOptions.
/////////////////////////////////////////////////////////////////////////////
class TCLIParser
{
public:
    // Builds the combined --filter/--partition-index/--partition-count predicate and its
    // human-readable description for the "Filter: ..."/"No filter: ..." console line. Calls
    // tester.GetAllTestFullNames() only when partitioning is active.
    static TestFilter BuildTestFilter(TTestHandler& tester, TCLIOptions const& options,
        std::string& filterDescription);

    // Returns the exit code the program should terminate with immediately (--version/--help, or a
    // parse/validation error), or std::nullopt if 'options' was parsed successfully and the program
    // should proceed to run tests.
    static std::optional<int> ParseArguments(int argc, char* argv[], TCLIOptions& options);

    // Parses "auto"/"always"/"never" as accepted by --color; returns std::nullopt if 'text' isn't
    // recognized.
    static std::optional<TColorMode> ParseColorMode(std::string const& text);

    // Parses 'text' as a non-negative integer in full (no partial matches, no sign, no whitespace);
    // returns std::nullopt if 'text' isn't a valid unsigned integer or overflows unsigned int.
    static std::optional<unsigned int> ParseUnsignedInt(std::string const& text);
};

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_CLIH
