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

//---------------------------------------------------------------------------

std::vector<TTestId> SampleTests();
std::vector<TTestId> SuiteTests();

//---------------------------------------------------------------------------
/*
    SampleTests

    Four tests in three groups, one with a '.' in its name: indexes 0 and 1 in "Alpha", 2 in "Beta", 3 in "Alpha.Sub".
*/
std::vector<TTestId> SampleTests()
{
    return { { "Alpha", "One" }, { "Alpha", "Two" }, { "Beta", "Three" }, { "Alpha.Sub", "Four" } };
}
//---------------------------------------------------------------------------
/*
    SuiteTests

    Six tests, with groups in suites: index 0 in top-level group "Top"; 1 and 2 in group
    "Deep" in suite "Outer.Inner"; 3 in group "Mid" in suite "Outer"; 4 in group "Solo" in suite "Other"; and
    5 in group "Near" in suite "Outermost", whose path starts with "Outer" but isn't inside it.
*/
std::vector<TTestId> SuiteTests()
{
    return { { "Top", "One", "" }, { "Outer.Inner.Deep", "Two", "Outer.Inner" },
        { "Outer.Inner.Deep", "Three", "Outer.Inner" }, { "Outer.Mid", "Four", "Outer" },
        { "Other.Solo", "Five", "Other" }, { "Outermost.Near", "Six", "Outermost" } };
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

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
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_GroupSuitePath_GivesEachGroupsSuite,
        "GroupSuitePath_GivesEachGroupsSuite");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_IndexOf_FindsTestsByGroupAndTestName, "IndexOf_FindsTestsByGroupAndTestName");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_Load_ChecksEveryTestAndListsGroupsInOrder,
        "Load_ChecksEveryTestAndListsGroupsInOrder");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_Load_ListsEachSuiteAfterTheSuitesHoldingIt,
        "Load_ListsEachSuiteAfterTheSuitesHoldingIt");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_NameInSuite_DropsTheSuitePath, "NameInSuite_DropsTheSuitePath");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_ParentSuitePath_DropsTheLastSuiteName,
        "ParentSuitePath_DropsTheLastSuiteName");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_ResetResults_SetsEveryTestBackToNotRun,
        "ResetResults_SetsEveryTestBackToNotRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetAllChecked_ChecksOrUnchecksEveryTest,
        "SetAllChecked_ChecksOrUnchecksEveryTest");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetChecked_UpdatesItsGroupsCheckState, "SetChecked_UpdatesItsGroupsCheckState");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetGroupChecked_ChangesOnlyThatGroup, "SetGroupChecked_ChangesOnlyThatGroup");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetResult_StoresTheResultAndSetsTheStatus,
        "SetResult_StoresTheResultAndSetsTheStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetSuiteChecked_ChangesTheSuiteAndTheSuitesInIt,
        "SetSuiteChecked_ChangesTheSuiteAndTheSuitesInIt");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_HidesNonMatchingTests,
        "SetVisibleFilter_HidesNonMatchingTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SetVisibleFilter_LimitsCheckingToVisibleTests,
        "SetVisibleFilter_LimitsCheckingToVisibleTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusCount_CountsTestsWithEachStatus, "StatusCount_CountsTestsWithEachStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusFromOutcome_MapsEachOutcome, "StatusFromOutcome_MapsEachOutcome");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_StatusName_NamesEachStatus, "StatusName_NamesEachStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SuiteDetailText_CountsEachStatus, "SuiteDetailText_CountsEachStatus");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_SuiteStatus_IncludesTheSuitesInIt, "SuiteStatus_IncludesTheSuitesInIt");
    RegisterTest(&TTest_ASWUnitTests_GUI_TestList::Test_TestsInSuite_IncludesNestedSuitesOnly,
        "TestsInSuite_IncludesNestedSuitesOnly");
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
    AssertNotNull(filter, __func__, __LINE__, "a filter is returned");
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
    CheckStartsWith(text, "Alpha.Two\n", __func__, __LINE__, "it starts with the full name");
    CheckContains(text, "Result: Failed (0.412 ms)", __func__, __LINE__,
        "then the status and duration in milliseconds");
    CheckContains(text, "\nCheck failed for: needle\n", __func__, __LINE__, "then the failure detail");
    CheckContains(text, "\nLog:\nRunning test: Alpha.Two\n", __func__, __LINE__, "then the test's own log");
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
    CheckEquals(TGUITestStatus::Passed, list.GroupStatus("Alpha"), __func__, __LINE__,
        "one test passed, the other not run: passed");

    list.SetStatus(1, TGUITestStatus::Running);
    CheckEquals(TGUITestStatus::Running, list.GroupStatus("Alpha"), __func__, __LINE__, "running while any test is");

    list.SetStatus(1, TGUITestStatus::Skipped);
    CheckEquals(TGUITestStatus::Skipped, list.GroupStatus("Alpha"), __func__, __LINE__, "a skip outranks a pass");

    list.SetStatus(0, TGUITestStatus::Failed);
    CheckEquals(TGUITestStatus::Failed, list.GroupStatus("Alpha"), __func__, __LINE__, "a failure outranks a skip");

    CheckEquals(TGUITestStatus::NotRun, beforeRun, __func__, __LINE__, "nothing run yet: not run");
    CheckEquals(TGUITestStatus::NotRun, list.GroupStatus("Alpha.Sub"), __func__, __LINE__,
        "another group, even one whose name starts with this one's, is unaffected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_GroupSuitePath_GivesEachGroupsSuite()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());

    // Act and Assert
    CheckEquals(std::string("Outer.Inner"), list.GroupSuitePath("Outer.Inner.Deep"), __func__, __LINE__,
        "a group in a nested suite gives the whole suite path");
    CheckEquals(std::string("Outer"), list.GroupSuitePath("Outer.Mid"), __func__, __LINE__,
        "a group in a top-level suite gives that suite");
    CheckEmpty(list.GroupSuitePath("Top"), __func__, __LINE__, "a group in no suite gives an empty path");
    CheckEmpty(list.GroupSuitePath("Missing"), __func__, __LINE__, "so does a group that isn't in the list");
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
    CheckEmpty(list.SuitePaths(), __func__, __LINE__, "groups in no suite list no suites");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_Load_ListsEachSuiteAfterTheSuitesHoldingIt()
{
    // Arrange
    TGUITestList list;

    // Act
    list.Load(SuiteTests());

    // Assert
    CheckTrue(list.SuitePaths() == std::vector<std::string>{ "Outer", "Outer.Inner", "Other", "Outermost" },
        __func__, __LINE__, "each suite is listed once, after the suite holding it, in the order its first test "
        "appears, including \"Outer\", whose first test is in a suite inside it");

    std::optional<size_t> const inner = list.SuiteIndexOf("Outer.Inner");
    CheckTrue(inner.has_value() && *inner == 1, __func__, __LINE__, "a suite is found by its path");
    CheckFalse(list.SuiteIndexOf("Inner").has_value(), __func__, __LINE__, "but not by its own name alone");
    CheckFalse(list.SuiteIndexOf("Top").has_value(), __func__, __LINE__, "and a group isn't a suite");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_NameInSuite_DropsTheSuitePath()
{
    CheckEquals(std::string("Deep"), NameInSuite("Outer.Inner.Deep", "Outer.Inner"), __func__, __LINE__,
        "a group's own name, without its suite path");
    CheckEquals(std::string("Inner"), NameInSuite("Outer.Inner", "Outer"), __func__, __LINE__,
        "a suite's own name, without the path of the suite holding it");
    CheckEquals(std::string("Alpha.Sub"), NameInSuite("Alpha.Sub", ""), __func__, __LINE__,
        "the whole name when it isn't in a suite, even one containing '.'");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_ParentSuitePath_DropsTheLastSuiteName()
{
    CheckEquals(std::string("Outer.Inner"), ParentSuitePath("Outer.Inner.Deep"), __func__, __LINE__,
        "a nested suite's parent is the path before its last '.'");
    CheckEquals(std::string("Outer"), ParentSuitePath("Outer.Inner"), __func__, __LINE__,
        "likewise one level up");
    CheckEmpty(ParentSuitePath("Outer"), __func__, __LINE__, "a top-level suite has no parent");
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
    CheckNull(list.Result(2), __func__, __LINE__, "and no longer has a result");
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
    CheckEquals(TGUICheckState::Partial, oneUnchecked, __func__, __LINE__, "some of a group's tests checked: partial");
    CheckEquals(TGUICheckState::Unchecked, bothUnchecked, __func__, __LINE__, "none checked: unchecked");
    CheckEquals(TGUICheckState::Checked, list.GroupCheckState("Beta"), __func__, __LINE__,
        "another group is unaffected");
    CheckEquals(TGUICheckState::Checked, list.GroupCheckState("Alpha.Sub"), __func__, __LINE__,
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
    CheckNull(before, __func__, __LINE__, "a test has no result before it finishes");
    TGUITestResult const* const after = list.Result(2);
    AssertNotNull(after, __func__, __LINE__, "it has one after");
    CheckEquals(std::string("not on this platform"), after->Record.Message, __func__, __LINE__, "with its record");
    CheckEquals(std::string("log"), after->Log, __func__, __LINE__, "and its log");
    CheckEquals(TGUITestStatus::Skipped, list.Status(2), __func__, __LINE__, "its status follows the outcome");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SetSuiteChecked_ChangesTheSuiteAndTheSuitesInIt()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());

    // Act
    list.SetSuiteChecked("Outer", false);

    // Assert
    CheckFalse(list.IsChecked(1) || list.IsChecked(2) || list.IsChecked(3), __func__, __LINE__,
        "the suite's own group and the groups in the suite inside it are unchecked");
    CheckTrue(list.IsChecked(0) && list.IsChecked(4) && list.IsChecked(5), __func__, __LINE__,
        "tests outside it, including a suite whose path only starts with its path, are unchanged");
    CheckEquals(TGUICheckState::Unchecked, list.SuiteCheckState("Outer"), __func__, __LINE__,
        "the suite shows unchecked");
    CheckEquals(TGUICheckState::Unchecked, list.SuiteCheckState("Outer.Inner"), __func__, __LINE__,
        "and so does the suite inside it");
    CheckEquals(TGUICheckState::Checked, list.SuiteCheckState("Outermost"), __func__, __LINE__,
        "while a suite with a similar path stays checked");

    // Act and Assert: one test inside the nested suite checked again.
    list.SetChecked(1, true);
    CheckEquals(TGUICheckState::Partial, list.SuiteCheckState("Outer"), __func__, __LINE__,
        "the outer suite is partly checked");
    CheckEquals(TGUICheckState::Partial, list.SuiteCheckState("Outer.Inner"), __func__, __LINE__,
        "and so is the suite holding that test");
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
    CheckEquals(TGUICheckState::Checked, list.GroupCheckState("Alpha"), __func__, __LINE__,
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
    CheckEquals(TGUITestStatus::Passed, StatusFromOutcome(TTestOutcome::Pass), __func__, __LINE__, "Pass: Passed");
    CheckEquals(TGUITestStatus::Failed, StatusFromOutcome(TTestOutcome::Fail), __func__, __LINE__, "Fail: Failed");
    CheckEquals(TGUITestStatus::Skipped, StatusFromOutcome(TTestOutcome::Skip), __func__, __LINE__, "Skip: Skipped");
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
void TTest_ASWUnitTests_GUI_TestList::Test_SuiteDetailText_CountsEachStatus()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());
    list.SetStatus(1, TGUITestStatus::Failed);
    list.SetStatus(3, TGUITestStatus::Passed);

    // Act
    std::string const text = list.SuiteDetailText("Outer");

    // Assert
    CheckEquals(std::string("Outer\n3 tests: 1 passed, 1 failed, 0 skipped, 1 not run\n"), text, __func__,
        __LINE__, "the suite's path, then counts over its tests, including those in the suite inside it");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_SuiteStatus_IncludesTheSuitesInIt()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());
    TGUITestStatus const beforeRun = list.SuiteStatus("Outer");

    // Act, and Assert as the suite's tests progress
    list.SetStatus(3, TGUITestStatus::Passed);
    CheckEquals(TGUITestStatus::Passed, list.SuiteStatus("Outer"), __func__, __LINE__,
        "its own group's test passed, the others not run: passed");

    list.SetStatus(1, TGUITestStatus::Failed);
    CheckEquals(TGUITestStatus::Failed, list.SuiteStatus("Outer"), __func__, __LINE__,
        "a failure in the suite inside it fails it too");
    CheckEquals(TGUITestStatus::Failed, list.SuiteStatus("Outer.Inner"), __func__, __LINE__,
        "as it does the suite holding the failed test");

    list.SetStatus(2, TGUITestStatus::Running);
    CheckEquals(TGUITestStatus::Running, list.SuiteStatus("Outer"), __func__, __LINE__,
        "running while any test in it is");

    CheckEquals(TGUITestStatus::NotRun, beforeRun, __func__, __LINE__, "nothing run yet: not run");
    CheckEquals(TGUITestStatus::NotRun, list.SuiteStatus("Outermost"), __func__, __LINE__,
        "a suite whose path only starts with this one's is unaffected");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_TestList::Test_TestsInSuite_IncludesNestedSuitesOnly()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());

    // Act and Assert
    CheckTrue(list.TestsInSuite("Outer") == std::vector<size_t>{ 1, 2, 3 }, __func__, __LINE__,
        "a suite's tests include those in the suite inside it, in order");
    CheckTrue(list.TestsInSuite("Outer.Inner") == std::vector<size_t>{ 1, 2 }, __func__, __LINE__,
        "a nested suite's tests are only its own");
    CheckTrue(list.TestsInSuite("Outermost") == std::vector<size_t>{ 5 }, __func__, __LINE__,
        "a suite whose path starts with another's isn't inside it");
    CheckEmpty(list.TestsInSuite("Out"), __func__, __LINE__, "a partial suite name matches nothing");

    list.SetVisibleFilter([](std::string const& fullName)
        {
            return fullName != "Outer.Inner.Deep.Two";
        });
    CheckTrue(list.TestsInSuite("Outer") == std::vector<size_t>{ 2, 3 }, __func__, __LINE__,
        "only visible tests are included");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_TestList)
