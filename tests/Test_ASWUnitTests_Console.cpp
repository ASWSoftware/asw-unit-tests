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
#include <string>
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
