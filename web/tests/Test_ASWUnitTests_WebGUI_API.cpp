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

std::string ExpectedSnapshot(ASWUnitTests::TWebGUIState const& state, std::string const& projectName,
    std::string const& failures);
std::vector<ASWUnitTests::TTestId> FixtureTests();
std::string JSONStrings(std::vector<std::string> const& texts);
ASWUnitTests::TWebGUIState RunState();
unsigned int SevenSeed();

//---------------------------------------------------------------------------
/*
    ExpectedSnapshot

    The snapshot 'state' should give: its type, the caption for 'projectName', the tree event's members and the
    state event's (each tested on its own), and 'failures', the rows' JSON array.
*/
std::string ExpectedSnapshot(ASWUnitTests::TWebGUIState const& state, std::string const& projectName,
    std::string const& failures)
{
    std::string const tree = ASWUnitTests::WebGUITreeEvent(state);
    std::string const stateEvent = ASWUnitTests::WebGUIStateEvent(state);
    std::size_t const treeStart = std::string("{\"type\":\"tree\",").size();
    std::size_t const stateStart = std::string("{\"type\":\"state\",").size();

    return "{\"type\":\"snapshot\",\"caption\":" + ASWUnitTests::WebGUIJSONString(ASWUnitTests::GUICaption(
        projectName)) + "," + tree.substr(treeStart, tree.size() - treeStart - 1) + "," +
        stateEvent.substr(stateStart, stateEvent.size() - stateStart - 1) + ",\"failures\":" + failures + "}";
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
    failed; t2 and t3 unchecked; shuffled with seed 42; the Dark theme, with dark theme "Nord"; project "Demo".
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
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_SnapshotEvent_HoldsEverything, "SnapshotEvent_HoldsEverything");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_API::Test_StateEvent_CommandsPanelsProgressShuffleTheme,
        "StateEvent_CommandsPanelsProgressShuffleTheme");
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
    CheckEquals(ExpectedSnapshot(state, "Demo", "[[\"t3\",\"Skipped\",\"Other.Mid.Bad" "\xEF\xBF\xBD" "\","
        "\"not here\"],[\"t2\",\"Failed\",\"Outer.Inner.Deep.Thr" "\xC3\xA9" "e\",\"line one\"]]"), snapshot,
        __func__, __LINE__, "the caption, the tree, the state, and the failures in the order they finished");
    CheckEquals(ExpectedSnapshot(fresh, "ASWUnitTests", "[]"), WebGUISnapshotEvent(fresh), __func__, __LINE__,
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
