/* **************************************************************************
Test_ASWUnitTests_Console.cpp
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
#include "Test_ASWUnitTests_Console.h"
//---------------------------------------------------------------------------
#include <cstdint>
#include <string>
#if defined(_WIN32)
#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#endif
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Console
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Console::TTest_ASWUnitTests_Console()
    : inherited("ASWUnitTests_Console_Tests")
{
#if defined(_WIN32)
    RegisterTest(&TTest_ASWUnitTests_Console::Test_ConsoleUTF8Output_SetsUTF8UntilRestored,
        "ConsoleUTF8Output_SetsUTF8UntilRestored");
#endif
    RegisterTest(&TTest_ASWUnitTests_Console::Test_ParseColorName_RejectsInvalid, "ParseColorName_RejectsInvalid");
    RegisterTest(&TTest_ASWUnitTests_Console::Test_ParseColorName_Recognizes, "ParseColorName_Recognizes");
    RegisterTest(&TTest_ASWUnitTests_Console::Test_StripColor_RemovesColorCodesOnly, "StripColor_RemovesColorCodesOnly");
    RegisterTest(&TTest_ASWUnitTests_Console::Test_StripColor_RemovesWhatColorizeAdds,
        "StripColor_RemovesWhatColorizeAdds");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Console::~TTest_ASWUnitTests_Console()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
#if defined(_WIN32)
void TTest_ASWUnitTests_Console::Test_ConsoleUTF8Output_SetsUTF8UntilRestored()
{
    // Arrange: the console runner has already set UTF-8 (see main()), so another code page is set first, and the
    // original is put back at the end.
    UINT const outerCodePage = GetConsoleOutputCP();
    if (outerCodePage == 0 || !SetConsoleOutputCP(437))
        Skip(__func__, __LINE__, "needs a console, which a GUI runner doesn't have");

    // Act
    UINT duringFirst = 0;
    UINT afterRestore = 0;
    UINT duringSecond = 0;
    UINT afterDestructor = 0;
    {
        TConsoleUTF8Output first;
        duringFirst = GetConsoleOutputCP();
        first.Restore();
        afterRestore = GetConsoleOutputCP();
        SetConsoleOutputCP(850); // Which the destructor, restoring again, mustn't undo.
    }
    UINT const afterFirstDestructor = GetConsoleOutputCP();
    {
        TConsoleUTF8Output const second;
        duringSecond = GetConsoleOutputCP();
    }
    afterDestructor = GetConsoleOutputCP();

    SetConsoleOutputCP(outerCodePage);

    // Assert
    CheckEquals(static_cast<uint32_t>(CP_UTF8), static_cast<uint32_t>(duringFirst), __func__, __LINE__,
        "UTF-8 while it exists");
    CheckEquals(437u, static_cast<uint32_t>(afterRestore), __func__, __LINE__, "Restore() puts the original back");
    CheckEquals(850u, static_cast<uint32_t>(afterFirstDestructor), __func__, __LINE__,
        "and the destructor then leaves the console alone");
    CheckEquals(static_cast<uint32_t>(CP_UTF8), static_cast<uint32_t>(duringSecond), __func__, __LINE__,
        "UTF-8 again for a second one");
    CheckEquals(850u, static_cast<uint32_t>(afterDestructor), __func__, __LINE__,
        "whose destructor puts the original back");
}
#endif
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::Test_ParseColorName_RejectsInvalid()
{
    // Act & Assert
    CheckFalse(TConsole::ParseColorName("").has_value(), __func__, __LINE__, "empty name");
    CheckFalse(TConsole::ParseColorName("bogus").has_value(), __func__, __LINE__, "unrecognized name");
    CheckFalse(TConsole::ParseColorName("Red").has_value(), __func__, __LINE__, "case-sensitive: capitalized not recognized");
    CheckFalse(TConsole::ParseColorName("bright").has_value(), __func__, __LINE__, "'bright' alone, without a color, is not valid");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::Test_ParseColorName_Recognizes()
{
    // Act & Assert
    CheckTrue(TConsole::ParseColorName("default") == TConsoleColor::Default, __func__, __LINE__, "default");
    CheckTrue(TConsole::ParseColorName("black") == TConsoleColor::Black, __func__, __LINE__, "black");
    CheckTrue(TConsole::ParseColorName("red") == TConsoleColor::Red, __func__, __LINE__, "red");
    CheckTrue(TConsole::ParseColorName("white") == TConsoleColor::White, __func__, __LINE__, "white");
    CheckTrue(TConsole::ParseColorName("bright-red") == TConsoleColor::BrightRed, __func__, __LINE__, "bright-red");
    CheckTrue(TConsole::ParseColorName("bright-white") == TConsoleColor::BrightWhite, __func__, __LINE__, "bright-white");
}
//---------------------------------------------------------------------------

void TTest_ASWUnitTests_Console::Test_StripColor_RemovesColorCodesOnly()
{
    // Act & Assert
    CheckEquals(std::string("red"), TConsole::StripColor("\x1b[31mred\x1b[0m"), __func__, __LINE__,
        "a color and its reset");
    CheckEquals(std::string("a b"), TConsole::StripColor("a\x1b[1;92m b"), __func__, __LINE__,
        "a code with several numbers");
    CheckEquals(std::string("ab"), TConsole::StripColor("a\x1b[mb"), __func__, __LINE__, "a code with no number");
    CheckEquals(std::string("plain [31m text"), TConsole::StripColor("plain [31m text"), __func__, __LINE__,
        "no ESC, so no code");
    CheckEquals(std::string("\x1b[2J"), TConsole::StripColor("\x1b[2J"), __func__, __LINE__,
        "another kind of sequence is kept");
    CheckEquals(std::string("x\x1b[31"), TConsole::StripColor("x\x1b[31"), __func__, __LINE__,
        "an incomplete code at the end is kept");
    CheckEquals(std::string("\x1b"), TConsole::StripColor("\x1b"), __func__, __LINE__, "a lone ESC is kept");
    CheckEquals(std::string(), TConsole::StripColor(std::string()), __func__, __LINE__, "empty text");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Console::Test_StripColor_RemovesWhatColorizeAdds()
{
    // Arrange
    TColorMode const savedMode = TConsole::GetColorMode();
    TConsole::SetColorMode(TColorMode::Always);
    bool const modeSet = TConsole::GetColorMode() == TColorMode::Always;

    // Act
    std::string const colored = TConsole::Colorize("failed", TLogKind::Fail);
    TConsole::SetColorMode(savedMode);

    // Assert
    CheckTrue(modeSet, __func__, __LINE__, "GetColorMode() returns the mode SetColorMode() set");
    CheckTrue(TConsole::GetColorMode() == savedMode, __func__, __LINE__, "and the mode restored afterwards");
    CheckNotEquals(std::string("failed"), colored, __func__, __LINE__, "Colorize() added color");
    CheckEquals(std::string("failed"), TConsole::StripColor(colored), __func__, __LINE__,
        "StripColor() removes all of it");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Console)
