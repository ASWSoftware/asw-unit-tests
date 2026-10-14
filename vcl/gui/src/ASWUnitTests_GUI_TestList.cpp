/* **************************************************************************
ASWUnitTests_GUI_TestList.cpp
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
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <iomanip>
#include <memory>
#include <set>
#include <sstream>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::string NameInSuite(std::string const& qualifiedName, std::string const& suitePath)
{
    if (suitePath.empty())
        return qualifiedName;

    return qualifiedName.substr(std::min(suitePath.size() + 1, qualifiedName.size()));
}
//---------------------------------------------------------------------------
std::string ParentSuitePath(std::string const& suitePath)
{
    // A suite name can't contain '.' (see TSuitePath), so the last one ends the parent's path.
    size_t const dot = suitePath.rfind('.');
    return (dot == std::string::npos) ? std::string() : suitePath.substr(0, dot);
}
//---------------------------------------------------------------------------
TGUITestStatus StatusFromOutcome(TTestOutcome outcome)
{
    switch (outcome)
    {
        case TTestOutcome::Pass: return TGUITestStatus::Passed;
        case TTestOutcome::Fail: return TGUITestStatus::Failed;
        case TTestOutcome::Skip: return TGUITestStatus::Skipped;
    }

    return TGUITestStatus::Failed;
}
//---------------------------------------------------------------------------
std::string StatusName(TGUITestStatus status)
{
    switch (status)
    {
        case TGUITestStatus::NotRun: return "Not run";
        case TGUITestStatus::Running: return "Running";
        case TGUITestStatus::Passed: return "Passed";
        case TGUITestStatus::Failed: return "Failed";
        case TGUITestStatus::Skipped: return "Skipped";
    }

    return "Unknown";
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TGUITestList
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
size_t TGUITestList::CheckedCount() const
{
    size_t count = 0;

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Visible[i] && m_Checked[i])
            ++count;
    }

    return count;
}
//---------------------------------------------------------------------------
TestFilter TGUITestList::CheckedFilter() const
{
    // Shared, not copied per call, since TestFilter is itself copied freely.
    std::shared_ptr<std::set<std::string> > checkedNames = std::make_shared<std::set<std::string> >();

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Visible[i] && m_Checked[i])
            checkedNames->insert(m_Tests[i].GroupName + "." + m_Tests[i].TestName);
    }

    return [checkedNames](std::string const& fullTestName)
        {
            return checkedNames->count(fullTestName) != 0;
        };
}
//---------------------------------------------------------------------------
void TGUITestList::CheckMatching(TestFilter const& filter)
{
    for (size_t i = 0; i < m_Tests.size(); ++i)
        m_Checked[i] = (filter == nullptr) || filter(m_Tests[i].GroupName + "." + m_Tests[i].TestName);
}
//---------------------------------------------------------------------------
void TGUITestList::CheckOnlyFailed()
{
    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Visible[i])
            m_Checked[i] = (m_Statuses[i] == TGUITestStatus::Failed);
    }
}
//---------------------------------------------------------------------------
TGUICheckState TGUITestList::CheckStateOf(std::vector<size_t> const& indexes) const
{
    size_t checked = 0;

    for (size_t index : indexes)
    {
        if (m_Checked[index])
            ++checked;
    }

    if (checked == 0)
        return TGUICheckState::Unchecked;

    return (checked == indexes.size()) ? TGUICheckState::Checked : TGUICheckState::Partial;
}
//---------------------------------------------------------------------------
size_t TGUITestList::Count() const
{
    return m_Tests.size();
}
//---------------------------------------------------------------------------
size_t TGUITestList::CountMatching(TestFilter const& filter) const
{
    if (filter == nullptr)
        return m_Tests.size();

    size_t count = 0;

    for (TTestId const& test : m_Tests)
    {
        if (filter(test.GroupName + "." + test.TestName))
            ++count;
    }

    return count;
}
//---------------------------------------------------------------------------
std::string TGUITestList::DetailText(size_t index) const
{
    TTestId const& test = m_Tests.at(index);
    std::ostringstream text;

    text << test.GroupName << "." << test.TestName << "\n";
    text << "Result: " << StatusName(m_Statuses[index]);

    TGUITestResult const* const result = Result(index);
    if (result == nullptr)
        return text.str() + "\n";

    text << " (" << std::fixed << std::setprecision(3) << (result->Record.DurationSeconds * 1000.0) << " ms)\n";

    if (!result->Record.Message.empty())
        text << "\n" << result->Record.Message << "\n";

    if (!result->Log.empty())
        text << "\nLog:\n" << result->Log;

    return text.str();
}
//---------------------------------------------------------------------------
std::string TGUITestList::DetailTextOf(std::string const& name, std::vector<size_t> const& indexes) const
{
    size_t counts[5] = {}; // By TGUITestStatus.

    for (size_t index : indexes)
        ++counts[static_cast<size_t>(m_Statuses[index])];

    std::ostringstream text;
    text << name << "\n";
    text << indexes.size() << (indexes.size() == 1 ? " test: " : " tests: ");
    text << counts[static_cast<size_t>(TGUITestStatus::Passed)] << " passed, ";
    text << counts[static_cast<size_t>(TGUITestStatus::Failed)] << " failed, ";
    text << counts[static_cast<size_t>(TGUITestStatus::Skipped)] << " skipped, ";
    text << counts[static_cast<size_t>(TGUITestStatus::NotRun)] << " not run";

    if (counts[static_cast<size_t>(TGUITestStatus::Running)] != 0)
        text << ", " << counts[static_cast<size_t>(TGUITestStatus::Running)] << " running";

    text << "\n";
    return text.str();
}
//---------------------------------------------------------------------------
TestFilter TGUITestList::FailedFilter() const
{
    std::shared_ptr<std::set<std::string> > failedNames = std::make_shared<std::set<std::string> >();

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Statuses[i] == TGUITestStatus::Failed)
            failedNames->insert(m_Tests[i].GroupName + "." + m_Tests[i].TestName);
    }

    return [failedNames](std::string const& fullTestName)
        {
            return failedNames->count(fullTestName) != 0;
        };
}
//---------------------------------------------------------------------------
TGUICheckState TGUITestList::GroupCheckState(std::string const& groupName) const
{
    return CheckStateOf(TestsInGroup(groupName));
}
//---------------------------------------------------------------------------
std::string TGUITestList::GroupDetailText(std::string const& groupName) const
{
    return DetailTextOf(groupName, TestsInGroup(groupName));
}
//---------------------------------------------------------------------------
std::vector<std::string> const& TGUITestList::GroupNames() const
{
    return m_GroupNames;
}
//---------------------------------------------------------------------------
TGUITestStatus TGUITestList::GroupStatus(std::string const& groupName) const
{
    return StatusOf(TestsInGroup(groupName));
}
//---------------------------------------------------------------------------
std::string TGUITestList::GroupSuitePath(std::string const& groupName) const
{
    std::map<std::string, std::string>::const_iterator const it = m_GroupSuitePaths.find(groupName);
    return (it == m_GroupSuitePaths.end()) ? std::string() : it->second;
}
//---------------------------------------------------------------------------
std::optional<size_t> TGUITestList::IndexOf(std::string const& groupName, std::string const& testName) const
{
    std::map<std::pair<std::string, std::string>, size_t>::const_iterator const it =
        m_IndexByName.find(std::make_pair(groupName, testName));

    if (it == m_IndexByName.end())
        return std::nullopt;

    return it->second;
}
//---------------------------------------------------------------------------
bool TGUITestList::IsChecked(size_t index) const
{
    return m_Checked.at(index);
}
//---------------------------------------------------------------------------
bool TGUITestList::IsVisible(size_t index) const
{
    return m_Visible.at(index);
}
//---------------------------------------------------------------------------
void TGUITestList::Load(std::vector<TTestId> const& tests)
{
    m_Tests = tests;
    m_Checked.assign(m_Tests.size(), true);
    m_Visible.assign(m_Tests.size(), true);
    m_Statuses.assign(m_Tests.size(), TGUITestStatus::NotRun);
    m_Results.assign(m_Tests.size(), std::nullopt);
    m_GroupNames.clear();
    m_GroupSuitePaths.clear();
    m_IndexByName.clear();
    m_SuiteIndexByPath.clear();
    m_SuitePaths.clear();

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        TTestId const& test = m_Tests[i];

        if (std::find(m_GroupNames.begin(), m_GroupNames.end(), test.GroupName) == m_GroupNames.end())
        {
            m_GroupNames.push_back(test.GroupName);
            m_GroupSuitePaths[test.GroupName] = test.SuitePath;
        }

        m_IndexByName[std::make_pair(test.GroupName, test.TestName)] = i;

        // The suite itself, after each suite holding it, outermost first.
        TSuitePath const suitePath(test.SuitePath);
        std::string path;

        for (std::string const& suiteName : suitePath.GetNames())
        {
            path += (path.empty() ? std::string() : std::string(".")) + suiteName;

            if (m_SuiteIndexByPath.emplace(path, m_SuitePaths.size()).second)
                m_SuitePaths.push_back(path);
        }
    }
}
//---------------------------------------------------------------------------
void TGUITestList::ResetResults()
{
    m_Statuses.assign(m_Tests.size(), TGUITestStatus::NotRun);
    m_Results.assign(m_Tests.size(), std::nullopt);
}
//---------------------------------------------------------------------------
TGUITestResult const* TGUITestList::Result(size_t index) const
{
    std::optional<TGUITestResult> const& result = m_Results.at(index);
    return result.has_value() ? &*result : nullptr;
}
//---------------------------------------------------------------------------
void TGUITestList::SetAllChecked(bool checked)
{
    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Visible[i])
            m_Checked[i] = checked;
    }
}
//---------------------------------------------------------------------------
void TGUITestList::SetChecked(size_t index, bool checked)
{
    m_Checked.at(index) = checked;
}
//---------------------------------------------------------------------------
void TGUITestList::SetGroupChecked(std::string const& groupName, bool checked)
{
    for (size_t index : TestsInGroup(groupName))
        m_Checked[index] = checked;
}
//---------------------------------------------------------------------------
void TGUITestList::SetResult(size_t index, TTestCaseRecord const& record, std::string const& log)
{
    m_Results.at(index) = TGUITestResult{ record, log };
    m_Statuses[index] = StatusFromOutcome(record.Outcome);
}
//---------------------------------------------------------------------------
void TGUITestList::SetStatus(size_t index, TGUITestStatus status)
{
    m_Statuses.at(index) = status;
}
//---------------------------------------------------------------------------
void TGUITestList::SetSuiteChecked(std::string const& suitePath, bool checked)
{
    for (size_t index : TestsInSuite(suitePath))
        m_Checked[index] = checked;
}
//---------------------------------------------------------------------------
void TGUITestList::SetVisibleFilter(TestFilter const& filter)
{
    for (size_t i = 0; i < m_Tests.size(); ++i)
        m_Visible[i] = (filter == nullptr) || filter(m_Tests[i].GroupName + "." + m_Tests[i].TestName);
}
//---------------------------------------------------------------------------
TGUITestStatus TGUITestList::Status(size_t index) const
{
    return m_Statuses.at(index);
}
//---------------------------------------------------------------------------
size_t TGUITestList::StatusCount(TGUITestStatus status) const
{
    return static_cast<size_t>(std::count(m_Statuses.begin(), m_Statuses.end(), status));
}
//---------------------------------------------------------------------------
TGUITestStatus TGUITestList::StatusOf(std::vector<size_t> const& indexes) const
{
    bool anyRunning = false;
    bool anyFailed = false;
    bool anySkipped = false;
    bool anyPassed = false;

    for (size_t index : indexes)
    {
        switch (m_Statuses[index])
        {
            case TGUITestStatus::Running: anyRunning = true; break;
            case TGUITestStatus::Failed: anyFailed = true; break;
            case TGUITestStatus::Skipped: anySkipped = true; break;
            case TGUITestStatus::Passed: anyPassed = true; break;
            case TGUITestStatus::NotRun: break;
        }
    }

    if (anyRunning)
        return TGUITestStatus::Running;

    if (anyFailed)
        return TGUITestStatus::Failed;

    if (anySkipped)
        return TGUITestStatus::Skipped;

    if (anyPassed)
        return TGUITestStatus::Passed;

    return TGUITestStatus::NotRun;
}
//---------------------------------------------------------------------------
TGUICheckState TGUITestList::SuiteCheckState(std::string const& suitePath) const
{
    return CheckStateOf(TestsInSuite(suitePath));
}
//---------------------------------------------------------------------------
std::string TGUITestList::SuiteDetailText(std::string const& suitePath) const
{
    return DetailTextOf(suitePath, TestsInSuite(suitePath));
}
//---------------------------------------------------------------------------
std::optional<size_t> TGUITestList::SuiteIndexOf(std::string const& suitePath) const
{
    std::map<std::string, size_t>::const_iterator const it = m_SuiteIndexByPath.find(suitePath);

    if (it == m_SuiteIndexByPath.end())
        return std::nullopt;

    return it->second;
}
//---------------------------------------------------------------------------
std::vector<std::string> const& TGUITestList::SuitePaths() const
{
    return m_SuitePaths;
}
//---------------------------------------------------------------------------
TGUITestStatus TGUITestList::SuiteStatus(std::string const& suitePath) const
{
    return StatusOf(TestsInSuite(suitePath));
}
//---------------------------------------------------------------------------
TTestId const& TGUITestList::Test(size_t index) const
{
    return m_Tests.at(index);
}
//---------------------------------------------------------------------------
std::vector<size_t> TGUITestList::TestsInGroup(std::string const& groupName) const
{
    std::vector<size_t> indexes;

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        if (m_Visible[i] && m_Tests[i].GroupName == groupName)
            indexes.push_back(i);
    }

    return indexes;
}
//---------------------------------------------------------------------------
std::vector<size_t> TGUITestList::TestsInSuite(std::string const& suitePath) const
{
    std::vector<size_t> indexes;
    std::string const nestedPrefix = suitePath + ".";

    for (size_t i = 0; i < m_Tests.size(); ++i)
    {
        std::string const& testSuitePath = m_Tests[i].SuitePath;

        if (m_Visible[i] && (testSuitePath == suitePath || testSuitePath.rfind(nestedPrefix, 0) == 0))
            indexes.push_back(i);
    }

    return indexes;
}
//---------------------------------------------------------------------------
size_t TGUITestList::VisibleCount() const
{
    return static_cast<size_t>(std::count(m_Visible.begin(), m_Visible.end(), true));
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
