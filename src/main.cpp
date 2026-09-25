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
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
using namespace ASWUnitTests;
//---------------------------------------------------------------------------

namespace
{

void ApplyColorOptions(TCLIOptions const& options);
void PauseIfRequested(bool pauseOnExit);
int RunTests(TTestHandler& tester, TCLIOptions const& options, TestFilter const& filter, std::string const& filterDescription);
std::vector<TJUnitTestCase> ToJUnitTestCases(std::vector<TTestCaseRecord> const& records);

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
    int const returnCode = testResults.TimedOut ? ExitCode_TestTimedOut :
            testResults.Crashed ? ExitCode_TestCrashed :
                (testResults.FailedCount > 0) ? ExitCode_TestsFailed : ExitCode_Success;

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
    TCLIOptions options;

    std::optional<int> const earlyExitCode = TCLIParser::ParseArguments(argc, argv, options);
    if (earlyExitCode.has_value())
        return *earlyExitCode;

    ApplyColorOptions(options);

    int returnCode = ExitCode_Success;

    try
    {
        TTestHandler tester;

        tester.Initialize(options.ProjectName);

        std::string filterDescription;
        TestFilter const filter = TCLIParser::BuildTestFilter(tester, options, filterDescription);

        returnCode = RunTests(tester, options, filter, filterDescription);
    }
    catch (std::exception const& ex)
    {
        std::cout << "\nTerminating app. Unhandled exception: " << ex.what() << std::endl;
        returnCode = ExitCode_UnhandledException;
    }
    catch (...)
    {
        std::cout << "\nTerminating app. Unhandled exception: Unknown" << std::endl;
        returnCode = ExitCode_UnhandledExceptionUnknown;
    }

    PauseIfRequested(options.PauseOnExit);

    if (returnCode == ExitCode_TestTimedOut)
    {
        // A test's worker thread may still be stuck (see TTestGroupBase::RunWithTimeout()) and was
        // deliberately abandoned rather than joined. Skip static destructors/atexit handlers, which
        // could otherwise race with whatever that thread eventually does, and exit immediately.
        std::_Exit(returnCode);
    }

    if (returnCode == ExitCode_TestCrashed)
    {
        // The crash that got us here (a stack overflow, or a POSIX SIGSEGV that might have been one -
        // see TCrashGuard::Run()) may have left global/static state corrupted. Skip static
        // destructors/atexit handlers, which could misbehave against that corrupted state, and exit
        // immediately rather than trust them to run cleanly.
        std::_Exit(returnCode);
    }

    return returnCode;
}
