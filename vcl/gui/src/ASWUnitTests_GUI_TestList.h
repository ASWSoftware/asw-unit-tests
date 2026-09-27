/* **************************************************************************
ASWUnitTests_GUI_TestList.h
Author: Anthony S. West - ASW Software

Copyright 2026 Anthony S. West

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
// TTestHandler::GetTests() returns them, and groups appear in the order
// their first test does. Contains no VCL code, so it's unit tested from the
// VCL console runner (see vcl/tests/Test_ASWUnitTests_GUI_TestList.cpp).
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
    std::map<std::pair<std::string, std::string>, size_t> m_IndexByName;
    std::vector<std::optional<TGUITestResult> > m_Results;
    std::vector<TGUITestStatus> m_Statuses;
    std::vector<TTestId> m_Tests;
    std::vector<bool> m_Visible;

public:
    // Number of visible tests that are checked.
    size_t CheckedCount() const;
    // A filter matching exactly the visible tests that are checked, for TTestHandler::Run().
    TestFilter CheckedFilter() const;
    // Checks exactly the tests 'filter' matches by full name, or all of them if it's empty, e.g. to start
    // from the command line's --filter and partition options.
    void CheckMatching(TestFilter const& filter);
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
    // Shows only the tests 'filter' matches by full name, or all of them if it's empty.
    void SetVisibleFilter(TestFilter const& filter);
    TGUITestStatus Status(size_t index) const;
    // Number of tests with this status, visible or not.
    size_t StatusCount(TGUITestStatus status) const;
    TTestId const& Test(size_t index) const;
    // Indexes of 'groupName's visible tests, in order.
    std::vector<size_t> TestsInGroup(std::string const& groupName) const;
    // Number of visible tests.
    size_t VisibleCount() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_TestListH
