/* **************************************************************************
Test_ASWUnitTests_GUI_Tree.h
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
#ifndef Test_ASWUnitTests_GUI_TreeH
#define Test_ASWUnitTests_GUI_TreeH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Tree
//
// Exercises the GUI runners' test tree (gui/src/ASWUnitTests_GUI_Tree.h).
// Listed in every project, so it also runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_Tree : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_BuildGUITree_EmptyListHasNoNodes();
    void Test_BuildGUITree_GroupsWithoutSuitesAreTopLevel();
    void Test_BuildGUITree_LeavesOutHiddenTests();
    void Test_BuildGUITree_NestsSuites();
    void Test_EncodeGUINodeRef_RoundTrips();
    void Test_GUINodeCheckState_FollowsTheList();
    void Test_GUINodeDetailText_FollowsTheList();
    void Test_GUINodeStatus_FollowsTheList();
    void Test_SetGUINodeChecked_ChecksTheNodesTests();

public:
    TTest_ASWUnitTests_GUI_Tree();
    ~TTest_ASWUnitTests_GUI_Tree() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_TreeH
