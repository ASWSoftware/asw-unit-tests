/* **************************************************************************
Test_ASWUnitTests_GUI_VCLSystem.cpp
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
#include "Test_ASWUnitTests_GUI_VCLSystem.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
#include <System.Win.Registry.hpp>
#include <Vcl.Themes.hpp>
#include <windows.h>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
#include "ASWUnitTests_GUI_Theme.h"
#include "ASWUnitTests_GUI_VCLSystem.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_VCLSystem
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_VCLSystem::TTest_ASWUnitTests_GUI_VCLSystem()
    : inherited("ASWUnitTests_GUI_VCLSystem_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_BuiltInGUIDarkThemeNames_AreCarbonAndTheLinkedStyles,
        "BuiltInGUIDarkThemeNames_AreCarbonAndTheLinkedStyles");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_FileNamesInFolder_ListsOnlyTheFiles,
        "FileNamesInFolder_ListsOnlyTheFiles");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_GUILayoutFilePath_IsPerExeUnderAppData,
        "GUILayoutFilePath_IsPerExeUnderAppData");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_GUISelectionFilePath_IsBesideTheLayoutFile,
        "GUISelectionFilePath_IsBesideTheLayoutFile");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_GUIThemesFolderPath_IsSharedBesideTheLayoutFiles,
        "GUIThemesFolderPath_IsSharedBesideTheLayoutFiles");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_LoadGUIDarkStyle_GivesCarbonForAnyProblem,
        "LoadGUIDarkStyle_GivesCarbonForAnyProblem");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote,
        "LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_ProcessArguments_MatchesParamCount, "ProcessArguments_MatchesParamCount");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote,
        "ReadFileBytes_ReadsWhatWriteFileBytesWrote");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_WindowsAppsUseDarkTheme_MatchesTheRegistry,
        "WindowsAppsUseDarkTheme_MatchesTheRegistry");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_VCLSystem::~TTest_ASWUnitTests_GUI_VCLSystem()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_BuiltInGUIDarkThemeNames_AreCarbonAndTheLinkedStyles()
{
    // Arrange: the styles this runner links in, none in the console runner, all of them in the GUI.
    System::DynamicArray<System::UnicodeString> const styles = Vcl::Themes::TStyleManager::StyleNames;
    std::string expected = GUIDefaultDarkThemeName() + "\n";

    for (std::string const& name : GUIVCLStyleDarkThemeNames())
    {
        for (int i = 0; i < styles.Length; ++i)
        {
            if (System::Sysutils::SameText(styles[i], FromUTF8(name)))
                expected += name + "\n";
        }
    }

    // Act
    std::string names;

    for (std::string const& name : BuiltInGUIDarkThemeNames())
        names += name + "\n";

    // Assert
    CheckEquals(expected, names, __func__, __LINE__, "Carbon first, then each linked style, in order");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_FileNamesInFolder_ListsOnlyTheFiles()
{
    // Arrange: two files, one with a non-ASCII name (U+4E2D), and a folder named like a style file, in a folder
    // deleted however the test ends.
    System::UnicodeString const folder = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetTempPath(),
        System::UnicodeString("ASWUnitTests_VCLSystem_") + System::Sysutils::IntToStr(static_cast<int>(
            GetCurrentProcessId())));

    struct TDeleteFolder
    {
        System::UnicodeString const Folder;

        ~TDeleteFolder()
        {
            try
            {
                System::Ioutils::TDirectory::Delete(Folder, true);
            }
            catch (...)
            {
            }
        }
    } const deleteFolder{ folder };

    System::Sysutils::ForceDirectories(System::Ioutils::TPath::Combine(folder, "Folder.vsf"));
    WriteFileBytes(System::Ioutils::TPath::Combine(folder, "Ocean.vsf"), "x");
    WriteFileBytes(System::Ioutils::TPath::Combine(folder, System::UnicodeString(L"\x4E2D.vsf")), std::string());
    WriteFileBytes(System::Ioutils::TPath::Combine(folder, "Folder.vsf\\Inside.vsf"), "x");

    // Act
    std::vector<std::string> names = FileNamesInFolder(folder);
    std::vector<std::string> const missing = FileNamesInFolder(System::Ioutils::TPath::Combine(folder, "Missing"));

    // Assert: both sorted the same way, as where a non-ASCII byte sorts depends on whether char is signed.
    std::vector<std::string> expected = { "Ocean.vsf", "\xE4\xB8\xAD.vsf" };
    std::sort(expected.begin(), expected.end());
    std::sort(names.begin(), names.end());
    std::string expectedText;
    std::string listed;

    for (std::string const& name : expected)
        expectedText += name + "\n";

    for (std::string const& name : names)
        listed += name + "\n";

    CheckEquals(expectedText, listed, __func__, __LINE__,
        "the files' names, in UTF-8, without the folder, its subfolder or the files in it");
    CheckTrue(missing.empty(), __func__, __LINE__, "none for a missing folder");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_GUILayoutFilePath_IsPerExeUnderAppData()
{
    // Arrange
    System::UnicodeString const exeName = System::Sysutils::ExtractFileName(System::ParamStr(0));
    System::UnicodeString const appData = System::Sysutils::GetEnvironmentVariable("APPDATA");

    // Act
    System::UnicodeString const path = GUILayoutFilePath();

    // Assert
    System::UnicodeString const folder = System::Sysutils::ExtractFileDir(path);
    CheckEquals(ToUTF8(System::Sysutils::ChangeFileExt(exeName, ".ini")),
        ToUTF8(System::Sysutils::ExtractFileName(path)), __func__, __LINE__, "named after this executable");
    CheckEquals(std::string("ASWUnitTests"), ToUTF8(System::Sysutils::ExtractFileName(folder)), __func__, __LINE__,
        "in an ASWUnitTests folder");
    AssertFalse(appData.IsEmpty(), __func__, __LINE__, "%APPDATA% is set");
    CheckTrue(System::Sysutils::SameText(System::Sysutils::ExtractFileDir(folder),
        System::Sysutils::ExcludeTrailingPathDelimiter(appData)), __func__, __LINE__, "under %APPDATA%");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_GUISelectionFilePath_IsBesideTheLayoutFile()
{
    // Act
    System::UnicodeString const selectionPath = GUISelectionFilePath();
    System::UnicodeString const layoutPath = GUILayoutFilePath();

    // Assert
    CheckEquals(ToUTF8(System::Sysutils::ChangeFileExt(layoutPath, ".selection")), ToUTF8(selectionPath), __func__,
        __LINE__, "the layout file's path with a .selection extension");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_GUIThemesFolderPath_IsSharedBesideTheLayoutFiles()
{
    // Act and Assert: the same for every executable, unlike the layout file.
    CheckEquals(ToUTF8(System::Ioutils::TPath::Combine(System::Sysutils::ExtractFileDir(GUILayoutFilePath()),
        "Themes")), ToUTF8(GUIThemesFolderPath()), __func__, __LINE__, "a Themes folder beside the layout file");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_LoadGUIDarkStyle_GivesCarbonForAnyProblem()
{
    // Arrange: a missing theme, and a file that isn't a style, in a Themes folder of the test's own, not the real
    // one, which a GUI running these tests lists. Each name is the process's, as a style file, once loaded, isn't
    // read again. The folder is deleted however the test ends.
    std::string const suffix = std::to_string(GetCurrentProcessId());
    std::string const missingName = "NoSuchTheme_" + suffix;
    std::string const invalidName = "NotAStyle_" + suffix;
    System::UnicodeString const folder = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetTempPath(),
        System::UnicodeString("ASWUnitTests_VCLSystem_Themes_") + FromUTF8(suffix));

    struct TDeleteFolder
    {
        System::UnicodeString const Folder;

        ~TDeleteFolder()
        {
            try
            {
                System::Ioutils::TDirectory::Delete(Folder, true);
            }
            catch (...)
            {
            }
        }
    } const deleteFolder{ folder };

    System::Sysutils::ForceDirectories(folder);
    WriteFileBytes(System::Ioutils::TPath::Combine(folder, FromUTF8(invalidName) + ".vsf"), "not a VCL style");

    // Act
    std::vector<std::string> carbonProblems;
    System::UnicodeString const carbon = LoadGUIDarkStyle(folder, "Carbon", carbonProblems);
    std::vector<std::string> missingProblems;
    System::UnicodeString const missing = LoadGUIDarkStyle(folder, missingName, missingProblems);
    std::vector<std::string> invalidProblems;
    System::UnicodeString const invalid = LoadGUIDarkStyle(folder, invalidName, invalidProblems);

    // Assert
    CheckEquals(std::string("Carbon"), ToUTF8(carbon), __func__, __LINE__, "the built-in theme's style");
    CheckTrue(carbonProblems.empty(), __func__, __LINE__, "with no problems");
    CheckEquals(std::string("Carbon"), ToUTF8(missing), __func__, __LINE__, "Carbon for a missing file");
    AssertEquals(static_cast<size_t>(1), missingProblems.size(), __func__, __LINE__, "with one problem");
    std::string const missingStart = "Dark theme \"" + missingName + "\": There's no style file ";
    CheckEquals(missingStart, missingProblems.front().substr(0, missingStart.size()), __func__, __LINE__,
        "saying so");
    CheckEquals(std::string("Carbon"), ToUTF8(invalid), __func__, __LINE__, "Carbon for a file that isn't a style");
    AssertEquals(static_cast<size_t>(1), invalidProblems.size(), __func__, __LINE__, "with one problem");
    CheckEquals("Dark theme \"" + invalidName + "\": Its file isn't a VCL style file, so Carbon is used instead.",
        invalidProblems.front(), __func__, __LINE__, "saying so");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote()
{
    // Arrange: a layout file in a folder that doesn't exist yet, not this runner's own, which a GUI running these
    // tests is using. The folder is deleted however the test ends.
    System::UnicodeString const folder = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetTempPath(),
        System::UnicodeString("ASWUnitTests_VCLSystem_Layout_") + System::Sysutils::IntToStr(static_cast<int>(
            GetCurrentProcessId())));
    System::UnicodeString const path = System::Ioutils::TPath::Combine(System::Ioutils::TPath::Combine(folder,
        "Settings"), "Layout.ini");

    struct TDeleteFolder
    {
        System::UnicodeString const Folder;

        ~TDeleteFolder()
        {
            try
            {
                System::Ioutils::TDirectory::Delete(Folder, true);
            }
            catch (...)
            {
            }
        }
    } const deleteFolder{ folder };

    EGUITheme const noFile = LoadGUIThemeFromLayoutFile(path);
    std::string const noFileDarkTheme = LoadGUIDarkThemeFromLayoutFile(path);

    // Act
    SaveGUIThemeToLayoutFile(path, EGUITheme::Dark);
    EGUITheme const dark = LoadGUIThemeFromLayoutFile(path);

    WriteFileBytes(path, "[Window]\r\nMaximized=1\r\n");
    SaveGUIThemeToLayoutFile(path, EGUITheme::Light);
    EGUITheme const light = LoadGUIThemeFromLayoutFile(path);
    SaveGUIDarkThemeToLayoutFile(path, "Deep \xC3\x89" "cume");
    std::string const darkTheme = LoadGUIDarkThemeFromLayoutFile(path);
    std::string const text = ReadFileBytes(path);

    System::Sysutils::DeleteFile(path);
    SaveGUIDarkThemeToLayoutFile(path, "Ocean");
    std::string const darkThemeInNewFile = LoadGUIDarkThemeFromLayoutFile(path);

    // Assert
    CheckTrue(noFile == EGUITheme::Windows, __func__, __LINE__, "Windows without a layout file");
    CheckEquals(std::string("Carbon"), noFileDarkTheme, __func__, __LINE__, "and the dark theme Carbon");
    CheckTrue(dark == EGUITheme::Dark, __func__, __LINE__, "the theme saved in a new file, and its new folder");
    CheckTrue(light == EGUITheme::Light, __func__, __LINE__, "the theme saved in an existing file");
    CheckEquals(std::string("Deep \xC3\x89" "cume"), darkTheme, __func__, __LINE__,
        "the dark theme saved beside it, in UTF-8");
    CheckEquals(std::string("[Window]\r\nMaximized=1\r\n\r\n[View]\r\nTheme=Light\r\nDarkTheme=Deep \xC3\x89" "cume\r\n"
        "\r\n"), text, __func__, __LINE__, "the layout stays");
    CheckEquals(std::string("Ocean"), darkThemeInNewFile, __func__, __LINE__, "the dark theme saved in a new file");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_ProcessArguments_MatchesParamCount()
{
    // Arrange
    // The runner's own arguments vary with how it was started, so only their count can be checked here.
    int const expectedCount = System::ParamCount();

    // Act
    std::vector<std::string> const args = ProcessArguments();

    // Assert
    CheckEquals(static_cast<size_t>(expectedCount), args.size(), __func__, __LINE__,
        "one entry per argument, without the program name");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote()
{
    // Arrange: a file name with a character outside every ANSI code page but UTF-8's (U+4E2D), and bytes a
    // text conversion would change: CRLF and LF line breaks, a NUL, and UTF-8 for U+00E9.
    System::UnicodeString const path = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetTempPath(),
        System::UnicodeString(L"ASWUnitTests_VCLSystem_\x4E2D.tmp"));
    std::string const bytes("a\r\nb\nc\0d\xC3\xA9", 10);

    // Deletes the file however the test ends.
    struct TDeleteFile
    {
        System::UnicodeString const Path;

        ~TDeleteFile()
        {
            System::Sysutils::DeleteFile(Path);
        }
    } const deleteFile{ path };

    // Act
    WriteFileBytes(path, bytes);
    std::string const read = ReadFileBytes(path);

    WriteFileBytes(path, "xy");
    std::string const shorter = ReadFileBytes(path);

    WriteFileBytes(path, std::string());
    std::string const empty = ReadFileBytes(path);

    // Assert
    CheckEquals(bytes, read, __func__, __LINE__, "every byte is read back unchanged");
    CheckEquals(std::string("xy"), shorter, __func__, __LINE__, "writing again replaces the whole file");
    CheckEmpty(empty, __func__, __LINE__, "an empty file reads as empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_VCLSystem::Test_WindowsAppsUseDarkTheme_MatchesTheRegistry()
{
    // Arrange: the setting read another way, through the RTL. This machine's setting can't be changed here,
    // so only agreement with it can be checked.
    bool expected = false;
    std::unique_ptr<System::Win::Registry::TRegistry> const registry(new System::Win::Registry::TRegistry(KEY_READ));
    registry->RootKey = HKEY_CURRENT_USER;

    if (registry->OpenKeyReadOnly("Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize") &&
        registry->ValueExists("AppsUseLightTheme"))
    {
        expected = (registry->ReadInteger("AppsUseLightTheme") == 0);
    }

    // Act
    bool const dark = WindowsAppsUseDarkTheme();

    // Assert
    CheckEquals(expected, dark, __func__, __LINE__, "dark when AppsUseLightTheme is 0");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_VCLSystem)
