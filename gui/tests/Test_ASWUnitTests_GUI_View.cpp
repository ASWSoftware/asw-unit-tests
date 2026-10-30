/* **************************************************************************
Test_ASWUnitTests_GUI_View.cpp
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
#include "Test_ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_View
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_View::TTest_ASWUnitTests_GUI_View()
    : inherited("ASWUnitTests_GUI_View_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_ClassifyGUILogLine_FailuresAndSkipsStandOut,
        "ClassifyGUILogLine_FailuresAndSkipsStandOut");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_FormatGUISeconds_TwoDecimals, "FormatGUISeconds_TwoDecimals");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIAboutText_NamesTheRunnerAndVersion,
        "GUIAboutText_NamesTheRunnerAndVersion");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUICaption_AddsAProjectNameOtherThanTheDefault,
        "GUICaption_AddsAProjectNameOtherThanTheDefault");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUICommandLineNotes_OnePerOptionHandledDifferently,
        "GUICommandLineNotes_OnePerOptionHandledDifferently");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIDefaultJUnitReportFileName_ReplacesTheExtension,
        "GUIDefaultJUnitReportFileName_ReplacesTheExtension");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIFilterBoxFilter_MatchesAnywhereIgnoringCase,
        "GUIFilterBoxFilter_MatchesAnywhereIgnoringCase");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIFirstLine_StopsAtTheFirstLineBreak,
        "GUIFirstLine_StopsAtTheFirstLineBreak");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIMenuItemText_DoublesEachAmpersand,
        "GUIMenuItemText_DoublesEachAmpersand");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIStatusPanels_ShowCountsAndMessage,
        "GUIStatusPanels_ShowCountsAndMessage");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIStatusPanelWidths_OnePerPanel, "GUIStatusPanelWidths_OnePerPanel");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_GUIUnhandledExceptionRecord_FailsTheRunningTest,
        "GUIUnhandledExceptionRecord_FailsTheRunningTest");
    RegisterTest(&TTest_ASWUnitTests_GUI_View::Test_SummarizeGUIRun_DescribesHowTheRunEnded,
        "SummarizeGUIRun_DescribesHowTheRunEnded");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_View::~TTest_ASWUnitTests_GUI_View()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_ClassifyGUILogLine_FailuresAndSkipsStandOut()
{
    // Arrange: each kind of failure and skip line the framework logs.
    std::vector<std::string> const failures{
        "  ***Test failed: Run_X",
        "  **Check failed for: \"Test_X\" (12)",
        "Finished test: \"G.T\" - failed (0.1 ms)",
        "Unhandled exception: boom",
        "!!FATAL ERROR!! in SetUp_Test()",
        "!!ERROR!! SetUp_Group() threw",
        "!!TIMEOUT!! G.T",
        "!!CRASH!! G.T"
    };
    std::vector<std::string> const skips{ "  ***Test skipped: Run_X", "Finished test: \"G.T\" - skipped (0.1 ms)" };

    // Act and Assert
    for (std::string const& line : failures)
        CheckTrue(ClassifyGUILogLine(line) == TGUILogLineKind::Failure, __func__, __LINE__, "a failure: " + line);

    for (std::string const& line : skips)
        CheckTrue(ClassifyGUILogLine(line) == TGUILogLineKind::Skip, __func__, __LINE__, "a skip: " + line);

    CheckTrue(ClassifyGUILogLine("Finished test: \"G.T\" - passed (0.1 ms)") == TGUILogLineKind::Normal, __func__,
        __LINE__, "a passing test's line");
    CheckTrue(ClassifyGUILogLine(std::string()) == TGUILogLineKind::Normal, __func__, __LINE__, "an empty line");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_FormatGUISeconds_TwoDecimals()
{
    // Act and Assert
    CheckEquals(std::string("1.35 s"), FormatGUISeconds(1.346), __func__, __LINE__, "rounded to two decimals");
    CheckEquals(std::string("0.00 s"), FormatGUISeconds(0.0), __func__, __LINE__, "zero");
    CheckEquals(std::string("125.50 s"), FormatGUISeconds(125.5), __func__, __LINE__, "not split into minutes");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIAboutText_NamesTheRunnerAndVersion()
{
    // Act
    std::string const text = GUIAboutText("Example GUI runner");

    // Assert
    std::string const bits = (sizeof(void*) == 8) ? "64" : "32";
    CheckEquals(TTestHandler::GetVersionFullStr() + "\n"
        "Example GUI runner\n"
        "\n"
        "App bits: " + bits + "\n"
        "\n"
        "https://github.com/ASWSoftware/asw-unit-tests\n"
        "\n"
        "Copyright 2026 ASW Software\n"
        "Licensed under the Apache License, Version 2.0.\n", text, __func__, __LINE__,
        "the VCL GUI runner's About text, with the runner's own name");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUICaption_AddsAProjectNameOtherThanTheDefault()
{
    // Arrange
    std::string const plain = "ASWUnitTests - Framework version: " + TTestHandler::GetVersionStr();

    // Act and Assert
    CheckEquals(plain, GUICaption(TCLIOptions().ProjectName), __func__, __LINE__,
        "the default project name isn't shown");
    CheckEquals(plain + " - My Project", GUICaption("My Project"), __func__, __LINE__, "another one is");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUICommandLineNotes_OnePerOptionHandledDifferently()
{
    // Arrange
    TGUIOptions options;
    options.IgnoredOptions = { "--list", "--pause" };
    options.CLI.TestTimeoutSeconds = 30;
    options.LayoutIgnore = true;

    // Act
    std::vector<std::string> const notes = GUICommandLineNotes(options, "widgets");

    // Assert
    AssertEquals(size_t(4), notes.size(), __func__, __LINE__, "one note each");
    CheckEquals(std::string("Note: --list has no effect in the GUI, so it was ignored."), notes[0], __func__, __LINE__,
        "the first ignored option");
    CheckEquals(std::string("Note: --pause has no effect in the GUI, so it was ignored."), notes[1], __func__,
        __LINE__, "the second");
    CheckEquals(std::string("Note: --test-timeout-seconds runs each test on a worker thread, so a test that creates "
        "widgets won't work with it."), notes[2], __func__, __LINE__, "the timeout, naming the GUI's own UI objects");
    CheckEquals(std::string("Note: --layout-ignore: the window layout isn't loaded or saved, so it opens with the "
        "defaults."), notes[3], __func__, __LINE__, "--layout-ignore");
    CheckTrue(GUICommandLineNotes(TGUIOptions(), "widgets").empty(), __func__, __LINE__,
        "no notes without such options");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIDefaultJUnitReportFileName_ReplacesTheExtension()
{
    // Act and Assert
    CheckEquals(std::string("ASWUnitTests_VCL_GUI_JUnit.xml"), GUIDefaultJUnitReportFileName("ASWUnitTests_VCL_GUI.exe"),
        __func__, __LINE__, "the extension is replaced");
    CheckEquals(std::string("my.app_JUnit.xml"), GUIDefaultJUnitReportFileName("my.app.exe"), __func__, __LINE__,
        "only the last one");
    CheckEquals(std::string("runner_JUnit.xml"), GUIDefaultJUnitReportFileName("runner"), __func__, __LINE__,
        "a name without one gets the suffix added");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIFilterBoxFilter_MatchesAnywhereIgnoringCase()
{
    // Arrange
    TestFilter const filter = GUIFilterBoxFilter("  parse?rg\t");

    // Act and Assert
    AssertTrue(static_cast<bool>(filter), __func__, __LINE__, "text gives a filter");
    CheckTrue(filter("Framework.TTest_TCLIParser.ParseArguments_Help"), __func__, __LINE__,
        "matches anywhere in the name, ignoring case, with a wildcard, once trimmed");
    CheckFalse(filter("Framework.TTest_TConsole.Color"), __func__, __LINE__, "and not a name without it");
    CheckFalse(static_cast<bool>(GUIFilterBoxFilter(std::string())), __func__, __LINE__, "no text, no filter");
    CheckFalse(static_cast<bool>(GUIFilterBoxFilter(" \t ")), __func__, __LINE__, "nor only spaces");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIFirstLine_StopsAtTheFirstLineBreak()
{
    // Act and Assert
    CheckEquals(std::string("first"), GUIFirstLine("first\nsecond\nthird"), __func__, __LINE__, "the first line");
    CheckEquals(std::string("only"), GUIFirstLine("only"), __func__, __LINE__, "a single line");
    CheckEmpty(GUIFirstLine("\nsecond"), __func__, __LINE__, "an empty first line");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIMenuItemText_DoublesEachAmpersand()
{
    // Act and Assert
    CheckEquals(std::string("Ocean"), GUIMenuItemText("Ocean"), __func__, __LINE__, "plain text as it is");
    CheckEquals(std::string("Salt && Pepper"), GUIMenuItemText("Salt & Pepper"), __func__, __LINE__, "one '&'");
    CheckEquals(std::string("&&&&a&&"), GUIMenuItemText("&&a&"), __func__, __LINE__, "each of several");
    CheckEquals(std::string("\xC3\x89" "cume && co"), GUIMenuItemText("\xC3\x89" "cume & co"), __func__, __LINE__,
        "UTF-8 left as it is");
    CheckEmpty(GUIMenuItemText(std::string()), __func__, __LINE__, "empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIStatusPanels_ShowCountsAndMessage()
{
    // Arrange: three tests, one hidden by the filter box, one unchecked, one passed and one failed.
    TGUITestList list;
    list.Load({ { "Alpha", "One" }, { "Alpha", "Two" }, { "Beta", "Three" } });
    list.SetChecked(1, false);
    list.SetStatus(0, TGUITestStatus::Passed);
    list.SetStatus(1, TGUITestStatus::Failed);
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Beta.Three";
        });

    // Act
    std::vector<std::string> const panels = GUIStatusPanels(list, 2, 3, 1.346, "Shuffle: off", "Running...");
    std::vector<std::string> const beforeRun = GUIStatusPanels(list, 0, 0, std::nullopt, "Shuffle: off", "Ready.");

    // Assert
    AssertEquals(size_t(8), panels.size(), __func__, __LINE__, "eight panels");
    CheckEquals(std::string("Checked: 1 / 2 shown"), panels[0], __func__, __LINE__,
        "checked of the shown tests, noting that some are hidden");
    CheckEquals(std::string("Run: 2 / 3"), panels[1], __func__, __LINE__, "the run's progress");
    CheckEquals(std::string("Passed: 1"), panels[2], __func__, __LINE__, "passed");
    CheckEquals(std::string("Failed: 1"), panels[3], __func__, __LINE__, "failed");
    CheckEquals(std::string("Skipped: 0"), panels[4], __func__, __LINE__, "skipped");
    CheckEquals(std::string("1.35 s"), panels[5], __func__, __LINE__, "elapsed time");
    CheckEquals(std::string("Shuffle: off"), panels[6], __func__, __LINE__, "the shuffle setting");
    CheckEquals(std::string("Running..."), panels[7], __func__, __LINE__, "the message");
    CheckEmpty(beforeRun[5], __func__, __LINE__, "no elapsed time before a run");

    list.SetVisibleFilter(TestFilter());
    CheckEquals(std::string("Checked: 2 / 3"), GUIStatusPanels(list, 0, 0, std::nullopt, "", "")[0], __func__,
        __LINE__, "no \"shown\" while every test is");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIStatusPanelWidths_OnePerPanel()
{
    // Arrange
    TGUITestList list;
    list.Load(std::vector<TTestId>());

    // Act and Assert
    CheckEquals(GUIStatusPanels(list, 0, 0, std::nullopt, "", "").size(), GUIStatusPanelWidths().size(), __func__,
        __LINE__, "one width per panel");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_GUIUnhandledExceptionRecord_FailsTheRunningTest()
{
    // Act
    TTestCaseRecord const record = GUIUnhandledExceptionRecord(TTestId{ "Group", "Test", "Suite" }, "boom");

    // Assert
    CheckEquals(std::string("Group"), record.GroupName, __func__, __LINE__, "the test's group");
    CheckEquals(std::string("Test"), record.TestName, __func__, __LINE__, "its name");
    CheckEquals(std::string("Suite"), record.SuitePath, __func__, __LINE__, "its suite");
    CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, "failed");
    CheckEquals(std::string("Unhandled exception: boom"), record.Message, __func__, __LINE__, "with the description");
    CheckEquals(std::string("The run was ended by an unhandled exception: boom"), GUIUnhandledExceptionStatus("boom"),
        __func__, __LINE__, "and the status bar says why the run ended");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_View::Test_SummarizeGUIRun_DescribesHowTheRunEnded()
{
    // Arrange
    TTestResults done;
    done.SuccessCount = 2;
    TTestResults stopped;
    stopped.Stopped = true;
    TTestResults timedOut;
    timedOut.TimedOut = true;
    TTestResults crashed;
    crashed.Crashed = true;

    // Act
    TGUIRunSummary const doneSummary = SummarizeGUIRun(done);
    TGUIRunSummary const stoppedSummary = SummarizeGUIRun(stopped);
    TGUIRunSummary const timedOutSummary = SummarizeGUIRun(timedOut);
    TGUIRunSummary const crashedSummary = SummarizeGUIRun(crashed);

    // Assert
    CheckEquals(std::string("Done."), doneSummary.StatusMessage, __func__, __LINE__, "a run that finished");
    CheckFalse(doneSummary.Aborted, __func__, __LINE__, "isn't aborted");
    CheckEquals(ExitCodeForResults(done), doneSummary.ExitCode, __func__, __LINE__, "and has the console's exit code");
    CheckEquals(std::string("Stopped before every test ran."), stoppedSummary.StatusMessage, __func__, __LINE__,
        "a stopped run");
    CheckFalse(stoppedSummary.Aborted, __func__, __LINE__, "can be run again");
    CheckEquals(ExitCodeForResults(stopped), stoppedSummary.ExitCode, __func__, __LINE__, "its exit code");
    CheckEquals(std::string("A test timed out, so the rest of the run was abandoned. Restart to run tests again."),
        timedOutSummary.StatusMessage, __func__, __LINE__, "a timed-out run");
    CheckTrue(timedOutSummary.Aborted, __func__, __LINE__, "is aborted");
    CheckEquals(std::string("A test crashed, so the rest of the run was abandoned. Restart to run tests again."),
        crashedSummary.StatusMessage, __func__, __LINE__, "a crashed run");
    CheckTrue(crashedSummary.Aborted, __func__, __LINE__, "is aborted");
    CheckEquals(ExitCodeForResults(crashed), crashedSummary.ExitCode, __func__, __LINE__, "its exit code");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_View)
