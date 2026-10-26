/* **************************************************************************
ASWUnitTests_GUI_Tree.cpp
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
#include "ASWUnitTests_GUI_Tree.h"
//---------------------------------------------------------------------------
#include <functional>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::vector<TGUITreeNode> BuildGUITree(TGUITestList const& testList)
{
    std::vector<TGUITreeNode> nodes;

    // By index into testList.SuitePaths(): each suite's node's position in 'nodes', once added. A suite's
    // node, and those of the suites holding it, are added just before its first group's, so suites and groups
    // keep the list's order, where each suite's groups are together.
    std::vector<std::optional<size_t> > suiteNodes(testList.SuitePaths().size());
    std::function<std::optional<size_t>(std::string const&)> suiteNode =
        [&](std::string const& suitePath) -> std::optional<size_t>
        {
            if (suitePath.empty())
                return std::nullopt;

            size_t const suiteIndex = testList.SuiteIndexOf(suitePath).value();

            if (!suiteNodes[suiteIndex].has_value())
            {
                std::string const parentPath = ParentSuitePath(suitePath);
                std::optional<size_t> const parent = suiteNode(parentPath);
                nodes.push_back(TGUITreeNode{ parent, TGUINodeRef{ suiteIndex, TGUINodeKind::Suite },
                                              NameInSuite(suitePath, parentPath) });
                suiteNodes[suiteIndex] = nodes.size() - 1;
            }

            return suiteNodes[suiteIndex];
        };

    // Only visible tests get nodes, and a group or suite only gets one if any of its tests are visible.
    std::vector<std::string> const& groupNames = testList.GroupNames();
    for (size_t groupIndex = 0; groupIndex < groupNames.size(); ++groupIndex)
    {
        std::vector<size_t> const testIndexes = testList.TestsInGroup(groupNames[groupIndex]);
        if (testIndexes.empty())
            continue;

        std::string const suitePath = testList.GroupSuitePath(groupNames[groupIndex]);
        std::optional<size_t> const parent = suiteNode(suitePath);
        nodes.push_back(TGUITreeNode{ parent, TGUINodeRef{ groupIndex, TGUINodeKind::Group },
                                      NameInSuite(groupNames[groupIndex], suitePath) });
        size_t const groupNode = nodes.size() - 1;

        for (size_t testIndex : testIndexes)
        {
            nodes.push_back(TGUITreeNode{ groupNode, TGUINodeRef{ testIndex, TGUINodeKind::Test },
                                          testList.Test(testIndex).TestName });
        }
    }

    return nodes;
}
//---------------------------------------------------------------------------
TGUINodeRef DecodeGUINodeRef(std::uintptr_t data)
{
    return TGUINodeRef{ static_cast<size_t>(data / 4), static_cast<TGUINodeKind>(data % 4) };
}
//---------------------------------------------------------------------------
std::uintptr_t EncodeGUINodeRef(TGUINodeRef const& ref)
{
    return static_cast<std::uintptr_t>(ref.Index) * 4 + static_cast<std::uintptr_t>(ref.Kind);
}
//---------------------------------------------------------------------------
TGUICheckState GUINodeCheckState(TGUITestList const& testList, TGUINodeRef const& ref)
{
    switch (ref.Kind)
    {
        case TGUINodeKind::Suite:
            return testList.SuiteCheckState(testList.SuitePaths()[ref.Index]);

        case TGUINodeKind::Group:
            return testList.GroupCheckState(testList.GroupNames()[ref.Index]);

        case TGUINodeKind::Test:
            return testList.IsChecked(ref.Index) ? TGUICheckState::Checked : TGUICheckState::Unchecked;
    }

    return TGUICheckState::Unchecked;
}
//---------------------------------------------------------------------------
std::string GUINodeDetailText(TGUITestList const& testList, TGUINodeRef const& ref)
{
    switch (ref.Kind)
    {
        case TGUINodeKind::Suite:
            return testList.SuiteDetailText(testList.SuitePaths()[ref.Index]);

        case TGUINodeKind::Group:
            return testList.GroupDetailText(testList.GroupNames()[ref.Index]);

        case TGUINodeKind::Test:
            return testList.DetailText(ref.Index);
    }

    return std::string();
}
//---------------------------------------------------------------------------
TGUITestStatus GUINodeStatus(TGUITestList const& testList, TGUINodeRef const& ref)
{
    switch (ref.Kind)
    {
        case TGUINodeKind::Suite:
            return testList.SuiteStatus(testList.SuitePaths()[ref.Index]);

        case TGUINodeKind::Group:
            return testList.GroupStatus(testList.GroupNames()[ref.Index]);

        case TGUINodeKind::Test:
            return testList.Status(ref.Index);
    }

    return TGUITestStatus::NotRun;
}
//---------------------------------------------------------------------------
void SetGUINodeChecked(TGUITestList& testList, TGUINodeRef const& ref, bool checked)
{
    switch (ref.Kind)
    {
        case TGUINodeKind::Suite:
            testList.SetSuiteChecked(testList.SuitePaths()[ref.Index], checked);
            break;

        case TGUINodeKind::Group:
            testList.SetGroupChecked(testList.GroupNames()[ref.Index], checked);
            break;

        case TGUINodeKind::Test:
            testList.SetChecked(ref.Index, checked);
            break;
    }
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
