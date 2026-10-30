/* **************************************************************************
Test_ASWUnitTests_GUI_Tree.cpp
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
#include "Test_ASWUnitTests_GUI_Tree.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_TestList.h"
#include "ASWUnitTests_GUI_Tree.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

//---------------------------------------------------------------------------

std::string DescribeTree(std::vector<TGUITreeNode> const& nodes);
std::vector<TTestId> SuiteTests();

//---------------------------------------------------------------------------
/*
    DescribeTree

    One line per node, in order: its parent's position ("-" for none), its kind and index, and its text, e.g.
    "0 Test 1 Two". Compared whole, so a failure shows the entire tree.
*/
std::string DescribeTree(std::vector<TGUITreeNode> const& nodes)
{
    std::string text;

    for (TGUITreeNode const& node : nodes)
    {
        char const* const kind = (node.Ref.Kind == TGUINodeKind::Suite) ? "Suite" :
                (node.Ref.Kind == TGUINodeKind::Group) ? "Group" : "Test";
        text += (node.Parent.has_value() ? std::to_string(*node.Parent) : std::string("-")) + " " + kind + " " +
            std::to_string(node.Ref.Index) + " " + node.Text + "\n";
    }

    return text;
}
//---------------------------------------------------------------------------
/*
    SuiteTests

    Seven tests: index 0 in top-level group "Top"; 1 and 2 in group "Deep" in suite "Outer.Inner"; 3 in group
    "Mid" in suite "Outer"; 4 in group "Solo" in suite "Other"; 5 in group "Near" in suite "Outermost", whose
    path starts with "Outer" but isn't inside it; and 6 in top-level group "Last".
*/
std::vector<TTestId> SuiteTests()
{
    return { { "Top", "One", "" }, { "Outer.Inner.Deep", "Two", "Outer.Inner" },
        { "Outer.Inner.Deep", "Three", "Outer.Inner" }, { "Outer.Mid", "Four", "Outer" },
        { "Other.Solo", "Five", "Other" }, { "Outermost.Near", "Six", "Outermost" }, { "Last", "Seven", "" } };
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Tree
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Tree::TTest_ASWUnitTests_GUI_Tree()
    : inherited("ASWUnitTests_GUI_Tree_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_EmptyListHasNoNodes, "BuildGUITree_EmptyListHasNoNodes");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_GroupsWithoutSuitesAreTopLevel,
        "BuildGUITree_GroupsWithoutSuitesAreTopLevel");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_LeavesOutHiddenTests, "BuildGUITree_LeavesOutHiddenTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_NestsSuites, "BuildGUITree_NestsSuites");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_EncodeGUINodeRef_RoundTrips, "EncodeGUINodeRef_RoundTrips");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_GUINodeCheckState_FollowsTheList, "GUINodeCheckState_FollowsTheList");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_GUINodeDetailText_FollowsTheList, "GUINodeDetailText_FollowsTheList");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_GUINodeStatus_FollowsTheList, "GUINodeStatus_FollowsTheList");
    RegisterTest(&TTest_ASWUnitTests_GUI_Tree::Test_SetGUINodeChecked_ChecksTheNodesTests,
        "SetGUINodeChecked_ChecksTheNodesTests");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Tree::~TTest_ASWUnitTests_GUI_Tree()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_EmptyListHasNoNodes()
{
    // Arrange
    TGUITestList list;
    list.Load(std::vector<TTestId>());

    // Act and Assert
    CheckTrue(BuildGUITree(list).empty(), __func__, __LINE__, "no tests, no nodes");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_GroupsWithoutSuitesAreTopLevel()
{
    // Arrange: a group whose name has a '.' but isn't in a suite.
    TGUITestList list;
    list.Load({ { "Alpha", "One" }, { "Alpha", "Two" }, { "Beta", "Three" }, { "Alpha.Sub", "Four" } });

    // Act
    std::vector<TGUITreeNode> const nodes = BuildGUITree(list);

    // Assert
    CheckEquals(std::string("- Group 0 Alpha\n"
        "0 Test 0 One\n"
        "0 Test 1 Two\n"
        "- Group 1 Beta\n"
        "3 Test 2 Three\n"
        "- Group 2 Alpha.Sub\n"
        "5 Test 3 Four\n"), DescribeTree(nodes), __func__, __LINE__,
        "each group at the top level, with its full name, holding its tests, in the list's order");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_LeavesOutHiddenTests()
{
    // Arrange: only test 3 (group "Mid" in suite "Outer") and test 6 (top-level group "Last") are visible.
    TGUITestList list;
    list.Load(SuiteTests());
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName == "Outer.Mid.Four" || fullTestName == "Last.Seven";
        });
    std::string const outer = std::to_string(list.SuiteIndexOf("Outer").value());

    // Act
    std::vector<TGUITreeNode> const nodes = BuildGUITree(list);

    // Assert
    CheckEquals("- Suite " + outer + " Outer\n"
        "0 Group 2 Mid\n"
        "1 Test 3 Four\n"
        "- Group 5 Last\n"
        "3 Test 6 Seven\n", DescribeTree(nodes), __func__, __LINE__,
        "groups and suites with no visible tests get no node, including suite Outer.Inner inside a shown suite");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_BuildGUITree_NestsSuites()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());
    auto const suite = [&list](std::string const& path)
        {
            return std::to_string(list.SuiteIndexOf(path).value());
        };

    // Act
    std::vector<TGUITreeNode> const nodes = BuildGUITree(list);

    // Assert
    CheckEquals("- Group 0 Top\n"
        "0 Test 0 One\n"
        "- Suite " + suite("Outer") + " Outer\n"
        "2 Suite " + suite("Outer.Inner") + " Inner\n"
        "3 Group 1 Deep\n"
        "4 Test 1 Two\n"
        "4 Test 2 Three\n"
        "2 Group 2 Mid\n"
        "7 Test 3 Four\n"
        "- Suite " + suite("Other") + " Other\n"
        "9 Group 3 Solo\n"
        "10 Test 4 Five\n"
        "- Suite " + suite("Outermost") + " Outermost\n"
        "12 Group 4 Near\n"
        "13 Test 5 Six\n"
        "- Group 5 Last\n"
        "15 Test 6 Seven\n", DescribeTree(nodes), __func__, __LINE__,
        "each suite just before its first group, holding its suites and groups by their own names");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_EncodeGUINodeRef_RoundTrips()
{
    // Arrange
    std::vector<TGUINodeRef> const refs{ { 0, TGUINodeKind::Suite }, { 0, TGUINodeKind::Group },
        { 0, TGUINodeKind::Test }, { 5, TGUINodeKind::Group }, { 123456, TGUINodeKind::Test } };

    for (TGUINodeRef const& ref : refs)
    {
        // Act
        TGUINodeRef const decoded = DecodeGUINodeRef(EncodeGUINodeRef(ref));

        // Assert
        std::string const which = std::to_string(ref.Index) + "/" + std::to_string(static_cast<int>(ref.Kind));
        CheckEquals(ref.Index, decoded.Index, __func__, __LINE__, "index survives: " + which);
        CheckTrue(ref.Kind == decoded.Kind, __func__, __LINE__, "kind survives: " + which);
    }

    CheckEquals(std::uintptr_t(21), EncodeGUINodeRef(TGUINodeRef{ 5, TGUINodeKind::Group }), __func__, __LINE__,
        "the index times four, plus the kind");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_GUINodeCheckState_FollowsTheList()
{
    // Arrange: test 1 unchecked, so group Deep (tests 1 and 2) and suite Outer (1 to 3) are partly checked.
    TGUITestList list;
    list.Load(SuiteTests());
    list.SetChecked(1, false);
    size_t const outer = list.SuiteIndexOf("Outer").value();
    size_t const other = list.SuiteIndexOf("Other").value();

    // Act and Assert
    CheckTrue(GUINodeCheckState(list, { 0, TGUINodeKind::Test }) == TGUICheckState::Checked, __func__, __LINE__,
        "a checked test");
    CheckTrue(GUINodeCheckState(list, { 1, TGUINodeKind::Test }) == TGUICheckState::Unchecked, __func__, __LINE__,
        "an unchecked test");
    CheckTrue(GUINodeCheckState(list, { 1, TGUINodeKind::Group }) == TGUICheckState::Partial, __func__, __LINE__,
        "a partly checked group");
    CheckTrue(GUINodeCheckState(list, { outer, TGUINodeKind::Suite }) == TGUICheckState::Partial, __func__, __LINE__,
        "a partly checked suite");
    CheckTrue(GUINodeCheckState(list, { other, TGUINodeKind::Suite }) == TGUICheckState::Checked, __func__, __LINE__,
        "a fully checked suite");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_GUINodeDetailText_FollowsTheList()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());
    size_t const outer = list.SuiteIndexOf("Outer").value();

    // Act and Assert
    CheckEquals(list.DetailText(3), GUINodeDetailText(list, { 3, TGUINodeKind::Test }), __func__, __LINE__,
        "a test's own detail");
    CheckEquals(list.GroupDetailText("Outer.Inner.Deep"), GUINodeDetailText(list, { 1, TGUINodeKind::Group }),
        __func__, __LINE__, "a group's");
    CheckEquals(list.SuiteDetailText("Outer"), GUINodeDetailText(list, { outer, TGUINodeKind::Suite }), __func__,
        __LINE__, "a suite's");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_GUINodeStatus_FollowsTheList()
{
    // Arrange: test 1 passed and test 2 failed, in group Deep in suite Outer.Inner, so the group and suite each
    // show a status other than test 1's, though their indexes are 1 too.
    TGUITestList list;
    list.Load(SuiteTests());
    list.SetStatus(1, TGUITestStatus::Passed);
    list.SetStatus(2, TGUITestStatus::Failed);
    size_t const inner = list.SuiteIndexOf("Outer.Inner").value();

    // Act and Assert
    CheckTrue(GUINodeStatus(list, { 1, TGUINodeKind::Test }) == TGUITestStatus::Passed, __func__, __LINE__,
        "a test's own status");
    CheckTrue(GUINodeStatus(list, { 1, TGUINodeKind::Group }) == TGUITestStatus::Failed, __func__, __LINE__,
        "a group's, from its tests");
    CheckTrue(GUINodeStatus(list, { inner, TGUINodeKind::Suite }) == TGUITestStatus::Failed, __func__, __LINE__,
        "a suite's, from its tests");
    CheckTrue(GUINodeStatus(list, { 0, TGUINodeKind::Group }) == TGUITestStatus::NotRun, __func__, __LINE__,
        "a group none of whose tests ran");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Tree::Test_SetGUINodeChecked_ChecksTheNodesTests()
{
    // Arrange
    TGUITestList list;
    list.Load(SuiteTests());
    size_t const outer = list.SuiteIndexOf("Outer").value();

    // Act and Assert
    SetGUINodeChecked(list, { outer, TGUINodeKind::Suite }, false);
    CheckFalse(list.IsChecked(1) || list.IsChecked(2) || list.IsChecked(3), __func__, __LINE__,
        "unchecking suite Outer unchecks every test in it and in the suites inside it");
    CheckTrue(list.IsChecked(0) && list.IsChecked(5), __func__, __LINE__,
        "but not the tests outside it, even in suite Outermost");

    SetGUINodeChecked(list, { 1, TGUINodeKind::Group }, true);
    CheckTrue(list.IsChecked(1) && list.IsChecked(2), __func__, __LINE__, "checking group Deep checks its tests");
    CheckFalse(list.IsChecked(3), __func__, __LINE__, "and only them");

    SetGUINodeChecked(list, { 3, TGUINodeKind::Test }, true);
    CheckTrue(list.IsChecked(3), __func__, __LINE__, "checking a test checks just it");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Tree)
