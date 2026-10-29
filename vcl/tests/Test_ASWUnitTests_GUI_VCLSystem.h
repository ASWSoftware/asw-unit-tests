/* **************************************************************************
Test_ASWUnitTests_GUI_VCLSystem.h
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
#ifndef Test_ASWUnitTests_GUI_VCLSystemH
#define Test_ASWUnitTests_GUI_VCLSystemH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_VCLSystem
//
// Exercises the VCL GUI runner's process and system access
// (vcl/gui/src/ASWUnitTests_GUI_VCLSystem.h). Listed in both vcl/ projects,
// so it also runs from the VCL console runner.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_VCLSystem : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_GUILayoutFilePath_IsPerExeUnderAppData();
    void Test_GUISelectionFilePath_IsBesideTheLayoutFile();
    void Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote();
    void Test_ProcessArguments_MatchesParamCount();
    void Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote();
    void Test_WindowsAppsUseDarkTheme_MatchesTheRegistry();

public:
    TTest_ASWUnitTests_GUI_VCLSystem();
    ~TTest_ASWUnitTests_GUI_VCLSystem() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_VCLSystemH
