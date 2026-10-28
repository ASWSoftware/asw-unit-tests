/* **************************************************************************
ASWUnitTests_GUI_Tree.h
Author: Anthony S. West - ASW Software

What a GUI runner's test tree shows, independent of the tree control that
shows it.

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
#ifndef ASWUnitTests_GUI_TreeH
#define ASWUnitTests_GUI_TreeH
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUINodeKind
//
// What a test tree node stands for.
/////////////////////////////////////////////////////////////////////////////
enum class TGUINodeKind
{
    Suite,
    Group,
    Test,
};


/////////////////////////////////////////////////////////////////////////////
// TGUINodeRef
//
// Which suite, group or test a test tree node stands for: its kind, and an
// index, a suite's into TGUITestList::SuitePaths(), a group's into its
// GroupNames(), and a test's into the list itself. A tree control keeps it
// in each node's data, encoded by EncodeGUINodeRef().
/////////////////////////////////////////////////////////////////////////////
struct TGUINodeRef
{
    size_t Index = 0;
    TGUINodeKind Kind = TGUINodeKind::Test;
};


/////////////////////////////////////////////////////////////////////////////
// TGUITreeNode
//
// One node of the test tree, as BuildGUITree() returns it.
/////////////////////////////////////////////////////////////////////////////
struct TGUITreeNode
{
    // The parent node's position in BuildGUITree()'s result, which is always before this one's; empty for a
    // top-level node.
    std::optional<size_t> Parent;
    TGUINodeRef Ref;
    // The node's caption: a suite's or group's own name within the suite holding it, or a test's name.
    std::string Text;
};

//---------------------------------------------------------------------------

// A GUI runner's test tree, independent of the tree control that shows it. Contains no UI code, so it's unit
// tested from the console runners (see gui/tests/Test_ASWUnitTests_GUI_Tree.cpp).

// The test tree's nodes for 'testList's visible tests, each after its parent, so a tree control can add them in
// order: one node per group, in the list's order, holding one node per test, and one node per suite, holding
// the nodes of the groups and suites in it, added just before its first group's. A group or suite only gets a
// node if any of its tests are visible.
std::vector<TGUITreeNode> BuildGUITree(TGUITestList const& testList);
// The node reference EncodeGUINodeRef() encoded.
TGUINodeRef DecodeGUINodeRef(std::uintptr_t data);
// 'ref' as one number, for a tree node's data: its index times four, plus its kind.
std::uintptr_t EncodeGUINodeRef(TGUINodeRef const& ref);
// A node's check box: a test's from whether it's checked, and a group's or suite's from its visible tests.
TGUICheckState GUINodeCheckState(TGUITestList const& testList, TGUINodeRef const& ref);
// The detail pane's text for a node's test, group, or suite.
std::string GUINodeDetailText(TGUITestList const& testList, TGUINodeRef const& ref);
// The status shown for a node: a test's own, or a group's or suite's from its visible tests.
TGUITestStatus GUINodeStatus(TGUITestList const& testList, TGUINodeRef const& ref);
// Checks or unchecks a node's test, or every visible test of its group or suite, as a click on its check box
// does.
void SetGUINodeChecked(TGUITestList& testList, TGUINodeRef const& ref, bool checked);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_TreeH
