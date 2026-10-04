/* **************************************************************************
Test_ASWUnitTests_GUI_CommandLine.cpp
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
#include "Test_ASWUnitTests_GUI_CommandLine.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

bool Contains(std::vector<std::string> const& list, std::string const& item);

//---------------------------------------------------------------------------

bool Contains(std::vector<std::string> const& list, std::string const& item)
{
    return std::find(list.begin(), list.end(), item) != list.end();
}

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_CommandLine
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_CommandLine::TTest_ASWUnitTests_GUI_CommandLine()
    : inherited("ASWUnitTests_GUI_CommandLine_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_HelpText_MatchesHelpOption, "HelpText_MatchesHelpOption");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_ConsoleOnlyOptionsAreReportedAsIgnored,
        "Parse_ConsoleOnlyOptionsAreReportedAsIgnored");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_ExitWithoutRunIsInvalid, "Parse_ExitWithoutRunIsInvalid");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_HelpIncludesGUIOptions, "Parse_HelpIncludesGUIOptions");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_LayoutOptionsAreGUIOnlyOptions,
        "Parse_LayoutOptionsAreGUIOnlyOptions");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_NoArgumentsUsesDefaults, "Parse_NoArgumentsUsesDefaults");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_PassesConsoleOptionsThrough, "Parse_PassesConsoleOptionsThrough");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_RunAndExitAreGUIOnlyOptions, "Parse_RunAndExitAreGUIOnlyOptions");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_UnrecognizedOptionIsInvalid, "Parse_UnrecognizedOptionIsInvalid");
    RegisterTest(&TTest_ASWUnitTests_GUI_CommandLine::Test_ProcessArguments_MatchesParamCount, "ProcessArguments_MatchesParamCount");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_CommandLine::~TTest_ASWUnitTests_GUI_CommandLine()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_HelpText_MatchesHelpOption()
{
    // Arrange
    TGUIOptions options;
    std::string const helpOutput = TGUICommandLine::Parse({ "--help" }, options).Message;
    std::string consoleOutput;
    std::string text;

    // Act
    {
        TStdOutRedirect redirect;
        text = TGUICommandLine::HelpText();
        consoleOutput = redirect.Str();
    }

    // Assert
    CheckEquals(helpOutput, text, __func__, __LINE__, "the same text --help shows");
    CheckTrue(text.find("--run ") != std::string::npos, __func__, __LINE__, "including the GUI-only options");
    CheckTrue(consoleOutput.empty(), __func__, __LINE__, "nothing was written to std::cout");
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_ConsoleOnlyOptionsAreReportedAsIgnored()
{
    // Arrange
    TGUIOptions options;

    // Act
    TGUICommandLineResult const result = TGUICommandLine::Parse(
        { "--list", "--pause", "--no-color", "--color-fail", "red" }, options);

    // Assert
    CheckFalse(result.EarlyExitCode.has_value(), __func__, __LINE__, "console-only options still open the GUI");
    CheckTrue(Contains(options.IgnoredOptions, "--list"), __func__, __LINE__, "--list is reported as ignored");
    CheckTrue(Contains(options.IgnoredOptions, "--pause"), __func__, __LINE__, "--pause is reported as ignored");
    CheckTrue(Contains(options.IgnoredOptions, "--color/--no-color"), __func__, __LINE__,
        "--no-color is reported as ignored");
    CheckTrue(Contains(options.IgnoredOptions, "--color-pass/--color-fail/--color-skip"), __func__, __LINE__,
        "--color-fail is reported as ignored");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_ExitWithoutRunIsInvalid()
{
    // Arrange
    TGUIOptions options;

    // Act
    TGUICommandLineResult const result = TGUICommandLine::Parse({ "--exit" }, options);

    // Assert
    AssertTrue(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI doesn't open");
    CheckEquals(ExitCode_InvalidArguments, *result.EarlyExitCode, __func__, __LINE__,
        "with the invalid-arguments exit code");
    CheckTrue(result.Message.find("--exit requires --run") != std::string::npos, __func__, __LINE__,
        "and says why");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_HelpIncludesGUIOptions()
{
    // Arrange
    TGUIOptions options;
    std::string consoleOutput;
    TGUICommandLineResult result;

    // Act
    {
        TStdOutRedirect redirect;
        result = TGUICommandLine::Parse({ "--help" }, options);
        consoleOutput = redirect.Str();
    }

    // Assert
    AssertTrue(result.EarlyExitCode.has_value(), __func__, __LINE__, "--help doesn't open the GUI");
    CheckEquals(ExitCode_Success, *result.EarlyExitCode, __func__, __LINE__, "and isn't an error");
    CheckTrue(result.Message.find("--filter <pattern>") != std::string::npos, __func__, __LINE__,
        "the console runner's options are listed");
    CheckTrue(result.Message.find("--run ") != std::string::npos, __func__, __LINE__,
        "and so are the GUI-only ones");
    CheckTrue(result.Message.find("--layout-ignore ") != std::string::npos, __func__, __LINE__,
        "including --layout-ignore");
    CheckTrue(result.Message.find("--layout-reset ") != std::string::npos, __func__, __LINE__,
        "and --layout-reset");
    CheckTrue(consoleOutput.empty(), __func__, __LINE__, "the parser's output was captured, not written to std::cout");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_LayoutOptionsAreGUIOnlyOptions()
{
    // Arrange
    TGUIOptions options;

    // Act
    // The console runner's parser would reject both as unrecognized, so this also proves they never reach it.
    TGUICommandLineResult const result = TGUICommandLine::Parse(
        { "--layout-ignore", "--filter", "*String*", "--layout-reset" }, options);

    // Assert
    CheckFalse(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI opens");
    CheckTrue(options.LayoutIgnore, __func__, __LINE__, "--layout-ignore is recognized");
    CheckTrue(options.LayoutReset, __func__, __LINE__, "--layout-reset is recognized");
    CheckEquals(std::string("*String*"), options.CLI.FilterPattern, __func__, __LINE__,
        "the console options between them are still parsed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_NoArgumentsUsesDefaults()
{
    // Arrange
    TGUIOptions options;

    // Act
    TGUICommandLineResult const result = TGUICommandLine::Parse({}, options);

    // Assert
    CheckFalse(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI opens");
    CheckTrue(result.Message.empty(), __func__, __LINE__, "with nothing to report");
    CheckFalse(options.RunOnStart, __func__, __LINE__, "without running anything");
    CheckFalse(options.ExitAfterRun, __func__, __LINE__, "or exiting");
    CheckFalse(options.LayoutIgnore, __func__, __LINE__, "the saved layout is loaded and saved");
    CheckFalse(options.LayoutReset, __func__, __LINE__, "and not reset");
    CheckTrue(options.IgnoredOptions.empty(), __func__, __LINE__, "no options were ignored");
    CheckEquals(std::string("ASWUnitTests"), options.CLI.ProjectName, __func__, __LINE__,
        "the console runner's defaults apply");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_PassesConsoleOptionsThrough()
{
    // Arrange
    TGUIOptions options;

    // Act
    TGUICommandLineResult const result = TGUICommandLine::Parse(
        { "--filter", "*String*", "--project-name", "MyProject", "--test-timeout-seconds", "30", "--catch-crashes" },
        options);

    // Assert
    CheckFalse(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI opens");
    CheckTrue(options.CLI.HasFilter, __func__, __LINE__, "--filter is parsed by the console runner's parser");
    CheckEquals(std::string("*String*"), options.CLI.FilterPattern, __func__, __LINE__, "with its pattern");
    CheckEquals(std::string("MyProject"), options.CLI.ProjectName, __func__, __LINE__, "--project-name too");
    CheckTrue(options.CLI.TestTimeoutSeconds.has_value() && *options.CLI.TestTimeoutSeconds == 30u, __func__,
        __LINE__, "--test-timeout-seconds too");
    CheckTrue(options.CLI.CatchCrashes, __func__, __LINE__, "--catch-crashes too");
    CheckTrue(options.IgnoredOptions.empty(), __func__, __LINE__, "none of those is ignored");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_RunAndExitAreGUIOnlyOptions()
{
    // Arrange
    TGUIOptions options;

    // Act
    // The console runner's parser would reject both as unrecognized, so this also proves they never reach it.
    TGUICommandLineResult const result = TGUICommandLine::Parse({ "--run", "--filter", "*String*", "--exit" }, options);

    // Assert
    CheckFalse(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI opens");
    CheckTrue(options.RunOnStart, __func__, __LINE__, "--run is recognized");
    CheckTrue(options.ExitAfterRun, __func__, __LINE__, "--exit is recognized");
    CheckEquals(std::string("*String*"), options.CLI.FilterPattern, __func__, __LINE__,
        "the console options between them are still parsed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_Parse_UnrecognizedOptionIsInvalid()
{
    // Arrange
    TGUIOptions options;

    // Act
    TGUICommandLineResult const result = TGUICommandLine::Parse({ "--no-such-option" }, options);

    // Assert
    AssertTrue(result.EarlyExitCode.has_value(), __func__, __LINE__, "the GUI doesn't open");
    CheckEquals(ExitCode_InvalidArguments, *result.EarlyExitCode, __func__, __LINE__,
        "with the invalid-arguments exit code");
    CheckTrue(result.Message.find("unrecognized option \"--no-such-option\"") != std::string::npos, __func__,
        __LINE__, "the console runner's own error message is passed on");
    CheckTrue(result.Message.find("--run ") != std::string::npos, __func__, __LINE__,
        "and the usage text that follows it lists the GUI-only options too");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_CommandLine::Test_ProcessArguments_MatchesParamCount()
{
    // Arrange
    // The runner's own arguments vary with how it was started, so only their count can be checked here.
    int const expectedCount = System::ParamCount();

    // Act
    std::vector<std::string> const args = TGUICommandLine::ProcessArguments();

    // Assert
    CheckEquals(static_cast<size_t>(expectedCount), args.size(), __func__, __LINE__,
        "one entry per argument, without the program name");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_CommandLine)
