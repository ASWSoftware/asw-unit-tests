/* **************************************************************************
Main.cpp
Author: Anthony S. West - ASW Software

Console app that runs AswUnitTests framework.

Copyright 2025-2026 ASW Software

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
#include <memory>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_JUnitReport.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#if defined(_WIN32)
#  if !defined(NOMINMAX)
#    define NOMINMAX
#  endif
#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <tchar.h>
#endif
//---------------------------------------------------------------------------
using namespace ASWUnitTests;
//---------------------------------------------------------------------------

#if defined(__MINGW32__)
// MinGW-w64's C runtime can expand wildcards in the command line before main() runs (WinLibs' builds do by default),
// turning a --filter pattern such as "*String*" into the names of matching files in the current directory, however
// it was quoted. Defining _dowildcard as 0 here takes the place of the runtime library's own definition and turns
// that off, so arguments arrive as given, as they do with MSVC and a quoted argument in a POSIX shell. RAD Studio's
// Win64x compiler (bcc64x) also uses the MinGW-w64 runtime, so it compiles this too, though its runtime already
// leaves wildcards alone; its Win32 compiler (bcc32c) doesn't need it.
extern "C"
{
int _dowildcard = 0;
}
#endif

namespace
{

//---------------------------------------------------------------------------

void ApplyColorOptions(TCLIOptions const& options);
void PauseIfRequested(bool pauseOnExit);
#if defined(_WIN32)
std::vector<std::string> ProgramArguments();
#endif
int RunTests(TTestHandler& tester, TCLIOptions const& options, TestFilter const& filter, std::string const& filterDescription);

//---------------------------------------------------------------------------
void ApplyColorOptions(TCLIOptions const& options)
{
    TConsole::SetColorMode(options.ColorMode);

    if (options.ColorPass.has_value())
        TConsole::SetColor(TLogKind::Pass, *options.ColorPass);

    if (options.ColorFail.has_value())
        TConsole::SetColor(TLogKind::Fail, *options.ColorFail);

    if (options.ColorSkip.has_value())
        TConsole::SetColor(TLogKind::Skip, *options.ColorSkip);
}

//---------------------------------------------------------------------------
void PauseIfRequested(bool pauseOnExit)
{
    if (!pauseOnExit)
        return;

#if defined(_WIN32)
    system("pause");
#else
    std::cout << "Press enter to continue..." << std::endl;
    std::cin.get();
#endif
}

//---------------------------------------------------------------------------
#if defined(_WIN32)
/*
    ProgramArguments

    The arguments for TCLIParser::ParseArguments(), in UTF-8, from the UTF-16 command line. The C runtime's argv
    has them in the ANSI code page instead, which turns any character it lacks into a '?' or a lookalike.
*/
std::vector<std::string> ProgramArguments()
{
    std::vector<std::string> args = CommandLineArguments(GetCommandLineW());
    args.insert(args.begin(), "ASWUnitTests"); // In place of the program name, which ParseArguments() skips.
    return args;
}
#endif

//---------------------------------------------------------------------------
int RunTests(TTestHandler& tester, TCLIOptions const& options, TestFilter const& filter, std::string const& filterDescription)
{
    if (options.ListOnly)
    {
        // Applying the filter here lets the caller verify a pattern's matches before running it.
        tester.ListTests(filter, filterDescription);
        return ExitCode_Success;
    }

    TTestResults const testResults = tester.Run(filter, filterDescription, options.Shuffle, options.ShuffleSeed,
        options.TestTimeoutSeconds, options.CatchCrashes);
    int const returnCode = ExitCodeForResults(testResults);

    if (!options.JunitReportPath.empty())
    {
        std::vector<TJUnitTestCase> const junitTestCases = ToJUnitTestCases(testResults.CaseRecords);
        if (TJUnitReportWriter::Write(options.JunitReportPath, tester.GetProjectName(), junitTestCases))
            tester.Log("JUnit report written to: " + options.JunitReportPath);
        else
            tester.Log("Error: could not write JUnit report to: " + options.JunitReportPath);
    }

    return returnCode;
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

#if defined(_WIN32)
// _tmain is main, or wmain where TCHAR is wchar_t, as in a RAD Studio project whose "TCHAR maps to" is wchar_t. It
// takes no arguments: on Windows they come from ProgramArguments().
int _tmain()
#else
int main(int argc, char* argv[])
#endif
{
    // UTF-8 while the tests run, so non-ASCII test names and messages show correctly in the console.
    TConsoleUTF8Output consoleOutput;
    TCLIOptions options;

#if defined(_WIN32)
    std::optional<int> const earlyExitCode = TCLIParser::ParseArguments(ProgramArguments(), options);
#else
    std::optional<int> const earlyExitCode = TCLIParser::ParseArguments(argc, argv, options);
#endif
    if (earlyExitCode.has_value())
        return *earlyExitCode;

    ApplyColorOptions(options);

    int returnCode = ExitCode_Success;

    // Outside the try block, so that after a timeout or a crash the std::_Exit() calls below end the process with the
    // handler and its groups still alive: a timed-out test's abandoned worker thread may still be running one of their
    // tests.
    std::unique_ptr<TTestHandler> tester;

    try
    {
        tester = std::make_unique<TTestHandler>();

        tester->Initialize(options.ProjectName);

        std::string filterDescription;
        TestFilter const filter = TCLIParser::BuildTestFilter(*tester, options, filterDescription);

        returnCode = RunTests(*tester, options, filter, filterDescription);
    }
    catch (std::exception const& ex)
    {
        std::cout << "\nTerminating app. Unhandled exception: " << ex.what() << std::endl;
        returnCode = ExitCode_UnhandledException;
    }
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    catch (System::Sysutils::Exception& ex)
    {
        std::cout << "\nTerminating app. Unhandled exception: " << DescribeRTLException(ex) << std::endl;
        returnCode = ExitCode_UnhandledException;
    }
#endif
    catch (...)
    {
        std::cout << "\nTerminating app. Unhandled exception: Unknown" << std::endl;
        returnCode = ExitCode_UnhandledExceptionUnknown;
    }

    PauseIfRequested(options.PauseOnExit);

    // Now, since the std::_Exit() calls below skip its destructor.
    consoleOutput.Restore();

    if (returnCode == ExitCode_TestTimedOut)
    {
        // A test's worker thread may still be stuck (see TTestGroupBase::RunWithTimeout()) and was
        // deliberately abandoned rather than joined. Skip the handler's destructor and static
        // destructors/atexit handlers, which could otherwise race with whatever that thread
        // eventually does, and exit immediately.
        std::_Exit(returnCode);
    }

    if (returnCode == ExitCode_TestCrashed)
    {
        // The crash that got us here (a stack overflow, or a POSIX SIGSEGV that might have been one -
        // see TCrashGuard::Run()) may have left global/static state corrupted. Skip the handler's
        // destructor and static destructors/atexit handlers, which could misbehave against that
        // corrupted state, and exit immediately rather than trust them to run cleanly.
        std::_Exit(returnCode);
    }

    return returnCode;
}
