/* **************************************************************************
ASWUnitTests_GUI_View.cpp
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
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include <iomanip>
#include <sstream>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

bool Contains(std::string const& text, char const* part);
std::string Trim(std::string const& text);

//---------------------------------------------------------------------------

bool Contains(std::string const& text, char const* part)
{
    return text.find(part) != std::string::npos;
}
//---------------------------------------------------------------------------
// 'text' without the spaces, tabs and other control characters at either end.
std::string Trim(std::string const& text)
{
    auto const isTrimmed = [](char c)
        {
            return static_cast<unsigned char>(c) <= ' ';
        };

    size_t start = 0;
    while (start < text.size() && isTrimmed(text[start]))
        ++start;

    size_t end = text.size();
    while (end > start && isTrimmed(text[end - 1]))
        --end;

    return text.substr(start, end - start);
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
TGUILogLineKind ClassifyGUILogLine(std::string const& line)
{
    if (Contains(line, "***Test failed") || Contains(line, "**Check failed") || Contains(line, "\" - failed (") ||
        Contains(line, "Unhandled exception") || Contains(line, "!!FATAL ERROR!!") || Contains(line, "!!ERROR!!") ||
        Contains(line, "!!TIMEOUT!!") || Contains(line, "!!CRASH!!"))
        return TGUILogLineKind::Failure;

    if (Contains(line, "***Test skipped") || Contains(line, "\" - skipped ("))
        return TGUILogLineKind::Skip;

    return TGUILogLineKind::Normal;
}
//---------------------------------------------------------------------------
std::string FormatGUISeconds(double seconds)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(2) << seconds << " s";
    return text.str();
}
//---------------------------------------------------------------------------
std::string GUIAboutText(std::string const& runnerName)
{
    return TTestHandler::GetVersionFullStr() + "\n" +
        runnerName + "\n"
        "\n"
        "App bits: " + std::to_string(sizeof(void*) * 8) + "\n"
        "\n"
        "https://github.com/ASWSoftware/asw-unit-tests\n"
        "\n"
        "Copyright 2026 ASW Software\n"
        "Licensed under the Apache License, Version 2.0.\n";
}
//---------------------------------------------------------------------------
std::string GUICaption(std::string const& projectName)
{
    std::string caption = "ASWUnitTests - Framework version: " + TTestHandler::GetVersionStr();

    if (projectName != TCLIOptions().ProjectName)
        caption += " - " + projectName;

    return caption;
}
//---------------------------------------------------------------------------
std::vector<std::string> GUICommandLineNotes(TGUIOptions const& options, std::string const& uiObjects)
{
    std::vector<std::string> notes;

    for (std::string const& ignored : options.IgnoredOptions)
        notes.push_back("Note: " + ignored + " has no effect in the GUI, so it was ignored.");

    if (options.CLI.TestTimeoutSeconds.has_value())
    {
        notes.push_back("Note: --test-timeout-seconds runs each test on a worker thread, so a test that creates " +
            uiObjects + " won't work with it.");
    }

    if (options.LayoutIgnore)
        notes.push_back("Note: --layout-ignore: the window layout isn't loaded or saved, so it opens with the "
            "defaults.");

    return notes;
}
//---------------------------------------------------------------------------
std::string GUIDefaultJUnitReportFileName(std::string const& exeFileName)
{
    size_t const dot = exeFileName.rfind('.');
    return (dot == std::string::npos ? exeFileName : exeFileName.substr(0, dot)) + "_JUnit.xml";
}
//---------------------------------------------------------------------------
TestFilter GUIFilterBoxFilter(std::string const& text)
{
    std::string const trimmed = Trim(text);
    if (trimmed.empty())
        return TestFilter();

    std::string const pattern = "*" + trimmed + "*";
    return [pattern](std::string const& fullTestName)
        {
            return TTestHandler::WildcardMatch(pattern, fullTestName, true);
        };
}
//---------------------------------------------------------------------------
std::string GUIFirstLine(std::string const& text)
{
    return text.substr(0, text.find('\n'));
}
//---------------------------------------------------------------------------
std::vector<std::string> GUIStatusPanels(TGUITestList const& testList, size_t runFinishedCount, size_t runTotalCount,
    std::optional<double> elapsedSeconds, std::string const& shuffleText, std::string const& message)
{
    // Counts only the tests the filter box shows, since those are the ones Run Selected runs.
    std::string checked = "Checked: " + std::to_string(testList.CheckedCount()) + " / " +
        std::to_string(testList.VisibleCount());
    if (testList.VisibleCount() != testList.Count())
        checked += " shown";

    return {
        checked,
        "Run: " + std::to_string(runFinishedCount) + " / " + std::to_string(runTotalCount),
        "Passed: " + std::to_string(testList.StatusCount(TGUITestStatus::Passed)),
        "Failed: " + std::to_string(testList.StatusCount(TGUITestStatus::Failed)),
        "Skipped: " + std::to_string(testList.StatusCount(TGUITestStatus::Skipped)),
        elapsedSeconds.has_value() ? FormatGUISeconds(*elapsedSeconds) : std::string(),
        shuffleText,
        message,
    };
}
//---------------------------------------------------------------------------
std::vector<int> GUIStatusPanelWidths()
{
    return { 170, 100, 90, 90, 90, 70, 190, 300 };
}
//---------------------------------------------------------------------------
TTestCaseRecord GUIUnhandledExceptionRecord(TTestId const& test, std::string const& description)
{
    return TTestCaseRecord{ test.GroupName, test.TestName, 0.0, TTestOutcome::Fail,
                            "Unhandled exception: " + description, test.SuitePath };
}
//---------------------------------------------------------------------------
std::string GUIUnhandledExceptionStatus(std::string const& description)
{
    return "The run was ended by an unhandled exception: " + description;
}
//---------------------------------------------------------------------------
TGUIRunSummary SummarizeGUIRun(TTestResults const& results)
{
    TGUIRunSummary summary;
    summary.Aborted = results.TimedOut || results.Crashed;
    summary.ExitCode = ExitCodeForResults(results);

    if (results.Stopped)
        summary.StatusMessage = "Stopped before every test ran.";
    else if (results.TimedOut)
        summary.StatusMessage = "A test timed out, so the rest of the run was abandoned. Restart to run tests again.";
    else if (results.Crashed)
        summary.StatusMessage = "A test crashed, so the rest of the run was abandoned. Restart to run tests again.";
    else
        summary.StatusMessage = "Done.";

    return summary;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
