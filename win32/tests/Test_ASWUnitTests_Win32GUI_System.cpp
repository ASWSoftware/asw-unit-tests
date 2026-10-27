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
std::string Joined(std::vector<std::string> const& items);
void RemoveFolderTree(std::wstring const& folder);

//---------------------------------------------------------------------------
bool IsFolder(std::string const& path)
{
    DWORD const attributes = GetFileAttributesW(UTF8ToWide(path).c_str());

    return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}
//---------------------------------------------------------------------------
/*
    Joined

    The items separated by '|', to compare a list with one CheckEquals().
*/
std::string Joined(std::vector<std::string> const& items)
{
    std::string joined;

    for (std::size_t i = 0; i < items.size(); ++i)
        joined += (i == 0 ? "" : "|") + items[i];

    return joined;
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
    : inherited("ASWUnitTests_Win32GUI_System_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_AdjustLineBreaks_MakesEveryLineBreakCRLF,
        "AdjustLineBreaks_MakesEveryLineBreakCRLF");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_ConvertsNonASCIIToUTF8,
        "CommandLineArguments_ConvertsNonASCIIToUTF8");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_FollowsTheCRuntimeQuotingRules,
        "CommandLineArguments_FollowsTheCRuntimeQuotingRules");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_SkipsTheProgramName,
        "CommandLineArguments_SkipsTheProgramName");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_CreatesEveryMissingFolder,
        "CreateFolderPath_CreatesEveryMissingFolder");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_CreateFolderPath_ThrowsWhenAFileIsInTheWay,
        "CreateFolderPath_ThrowsWhenAFileIsInTheWay");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ExeFileName_IsThisExecutablesNameWithoutItsFolder,
        "ExeFileName_IsThisExecutablesNameWithoutItsFolder");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_FileExists_IsTrueOnlyForAFile, "FileExists_IsTrueOnlyForAFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_GUILayoutFilePath_IsPerExeUnderAppData,
        "GUILayoutFilePath_IsPerExeUnderAppData");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_GUISelectionFilePath_IsBesideTheLayoutFile,
        "GUISelectionFilePath_IsBesideTheLayoutFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote,
        "ReadFileBytes_ReadsWhatWriteFileBytesWrote");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_ReadFileBytes_ThrowsForAMissingFile,
        "ReadFileBytes_ThrowsForAMissingFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_RemoveFile_DeletesOnlyAnExistingFile,
        "RemoveFile_DeletesOnlyAnExistingFile");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_UTF8ToWide_ConvertsEveryPlane, "UTF8ToWide_ConvertsEveryPlane");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_System::Test_UTF8ToWide_ReplacesInvalidUTF8, "UTF8ToWide_ReplacesInvalidUTF8");
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

    // e.g. "C:\Users\...\Temp\ASWUnitTests_Win32GUI_System_1234_" followed by U+4E2D.
    m_ScratchFolder = WideToUTF8(tempFolder) + "ASWUnitTests_Win32GUI_System_" + std::to_string(GetCurrentProcessId()) +
        "_\xE4\xB8\xAD";

    RemoveFolderTree(UTF8ToWide(m_ScratchFolder));

    if (CreateDirectoryW(UTF8ToWide(m_ScratchFolder).c_str(), nullptr) == 0)
        throw std::runtime_error("couldn't create " + m_ScratchFolder);
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
void TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_ConvertsNonASCIIToUTF8()
{
    // Arrange: U+00E9 (two UTF-8 bytes), U+4E2D (three) and U+1F600 (four; a surrogate pair in UTF-16).
    std::wstring const commandLine = std::wstring(L"prog.exe \x00E9t\x00E9 \x4E2D ") + L"\xD83D\xDE00";

    // Act
    std::vector<std::string> const args = CommandLineArguments(commandLine);

    // Assert
    CheckEquals(std::string("\xC3\xA9t\xC3\xA9|\xE4\xB8\xAD|\xF0\x9F\x98\x80"), Joined(args), __func__, __LINE__,
        "each argument in UTF-8");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_FollowsTheCRuntimeQuotingRules()
{
    // Arrange: a quoted program name with a space, a quoted argument with a space, an empty quoted argument, an
    // escaped quote, an escaped backslash before a closing quote, and backslashes that precede no quote.
    std::wstring const commandLine =
        L"\"C:\\Program Files\\prog.exe\" --filter \"a b\" \"\" a\\\"b \"c\\\\\" d\\\\e";

    // Act
    std::vector<std::string> const args = CommandLineArguments(commandLine);

    // Assert
    CheckEquals(std::string("--filter|a b||a\"b|c\\|d\\\\e"), Joined(args), __func__, __LINE__,
        "split and unquoted as the C runtime splits argv");
    CheckEquals(6u, args.size(), __func__, __LINE__, "the empty argument is kept");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_CommandLineArguments_SkipsTheProgramName()
{
    // Act
    std::vector<std::string> const programOnly = CommandLineArguments(L"prog.exe");
    std::vector<std::string> const spaced = CommandLineArguments(L"prog.exe   --run   --exit  ");
    // CommandLineToArgvW() gives an empty command line the executable's path as its program name.
    std::vector<std::string> const empty = CommandLineArguments(L"");

    // Assert
    CheckEmpty(programOnly, __func__, __LINE__, "no arguments after the program name");
    CheckEquals(std::string("--run|--exit"), Joined(spaced), __func__, __LINE__, "extra spaces separate only");
    CheckEmpty(empty, __func__, __LINE__, "an empty command line has no arguments");
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
void TTest_ASWUnitTests_Win32GUI_System::Test_UTF8ToWide_ConvertsEveryPlane()
{
    // Arrange: ASCII, U+00E9, U+4E2D and U+1F600 (a surrogate pair in UTF-16), and a NUL, which ends neither.
    std::string const utf8 = std::string("a\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80") + std::string("\0z", 2);
    std::wstring const expected = std::wstring(L"a\x00E9\x4E2D\xD83D\xDE00") + std::wstring(L"\0z", 2);

    // Act
    std::wstring const wide = UTF8ToWide(utf8);

    // Assert
    CheckEquals(expected, wide, __func__, __LINE__, "every character converted");
    CheckEquals(utf8, WideToUTF8(wide), __func__, __LINE__, "and back again");
    CheckEmpty(UTF8ToWide(std::string()), __func__, __LINE__, "empty stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_System::Test_UTF8ToWide_ReplacesInvalidUTF8()
{
    // Act: a byte that never appears in UTF-8, and a continuation byte with no lead byte.
    std::wstring const invalid = UTF8ToWide("a\xFF" "b");
    std::wstring const continuation = UTF8ToWide("\x80" "c");

    // Assert
    CheckEquals(std::wstring(L"a\xFFFD" L"b"), invalid, __func__, __LINE__, "0xFF becomes U+FFFD");
    CheckEquals(std::wstring(L"\xFFFD" L"c"), continuation, __func__, __LINE__, "a lone 0x80 becomes U+FFFD");
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
