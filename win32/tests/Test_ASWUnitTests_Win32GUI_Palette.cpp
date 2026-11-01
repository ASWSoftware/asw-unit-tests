/* **************************************************************************
Test_ASWUnitTests_Win32GUI_Palette.cpp
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
#include "Test_ASWUnitTests_Win32GUI_Palette.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Palette.h"
#include "ASWUnitTests_Win32GUI_Palette.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_Palette
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_Palette::TTest_ASWUnitTests_Win32GUI_Palette()
    : inherited("ASWUnitTests_Win32GUI_Palette_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_BlendColors_MixesEachChannelInProportion,
        "BlendColors_MixesEachChannelInProportion");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_FailureAndSkipTextColorOn_AreLighterOnADarkBackground,
        "FailureAndSkipTextColorOn_AreLighterOnADarkBackground");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_ToWin32Palette_ConvertsEveryRole,
        "ToWin32Palette_ConvertsEveryRole");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_Palette::~TTest_ASWUnitTests_Win32GUI_Palette()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_BlendColors_MixesEachChannelInProportion()
{
    // Arrange
    COLORREF const black = RGB(0, 0, 0);
    COLORREF const white = RGB(255, 255, 255);

    // Act, Assert
    CheckEquals(black, BlendColors(black, white, 0, 4), __func__, __LINE__, "'from' at 0");
    CheckEquals(white, BlendColors(black, white, 4, 4), __func__, __LINE__, "'to' at the denominator");
    CheckEquals(RGB(128, 128, 128), BlendColors(black, white, 1, 2), __func__, __LINE__, "halfway, rounded");
    CheckEquals(RGB(20, 40, 60), BlendColors(RGB(10, 20, 30), RGB(30, 60, 90), 1, 2), __func__, __LINE__,
        "each channel on its own");
    CheckEquals(RGB(64, 64, 64), BlendColors(black, white, 1, 4), __func__, __LINE__, "a quarter, rounded");
    CheckEquals(white, BlendColors(black, white, 5, 4), __func__, __LINE__, "past the denominator is 'to'");
    CheckEquals(black, BlendColors(black, white, -1, 4), __func__, __LINE__, "below 0 is 'from'");
    CheckEquals(black, BlendColors(black, white, 1, 0), __func__, __LINE__, "'from' with no denominator");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_FailureAndSkipTextColorOn_AreLighterOnADarkBackground()
{
    // Act, Assert: as the VCL GUI runner's colors, by the background's luma, so a dark high contrast theme's
    // window color also gets the lighter ones.
    CheckEquals(RGB(200, 0, 0), FailureTextColorOn(RGB(255, 255, 255)), __func__, __LINE__, "failure on white");
    CheckEquals(RGB(255, 100, 100), FailureTextColorOn(RGB(0x19, 0x19, 0x19)), __func__, __LINE__,
        "failure on Carbon's window color");
    CheckEquals(RGB(255, 100, 100), FailureTextColorOn(RGB(0, 0, 0)), __func__, __LINE__, "failure on black");
    CheckEquals(RGB(190, 120, 0), SkipTextColorOn(RGB(255, 255, 255)), __func__, __LINE__, "skip on white");
    CheckEquals(RGB(255, 190, 70), SkipTextColorOn(RGB(0x19, 0x19, 0x19)), __func__, __LINE__,
        "skip on Carbon's window color");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_ToWin32Palette_ConvertsEveryRole()
{
    // Arrange: each role a color of its own, with a different value in each channel.
    std::vector<TGUIPaletteRole> const& roles = GUIPaletteRoles();
    TGUIPalette palette = {};

    for (std::size_t i = 0; i < roles.size(); ++i)
        palette.*roles[i].Color = GUIRGB(static_cast<int>(i), static_cast<int>(100 + i), static_cast<int>(200 - i));

    // Act
    TWin32GUIPalette const converted = ToWin32Palette(palette);

    // Assert
    std::vector<TGUIPaletteRoleOf<COLORREF> > const& win32Roles = GUIPaletteRolesOf<COLORREF>();
    std::string wrong;

    for (std::size_t i = 0; i < win32Roles.size(); ++i)
    {
        COLORREF const expected = RGB(i, 100 + i, 200 - i);

        if (converted.*win32Roles[i].Color != expected)
            wrong += std::string(wrong.empty() ? "" : ", ") + win32Roles[i].Name;
    }

    CheckEquals(roles.size(), win32Roles.size(), __func__, __LINE__, "the same roles");
    CheckEmpty(wrong, __func__, __LINE__, "each role's color, as a COLORREF");
    CheckEquals(RGB(0x19, 0x19, 0x19), ToWin32Palette(CarbonGUIPalette()).Window, __func__, __LINE__,
        "Carbon's window color");
    CheckEquals(RGB(0x26, 0x5A, 0x73), ToWin32Palette(CarbonGUIPalette()).Highlight, __func__, __LINE__,
        "Carbon's highlight, a color with three different channels");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_Palette)
