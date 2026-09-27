/* **************************************************************************
Test_ASWUnitTests_GUI_Layout.h
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
#ifndef Test_ASWUnitTests_GUI_LayoutH
#define Test_ASWUnitTests_GUI_LayoutH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Layout
//
// Exercises the VCL GUI runner's saved window layout
// (vcl/gui/src/ASWUnitTests_GUI_Layout.h). Listed in both vcl/ projects, so
// it also runs from the VCL console runner.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_Layout : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_GUILayoutFilePath_IsPerExeUnderAppData();
    void Test_IsWindowReachable_FalseOffScreen();
    void Test_IsWindowReachable_FalseWhenTitleBarIsHidden();
    void Test_IsWindowReachable_TrueOnAWorkArea();
    void Test_LoadGUILayout_EmptyIniHasNoValues();
    void Test_LoadGUILayout_IgnoresInvalidValues();
    void Test_SaveGUILayout_RemovesMissingValues();
    void Test_SaveGUILayout_RoundTrips();
    void Test_ScaleFrom96_ScalesAndRounds();
    void Test_ScaleTo96_ReversesScaleFrom96();
    void Test_ScreenToWorkspace_ReversesWorkspaceToScreen();
    void Test_WorkspaceToScreen_OffsetsByPrimaryWorkArea();

public:
    TTest_ASWUnitTests_GUI_Layout();
    ~TTest_ASWUnitTests_GUI_Layout() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_LayoutH
