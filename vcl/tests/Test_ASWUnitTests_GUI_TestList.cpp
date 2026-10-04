/* **************************************************************************
Test_ASWUnitTests_GUI_TestList.cpp
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
#include "Test_ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

std::vector<TTestId> SampleTests();

//---------------------------------------------------------------------------

// Four tests in three groups, one with a '.' in its name: indexes 0 and 1 in "Alpha", 2 in "Beta", 3 in
// "Alpha.Sub".
std::vector<TTestId> SampleTests()
{
    return { { "Alpha", "One" }, { "Alpha", "Two" }, { "Beta", "Three" }, { "Alpha.Sub", "Four" } };
}

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_TestList
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_TestList::TTest_ASWUnitTests_GUI_TestList()
    : inherited("ASWUnitTests_GUI_TestList_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_CheckedFilter_MatchesExactlyTheCheckedTests,
        "CheckedFilter_MatchesExactlyTheCheckedTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_CheckMatching_ChecksExactlyTheMatches, "CheckMatching_ChecksExactlyTheMatches");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_CheckOnlyFailed_ChecksVisibleFailuresOnly,
        "CheckOnlyFailed_ChecksVisibleFailuresOnly");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_CountMatching_CountsFilterMatches, "CountMatching_CountsFilterMatches");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_DetailText_DescribesAFinishedTest, "DetailText_DescribesAFinishedTest");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_DetailText_DescribesATestThatHasNotRun,
        "DetailText_DescribesATestThatHasNotRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_FailedFilter_MatchesEveryFailedTestEvenHiddenOnes,
        "FailedFilter_MatchesEveryFailedTestEvenHiddenOnes");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_GroupDetailText_CountsEachStatus, "GroupDetailText_CountsEachStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_GroupStatus_ShowsRunningThenWorstOutcome,
        "GroupStatus_ShowsRunningThenWorstOutcome");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_IndexOf_FindsTestsByGroupAndTestName, "IndexOf_FindsTestsByGroupAndTestName");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_Load_ChecksEveryTestAndListsGroupsInOrder,
        "Load_ChecksEveryTestAndListsGroupsInOrder");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_ResetResults_SetsEveryTestBackToNotRun,
        "ResetResults_SetsEveryTestBackToNotRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetAllChecked_ChecksOrUnchecksEveryTest,
        "SetAllChecked_ChecksOrUnchecksEveryTest");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetChecked_UpdatesItsGroupsCheckState, "SetChecked_UpdatesItsGroupsCheckState");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetGroupChecked_ChangesOnlyThatGroup, "SetGroupChecked_ChangesOnlyThatGroup");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetResult_StoresTheResultAndSetsTheStatus,
        "SetResult_StoresTheResultAndSetsTheStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_HidesNonMatchingTests,
        "SetVisibleFilter_HidesNonMatchingTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_LimitsCheckingToVisibleTests,
        "SetVisibleFilter_LimitsCheckingToVisibleTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusCount_CountsTestsWithEachStatus, "StatusCount_CountsTestsWithEachStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusFromOutcome_MapsEachOutcome, "StatusFromOutcome_MapsEachOutcome");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusName_NamesEachStatus, "StatusName_NamesEachStatus");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_TestList::~TTest_ASWUnitTests_GUI_TestList()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_CheckedFilter_MatchesExactlyTheCheckedTests()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetChecked(1, false);
    list.SetChecked(2, false);

    // Act
    TestFilter const filter = list.CheckedFilter();

    // Assert
    AssertTrue(filter != nullptr, __func__, __LINE__, "a filter is returned");
    CheckTrue(filter("Alpha.One"), __func__, __LINE__, "a checked test matches");
    CheckFalse(filter("Alpha.Two"), __func__, __LINE__, "an unchecked test doesn't");
    CheckFalse(filter("Beta.Three"), __func__, __LINE__, "nor does another group's unchecked test");
    CheckTrue(filter("Alpha.Sub.Four"), __func__, __LINE__, "a group name containing '.' still matches exactly");
    CheckFalse(filter("Alpha.On"), __func__, __LINE__, "matching is exact, not by prefix");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_CheckMatching_ChecksExactlyTheMatches()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.CheckMatching([](std::string const& fullTestName)
        {
            return fullTestName.rfind("Alpha.", 0) == 0;
        });
    size_t const checkedByFilter = list.CheckedCount();
    bool const betaCheckedByFilter = list.IsChecked(2);
    list.CheckMatching(TestFilter());

    // Assert
    CheckEquals(static_cast<size_t>(3), checkedByFilter, __func__, __LINE__,
        "a filter checks exactly its matches (here, both Alpha tests and Alpha.Sub's)");
    CheckFalse(betaCheckedByFilter, __func__, __LINE__, "and unchecks the rest");
    CheckEquals(static_cast<size_t>(4), list.CheckedCount(), __func__, __LINE__, "an empty filter checks everything");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_CheckOnlyFailed_ChecksVisibleFailuresOnly()
{
    // Arrange: "Beta.Three" failed but is unchecked and hidden, which must leave it as it is.
    TGUITestList list;
    list.Load(SampleTests());
    list.SetStatus(1, TGUITestStatus::Failed);
    list.SetStatus(2, TGUITestStatus::Failed);
    list.SetStatus(3, TGUITestStatus::Skipped);
    list.SetChecked(2, false);
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Beta.Three";
        });

    // Act
    list.CheckOnlyFailed();

    // Assert
    CheckFalse(list.IsChecked(0), __func__, __LINE__, "a visible test that didn't fail is unchecked");
    CheckTrue(list.IsChecked(1), __func__, __LINE__, "a visible failed test is checked");
    CheckFalse(list.IsChecked(2), __func__, __LINE__, "a hidden failed test is left alone, like Select All does");
    CheckFalse(list.IsChecked(3), __func__, __LINE__, "a skipped test isn't a failure");
    CheckEquals(static_cast<size_t>(1), list.CheckedCount(), __func__, __LINE__, "only the visible failure");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_CountMatching_CountsFilterMatches()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    size_t const alphaTests = list.CountMatching([](std::string const& fullTestName)
        {
            return fullTestName.rfind("Alpha.", 0) == 0;
        });
    size_t const allTests = list.CountMatching(TestFilter());

    // Assert
    CheckEquals(static_cast<size_t>(3), alphaTests, __func__, __LINE__, "a filter's matches are counted");
    CheckEquals(static_cast<size_t>(4), allTests, __func__, __LINE__, "an empty filter matches everything");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_DetailText_DescribesAFinishedTest()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetResult(1, TTestCaseRecord{ "Alpha", "Two", 0.000412, TTestOutcome::Fail, "Check failed for: needle" },
        "Running test: Alpha.Two\n");

    // Act
    std::string const text = list.DetailText(1);

    // Assert
    CheckEquals(static_cast<size_t>(0), text.find("Alpha.Two\n"), __func__, __LINE__, "it starts with the full name");
    CheckTrue(text.find("Result: Failed (0.412 ms)") != std::string::npos, __func__, __LINE__,
        "then the status and duration in milliseconds");
    CheckTrue(text.find("\nCheck failed for: needle\n") != std::string::npos, __func__, __LINE__,
        "then the failure detail");
    CheckTrue(text.find("\nLog:\nRunning test: Alpha.Two\n") != std::string::npos, __func__, __LINE__,
        "then the test's own log");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_DetailText_DescribesATestThatHasNotRun()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    std::string const text = list.DetailText(3);

    // Assert
    CheckEquals(std::string("Alpha.Sub.Four\nResult: Not run\n"), text, __func__, __LINE__,
        "just the full name and status, with no duration, detail, or log");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_FailedFilter_MatchesEveryFailedTestEvenHiddenOnes()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetStatus(1, TGUITestStatus::Failed);
    list.SetStatus(2, TGUITestStatus::Failed);
    list.SetStatus(3, TGUITestStatus::Skipped);
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Beta.Three";
        });

    // Act
    TestFilter const filter = list.FailedFilter();

    // Assert
    CheckTrue(filter("Alpha.Two"), __func__, __LINE__, "a visible failed test matches");
    CheckTrue(filter("Beta.Three"), __func__, __LINE__, "so does a hidden one, since Run Failed reruns them all");
    CheckFalse(filter("Alpha.One"), __func__, __LINE__, "a test that didn't fail doesn't");
    CheckFalse(filter("Alpha.Sub.Four"), __func__, __LINE__, "nor does a skipped one");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_GroupDetailText_CountsEachStatus()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetStatus(0, TGUITestStatus::Passed);
    list.SetStatus(1, TGUITestStatus::Running);

    // Act
    std::string const text = list.GroupDetailText("Alpha");

    // Assert
    CheckEquals(std::string("Alpha\n2 tests: 1 passed, 0 failed, 0 skipped, 0 not run, 1 running\n"), text, __func__,
        __LINE__, "the group's name, then how many of its tests have each status");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_GroupStatus_ShowsRunningThenWorstOutcome()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    TGUITestStatus const beforeRun = list.GroupStatus("Alpha");

    // Act, and Assert as the group's tests progress
    list.SetStatus(0, TGUITestStatus::Passed);
    CheckTrue(list.GroupStatus("Alpha") == TGUITestStatus::Passed, __func__, __LINE__,
        "one test passed, the other not run: passed");

    list.SetStatus(1, TGUITestStatus::Running);
    CheckTrue(list.GroupStatus("Alpha") == TGUITestStatus::Running, __func__, __LINE__, "running while any test is");

    list.SetStatus(1, TGUITestStatus::Skipped);
    CheckTrue(list.GroupStatus("Alpha") == TGUITestStatus::Skipped, __func__, __LINE__,
        "a skip outranks a pass");

    list.SetStatus(0, TGUITestStatus::Failed);
    CheckTrue(list.GroupStatus("Alpha") == TGUITestStatus::Failed, __func__, __LINE__, "a failure outranks a skip");

    CheckTrue(beforeRun == TGUITestStatus::NotRun, __func__, __LINE__, "nothing run yet: not run");
    CheckTrue(list.GroupStatus("Alpha.Sub") == TGUITestStatus::NotRun, __func__, __LINE__,
        "another group, even one whose name starts with this one's, is unaffected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_IndexOf_FindsTestsByGroupAndTestName()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    std::optional<size_t> const found = list.IndexOf("Alpha.Sub", "Four");
    std::optional<size_t> const wrongGroup = list.IndexOf("Alpha", "Four");
    std::optional<size_t> const missing = list.IndexOf("Gamma", "One");

    // Assert
    CheckTrue(found.has_value() && *found == 3, __func__, __LINE__, "a test is found by its group and test names");
    CheckFalse(wrongGroup.has_value(), __func__, __LINE__, "a test name under the wrong group isn't found");
    CheckFalse(missing.has_value(), __func__, __LINE__, "nor is an unknown group");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_Load_ChecksEveryTestAndListsGroupsInOrder()
{
    // Arrange
    TGUITestList list;

    // Act
    list.Load(SampleTests());

    // Assert
    CheckEquals(static_cast<size_t>(4), list.Count(), __func__, __LINE__, "every test is listed");
    CheckEquals(static_cast<size_t>(4), list.CheckedCount(), __func__, __LINE__, "and checked");
    CheckTrue(list.GroupNames() == std::vector<std::string>{ "Alpha", "Beta", "Alpha.Sub" }, __func__, __LINE__,
        "each group is listed once, in the order its first test appears");
    CheckTrue(list.TestsInGroup("Alpha") == std::vector<size_t>{ 0, 1 }, __func__, __LINE__,
        "a group's tests are listed by index, in order");
    CheckEquals(std::string("Three"), list.Test(2).TestName, __func__, __LINE__, "tests keep their order");
    CheckEquals(static_cast<size_t>(4), list.StatusCount(TGUITestStatus::NotRun), __func__, __LINE__,
        "none has run yet");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_ResetResults_SetsEveryTestBackToNotRun()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetStatus(0, TGUITestStatus::Passed);
    list.SetResult(2, TTestCaseRecord{ "Beta", "Three", 0.001, TTestOutcome::Fail, "boom" }, "log text");

    // Act
    list.ResetResults();

    // Assert
    CheckEquals(static_cast<size_t>(4), list.StatusCount(TGUITestStatus::NotRun), __func__, __LINE__,
        "every test is back to not run");
    CheckTrue(list.Result(2) == nullptr, __func__, __LINE__, "and no longer has a result");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetAllChecked_ChecksOrUnchecksEveryTest()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.SetAllChecked(false);
    size_t const checkedAfterNone = list.CheckedCount();
    list.SetAllChecked(true);

    // Assert
    CheckEquals(static_cast<size_t>(0), checkedAfterNone, __func__, __LINE__, "none checked");
    CheckEquals(static_cast<size_t>(4), list.CheckedCount(), __func__, __LINE__, "all checked again");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetChecked_UpdatesItsGroupsCheckState()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.SetChecked(0, false);
    TGUICheckState const oneUnchecked = list.GroupCheckState("Alpha");
    list.SetChecked(1, false);
    TGUICheckState const bothUnchecked = list.GroupCheckState("Alpha");

    // Assert
    CheckTrue(oneUnchecked == TGUICheckState::Partial, __func__, __LINE__, "some of a group's tests checked: partial");
    CheckTrue(bothUnchecked == TGUICheckState::Unchecked, __func__, __LINE__, "none checked: unchecked");
    CheckTrue(list.GroupCheckState("Beta") == TGUICheckState::Checked, __func__, __LINE__,
        "another group is unaffected");
    CheckTrue(list.GroupCheckState("Alpha.Sub") == TGUICheckState::Checked, __func__, __LINE__,
        "including one whose name starts with the changed group's");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetGroupChecked_ChangesOnlyThatGroup()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.SetGroupChecked("Alpha", false);

    // Assert
    CheckFalse(list.IsChecked(0), __func__, __LINE__, "the group's first test is unchecked");
    CheckFalse(list.IsChecked(1), __func__, __LINE__, "and its second");
    CheckTrue(list.IsChecked(2), __func__, __LINE__, "another group's test isn't");
    CheckTrue(list.IsChecked(3), __func__, __LINE__, "nor is one in a group whose name starts with this one's");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetResult_StoresTheResultAndSetsTheStatus()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    TGUITestResult const* const before = list.Result(2);

    // Act
    list.SetResult(2, TTestCaseRecord{ "Beta", "Three", 0.25, TTestOutcome::Skip, "not on this platform" }, "log");

    // Assert
    CheckTrue(before == nullptr, __func__, __LINE__, "a test has no result before it finishes");
    TGUITestResult const* const after = list.Result(2);
    AssertTrue(after != nullptr, __func__, __LINE__, "it has one after");
    CheckEquals(std::string("not on this platform"), after->Record.Message, __func__, __LINE__, "with its record");
    CheckEquals(std::string("log"), after->Log, __func__, __LINE__, "and its log");
    CheckTrue(list.Status(2) == TGUITestStatus::Skipped, __func__, __LINE__, "its status follows the outcome");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_HidesNonMatchingTests()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Alpha.Two";
        });
    bool const twoVisibleWhileFiltered = list.IsVisible(1);
    std::vector<size_t> const alphaWhileFiltered = list.TestsInGroup("Alpha");
    size_t const visibleWhileFiltered = list.VisibleCount();
    list.SetVisibleFilter(TestFilter());

    // Assert
    CheckFalse(twoVisibleWhileFiltered, __func__, __LINE__, "a test the filter doesn't match is hidden");
    CheckTrue(alphaWhileFiltered == std::vector<size_t>{ 0 }, __func__, __LINE__,
        "and left out of its group's tests");
    CheckEquals(static_cast<size_t>(3), visibleWhileFiltered, __func__, __LINE__, "and the visible count");
    CheckEquals(static_cast<size_t>(4), list.VisibleCount(), __func__, __LINE__, "an empty filter shows everything");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_LimitsCheckingToVisibleTests()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Alpha.Two";
        });

    // Act
    list.SetAllChecked(false);
    bool const hiddenStillChecked = list.IsChecked(1);
    TestFilter const noneChecked = list.CheckedFilter();
    list.SetGroupChecked("Alpha", true);

    // Assert
    CheckTrue(hiddenStillChecked, __func__, __LINE__, "unchecking everything leaves a hidden test alone");
    CheckFalse(noneChecked("Alpha.Two"), __func__, __LINE__, "a hidden checked test isn't run");
    CheckTrue(list.GroupCheckState("Alpha") == TGUICheckState::Checked, __func__, __LINE__,
        "a group's check state only counts its visible tests");
    CheckEquals(static_cast<size_t>(1), list.CheckedCount(), __func__, __LINE__, "nor does the checked count");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_StatusCount_CountsTestsWithEachStatus()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());

    // Act
    list.SetStatus(0, TGUITestStatus::Passed);
    list.SetStatus(1, TGUITestStatus::Passed);
    list.SetStatus(2, TGUITestStatus::Failed);

    // Assert
    CheckEquals(static_cast<size_t>(2), list.StatusCount(TGUITestStatus::Passed), __func__, __LINE__, "two passed");
    CheckEquals(static_cast<size_t>(1), list.StatusCount(TGUITestStatus::Failed), __func__, __LINE__, "one failed");
    CheckEquals(static_cast<size_t>(0), list.StatusCount(TGUITestStatus::Skipped), __func__, __LINE__, "none skipped");
    CheckEquals(static_cast<size_t>(1), list.StatusCount(TGUITestStatus::NotRun), __func__, __LINE__, "one not run");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_StatusFromOutcome_MapsEachOutcome()
{
    CheckTrue(StatusFromOutcome(TTestOutcome::Pass) == TGUITestStatus::Passed, __func__, __LINE__, "Pass: Passed");
    CheckTrue(StatusFromOutcome(TTestOutcome::Fail) == TGUITestStatus::Failed, __func__, __LINE__, "Fail: Failed");
    CheckTrue(StatusFromOutcome(TTestOutcome::Skip) == TGUITestStatus::Skipped, __func__, __LINE__, "Skip: Skipped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_StatusName_NamesEachStatus()
{
    CheckEquals(std::string("Not run"), StatusName(TGUITestStatus::NotRun), __func__, __LINE__, "NotRun");
    CheckEquals(std::string("Running"), StatusName(TGUITestStatus::Running), __func__, __LINE__, "Running");
    CheckEquals(std::string("Passed"), StatusName(TGUITestStatus::Passed), __func__, __LINE__, "Passed");
    CheckEquals(std::string("Failed"), StatusName(TGUITestStatus::Failed), __func__, __LINE__, "Failed");
    CheckEquals(std::string("Skipped"), StatusName(TGUITestStatus::Skipped), __func__, __LINE__, "Skipped");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_TestList)
