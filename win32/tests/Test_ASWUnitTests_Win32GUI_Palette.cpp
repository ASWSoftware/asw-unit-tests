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
#include <algorithm>
#include <cctype>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
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
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_CarbonPalette_HasCarbonsColors,
        "CarbonPalette_HasCarbonsColors");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_CarbonThemeFileText_ReadsBackAsCarbon,
        "CarbonThemeFileText_ReadsBackAsCarbon");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_FailureAndSkipTextColorOn_AreLighterOnADarkBackground,
        "FailureAndSkipTextColorOn_AreLighterOnADarkBackground");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_IsDarkColor_IsByTheLuma, "IsDarkColor_IsByTheLuma");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_PaletteRoles_ListEveryMemberOnce,
        "PaletteRoles_ListEveryMemberOnce");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Palette::Test_PaletteWithThemeColors_SetsOnlyTheirRoles,
        "PaletteWithThemeColors_SetsOnlyTheirRoles");
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
void TTest_ASWUnitTests_Win32GUI_Palette::Test_CarbonPalette_HasCarbonsColors()
{
    // Act
    TWin32GUIPalette const palette = CarbonPalette();

    // Assert: a few of Carbon.vsf's own colors, and the VCL GUI runner's text colors on its window color.
    CheckEquals(RGB(0x19, 0x19, 0x19), palette.Window, __func__, __LINE__, "clWindow");
    CheckEquals(RGB(0xEA, 0xEA, 0xEA), palette.WindowText, __func__, __LINE__, "clWindowText");
    CheckEquals(RGB(0x33, 0x33, 0x33), palette.Panel, __func__, __LINE__, "clBtnFace");
    CheckEquals(RGB(0xC0, 0xC0, 0xC0), palette.ControlText, __func__, __LINE__, "sfButtonTextNormal");
    CheckEquals(RGB(0x26, 0x5A, 0x73), palette.Highlight, __func__, __LINE__, "the hot menu item");
    CheckEquals(RGB(0x19, 0x19, 0x19), palette.MenuBackground, __func__, __LINE__, "the menus' background");
    CheckEquals(RGB(255, 100, 100), palette.FailureText, __func__, __LINE__, "a failure on the dark window");
    CheckEquals(RGB(255, 190, 70), palette.SkipText, __func__, __LINE__, "a skip on the dark window");
    CheckTrue(IsDarkColor(palette.Window) && IsDarkColor(palette.Panel) && !IsDarkColor(palette.WindowText) &&
        !IsDarkColor(palette.ControlText), __func__, __LINE__, "dark backgrounds, light text");
    CheckEquals(RGB(215, 40, 40), palette.TreeStatusFailed, __func__, __LINE__, "the tree's failed dot, as light's");
    CheckEquals(RGB(16, 160, 16), palette.TreeStatusPassed, __func__, __LINE__, "the tree's passed dot, as light's");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_CarbonThemeFileText_ReadsBackAsCarbon()
{
    // Act
    std::string const text = CarbonThemeFileText();
    TGUIThemeFile const file = ReadGUIThemeFile(text, PaletteRoleNames());
    TWin32GUIPalette const blank = {};
    TWin32GUIPalette const readBack = PaletteWithThemeColors(blank, file.Colors);

    // Assert
    CheckTrue(file.Usable, __func__, __LINE__, "a usable theme file");
    CheckEquals(std::string(), std::string(file.Problems.empty() ? "" : file.Problems.front()), __func__, __LINE__,
        "with no problems");
    CheckEquals(PaletteRoles().size(), file.Colors.size(), __func__, __LINE__, "setting every role");

    TWin32GUIPalette const carbon = CarbonPalette();
    std::string differences;

    for (TWin32GUIPaletteRole const& role : PaletteRoles())
    {
        if (readBack.*role.Color != carbon.*role.Color)
            differences += std::string(differences.empty() ? "" : ", ") + role.Name;
    }

    CheckEmpty(differences, __func__, __LINE__, "to Carbon's colors");

    CheckTrue(text.compare(0, 30, "; An ASWUnitTests theme file: ") == 0, __func__, __LINE__,
        "starting with what it is");
    CheckTrue(text.find("\r\n[Theme]\r\nBase=Dark\r\n\r\n[Colors]\r\n; Push buttons: ") != std::string::npos,
        __func__, __LINE__, "Base=Dark, then the colors, under their groups' comments");
    CheckTrue(text.find("\r\n\r\n; Tooltips.\r\nTooltipBackground=#515151\r\nTooltipText=#EAEAEA\r\n\r\n") !=
        std::string::npos, __func__, __LINE__, "a group, with a blank line on each side");

    bool lineBreaksAreCRLF = true;
    bool ascii = true;
    std::size_t longest = 0;
    std::size_t lineStart = 0;

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        if (static_cast<unsigned char>(text[i]) > 0x7F)
            ascii = false;

        if (text[i] == '\n' && (i == 0 || text[i - 1] != '\r'))
            lineBreaksAreCRLF = false;

        if (text[i] == '\n')
        {
            longest = (std::max)(longest, i - lineStart - 1);
            lineStart = i + 1;
        }
    }

    CheckTrue(lineBreaksAreCRLF, __func__, __LINE__, "CRLF line breaks");
    CheckTrue(ascii, __func__, __LINE__, "plain ASCII");
    CheckTrue(longest <= 110, __func__, __LINE__, "no line longer than 110: " + std::to_string(longest));
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
void TTest_ASWUnitTests_Win32GUI_Palette::Test_IsDarkColor_IsByTheLuma()
{
    // Act, Assert: Rec. 601 luma, (299 R + 587 G + 114 B) / 1000, below 128.
    CheckTrue(IsDarkColor(RGB(127, 127, 127)), __func__, __LINE__, "gray 127");
    CheckFalse(IsDarkColor(RGB(128, 128, 128)), __func__, __LINE__, "gray 128");
    CheckTrue(IsDarkColor(RGB(0, 0, 255)), __func__, __LINE__, "blue, luma 29");
    CheckFalse(IsDarkColor(RGB(0, 255, 0)), __func__, __LINE__, "green, luma 149");
    CheckTrue(IsDarkColor(RGB(255, 0, 0)), __func__, __LINE__, "red, luma 76");
    CheckFalse(IsDarkColor(RGB(255, 255, 0)), __func__, __LINE__, "yellow, luma 225");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_PaletteRoles_ListEveryMemberOnce()
{
    // Arrange
    std::vector<TWin32GUIPaletteRole> const& roles = PaletteRoles();

    // Act: each role's member set to its own number, so two roles of one member would leave the first's wrong.
    TWin32GUIPalette palette = {};

    for (std::size_t i = 0; i < roles.size(); ++i)
        palette.*roles[i].Color = static_cast<COLORREF>(i + 1);

    // Assert
    CheckEquals(sizeof(TWin32GUIPalette) / sizeof(COLORREF), roles.size(), __func__, __LINE__,
        "as many roles as the palette has members");

    std::string wrong;

    for (std::size_t i = 0; i < roles.size(); ++i)
    {
        if (palette.*roles[i].Color != static_cast<COLORREF>(i + 1))
            wrong += std::string(wrong.empty() ? "" : ", ") + roles[i].Name;
    }

    CheckEmpty(wrong, __func__, __LINE__, "each a member of its own");

    std::vector<std::string> const names = PaletteRoleNames();
    std::string repeated;

    for (std::size_t i = 0; i < names.size(); ++i)
    {
        for (std::size_t j = i + 1; j < names.size(); ++j)
        {
            // Theme files match them ignoring case.
            bool const same = names[i].size() == names[j].size() && std::equal(names[i].begin(), names[i].end(),
                names[j].begin(), [](char a, char b)
                {
                    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
                });

            if (same)
                repeated += names[j] + " ";
        }
    }

    CheckEmpty(repeated, __func__, __LINE__, "names that differ, ignoring case");
    CheckEquals(roles.size(), names.size(), __func__, __LINE__, "PaletteRoleNames(), one per role");
    CheckTrue(!roles.empty() && roles.front().Group != nullptr, __func__, __LINE__, "the first role starts a group");
    CheckEquals(std::string("ButtonBorder"), names.front(), __func__, __LINE__, "named as its member");
    CheckEquals(std::string("WindowText"), names.back(), __func__, __LINE__, "in the struct's order");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Palette::Test_PaletteWithThemeColors_SetsOnlyTheirRoles()
{
    // Arrange
    TWin32GUIPalette const carbon = CarbonPalette();
    std::vector<TGUIThemeColor> colors(3);
    colors[0].Role = "Window";
    colors[0].RGB = 0x102030;
    colors[1].Role = "NotARole";
    colors[1].RGB = 0xFFFFFF;
    colors[2].Role = "TreeStatusPassed";
    colors[2].RGB = 0xA0B0C0;

    // Act
    TWin32GUIPalette const palette = PaletteWithThemeColors(carbon, colors);

    // Assert
    CheckEquals(RGB(0x10, 0x20, 0x30), palette.Window, __func__, __LINE__, "#RRGGBB as a COLORREF");
    CheckEquals(RGB(0xA0, 0xB0, 0xC0), palette.TreeStatusPassed, __func__, __LINE__, "a status dot");

    std::string changed;

    for (TWin32GUIPaletteRole const& role : PaletteRoles())
    {
        if (std::string(role.Name) != "Window" && std::string(role.Name) != "TreeStatusPassed" &&
            palette.*role.Color != carbon.*role.Color)
        {
            changed += std::string(changed.empty() ? "" : ", ") + role.Name;
        }
    }

    CheckEmpty(changed, __func__, __LINE__, "the others keep their colors, and an unknown role is left out");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_Palette)
