/* **************************************************************************
ASWUnitTests_GUI_Selection.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "ASWUnitTests_GUI_Selection.h"
//---------------------------------------------------------------------------
#include <map>
#include <utility>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
//---------------------------------------------------------------------------

namespace
{

// The selection file's first line, describing the lines after it for anyone who opens it.
char const SelectionFileComment[] =
    "; ASWUnitTests VCL GUI test selection: \"+\" checked or \"-\" unchecked, then the group and test names, "
    "separated by a tab";

bool IsStorableName(std::string const& name);

//---------------------------------------------------------------------------

// Whether 'name' fits on one line of the selection file, without its tab separator or line break.
bool IsStorableName(std::string const& name)
{
    return name.find_first_of("\t\r\n") == std::string::npos;
}

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::vector<TGUISavedTest> CaptureGUISelection(TGUITestList const& testList)
{
    std::vector<TGUISavedTest> tests;
    tests.reserve(testList.Count());

    for (size_t i = 0; i < testList.Count(); ++i)
    {
        TTestId const& test = testList.Test(i);
        tests.push_back(TGUISavedTest{ testList.IsChecked(i) && testList.IsVisible(i), test.GroupName, test.TestName });
    }

    return tests;
}
//---------------------------------------------------------------------------
std::string DescribeGUISelectionRestore(TGUISelectionRestore const& restore)
{
    if (restore.CheckedAll)
        return "None of the saved selection's checked tests exist any more, so every test is checked.";

    std::string text = "Restored the saved selection: " + std::to_string(restore.CheckedCount) +
        (restore.CheckedCount == 1 ? " test" : " tests") + " checked";

    if (restore.MissingCount > 0)
    {
        text += "; " + std::to_string(restore.MissingCount) +
            (restore.MissingCount == 1 ? " saved test no longer exists" : " saved tests no longer exist");
    }

    if (restore.NewCount > 0)
    {
        text += "; " + std::to_string(restore.NewCount) + " new or renamed" + (restore.NewCount == 1 ? " test" : " tests") +
            " (" + std::to_string(restore.NewCheckedCount) + " checked)";
    }

    return text + ".";
}
//---------------------------------------------------------------------------
std::string FormatGUISelection(std::vector<TGUISavedTest> const& tests)
{
    std::string text = std::string(SelectionFileComment) + "\r\n";

    for (TGUISavedTest const& test : tests)
    {
        if (IsStorableName(test.GroupName) && IsStorableName(test.TestName))
            text += (test.Checked ? "+" : "-") + test.GroupName + "\t" + test.TestName + "\r\n";
    }

    return text;
}
//---------------------------------------------------------------------------
System::UnicodeString GUISelectionFilePath()
{
    return System::Sysutils::ChangeFileExt(GUILayoutFilePath(), ".selection");
}
//---------------------------------------------------------------------------
std::vector<TGUISavedTest> ParseGUISelection(std::string const& text)
{
    std::vector<TGUISavedTest> tests;

    // Past a UTF-8 byte order mark, in case the file was edited with something that adds one.
    size_t start = (text.compare(0, 3, "\xEF\xBB\xBF") == 0) ? 3 : 0;

    while (start < text.size())
    {
        size_t end = text.find('\n', start);
        if (end == std::string::npos)
            end = text.size();

        std::string line = text.substr(start, end - start);
        start = end + 1;

        if (!line.empty() && line.back() == '\r')
            line.pop_back();

        size_t const tab = line.find('\t');
        if (line.empty() || (line[0] != '+' && line[0] != '-') || tab == std::string::npos)
            continue;

        tests.push_back(TGUISavedTest{ line[0] == '+', line.substr(1, tab - 1), line.substr(tab + 1) });
    }

    return tests;
}
//---------------------------------------------------------------------------
TGUISelectionRestore RestoreGUISelection(TGUITestList& testList, std::vector<TGUISavedTest> const& saved)
{
    TGUISelectionRestore restore;

    if (saved.empty())
        return restore;

    // By group and test name, so a test saved more than once counts once, with its last state.
    std::map<std::pair<std::string, std::string>, bool> savedChecked;
    for (TGUISavedTest const& test : saved)
        savedChecked[std::make_pair(test.GroupName, test.TestName)] = test.Checked;

    std::map<std::string, bool> groupFullyChecked;
    bool allChecked = true;
    bool anyChecked = false;

    for (auto const& entry : savedChecked)
    {
        bool const checked = entry.second;
        auto const group = groupFullyChecked.emplace(entry.first.first, true).first;
        group->second = group->second && checked;
        allChecked = allChecked && checked;
        anyChecked = anyChecked || checked;
    }

    size_t foundCount = 0;

    for (size_t i = 0; i < testList.Count(); ++i)
    {
        TTestId const& test = testList.Test(i);
        auto const savedTest = savedChecked.find(std::make_pair(test.GroupName, test.TestName));
        bool checked = false;

        if (savedTest != savedChecked.end())
        {
            checked = savedTest->second;
            ++foundCount;
        }
        else
        {
            auto const group = groupFullyChecked.find(test.GroupName);
            checked = (group != groupFullyChecked.end()) ? group->second : allChecked;
            ++restore.NewCount;

            if (checked)
                ++restore.NewCheckedCount;
        }

        testList.SetChecked(i, checked);

        if (checked)
            ++restore.CheckedCount;
    }

    restore.MissingCount = savedChecked.size() - foundCount;

    // An empty selection is no use when the saved one wasn't empty, e.g. after every checked test was renamed.
    if (anyChecked && restore.CheckedCount == 0)
    {
        testList.CheckMatching(TestFilter());
        restore.CheckedAll = true;
        restore.CheckedCount = testList.Count();
    }

    return restore;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
