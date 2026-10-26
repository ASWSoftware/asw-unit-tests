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
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
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
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_GUILayoutFilePath_IsPerExeUnderAppData,
        "GUILayoutFilePath_IsPerExeUnderAppData");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_GUISelectionFilePath_IsBesideTheLayoutFile,
        "GUISelectionFilePath_IsBesideTheLayoutFile");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_ProcessArguments_MatchesParamCount, "ProcessArguments_MatchesParamCount");
    RegisterTest(&TTest_ASWUnitTests_GUI_VCLSystem::Test_ReadFileBytes_ReadsWhatWriteFileBytesWrote,
        "ReadFileBytes_ReadsWhatWriteFileBytesWrote");
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

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_VCLSystem)
