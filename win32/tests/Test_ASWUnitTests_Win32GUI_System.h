/* **************************************************************************
Test_ASWUnitTests_Win32GUI_System.h
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
#ifndef Test_ASWUnitTests_Win32GUI_SystemH
#define Test_ASWUnitTests_Win32GUI_SystemH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_System
//
// Exercises the Win32 GUI runner's process and system access
// (win32/gui/src/ASWUnitTests_Win32GUI_System.h). Compiled by the Windows
// console runners as well as the Win32 GUI, so it runs without the GUI.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_Win32GUI_System : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    // A folder of its own for each test, in the temp folder, with a name outside every ANSI code page; deleted
    // with its content after the test.
    std::string m_ScratchFolder;
    // The scratch folders made so far, which numbers each test's own; see SetUp_Test().
    unsigned int m_ScratchFolderCount;

    std::string ScratchPath(std::string const& name) const;

private: // Test methods
    void Test_AdjustLineBreaks_MakesEveryLineBreakCRLF();
    void Test_CreateFolderPath_CreatesEveryMissingFolder();
    void Test_CreateFolderPath_ThrowsWhenAFileIsInTheWay();
    void Test_ExeFileName_IsThisExecutablesNameWithoutItsFolder();
    void Test_ExpandFileName_MakesARelativePathAbsolute();
    void Test_ExpandFileName_ResolvesDotFolders();
    void Test_FileExists_IsTrueOnlyForAFile();
    void Test_GUILayoutFilePath_IsPerExeUnderAppData();
    void Test_GUISelectionFilePath_IsBesideTheLayoutFile();
    void Test_IsThemeSettingChange_IsTrueForTheAppModeAndHighContrast();
    void Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote();
    void Test_MainIcon_IsThisExecutablesSharedIcon();
    void Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote();
    void Test_ReadFileBytes_ThrowsForAMissingFile();
    void Test_RemoveFile_DeletesOnlyAnExistingFile();
    void Test_WindowText_ConvertsNonASCIIToUTF8();
    void Test_WindowsAppsUseDarkTheme_MatchesTheRegistry();
    void Test_WriteFileBytes_ThrowsWhenTheFolderIsMissing();

public:
    TTest_ASWUnitTests_Win32GUI_System();
    ~TTest_ASWUnitTests_Win32GUI_System() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_Win32GUI_SystemH
