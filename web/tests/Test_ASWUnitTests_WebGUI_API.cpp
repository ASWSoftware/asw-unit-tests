/* **************************************************************************
Test_ASWUnitTests_WebGUI_API.cpp
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
#include "Test_ASWUnitTests_WebGUI_API.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_API.h"
#include "ASWUnitTests_WebGUI_JSON.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

// The commands while idle, with no failure and no report, and during a run, as the state event writes them.
std::string const IdleCommands = "{\"About\":true,\"CollapseAll\":true,\"CommandLineOptions\":true,"
    "\"CopyDetails\":true,\"Exit\":true,\"ExpandAll\":true,\"ExportJUnitReport\":false,\"FilterBox\":true,"
    "\"FocusFilter\":true,\"ResetLayout\":true,\"RunFailed\":false,\"RunSelected\":true,\"SelectAll\":true,"
    "\"SelectFailed\":false,\"SelectNone\":true,\"ShuffleOrder\":true,\"ShuffleSeed\":true,\"Stop\":false,"
    "\"Theme\":true}";
std::string const RunningCommands = "{\"About\":false,\"CollapseAll\":true,\"CommandLineOptions\":false,"
    "\"CopyDetails\":true,\"Exit\":true,\"ExpandAll\":true,\"ExportJUnitReport\":false,\"FilterBox\":false,"
    "\"FocusFilter\":false,\"ResetLayout\":true,\"RunFailed\":false,\"RunSelected\":false,\"SelectAll\":false,"
    "\"SelectFailed\":false,\"SelectNone\":false,\"ShuffleOrder\":false,\"ShuffleSeed\":false,\"Stop\":true,"
    "\"Theme\":false}";

//---------------------------------------------------------------------------

std::string DescribeLog(ASWUnitTests::TWebGUILog const& log);
std::string ExpectedSnapshot(ASWUnitTests::TWebGUIState const& state, std::string const& projectName,
    std::string const& log, std::string const& failures);
std::vector<ASWUnitTests::TTestId> FixtureTests();
std::string JSONStrings(std::vector<std::string> const& texts);
ASWUnitTests::TWebGUIState RunState();
unsigned int SevenSeed();

//---------------------------------------------------------------------------
/*
    DescribeLog

    The log's lines joined by '|', then " @" and its first unsent line, or "-" for none, e.g. "a|b @1".
*/
std::string DescribeLog(ASWUnitTests::TWebGUILog const& log)
{
    std::string text;

    for (std::string const& line : log.Lines())
        text += (&line == &log.Lines().front() ? "" : "|") + line;

    std::optional<std::size_t> const unsent = log.FirstUnsentLine();
    return text + " @" + (unsent.has_value() ? std::to_string(*unsent) : std::string("-"));
}
//---------------------------------------------------------------------------
/*
    ExpectedSnapshot

    The snapshot 'state' should give: its type, the caption for 'projectName', the tree event's members and the
    state event's (each tested on its own), and 'log' and 'failures', the lines' and the rows' JSON arrays.
*/
std::string ExpectedSnapshot(ASWUnitTests::TWebGUIState const& state, std::string const& projectName,
    std::string const& log, std::string const& failures)
{
    std::string const tree = ASWUnitTests::WebGUITreeEvent(state);
    std::string const stateEvent = ASWUnitTests::WebGUIStateEvent(state);
    std::size_t const treeStart = std::string("{\"type\":\"tree\",").size();
    std::size_t const stateStart = std::string("{\"type\":\"state\",").size();

    return "{\"type\":\"snapshot\",\"caption\":" + ASWUnitTests::WebGUIJSONString(ASWUnitTests::GUICaption(
        projectName)) + "," + tree.substr(treeStart, tree.size() - treeStart - 1) + "," +
        stateEvent.substr(stateStart, stateEvent.size() - stateStart - 1) + ",\"log\":" + log + ",\"failures\":" +
        failures + "}";
}
//---------------------------------------------------------------------------
/*
    FixtureTests

    Four tests: t0 "One" in top-level group "Top"; t1 "Two" and t2 "Three" (its 'e' an e-acute) in group
    "Deep" of suite "Outer.Inner", whose suite "Outer" holds only that suite; and t3, whose name ends with a
    byte that isn't UTF-8, in group "Mid" of suite "Other".
*/
std::vector<ASWUnitTests::TTestId> FixtureTests()
{
    return { { "Top", "One", "" }, { "Outer.Inner.Deep", "Two", "Outer.Inner" },
        { "Outer.Inner.Deep", "Thr" "\xC3\xA9" "e", "Outer.Inner" }, { "Other.Mid", "Bad" "\xFF", "Other" } };
}
//---------------------------------------------------------------------------
/*
    JSONStrings

    'texts' as a JSON array of strings.
*/
std::string JSONStrings(std::vector<std::string> const& texts)
{
    std::string json = "[";

    for (std::string const& text : texts)
        json += (json.size() > 1 ? "," : "") + ASWUnitTests::WebGUIJSONString(text);

    return json + "]";
}
//---------------------------------------------------------------------------
/*
    RunState

    FixtureTests() partway through a run of 4 tests, 1.5 s in: t0 running, t1 passed, t3 skipped, then t2
    failed; t2 and t3 unchecked; three log lines, a failure's among them, the last one unfinished; shuffled with
    seed 42; the Dark theme, with dark theme "Nord"; project "Demo".
*/
ASWUnitTests::TWebGUIState RunState()
{
    ASWUnitTests::TWebGUIState state;
    state.TestList.Load(FixtureTests());
    state.TestList.SetStatus(0, ASWUnitTests::TGUITestStatus::Running);
    state.TestList.SetResult(1, { "Outer.Inner.Deep", "Two", 0.5, ASWUnitTests::TTestOutcome::Pass, "",
                                  "Outer.Inner" }, "");
    state.TestList.SetResult(3, { "Other.Mid", "Bad" "\xFF", 0.0, ASWUnitTests::TTestOutcome::Skip, "not here",
                                  "Other" }, "");
    state.TestList.SetResult(2, { "Outer.Inner.Deep", "Thr" "\xC3\xA9" "e", 0.25, ASWUnitTests::TTestOutcome::Fail,
                                  "line one\nline two", "Outer.Inner" }, "logged\n");
    state.TestList.SetChecked(2, false);
    state.TestList.SetChecked(3, false);
    state.FailureOrder = { 3, 2 };
    state.Log.Append("Starting\n***Test failed in Thr" "\xC3\xA9" "e\nStill runn");
    state.ElapsedSeconds = 1.5;
    state.ProjectName = "Demo";
    state.RunFinishedCount = 3;
    state.Running = true;
    state.RunTotalCount = 4;
    state.Shuffle = ASWUnitTests::TGUIShuffle(SevenSeed);
    state.Shuffle.ApplyCommandLine(true, 42);
    state.StatusMessage = "Running Top.One";
    state.Theme = ASWUnitTests::EGUITheme::Dark;
    state.DarkTheme = "Nord";
    return state;
}
//---------------------------------------------------------------------------
/*
    SevenSeed

    A seed source for TGUIShuffle that always gives 7, so a state's seedToShow is known.
*/
unsigned int SevenSeed()
{
    return 7;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_API
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_API::TTest_ASWUnitTests_WebGUI_API()
    : inherited("ASWUnitTests_WebGUI_API_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_CommandStates_AsIfANodeWereSelected,
        "CommandStates_AsIfANodeWereSelected");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_DetailAndNodeIds_NameEachNode,
        "DetailAndNodeIds_NameEachNode");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_Log_AppendKeepsWholeLines, "Log_AppendKeepsWholeLines");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_Log_EventSendsLinesFromAnIndex,
        "Log_EventSendsLinesFromAnIndex");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_SnapshotEvent_HoldsEverything, "SnapshotEvent_HoldsEverything");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_StateEvent_CommandsPanelsProgressShuffleTheme,
        "StateEvent_CommandsPanelsProgressShuffleTheme");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_TestEvents_StatusOfEachNodeTheTestChanges,
        "TestEvents_StatusOfEachNodeTheTestChanges");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_TreeAndChecksEvents_FollowTheTestList,
        "TreeAndChecksEvents_FollowTheTestList");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_API::~TTest_ASWUnitTests_WebGUI_API()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_CommandStates_AsIfANodeWereSelected()
{
    // Arrange
    TWebGUIState idle;
    idle.TestList.Load(FixtureTests());

    TWebGUIState running = idle;
    running.Running = true;

    TWebGUIState allHidden = idle;
    allHidden.TestList.SetVisibleFilter(GUIFilterBoxFilter("no such test"));

    TWebGUIState empty;

    TWebGUIState failed = idle;
    failed.TestList.SetResult(0, { "Top", "One", 0.0, TTestOutcome::Fail, "no", "" }, "");

    TWebGUIState aborted = failed;
    aborted.RunAborted = true;

    TWebGUIState reported = idle;
    reported.HasReport = true;

    // Act, Assert
    CheckTrue(WebGUICommandStates(idle).CopyDetails, __func__, __LINE__, "idle: Copy Details, as if selected");
    CheckTrue(WebGUICommandStates(idle).ExpandAll, __func__, __LINE__, "idle: Expand All");
    CheckTrue(WebGUICommandStates(idle).RunSelected, __func__, __LINE__, "idle: Run Selected");
    CheckFalse(WebGUICommandStates(idle).RunFailed, __func__, __LINE__, "idle: no failure, no Run Failed");
    CheckFalse(WebGUICommandStates(idle).ExportJUnitReport, __func__, __LINE__, "idle: no report");
    CheckFalse(WebGUICommandStates(idle).Stop, __func__, __LINE__, "idle: no Stop");
    CheckTrue(WebGUICommandStates(running).Stop, __func__, __LINE__, "running: Stop");
    CheckTrue(WebGUICommandStates(running).CopyDetails, __func__, __LINE__, "running: Copy Details");
    CheckFalse(WebGUICommandStates(running).RunSelected, __func__, __LINE__, "running: no Run Selected");
    CheckFalse(WebGUICommandStates(allHidden).ExpandAll, __func__, __LINE__, "all hidden: no Expand All");
    CheckFalse(WebGUICommandStates(allHidden).CollapseAll, __func__, __LINE__, "all hidden: no Collapse All");
    CheckTrue(WebGUICommandStates(allHidden).CopyDetails, __func__, __LINE__, "all hidden: Copy Details");
    CheckFalse(WebGUICommandStates(empty).ExpandAll, __func__, __LINE__, "no tests: no Expand All");
    CheckTrue(WebGUICommandStates(failed).RunFailed, __func__, __LINE__, "a failure: Run Failed");
    CheckTrue(WebGUICommandStates(failed).SelectFailed, __func__, __LINE__, "a failure: Select Failed");
    CheckFalse(WebGUICommandStates(aborted).RunFailed, __func__, __LINE__, "aborted: no Run Failed");
    CheckFalse(WebGUICommandStates(aborted).RunSelected, __func__, __LINE__, "aborted: no Run Selected");
    CheckTrue(WebGUICommandStates(reported).ExportJUnitReport, __func__, __LINE__, "a report: Export");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_DetailAndNodeIds_NameEachNode()
{
    // Arrange
    TWebGUIState const state = RunState();
    TGUINodeRef const refs[] = { { 2, TGUINodeKind::Test }, { 1, TGUINodeKind::Group }, { 1, TGUINodeKind::Suite } };

    // Act, Assert
    CheckEquals(std::string("s0"), WebGUINodeId({ 0, TGUINodeKind::Suite }), __func__, __LINE__, "a suite");
    CheckEquals(std::string("g12"), WebGUINodeId({ 12, TGUINodeKind::Group }), __func__, __LINE__, "a group");
    CheckEquals(std::string("t123456789"), WebGUINodeId({ 123456789, TGUINodeKind::Test }), __func__, __LINE__,
        "a test");

    for (TGUINodeRef const& ref : refs)
    {
        CheckEquals("{\"node\":\"" + WebGUINodeId(ref) + "\",\"text\":" +
            WebGUIJSONString(GUINodeDetailText(state.TestList, ref)) + "}", WebGUIDetailJSON(state, ref), __func__,
            __LINE__, "detail of " + WebGUINodeId(ref));
    }

    CheckTrue(WebGUIDetailJSON(state, refs[0]).find("line two") != std::string::npos, __func__, __LINE__,
        "a failed test's detail has its whole message");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_Log_AppendKeepsWholeLines()
{
    // Arrange
    TWebGUILog log;
    std::vector<std::string> steps;

    // Act: each step's log, as DescribeLog() gives it.
    steps.push_back(DescribeLog(log));
    log.Append("");
    steps.push_back(DescribeLog(log));
    log.Append("a\nb");
    steps.push_back(DescribeLog(log));
    log.MarkSent();
    steps.push_back(DescribeLog(log));
    log.Append("c\n");
    steps.push_back(DescribeLog(log));
    log.Append("d");
    steps.push_back(DescribeLog(log));
    log.MarkSent();
    log.Append("\n");
    steps.push_back(DescribeLog(log));
    log.Append("\n");
    steps.push_back(DescribeLog(log));
    log.MarkSent();
    log.Append("e\n\nf\r\n");
    steps.push_back(DescribeLog(log));
    log.Clear();
    steps.push_back(DescribeLog(log));
    log.Append("x");
    steps.push_back(DescribeLog(log));
    log.MarkSent();
    log.Clear();
    log.MarkSent();
    steps.push_back(DescribeLog(log));
    log.Append("y");
    steps.push_back(DescribeLog(log));

    // Assert
    std::string const expected[] = {
        " @-",
        " @-",
        "a|b @0",
        "a|b @-",
        "a|bc @1",
        "a|bc|d @1",
        "a|bc|d @2",
        "a|bc|d| @2",
        "a|bc|d||e||f\r @4",
        " @0",
        "x @0",
        " @-",
        "y @0",
    };
    char const* const names[] = {
        "a new log",
        "empty text changes nothing",
        "two lines, the second unfinished",
        "sent",
        "text continues the unfinished line, which ends",
        "a new line after a finished one: the first unsent stays the earliest",
        "a line break alone ends the unfinished line",
        "another adds an empty line",
        "several lines at once, empty ones kept, a CR kept",
        "cleared: the page is sent the log from line 0",
        "a line after clearing",
        "cleared and sent: nothing unsent",
        "cleared while a line was unfinished: text starts a new line",
    };

    for (std::size_t i = 0; i < steps.size(); ++i)
        CheckEquals(expected[i], steps[i], __func__, __LINE__, names[i]);

    CheckEquals(std::size_t(13), steps.size(), __func__, __LINE__, "every step checked");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_Log_EventSendsLinesFromAnIndex()
{
    // Arrange
    TWebGUIState state;
    state.Log.Append("Running G.T\n***Test failed: boom\n***Test skipped: why\nFinished test: \"G.T\" - fai");
    std::string const before = WebGUILogEvent(state, 0);
    std::string const unfinished = WebGUILogEvent(state, 3);

    // Act
    state.Log.Append("led (1 ms)\n");

    // Assert
    CheckEquals(std::string("{\"type\":\"log\",\"from\":0,\"lines\":[[\"Running G.T\",0],"
        "[\"***Test failed: boom\",1],[\"***Test skipped: why\",2],[\"Finished test: \\\"G.T\\\" - fai\",0]]}"),
        before, __func__, __LINE__, "every line with its kind: normal, failure, skip, and an unfinished line");
    CheckEquals(std::string("{\"type\":\"log\",\"from\":3,\"lines\":[[\"Finished test: \\\"G.T\\\" - fai\",0]]}"),
        unfinished, __func__, __LINE__, "from an index");
    CheckEquals(std::string("{\"type\":\"log\",\"from\":3,\"lines\":"
        "[[\"Finished test: \\\"G.T\\\" - failed (1 ms)\",1]]}"), WebGUILogEvent(state, 3), __func__, __LINE__,
        "the line, finished, sent whole again: a failure now");
    CheckEquals(std::string("{\"type\":\"log\",\"from\":4,\"lines\":[]}"), WebGUILogEvent(state, 4), __func__,
        __LINE__, "from the end: no lines, as after Clear()");
    CheckThrows<std::logic_error>([&state]
        {
            WebGUILogEvent(state, 5);
        }, __func__, __LINE__, "past the end", "past the log's end");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_SnapshotEvent_HoldsEverything()
{
    // Arrange
    TWebGUIState const state = RunState();

    TWebGUIState fresh;
    fresh.Shuffle = TGUIShuffle(SevenSeed);

    TWebGUIState broken = RunState();
    broken.FailureOrder.push_back(4);

    // Act
    std::string const snapshot = WebGUISnapshotEvent(state);

    // Assert
    CheckEquals(ExpectedSnapshot(state, "Demo", "[[\"Starting\",0],[\"***Test failed in Thr" "\xC3\xA9" "e\",1],"
        "[\"Still runn\",0]]", "[[\"t3\",\"Skipped\",\"Other.Mid.Bad" "\xEF\xBF\xBD" "\",\"not here\"],"
        "[\"t2\",\"Failed\",\"Outer.Inner.Deep.Thr" "\xC3\xA9" "e\",\"line one\"]]"), snapshot, __func__, __LINE__,
        "the caption, the tree, the state, every log line, and the failures in the order they finished");
    CheckEquals(ExpectedSnapshot(fresh, "ASWUnitTests", "[]", "[]"), WebGUISnapshotEvent(fresh), __func__, __LINE__,
        "a new state: no tests, no failures, the default project");
    CheckThrows<std::logic_error>([&broken]
        {
            WebGUISnapshotEvent(broken);
        }, __func__, __LINE__, "a failure that isn't a test", "isn't a test's index");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_StateEvent_CommandsPanelsProgressShuffleTheme()
{
    // Arrange
    TWebGUIState const running = RunState();

    TWebGUIState idle;
    idle.TestList.Load(FixtureTests());
    idle.Shuffle = TGUIShuffle(SevenSeed);

    TWebGUIState light = idle;
    light.Theme = EGUITheme::Light;

    // Act, Assert
    CheckEquals("{\"type\":\"state\",\"commands\":" + RunningCommands + ",\"panels\":" +
        JSONStrings(GUIStatusPanels(running.TestList, 3, 4, 1.5, "Shuffle: seed 42", "Running Top.One")) +
        ",\"progress\":{\"finished\":3,\"total\":4,\"failed\":true},"
        "\"shuffle\":{\"enabled\":true,\"fixedSeed\":42,\"seedToShow\":42},"
        "\"theme\":{\"choice\":\"Dark\",\"dark\":\"Nord\"}}", WebGUIStateEvent(running), __func__, __LINE__,
        "during a run with a failure, shuffled with a fixed seed, the Dark theme");
    CheckEquals("{\"type\":\"state\",\"commands\":" + IdleCommands + ",\"panels\":" +
        JSONStrings(GUIStatusPanels(idle.TestList, 0, 0, std::nullopt, "Shuffle: off", "")) +
        ",\"progress\":{\"finished\":0,\"total\":0,\"failed\":false},"
        "\"shuffle\":{\"enabled\":false,\"fixedSeed\":null,\"seedToShow\":7},"
        "\"theme\":{\"choice\":\"Windows\",\"dark\":\"Carbon\"}}", WebGUIStateEvent(idle), __func__, __LINE__,
        "idle before any run, not shuffled, the Windows theme");
    CheckTrue(WebGUIStateEvent(light).find("\"theme\":{\"choice\":\"Light\",") != std::string::npos, __func__,
        __LINE__, "the Light theme");
    CheckEquals(std::string("Shuffle: seed 42"), running.Shuffle.StatusText(), __func__, __LINE__,
        "the shuffle panel's text the expectation uses");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_TestEvents_StatusOfEachNodeTheTestChanges()
{
    // Arrange
    TWebGUIState const state = RunState();

    TWebGUIState starting;
    starting.TestList.Load(FixtureTests());
    starting.TestList.SetStatus(2, TGUITestStatus::Running);

    TWebGUIState filtered = RunState();
    filtered.TestList.SetVisibleFilter(GUIFilterBoxFilter("two"));

    std::string const failedRow = "[\"t2\",\"Failed\",\"Outer.Inner.Deep.Thr" "\xC3\xA9" "e\",\"line one\"]";

    // Act, Assert
    CheckEquals(std::string("{\"type\":\"started\",\"test\":\"t0\",\"nodes\":{\"t0\":1,\"g0\":1}}"),
        WebGUITestStartedEvent(state, 0), __func__, __LINE__, "a top-level group's test: itself and its group");
    CheckEquals(std::string("{\"type\":\"started\",\"test\":\"t2\",\"nodes\":{\"t2\":1,\"g1\":1,\"s1\":1,\"s0\":1}}"),
        WebGUITestStartedEvent(starting, 2), __func__, __LINE__, "a nested suite's test: its suites, innermost first");
    CheckEquals(std::string("{\"type\":\"finished\",\"test\":\"t1\",\"nodes\":{\"t1\":2,\"g1\":3,\"s1\":3,\"s0\":3}}"),
        WebGUITestFinishedEvent(state, 1), __func__, __LINE__, "passed: no failure row; the group's status as it is");
    CheckEquals("{\"type\":\"finished\",\"test\":\"t2\",\"nodes\":{\"t2\":3,\"g1\":3,\"s1\":3,\"s0\":3},\"failure\":" +
        failedRow + "}", WebGUITestFinishedEvent(state, 2), __func__, __LINE__, "failed: its row");
    CheckEquals(std::string("{\"type\":\"finished\",\"test\":\"t3\",\"nodes\":{\"t3\":4,\"g2\":4,\"s2\":4},"
        "\"failure\":[\"t3\",\"Skipped\",\"Other.Mid.Bad" "\xEF\xBF\xBD" "\",\"not here\"]}"),
        WebGUITestFinishedEvent(state, 3), __func__, __LINE__, "skipped: its row");
    CheckEquals(std::string("{\"type\":\"started\",\"test\":\"t0\",\"nodes\":{}}"), WebGUITestStartedEvent(filtered, 0),
        __func__, __LINE__, "hidden by the filter box: no node changes");
    CheckEquals("{\"type\":\"finished\",\"test\":\"t2\",\"nodes\":{},\"failure\":" + failedRow + "}",
        WebGUITestFinishedEvent(filtered, 2), __func__, __LINE__, "hidden: still in the Failures & Skips list");
    CheckThrows<std::logic_error>([&state]
        {
            WebGUITestStartedEvent(state, 4);
        }, __func__, __LINE__, "started: not a test", "isn't a test's index");
    CheckThrows<std::logic_error>([&state]
        {
            WebGUITestFinishedEvent(state, 4);
        }, __func__, __LINE__, "finished: not a test", "isn't a test's index");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_API::Test_TreeAndChecksEvents_FollowTheTestList()
{
    // Arrange
    TWebGUIState const state = RunState();

    TWebGUIState filtered = RunState();
    filtered.FilterText = "two";
    filtered.TestList.SetVisibleFilter(GUIFilterBoxFilter("two"));

    TWebGUIState fresh;
    fresh.TestList.Load(FixtureTests());

    std::string const nodes = "[{\"id\":\"g0\",\"parent\":null,\"text\":\"Top\"},"
        "{\"id\":\"t0\",\"parent\":\"g0\",\"text\":\"One\"},"
        "{\"id\":\"s0\",\"parent\":null,\"text\":\"Outer\"},"
        "{\"id\":\"s1\",\"parent\":\"s0\",\"text\":\"Inner\"},"
        "{\"id\":\"g1\",\"parent\":\"s1\",\"text\":\"Deep\"},"
        "{\"id\":\"t1\",\"parent\":\"g1\",\"text\":\"Two\"},"
        "{\"id\":\"t2\",\"parent\":\"g1\",\"text\":\"Thr" "\xC3\xA9" "e\"},"
        "{\"id\":\"s2\",\"parent\":null,\"text\":\"Other\"},"
        "{\"id\":\"g2\",\"parent\":\"s2\",\"text\":\"Mid\"},"
        "{\"id\":\"t3\",\"parent\":\"g2\",\"text\":\"Bad" "\xEF\xBF\xBD" "\"}]";

    // Act, Assert
    CheckEquals("{\"type\":\"tree\",\"filter\":\"\",\"nodes\":" + nodes +
        ",\"statuses\":\"1133323444\",\"checks\":\"1122210000\"}", WebGUITreeEvent(state), __func__, __LINE__,
        "every status and check state, nested suites, a suite holding only a suite, repaired UTF-8");
    CheckEquals("{\"type\":\"tree\",\"filter\":\"\",\"nodes\":" + nodes +
        ",\"statuses\":\"0000000000\",\"checks\":\"1111111111\"}", WebGUITreeEvent(fresh), __func__, __LINE__,
        "a new list: nothing run, everything checked");
    CheckEquals(std::string("{\"type\":\"tree\",\"filter\":\"two\",\"nodes\":["
        "{\"id\":\"s0\",\"parent\":null,\"text\":\"Outer\"},"
        "{\"id\":\"s1\",\"parent\":\"s0\",\"text\":\"Inner\"},"
        "{\"id\":\"g1\",\"parent\":\"s1\",\"text\":\"Deep\"},"
        "{\"id\":\"t1\",\"parent\":\"g1\",\"text\":\"Two\"}],\"statuses\":\"2222\",\"checks\":\"1111\"}"),
        WebGUITreeEvent(filtered), __func__, __LINE__, "the filter box hides tests: only the visible ones count");
    CheckEquals(std::string("{\"type\":\"checks\",\"checks\":\"1122210000\"}"), WebGUIChecksEvent(state), __func__,
        __LINE__, "checks");
    CheckEquals(std::string("{\"type\":\"checks\",\"checks\":\"1111\"}"), WebGUIChecksEvent(filtered), __func__,
        __LINE__, "checks, filtered");
    CheckEquals(std::string("{\"type\":\"checks\",\"checks\":\"\"}"), WebGUIChecksEvent(TWebGUIState()), __func__,
        __LINE__, "checks, no tests");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_WebGUI_API)
