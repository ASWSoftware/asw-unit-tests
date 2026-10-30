/* **************************************************************************
ASWUnitTests_GUI_View.h
Author: Anthony S. West - ASW Software

The text a GUI runner's window shows, and how it classifies what it shows,
independent of the controls that show it.

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
#ifndef ASWUnitTests_GUI_ViewH
#define ASWUnitTests_GUI_ViewH
//---------------------------------------------------------------------------
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUILogLineKind
//
// How a GUI runner's log shows one line of the framework's log output; see
// ClassifyGUILogLine().
/////////////////////////////////////////////////////////////////////////////
enum class TGUILogLineKind
{
    Normal,
    Failure,
    Skip,
};


/////////////////////////////////////////////////////////////////////////////
// TGUIRunSummary
//
// What a finished run means for a GUI runner; see SummarizeGUIRun().
/////////////////////////////////////////////////////////////////////////////
struct TGUIRunSummary
{
    // Set when a test timed out or crashed, after which no further run can start (a timed-out test's abandoned
    // thread may still be running, and a crash may have left the tests' state corrupted).
    bool Aborted = false;
    // The exit code the console runner would return for this run.
    int ExitCode = 0;
    // The status bar's message, e.g. "Done.".
    std::string StatusMessage;
};

//---------------------------------------------------------------------------

// The text a GUI runner's window shows, shared so every GUI runner shows the same. Contains no UI code, so it's
// unit tested from the console runners (see gui/tests/Test_ASWUnitTests_GUI_View.cpp).

// How the log shows one line of the framework's log output: its failure lines (a failed test or check, an
// unhandled exception, a fatal SetUp_Test()/TearDown_Test() error, a group whose constructor or SetUp_Group()
// threw, or a test that timed out or crashed) and its skip lines stand out.
TGUILogLineKind ClassifyGUILogLine(std::string const& line);
// A number of seconds as the status bar shows it, e.g. "1.35 s".
std::string FormatGUISeconds(double seconds);
// The About dialog's text: the framework's version, 'runnerName' (e.g. "VCL GUI runner"), the app's bits, the
// project's URL, and the copyright and license.
std::string GUIAboutText(std::string const& runnerName);
// The window's caption: "ASWUnitTests - Framework version: <version>", plus " - <project name>" when
// --project-name gave one other than the default.
std::string GUICaption(std::string const& projectName);
// The notes a GUI runner logs as it starts, about command line options it handles differently from the console
// runner: one for each ignored option, one for --test-timeout-seconds, which runs each test on a worker thread
// where a test can't create 'uiObjects' (e.g. "VCL forms or controls"), and one for --layout-ignore.
std::vector<std::string> GUICommandLineNotes(TGUIOptions const& options, std::string const& uiObjects);
// The file name Export JUnit Report suggests when --report-junit gave none: 'exeFileName' (without its folder)
// with "_JUnit.xml" in place of its extension, e.g. "ASWUnitTests_VCL_GUI_JUnit.xml".
std::string GUIDefaultJUnitReportFileName(std::string const& exeFileName);
// The filter box's filter: matches anywhere in a test's "Group.Test" full name, ignoring case, with the same '*'
// and '?' wildcards as --filter. Unlike --filter, it's never anchored to the whole name, so "Parse?rg" finds
// "...ParseArguments...", just as plain text would. Empty (no filter) when 'text' is empty or only spaces.
TestFilter GUIFilterBoxFilter(std::string const& text);
// The first line of 'text', e.g. for the Failures & Skips list's Detail column.
std::string GUIFirstLine(std::string const& text);
// 'text', e.g. a theme file's name, as a menu item's caption: each '&' doubled, so it shows rather than making
// the next letter the item's accelerator.
std::string GUIMenuItemText(std::string const& text);
// 'texts', a menu's items in order, as captions (see GUIMenuItemText()), each with an accelerator of its own: the
// first ASCII letter or digit, ignoring case, that's neither in 'takenLetters' (the menu's other items') nor an
// earlier text's, preferring one that starts a word. A text with none left gets none.
std::vector<std::string> GUIMenuItemTextsWithAccelerators(std::vector<std::string> const& texts,
    std::string const& takenLetters);
// The status bar's panels' text, in order: the checked and shown tests, the run's progress, the passed, failed
// and skipped counts, the run's elapsed time (empty if no run has started), the shuffle setting ('shuffleText',
// from TGUIShuffle::StatusText()), and 'message'.
std::vector<std::string> GUIStatusPanels(TGUITestList const& testList, size_t runFinishedCount, size_t runTotalCount,
    std::optional<double> elapsedSeconds, std::string const& shuffleText, std::string const& message);
// The status bar's panel widths at 96 DPI, in order, one per panel GUIStatusPanels() fills, as the VCL GUI
// runner's form has them. The last panel, the status message's, takes the rest of the bar in both runners.
std::vector<int> GUIStatusPanelWidths();
// The result a test is given when an unhandled exception, described by 'description', ended the run while it
// was running.
TTestCaseRecord GUIUnhandledExceptionRecord(TTestId const& test, std::string const& description);
// The status bar's message when an unhandled exception, described by 'description', ended the run.
std::string GUIUnhandledExceptionStatus(std::string const& description);
// What a finished run's results mean for the GUI: whether it was aborted, its exit code, and the status bar's
// message.
TGUIRunSummary SummarizeGUIRun(TTestResults const& results);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_ViewH
