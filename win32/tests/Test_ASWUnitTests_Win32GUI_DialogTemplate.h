/* **************************************************************************
Test_ASWUnitTests_Win32GUI_DialogTemplate.h
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
#ifndef Test_ASWUnitTests_Win32GUI_DialogTemplateH
#define Test_ASWUnitTests_Win32GUI_DialogTemplateH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_DialogTemplate
//
// Exercises the in-memory dialog templates of the Win32 GUI runner's
// dialogs (win32/gui/src/ASWUnitTests_Win32GUI_DialogTemplate.h). Compiled
// by the Windows console runners as well as the Win32 GUI, so it runs
// without the GUI.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_Win32GUI_DialogTemplate : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_AddControl_AlignsEachControlOnADWORD();
    void Test_AddControl_CountsTheControls();
    void Test_Constructor_WritesTheHeaderWithTheGUIFont();
    void Test_Get_IsATemplateWindowsCreatesTheDialogFrom();

public:
    TTest_ASWUnitTests_Win32GUI_DialogTemplate();
    ~TTest_ASWUnitTests_Win32GUI_DialogTemplate() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_Win32GUI_DialogTemplateH
