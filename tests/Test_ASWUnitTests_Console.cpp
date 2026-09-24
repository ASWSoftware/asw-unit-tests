/* **************************************************************************
Test_ASWUnitTests_Console.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "Test_ASWUnitTests_Console.h"
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

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Console)
