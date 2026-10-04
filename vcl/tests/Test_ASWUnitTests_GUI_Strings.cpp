/* **************************************************************************
Test_ASWUnitTests_GUI_Strings.cpp
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
#include "Test_ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Strings
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Strings::TTest_ASWUnitTests_GUI_Strings()
    : inherited("ASWUnitTests_GUI_Strings_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Strings::Test_FromUTF8_ConvertsNonASCII, "FromUTF8_ConvertsNonASCII");
    RegisterTest(&TTest_ASWUnitTests_GUI_Strings::Test_ToUTF8_ConvertsNonASCII, "ToUTF8_ConvertsNonASCII");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Strings::~TTest_ASWUnitTests_GUI_Strings()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::Test_FromUTF8_ConvertsNonASCII()
{
    // Arrange
    std::string const utf8("caf\xC3\xA9");

    // Act
    System::UnicodeString const text = FromUTF8(utf8);

    // Assert
    CheckTrue(text == System::UnicodeString(L"caf\x00E9"), __func__, __LINE__,
        "two UTF-8 bytes become the one character U+00E9");
    CheckTrue(FromUTF8(std::string()).IsEmpty(), __func__, __LINE__, "an empty string stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Strings::Test_ToUTF8_ConvertsNonASCII()
{
    // Arrange
    System::UnicodeString const text(L"caf\x00E9");

    // Act
    std::string const utf8 = ToUTF8(text);

    // Assert
    CheckEquals(std::string("caf\xC3\xA9"), utf8, __func__, __LINE__, "U+00E9 becomes its two UTF-8 bytes");
    CheckTrue(ToUTF8(System::UnicodeString()).empty(), __func__, __LINE__, "an empty string stays empty");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Strings)
