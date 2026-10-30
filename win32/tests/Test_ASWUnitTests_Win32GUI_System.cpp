/* **************************************************************************
Test_ASWUnitTests_Win32GUI_System.cpp
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
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

bool IsFolder(std::string const& path);
void RemoveFolderTree(std::wstring const& folder);

//---------------------------------------------------------------------------
bool IsFolder(std::string const& path)
{
    DWORD const attributes = GetFileAttributesW(UTF8ToWide(path).c_str());

    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
//---------------------------------------------------------------------------
/*
    RemoveFolderTree

    Deletes the folder and everything in it, as far as it can.
*/
void RemoveFolderTree(std::wstring const& folder)
{
    WIN32_FIND_DATAW found;
    HANDLE const search = FindFirstFileW((folder + L"\\*").c_str(), &found);

    if (search != INVALID_HANDLE_VALUE)
    {
        for (bool more = true; more; more = FindNextFileW(search, &found) != 0)
        {
            std::wstring const name = found.cFileName;

            if (name == L"." || name == L"..")
                continue;

            std::wstring const path = folder + L"\\" + name;

            if ((found.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
                RemoveFolderTree(path);
            else
                DeleteFileW(path.c_str());
        }

        FindClose(search);
    }

    RemoveDirectoryW(folder.c_str());
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_System
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_System::TTest_ASWUnitTests_Win32GUI_System()
    : inherited("ASWUnitTests_Win32GUI_System_Tests"),
      m_ScratchFolderCount(0)
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_AdjustLineBreaks_MakesEveryLineBreakCRLF,
        "AdjustLineBreaks_MakesEveryLineBreakCRLF");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_CreatesEveryMissingFolder,
        "CreateFolderPath_CreatesEveryMissingFolder");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_ThrowsWhenAFileIsInTheWay,
        "CreateFolderPath_ThrowsWhenAFileIsInTheWay");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ExeFileName_IsThisExecutablesNameWithoutItsFolder,
        "ExeFileName_IsThisExecutablesNameWithoutItsFolder");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ExpandFileName_MakesARelativePathAbsolute,
        "ExpandFileName_MakesARelativePathAbsolute");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ExpandFileName_ResolvesDotFolders,
        "ExpandFileName_ResolvesDotFolders");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_FileExists_IsTrueOnlyForAFile, "FileExists_IsTrueOnlyForAFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_FileNamesInFolder_ListsOnlyTheFiles,
        "FileNamesInFolder_ListsOnlyTheFiles");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_GUILayoutFilePath_IsPerExeUnderAppData,
        "GUILayoutFilePath_IsPerExeUnderAppData");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_GUISelectionFilePath_IsBesideTheLayoutFile,
        "GUISelectionFilePath_IsBesideTheLayoutFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_GUIThemesFolderPath_IsSharedBesideTheLayoutFiles,
        "GUIThemesFolderPath_IsSharedBesideTheLayoutFiles");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_IsThemeSettingChange_IsTrueForTheAppModeAndHighContrast,
        "IsThemeSettingChange_IsTrueForTheAppModeAndHighContrast");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote,
        "LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_MainIcon_IsThisExecutablesSharedIcon,
        "MainIcon_IsThisExecutablesSharedIcon");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote,
        "ReadFileBytes_ReadsWhatWriteFileBytesWrote");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ThrowsForAMissingFile,
        "ReadFileBytes_ThrowsForAMissingFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_RemoveFile_DeletesOnlyAnExistingFile,
        "RemoveFile_DeletesOnlyAnExistingFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_WindowText_ConvertsNonASCIIToUTF8,
        "WindowText_ConvertsNonASCIIToUTF8");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_WindowsAppsUseDarkTheme_MatchesTheRegistry,
        "WindowsAppsUseDarkTheme_MatchesTheRegistry");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_WriteFileBytes_ThrowsWhenTheFolderIsMissing,
        "WriteFileBytes_ThrowsWhenTheFolderIsMissing");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_System::~TTest_ASWUnitTests_Win32GUI_System()
{
}
//---------------------------------------------------------------------------
std::string TTest_ASWUnitTests_Win32GUI_System::ScratchPath(std::string const& name) const
{
    return m_ScratchFolder + "\\" + name;
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::SetUp_Test(ITestCase& /*testCase*/)
{
    wchar_t tempFolder[MAX_PATH + 1] = {};

    if (GetTempPathW(MAX_PATH + 1, tempFolder) == 0)
        throw std::runtime_error("no temp folder");

    // e.g. "C:\Users\...\Temp\ASWUnitTests_Win32GUI_System_1234_5_" followed by U+4E2D. Numbered per test, since
    // the previous test's folder may outlive its TearDown_Test(): another program with one of its files open,
    // such as a virus scanner, keeps it from being deleted.
    m_ScratchFolder = WideToUTF8(tempFolder) + "ASWUnitTests_Win32GUI_System_" + std::to_string(GetCurrentProcessId()) +
        "_" + std::to_string(++m_ScratchFolderCount) + "_\xE4\xB8\xAD";

    RemoveFolderTree(UTF8ToWide(m_ScratchFolder));

    if (CreateDirectoryW(UTF8ToWide(m_ScratchFolder).c_str(), nullptr) == 0)
        throw std::runtime_error("couldn't create " + m_ScratchFolder + " (error " + std::to_string(GetLastError()) +
            ")");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::TearDown_Test(ITestCase& /*testCase*/)
{
    RemoveFolderTree(UTF8ToWide(m_ScratchFolder));
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_AdjustLineBreaks_MakesEveryLineBreakCRLF()
{
    // Act, Assert
    CheckEquals(std::string("a\r\nb\r\n"), AdjustLineBreaks("a\nb\n"), __func__, __LINE__, "LF becomes CRLF");
    CheckEquals(std::string("a\r\nb"), AdjustLineBreaks("a\r\nb"), __func__, __LINE__, "CRLF stays as it is");
    CheckEquals(std::string("a\r\nb"), AdjustLineBreaks("a\rb"), __func__, __LINE__, "a lone CR becomes CRLF");
    CheckEquals(std::string("\r\n\r\n\r\n"), AdjustLineBreaks("\r\n\n\r"), __func__, __LINE__,
        "each break counts once: CRLF, LF, CR");
    CheckEquals(std::string("a\r\n\r\nb"), AdjustLineBreaks("a\n\r\nb"), __func__, __LINE__,
        "an LF before a CRLF is a break of its own");
    CheckEmpty(AdjustLineBreaks(std::string()), __func__, __LINE__, "empty stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_CreatesEveryMissingFolder()
{
    // Arrange: three new levels, one with a non-ASCII name (U+00E9), and both kinds of separator.
    std::string const folder = ScratchPath("a\xC3\xA9" "/b\\c");

    // Act
    CreateFolderPath(folder);

    // Assert
    CheckTrue(IsFolder(ScratchPath("a\xC3\xA9\\b\\c")), __func__, __LINE__, "every level is created");
    CheckNoThrow([&]()
        {
            CreateFolderPath(folder);
        }, __func__, __LINE__, "an existing folder is fine");
    CheckNoThrow([&]()
        {
            CreateFolderPath(folder + "\\");
        }, __func__, __LINE__, "so is a trailing separator");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_ThrowsWhenAFileIsInTheWay()
{
    // Arrange
    std::string const file = ScratchPath("file");
    WriteFileBytes(file, "x");

    // Act, Assert
    CheckThrows<std::runtime_error>([&]()
        {
            CreateFolderPath(file + "\\sub");
        }, __func__, __LINE__, "a file where a folder must go");
    CheckThrows<std::runtime_error>([&]()
        {
            CreateFolderPath(file);
        }, __func__, __LINE__, "a file where the folder itself must go");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_ExeFileName_IsThisExecutablesNameWithoutItsFolder()
{
    // Act
    std::string const name = ExeFileName();

    // Assert
    AssertFalse(name.empty(), __func__, __LINE__, "has a name");
    CheckTrue(name.find_first_of("\\/:") == std::string::npos, __func__, __LINE__, "has no folder");
    // GetModuleHandleW() finds a loaded module by its file name alone.
    CheckTrue(GetModuleHandleW(UTF8ToWide(name).c_str()) == GetModuleHandleW(nullptr), __func__, __LINE__,
        "names this process's executable");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_ExpandFileName_MakesARelativePathAbsolute()
{
    // Arrange
    wchar_t currentFolder[MAX_PATH + 1] = {};
    DWORD const currentLength = GetCurrentDirectoryW(MAX_PATH + 1, currentFolder);
    AssertTrue(currentLength > 0 && currentLength <= MAX_PATH, __func__, __LINE__, "the current folder is known");

    std::string folder = WideToUTF8(currentFolder);

    if (folder.back() != '\\')
        folder += "\\"; // Unless it's a drive's root, such as "C:\".

    // Act
    std::string const expanded = ExpandFileName("report\xC3\xA9.xml");

    // Assert
    CheckEquals(folder + "report\xC3\xA9.xml", expanded, __func__, __LINE__, "in the current folder");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_ExpandFileName_ResolvesDotFolders()
{
    // Act
    std::string const expanded = ExpandFileName(ScratchPath("sub\xE4\xB8\xAD\\..\\.\\report.xml"));

    // Assert
    CheckEquals(ScratchPath("report.xml"), expanded, __func__, __LINE__, "without the dot folders");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_FileExists_IsTrueOnlyForAFile()
{
    // Arrange
    std::string const file = ScratchPath("file\xE4\xB8\xAD.txt");
    WriteFileBytes(file, std::string());

    // Act, Assert
    CheckTrue(FileExists(file), __func__, __LINE__, "an empty file exists");
    CheckFalse(FileExists(ScratchPath("missing.txt")), __func__, __LINE__, "a missing file doesn't");
    CheckFalse(FileExists(m_ScratchFolder), __func__, __LINE__, "a folder isn't a file");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_FileNamesInFolder_ListsOnlyTheFiles()
{
    // Arrange: two files, one with a non-ASCII name (U+4E2D), and a folder named like a theme file.
    WriteFileBytes(ScratchPath("Ocean.ini"), "x");
    WriteFileBytes(ScratchPath("\xE4\xB8\xAD.ini"), std::string());
    CreateFolderPath(ScratchPath("Folder.ini"));
    WriteFileBytes(ScratchPath("Folder.ini\\Inside.ini"), "x");

    // Act
    std::vector<std::string> names = FileNamesInFolder(m_ScratchFolder);
    std::vector<std::string> const missing = FileNamesInFolder(ScratchPath("Missing"));

    // Assert: both sorted the same way, as where a non-ASCII byte sorts depends on whether char is signed.
    std::vector<std::string> expected = { "Ocean.ini", "\xE4\xB8\xAD.ini" };
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
void TTest_ASWUnitTests_Win32GUI_System::Test_GUILayoutFilePath_IsPerExeUnderAppData()
{
    // Arrange
    std::string const exeName = ExeFileName();
    std::string const expectedName = exeName.substr(0, exeName.find_last_of('.')) + ".ini";
    wchar_t appData[MAX_PATH + 1] = {};
    DWORD const appDataLength = GetEnvironmentVariableW(L"APPDATA", appData, MAX_PATH + 1);

    // Act
    std::string const path = GUILayoutFilePath();
    std::string const folder = GUISettingsFolderPath();

    // Assert
    CheckEquals(folder + "\\" + expectedName, path, __func__, __LINE__, "named after this executable, in the folder");
    AssertTrue(appDataLength > 0 && appDataLength <= MAX_PATH, __func__, __LINE__, "%APPDATA% is set");
    CheckEqualsIC(WideToUTF8(appData) + "\\ASWUnitTests", folder, __func__, __LINE__,
        "an ASWUnitTests folder under %APPDATA%");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_GUISelectionFilePath_IsBesideTheLayoutFile()
{
    // Arrange
    std::string const layoutPath = GUILayoutFilePath();

    // Act
    std::string const selectionPath = GUISelectionFilePath();

    // Assert
    CheckEquals(layoutPath.substr(0, layoutPath.size() - 4) + ".selection", selectionPath, __func__, __LINE__,
        "the layout file's path with a .selection extension");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_GUIThemesFolderPath_IsSharedBesideTheLayoutFiles()
{
    // Act and Assert: the same for every executable, unlike the layout file.
    CheckEquals(GUISettingsFolderPath() + "\\Themes", GUIThemesFolderPath(), __func__, __LINE__,
        "a Themes folder in the settings folder");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_IsThemeSettingChange_IsTrueForTheAppModeAndHighContrast()
{
    // Arrange: WM_SETTINGCHANGE's parameters, as Windows sends them.
    wchar_t const appMode[] = L"ImmersiveColorSet";
    wchar_t const other[] = L"Policy";

    // Act
    bool const forAppMode = IsThemeSettingChange(0, reinterpret_cast<LPARAM>(appMode));
    bool const forHighContrast = IsThemeSettingChange(SPI_SETHIGHCONTRAST, 0);
    bool const forOther = IsThemeSettingChange(0, reinterpret_cast<LPARAM>(other));
    bool const forOtherSetting = IsThemeSettingChange(SPI_SETWORKAREA, 0);

    // Assert
    CheckTrue(forAppMode, __func__, __LINE__, "the app mode's change");
    CheckTrue(forHighContrast, __func__, __LINE__, "high contrast's change");
    CheckFalse(forOther, __func__, __LINE__, "another area's change");
    CheckFalse(forOtherSetting, __func__, __LINE__, "another setting's change, with no area");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_LoadGUIThemeFromLayoutFile_ReadsWhatSaveWrote()
{
    // Arrange: this runner's own layout file, which only a GUI would otherwise use, put back however the test
    // ends.
    std::string const path = GUILayoutFilePath();

    struct TRestoreFile
    {
        std::string const Path;
        bool const Existed;
        std::string const Bytes;

        ~TRestoreFile()
        {
            try
            {
                if (Existed)
                    WriteFileBytes(Path, Bytes);
                else
                    RemoveFile(Path);
            }
            catch (...)
            {
            }
        }
    };

    bool const existed = FileExists(path);
    TRestoreFile const restoreFile{ path, existed, existed ? ReadFileBytes(path) : std::string() };

    RemoveFile(path);
    EGUITheme const noFile = LoadGUIThemeFromLayoutFile();
    std::string const noFileDarkTheme = LoadGUIDarkThemeFromLayoutFile();

    // Act
    SaveGUIThemeToLayoutFile(EGUITheme::Dark);
    EGUITheme const dark = LoadGUIThemeFromLayoutFile();

    WriteFileBytes(path, "[Window]\r\nMaximized=1\r\n");
    SaveGUIThemeToLayoutFile(EGUITheme::Light);
    EGUITheme const light = LoadGUIThemeFromLayoutFile();
    SaveGUIDarkThemeToLayoutFile("Deep \xC3\x89" "cume");
    std::string const darkTheme = LoadGUIDarkThemeFromLayoutFile();
    std::string const text = ReadFileBytes(path);

    RemoveFile(path);
    SaveGUIDarkThemeToLayoutFile("Ocean");
    std::string const darkThemeInNewFile = LoadGUIDarkThemeFromLayoutFile();

    // Assert
    CheckTrue(noFile == EGUITheme::Windows, __func__, __LINE__, "Windows without a layout file");
    CheckEquals(std::string("Carbon"), noFileDarkTheme, __func__, __LINE__, "and the dark theme Carbon");
    CheckTrue(dark == EGUITheme::Dark, __func__, __LINE__, "the theme saved in a new file");
    CheckTrue(light == EGUITheme::Light, __func__, __LINE__, "the theme saved in an existing file");
    CheckEquals(std::string("Deep \xC3\x89" "cume"), darkTheme, __func__, __LINE__,
        "the dark theme saved beside it, in UTF-8");
    CheckEquals(std::string("[Window]\r\nMaximized=1\r\n\r\n[View]\r\nTheme=Light\r\nDarkTheme=Deep \xC3\x89" "cume\r\n"
        "\r\n"), text, __func__, __LINE__, "the layout stays");
    CheckEquals(std::string("Ocean"), darkThemeInNewFile, __func__, __LINE__, "the dark theme saved in a new file");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_MainIcon_IsThisExecutablesSharedIcon()
{
    // Act: every runner that compiles this test embeds the ASWUnitTests icon.
    HICON const icon = MainIcon();
    HICON const again = MainIcon();

    // Assert
    CheckTrue(icon != nullptr, __func__, __LINE__, "this executable has a MAINICON resource");
    CheckTrue(again == icon, __func__, __LINE__, "the same, shared, icon each time");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote()
{
    // Arrange: a file name with a character outside every ANSI code page but UTF-8's (U+4E2D), and bytes a
    // text conversion would change: CRLF and LF line breaks, a NUL, and UTF-8 for U+00E9.
    std::string const path = ScratchPath("bytes_\xE4\xB8\xAD.tmp");
    std::string const bytes("a\r\nb\nc\0d\xC3\xA9", 10);

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
void TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ThrowsForAMissingFile()
{
    // Act, Assert
    CheckThrows<std::runtime_error>([&]()
        {
            ReadFileBytes(ScratchPath("missing.txt"));
        }, __func__, __LINE__, "a missing file can't be read");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_RemoveFile_DeletesOnlyAnExistingFile()
{
    // Arrange
    std::string const file = ScratchPath("file\xE4\xB8\xAD.txt");
    WriteFileBytes(file, "x");

    // Act
    bool const removed = RemoveFile(file);
    bool const exists = FileExists(file);
    bool const removedAgain = RemoveFile(file);

    // Assert
    CheckTrue(removed, __func__, __LINE__, "an existing file is deleted");
    CheckFalse(exists, __func__, __LINE__, "and is gone");
    CheckFalse(removedAgain, __func__, __LINE__, "a missing file can't be deleted");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_WindowText_ConvertsNonASCIIToUTF8()
{
    // Arrange: a hidden window titled with ASCII, U+00E9, U+4E2D and U+1F600 (a surrogate pair in UTF-16), and
    // one with no title.
    HWND const titled = CreateWindowExW(0, L"STATIC", L"a\x00E9\x4E2D\xD83D\xDE00", WS_POPUP, 0, 0, 10, 10, nullptr,
        nullptr, GetModuleHandleW(nullptr), nullptr);
    HWND const untitled = CreateWindowExW(0, L"STATIC", L"", WS_POPUP, 0, 0, 10, 10, nullptr, nullptr,
        GetModuleHandleW(nullptr), nullptr);

    // Act
    std::string const titledText = titled != nullptr ? WindowText(titled) : std::string("(no window)");
    std::string const untitledText = untitled != nullptr ? WindowText(untitled) : std::string("(no window)");

    if (titled != nullptr)
        DestroyWindow(titled);

    if (untitled != nullptr)
        DestroyWindow(untitled);

    // Assert
    CheckEquals(std::string("a\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80"), titledText, __func__, __LINE__,
        "every character converted");
    CheckEquals(std::string(), untitledText, __func__, __LINE__, "no text");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_WindowsAppsUseDarkTheme_MatchesTheRegistry()
{
    // Arrange: the setting read another way, with RegQueryValueExW(). This machine's setting can't be changed
    // here, so only agreement with it can be checked.
    bool expected = false;
    HKEY key = nullptr;

    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", 0,
        KEY_READ, &key) == ERROR_SUCCESS)
    {
        DWORD value = 1;
        DWORD size = sizeof(value);
        DWORD type = 0;

        if (RegQueryValueExW(key, L"AppsUseLightTheme", nullptr, &type, reinterpret_cast<BYTE*>(&value), &size) ==
            ERROR_SUCCESS && type == REG_DWORD)
        {
            expected = value == 0;
        }

        RegCloseKey(key);
    }

    // Act
    bool const dark = WindowsAppsUseDarkTheme();

    // Assert
    CheckEquals(expected, dark, __func__, __LINE__, "dark when AppsUseLightTheme is 0");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_WriteFileBytes_ThrowsWhenTheFolderIsMissing()
{
    // Act, Assert
    CheckThrows<std::runtime_error>([&]()
        {
            WriteFileBytes(ScratchPath("missing\\file.txt"), "x");
        }, __func__, __LINE__, "the folder isn't created");
    CheckFalse(IsFolder(ScratchPath("missing")), __func__, __LINE__, "nor left behind");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_System)
