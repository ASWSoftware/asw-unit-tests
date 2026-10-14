/* **************************************************************************
ASWUnitTests_GUI_TestList.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_GUI_TestListH
#define ASWUnitTests_GUI_TestListH
//---------------------------------------------------------------------------
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUICheckState
//
// A group's check state in the VCL GUI runner's test tree: all, none, or
// only some of its tests checked.
/////////////////////////////////////////////////////////////////////////////
enum class TGUICheckState
{
    Unchecked,
    Checked,
    Partial
};


/////////////////////////////////////////////////////////////////////////////
// TGUITestStatus
//
// A test's status in the VCL GUI runner's current or latest run, in the
// order of the tree's status image list, so a status converts directly to
// its image index.
/////////////////////////////////////////////////////////////////////////////
enum class TGUITestStatus
{
    NotRun,
    Running,
    Passed,
    Failed,
    Skipped
};

// 'qualifiedName' (a group's qualified name, or a suite's path) without the 'suitePath' it's in and the '.'
// after it, e.g. "TTest_TConsole" for "Framework.Console.TTest_TConsole" in "Framework.Console"; all of it
// when 'suitePath' is empty.
std::string NameInSuite(std::string const& qualifiedName, std::string const& suitePath);
// The path of the suite holding suite 'suitePath', e.g. "Framework" for "Framework.Console"; empty for a
// top-level suite.
std::string ParentSuitePath(std::string const& suitePath);
// The status a finished test's recorded outcome corresponds to.
TGUITestStatus StatusFromOutcome(TTestOutcome outcome);
// A status as shown to the user, e.g. "Not run".
std::string StatusName(TGUITestStatus status);


/////////////////////////////////////////////////////////////////////////////
// TGUITestResult
//
// What the VCL GUI runner keeps about a test that finished in the current
// or latest run: its record, and the text it logged while running.
/////////////////////////////////////////////////////////////////////////////
struct TGUITestResult
{
    TTestCaseRecord Record;
    std::string Log;
};


/////////////////////////////////////////////////////////////////////////////
// TGUITestList
//
// The VCL GUI runner's list of registered tests and which of them are
// checked to run, and each one's status and result, independent of how the
// tree displays them. Tests are identified by their index, in the canonical order
// TTestHandler::GetTests() returns them, and groups (by qualified name) and
// suites (by path, including suites that only hold other suites) appear in
// the order their first test does. A suite's tests are those of every group
// in it or in the suites inside it. Contains no VCL code, so it's unit
// tested from the VCL console runner (see
// vcl/tests/Test_ASWUnitTests_GUI_TestList.cpp).
//
// Tests can be hidden by the filter box (see SetVisibleFilter()). While
// some are, everything about checking and grouping applies only to the
// visible ones, so nothing the user can't see is checked, unchecked, run,
// or counted by accident; hidden tests keep their check state for when
// they're shown again.
/////////////////////////////////////////////////////////////////////////////
class TGUITestList
{
private:
    std::vector<bool> m_Checked;
    std::vector<std::string> m_GroupNames;
    std::map<std::string, std::string> m_GroupSuitePaths;
    std::map<std::pair<std::string, std::string>, size_t> m_IndexByName;
    std::vector<std::optional<TGUITestResult> > m_Results;
    std::vector<TGUITestStatus> m_Statuses;
    std::map<std::string, size_t> m_SuiteIndexByPath;
    std::vector<std::string> m_SuitePaths;
    std::vector<TTestId> m_Tests;
    std::vector<bool> m_Visible;

private:
    // The check state of the tests at 'indexes': all, none, or only some of them checked.
    TGUICheckState CheckStateOf(std::vector<size_t> const& indexes) const;
    // A group's or suite's detail pane text: its name, then how many of the tests at 'indexes' have each status.
    std::string DetailTextOf(std::string const& name, std::vector<size_t> const& indexes) const;
    // The status shown for the tests at 'indexes' together; see GroupStatus().
    TGUITestStatus StatusOf(std::vector<size_t> const& indexes) const;

public:
    // Number of visible tests that are checked.
    size_t CheckedCount() const;
    // A filter matching exactly the visible tests that are checked, for TTestHandler::Run().
    TestFilter CheckedFilter() const;
    // Checks exactly the tests 'filter' matches by full name, or all of them if it's empty, e.g. to start
    // from the command line's --filter and partition options.
    void CheckMatching(TestFilter const& filter);
    // Checks the visible tests that failed in this run and unchecks the other visible tests, for Select Failed.
    void CheckOnlyFailed();
    size_t Count() const;
    // Number of tests 'filter' matches by full name, or all of them if it's empty.
    size_t CountMatching(TestFilter const& filter) const;
    // The detail pane's text for a test: its full name and status, and once it has finished, its duration,
    // failure or skip detail, and log.
    std::string DetailText(size_t index) const;
    // A filter matching every test that failed in this run, visible or not, for Run Failed.
    TestFilter FailedFilter() const;
    // A group's check state, from its visible tests; Unchecked if none are visible.
    TGUICheckState GroupCheckState(std::string const& groupName) const;
    // The detail pane's text for a group: its name and how many of its visible tests have each status.
    std::string GroupDetailText(std::string const& groupName) const;
    std::vector<std::string> const& GroupNames() const;
    // The status shown for a group, from its visible tests: Running while any of them is; otherwise the worst
    // outcome among those that ran (Failed, then Skipped, then Passed); NotRun if none did.
    TGUITestStatus GroupStatus(std::string const& groupName) const;
    // The path of the suite a group is in; empty if it isn't in one, or isn't a group in the list.
    std::string GroupSuitePath(std::string const& groupName) const;
    // The index of the test with these names, or std::nullopt if there isn't one.
    std::optional<size_t> IndexOf(std::string const& groupName, std::string const& testName) const;
    bool IsChecked(size_t index) const;
    bool IsVisible(size_t index) const;
    // Replaces the list with 'tests', all checked, visible, and not run.
    void Load(std::vector<TTestId> const& tests);
    // Sets every test back to NotRun with no result, e.g. at the start of a run.
    void ResetResults();
    // The result of a test that has finished in this run, or nullptr if it hasn't.
    TGUITestResult const* Result(size_t index) const;
    // Checks or unchecks every visible test.
    void SetAllChecked(bool checked);
    void SetChecked(size_t index, bool checked);
    // Checks or unchecks a group's visible tests.
    void SetGroupChecked(std::string const& groupName, bool checked);
    // Stores a finished test's result, and sets its status from the record's outcome.
    void SetResult(size_t index, TTestCaseRecord const& record, std::string const& log);
    void SetStatus(size_t index, TGUITestStatus status);
    // Checks or unchecks a suite's visible tests, including those in the suites inside it.
    void SetSuiteChecked(std::string const& suitePath, bool checked);
    // Shows only the tests 'filter' matches by full name, or all of them if it's empty.
    void SetVisibleFilter(TestFilter const& filter);
    TGUITestStatus Status(size_t index) const;
    // Number of tests with this status, visible or not.
    size_t StatusCount(TGUITestStatus status) const;
    // A suite's check state, from its visible tests, like GroupCheckState().
    TGUICheckState SuiteCheckState(std::string const& suitePath) const;
    // The detail pane's text for a suite: its path and how many of its visible tests have each status.
    std::string SuiteDetailText(std::string const& suitePath) const;
    // The index of a suite in SuitePaths(), or std::nullopt if there isn't one with this path.
    std::optional<size_t> SuiteIndexOf(std::string const& suitePath) const;
    // Every suite's path, each after the suite holding it.
    std::vector<std::string> const& SuitePaths() const;
    // The status shown for a suite, from its visible tests, like GroupStatus().
    TGUITestStatus SuiteStatus(std::string const& suitePath) const;
    TTestId const& Test(size_t index) const;
    // Indexes of 'groupName's visible tests, in order.
    std::vector<size_t> TestsInGroup(std::string const& groupName) const;
    // Indexes of a suite's visible tests, including those in the suites inside it, in order.
    std::vector<size_t> TestsInSuite(std::string const& suitePath) const;
    // Number of visible tests.
    size_t VisibleCount() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_TestListH
