/* **************************************************************************
Test_ASWUnitTests_GUI_Palette.cpp
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
#include "Test_ASWUnitTests_GUI_Palette.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Palette.h"
#include "ASWUnitTests_GUI_Theme.h"
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

double ContrastRatio(std::uint32_t a, std::uint32_t b);
double RelativeLuminance(std::uint32_t color);

//---------------------------------------------------------------------------
/*
    ContrastRatio

    WCAG 2's contrast ratio of two colors, 1 to 21; text needs at least 4.5 on its background to be readable.
*/
double ContrastRatio(std::uint32_t a, std::uint32_t b)
{
    double const lighter = (std::max)(RelativeLuminance(a), RelativeLuminance(b));
    double const darker = (std::min)(RelativeLuminance(a), RelativeLuminance(b));

    return (lighter + 0.05) / (darker + 0.05);
}
//---------------------------------------------------------------------------
/*
    RelativeLuminance

    WCAG 2's relative luminance of an sRGB color, 0 for black to 1 for white.
*/
double RelativeLuminance(std::uint32_t color)
{
    auto const linear = [](int channel)
        {
            double const value = channel / 255.0;

            return value <= 0.04045 ? value / 12.92 : std::pow((value + 0.055) / 1.055, 2.4);
        };

    return 0.2126 * linear(ASWUnitTests::GUIRed(color)) + 0.7152 * linear(ASWUnitTests::GUIGreen(color)) +
        0.0722 * linear(ASWUnitTests::GUIBlue(color));
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Palette
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Palette::TTest_ASWUnitTests_GUI_Palette()
    : inherited("ASWUnitTests_GUI_Palette_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_BlendGUIColors_MixesEachChannelInProportion,
        "BlendGUIColors_MixesEachChannelInProportion");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_CarbonGUIPalette_HasCarbonsColors,
        "CarbonGUIPalette_HasCarbonsColors");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_CarbonGUIThemeFileText_ReadsBackAsCarbon,
        "CarbonGUIThemeFileText_ReadsBackAsCarbon");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_FindGUIBuiltInDarkTheme_IgnoresCase,
        "FindGUIBuiltInDarkTheme_IgnoresCase");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIBuiltInDarkThemes_AreCarbonThenItsTints,
        "GUIBuiltInDarkThemes_AreCarbonThenItsTints");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIBuiltInDarkThemes_KeepTextReadable,
        "GUIBuiltInDarkThemes_KeepTextReadable");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIFailureAndSkipTextColorOn_AreLighterOnADarkBackground,
        "GUIFailureAndSkipTextColorOn_AreLighterOnADarkBackground");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIPaletteRoles_ListEveryMemberOnce,
        "GUIPaletteRoles_ListEveryMemberOnce");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIPaletteWithThemeColors_SetsOnlyTheirRoles,
        "GUIPaletteWithThemeColors_SetsOnlyTheirRoles");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_GUIRGB_PacksTheChannelsAsRRGGBB,
        "GUIRGB_PacksTheChannelsAsRRGGBB");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_IsDarkGUIColor_IsByTheLuma, "IsDarkGUIColor_IsByTheLuma");
    RegisterTest(&TTest_ASWUnitTests_GUI_Palette::Test_TintedCarbonGUIPalette_TintsOnlyTheGraysAndTheAccent,
        "TintedCarbonGUIPalette_TintsOnlyTheGraysAndTheAccent");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Palette::~TTest_ASWUnitTests_GUI_Palette()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_BlendGUIColors_MixesEachChannelInProportion()
{
    // Arrange
    std::uint32_t const black = GUIRGB(0, 0, 0);
    std::uint32_t const white = GUIRGB(255, 255, 255);

    // Act, Assert
    CheckEquals(black, BlendGUIColors(black, white, 0, 4), __func__, __LINE__, "'from' at 0");
    CheckEquals(white, BlendGUIColors(black, white, 4, 4), __func__, __LINE__, "'to' at the denominator");
    CheckEquals(GUIRGB(128, 128, 128), BlendGUIColors(black, white, 1, 2), __func__, __LINE__, "halfway, rounded");
    CheckEquals(GUIRGB(20, 40, 60), BlendGUIColors(GUIRGB(10, 20, 30), GUIRGB(30, 60, 90), 1, 2), __func__, __LINE__,
        "each channel on its own");
    CheckEquals(GUIRGB(64, 64, 64), BlendGUIColors(black, white, 1, 4), __func__, __LINE__, "a quarter, rounded");
    CheckEquals(white, BlendGUIColors(black, white, 5, 4), __func__, __LINE__, "past the denominator is 'to'");
    CheckEquals(black, BlendGUIColors(black, white, -1, 4), __func__, __LINE__, "below 0 is 'from'");
    CheckEquals(black, BlendGUIColors(black, white, 1, 0), __func__, __LINE__, "'from' with no denominator");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_CarbonGUIPalette_HasCarbonsColors()
{
    // Act
    TGUIPalette const palette = CarbonGUIPalette();

    // Assert: a few of Carbon.vsf's own colors, and the VCL GUI runner's text colors on its window color.
    CheckEquals(GUIRGB(0x19, 0x19, 0x19), palette.Window, __func__, __LINE__, "clWindow");
    CheckEquals(GUIRGB(0xEA, 0xEA, 0xEA), palette.WindowText, __func__, __LINE__, "clWindowText");
    CheckEquals(GUIRGB(0x33, 0x33, 0x33), palette.Panel, __func__, __LINE__, "clBtnFace");
    CheckEquals(GUIRGB(0xC0, 0xC0, 0xC0), palette.ControlText, __func__, __LINE__, "sfButtonTextNormal");
    CheckEquals(GUIRGB(0x26, 0x5A, 0x73), palette.Highlight, __func__, __LINE__, "the hot menu item");
    CheckEquals(GUIRGB(0x19, 0x19, 0x19), palette.MenuBackground, __func__, __LINE__, "the menus' background");
    CheckEquals(GUIRGB(255, 100, 100), palette.FailureText, __func__, __LINE__, "a failure on the dark window");
    CheckEquals(GUIRGB(255, 190, 70), palette.SkipText, __func__, __LINE__, "a skip on the dark window");
    CheckTrue(IsDarkGUIColor(palette.Window) && IsDarkGUIColor(palette.Panel) && !IsDarkGUIColor(palette.WindowText) &&
        !IsDarkGUIColor(palette.ControlText), __func__, __LINE__, "dark backgrounds, light text");
    CheckEquals(GUIRGB(215, 40, 40), palette.TreeStatusFailed, __func__, __LINE__, "the tree's failed dot, as light's");
    CheckEquals(GUIRGB(16, 160, 16), palette.TreeStatusPassed, __func__, __LINE__, "the tree's passed dot, as light's");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_CarbonGUIThemeFileText_ReadsBackAsCarbon()
{
    // Act
    std::string const text = CarbonGUIThemeFileText();
    TGUIThemeFile const file = ReadGUIThemeFile(text, GUIPaletteRoleNames());
    TGUIPalette const blank = {};
    TGUIPalette const readBack = GUIPaletteWithThemeColors(blank, file.Colors);

    // Assert
    CheckTrue(file.Usable, __func__, __LINE__, "a usable theme file");
    CheckEquals(std::string(), std::string(file.Problems.empty() ? "" : file.Problems.front()), __func__, __LINE__,
        "with no problems");
    CheckEquals(GUIPaletteRoles().size(), file.Colors.size(), __func__, __LINE__, "setting every role");

    TGUIPalette const carbon = CarbonGUIPalette();
    std::string differences;

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
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
void TTest_ASWUnitTests_GUI_Palette::Test_FindGUIBuiltInDarkTheme_IgnoresCase()
{
    // Act
    TGUIBuiltInTheme const* const midnight = FindGUIBuiltInDarkTheme("mIDNIGHT");
    TGUIBuiltInTheme const* const carbon = FindGUIBuiltInDarkTheme("Carbon");

    // Assert
    AssertTrue(midnight != nullptr, __func__, __LINE__, "a built-in theme, in another case");
    CheckEquals(std::string("Midnight"), midnight->Name, __func__, __LINE__, "with its own name");
    AssertTrue(carbon != nullptr, __func__, __LINE__, "Carbon");
    CheckEquals(std::string("Carbon"), carbon->Name, __func__, __LINE__, "named so");
    CheckTrue(FindGUIBuiltInDarkTheme("Midnight Blue") == nullptr, __func__, __LINE__, "none for a longer name");
    CheckTrue(FindGUIBuiltInDarkTheme("Midnigh") == nullptr, __func__, __LINE__, "none for a shorter one");
    CheckTrue(FindGUIBuiltInDarkTheme(std::string()) == nullptr, __func__, __LINE__, "none for an empty one");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_GUIBuiltInDarkThemes_AreCarbonThenItsTints()
{
    // Act
    std::vector<TGUIBuiltInTheme> const& themes = GUIBuiltInDarkThemes();

    // Assert
    std::vector<std::string> nameList;
    std::string names;

    for (TGUIBuiltInTheme const& theme : themes)
    {
        nameList.push_back(theme.Name);
        names += theme.Name + "\n";
    }

    CheckEquals(std::string("Carbon\nCharcoal Dark Slate\nCobalt XEMedia\nEmber\nForest\nMidnight\nOnyx Blue\nPlum\n"
        "Tablet Dark\n"), names, __func__, __LINE__, "Carbon first, then the VCL styles and the tints by name");

    for (std::string const& style : GUIVCLStyleDarkThemeNames())
    {
        CheckTrue(std::find(nameList.begin(), nameList.end(), style) != nameList.end(), __func__, __LINE__,
            "the VCL GUI runner's built-in style " + style + ", by its name");
    }

    // The menu's other items' accelerators: Save Theme Template's and Open Themes Folder's.
    std::set<char> letters = { 's', 'o' };
    bool lettersDiffer = true;

    for (std::string const& caption : GUIMenuItemTextsWithAccelerators(nameList, "so"))
    {
        std::size_t const accelerator = caption.find('&');
        lettersDiffer = lettersDiffer && accelerator != std::string::npos && accelerator + 1 < caption.size() &&
            letters.insert(static_cast<char>(std::tolower(static_cast<unsigned char>(caption[accelerator + 1])))).second;
    }

    CheckTrue(lettersDiffer, __func__, __LINE__, "each with an accelerator of its own in the menu");
    AssertFalse(themes.empty(), __func__, __LINE__, "Carbon is there");
    TGUIPalette const carbon = CarbonGUIPalette();
    bool sameAsCarbon = true;

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
        sameAsCarbon = sameAsCarbon && themes.front().Palette.*role.Color == carbon.*role.Color;

    CheckEquals(GUIDefaultDarkThemeName(), themes.front().Name, __func__, __LINE__, "the default first");
    CheckTrue(sameAsCarbon, __func__, __LINE__, "in CarbonGUIPalette()'s colors");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_GUIBuiltInDarkThemes_KeepTextReadable()
{
    // Arrange: each text color the GUI draws on a background, with that background, a gradient's by its middle,
    // where the text is.
    struct TPair
    {
        std::uint32_t TGUIPalette::* Text;
        std::uint32_t TGUIPalette::* Background;
        std::uint32_t TGUIPalette::* BackgroundBottom;
        char const* Name;
    };

    TPair const pairs[] = {
        { &TGUIPalette::WindowText, &TGUIPalette::Window, nullptr, "WindowText on Window" },
        { &TGUIPalette::ControlText, &TGUIPalette::Panel, nullptr, "ControlText on Panel" },
        { &TGUIPalette::ControlText, &TGUIPalette::MenuBackground, nullptr, "ControlText on MenuBackground" },
        { &TGUIPalette::ButtonText, &TGUIPalette::ButtonFaceTop, &TGUIPalette::ButtonFaceBottom,
          "ButtonText on the button's face" },
        { &TGUIPalette::ButtonHotText, &TGUIPalette::ButtonHotFaceTop, &TGUIPalette::ButtonHotFaceBottom,
          "ButtonHotText on the hot button's face" },
        { &TGUIPalette::HotText, &TGUIPalette::Highlight, nullptr, "HotText on Highlight" },
        { &TGUIPalette::SelectionText, &TGUIPalette::Selection, nullptr, "SelectionText on Selection" },
        { &TGUIPalette::FailureText, &TGUIPalette::Window, nullptr, "FailureText on Window" },
        { &TGUIPalette::SkipText, &TGUIPalette::Window, nullptr, "SkipText on Window" },
        { &TGUIPalette::TooltipText, &TGUIPalette::TooltipBackground, nullptr,
          "TooltipText on TooltipBackground" },
    };

    // Act
    std::string unreadable;

    for (TGUIBuiltInTheme const& theme : GUIBuiltInDarkThemes())
    {
        for (TPair const& pair : pairs)
        {
            std::uint32_t const background = pair.BackgroundBottom == nullptr ? theme.Palette.*pair.Background :
                    BlendGUIColors(theme.Palette.*pair.Background, theme.Palette.*pair.BackgroundBottom, 1, 2);

            if (ContrastRatio(theme.Palette.*pair.Text, background) < 4.5)
                unreadable += theme.Name + ": " + pair.Name + "\n";
        }
    }

    // Assert
    CheckEmpty(unreadable, __func__, __LINE__, "every text at WCAG's 4.5 to 1 or more on its background");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_GUIFailureAndSkipTextColorOn_AreLighterOnADarkBackground()
{
    // Act, Assert: as the VCL GUI runner's colors, by the background's luma, so a dark high contrast theme's
    // window color also gets the lighter ones.
    CheckEquals(GUIRGB(200, 0, 0), GUIFailureTextColorOn(GUIRGB(255, 255, 255)), __func__, __LINE__,
        "failure on white");
    CheckEquals(GUIRGB(255, 100, 100), GUIFailureTextColorOn(GUIRGB(0x19, 0x19, 0x19)), __func__, __LINE__,
        "failure on Carbon's window color");
    CheckEquals(GUIRGB(255, 100, 100), GUIFailureTextColorOn(GUIRGB(0, 0, 0)), __func__, __LINE__, "failure on black");
    CheckEquals(GUIRGB(190, 120, 0), GUISkipTextColorOn(GUIRGB(255, 255, 255)), __func__, __LINE__, "skip on white");
    CheckEquals(GUIRGB(255, 190, 70), GUISkipTextColorOn(GUIRGB(0x19, 0x19, 0x19)), __func__, __LINE__,
        "skip on Carbon's window color");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_GUIPaletteRoles_ListEveryMemberOnce()
{
    // Arrange
    std::vector<TGUIPaletteRole> const& roles = GUIPaletteRoles();

    // Act: each role's member set to its own number, so two roles of one member would leave the first's wrong.
    TGUIPalette palette = {};

    for (std::size_t i = 0; i < roles.size(); ++i)
        palette.*roles[i].Color = static_cast<std::uint32_t>(i + 1);

    // Assert
    CheckEquals(sizeof(TGUIPalette) / sizeof(std::uint32_t), roles.size(), __func__, __LINE__,
        "as many roles as the palette has members");

    std::string wrong;

    for (std::size_t i = 0; i < roles.size(); ++i)
    {
        if (palette.*roles[i].Color != static_cast<std::uint32_t>(i + 1))
            wrong += std::string(wrong.empty() ? "" : ", ") + roles[i].Name;
    }

    CheckEmpty(wrong, __func__, __LINE__, "each a member of its own");

    std::vector<std::string> const names = GUIPaletteRoleNames();
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
    CheckEquals(roles.size(), names.size(), __func__, __LINE__, "GUIPaletteRoleNames(), one per role");
    CheckTrue(!roles.empty() && roles.front().Group != nullptr, __func__, __LINE__, "the first role starts a group");
    CheckEquals(std::string("ButtonBorder"), names.front(), __func__, __LINE__, "named as its member");
    CheckEquals(std::string("WindowText"), names.back(), __func__, __LINE__, "in the struct's order");

    // Another color type's roles, which a runner converts a palette through, pair up with these by position.
    std::vector<TGUIPaletteRoleOf<double> > const& otherRoles = GUIPaletteRolesOf<double>();
    std::string unpaired;

    for (std::size_t i = 0; i < roles.size() && i < otherRoles.size(); ++i)
    {
        std::string const group = roles[i].Group == nullptr ? "-" : roles[i].Group;
        std::string const otherGroup = otherRoles[i].Group == nullptr ? "-" : otherRoles[i].Group;

        if (std::string(roles[i].Name) != otherRoles[i].Name || group != otherGroup)
            unpaired += std::string(unpaired.empty() ? "" : ", ") + roles[i].Name;
    }

    CheckEquals(roles.size(), otherRoles.size(), __func__, __LINE__, "another color type's roles, as many");
    CheckEmpty(unpaired, __func__, __LINE__, "in the same order, with the same groups");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_GUIPaletteWithThemeColors_SetsOnlyTheirRoles()
{
    // Arrange
    TGUIPalette const carbon = CarbonGUIPalette();
    std::vector<TGUIThemeColor> colors(3);
    colors[0].Role = "Window";
    colors[0].RGB = 0x102030;
    colors[1].Role = "NotARole";
    colors[1].RGB = 0xFFFFFF;
    colors[2].Role = "TreeStatusPassed";
    colors[2].RGB = 0xA0B0C0;

    // Act
    TGUIPalette const palette = GUIPaletteWithThemeColors(carbon, colors);

    // Assert
    CheckEquals(GUIRGB(0x10, 0x20, 0x30), palette.Window, __func__, __LINE__, "#RRGGBB as is");
    CheckEquals(GUIRGB(0xA0, 0xB0, 0xC0), palette.TreeStatusPassed, __func__, __LINE__, "a status dot");

    std::string changed;

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
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
void TTest_ASWUnitTests_GUI_Palette::Test_GUIRGB_PacksTheChannelsAsRRGGBB()
{
    // Act, Assert
    CheckEquals(std::uint32_t(0x123456), GUIRGB(0x12, 0x34, 0x56), __func__, __LINE__, "red, green, blue");
    CheckEquals(std::uint32_t(0xFFFFFF), GUIRGB(255, 255, 255), __func__, __LINE__, "white");
    CheckEquals(std::uint32_t(0), GUIRGB(0, 0, 0), __func__, __LINE__, "black");
    CheckEquals(0x12, GUIRed(0x123456), __func__, __LINE__, "the red channel");
    CheckEquals(0x34, GUIGreen(0x123456), __func__, __LINE__, "the green channel");
    CheckEquals(0x56, GUIBlue(0x123456), __func__, __LINE__, "the blue channel");
    CheckEquals(0, GUIRed(0x00FFFF), __func__, __LINE__, "no red");
    CheckEquals(255, GUIBlue(0xFF00FF), __func__, __LINE__, "all of the blue");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_IsDarkGUIColor_IsByTheLuma()
{
    // Act, Assert: Rec. 601 luma, (299 R + 587 G + 114 B) / 1000, below 128.
    CheckTrue(IsDarkGUIColor(GUIRGB(127, 127, 127)), __func__, __LINE__, "gray 127");
    CheckFalse(IsDarkGUIColor(GUIRGB(128, 128, 128)), __func__, __LINE__, "gray 128");
    CheckTrue(IsDarkGUIColor(GUIRGB(0, 0, 255)), __func__, __LINE__, "blue, luma 29");
    CheckFalse(IsDarkGUIColor(GUIRGB(0, 255, 0)), __func__, __LINE__, "green, luma 149");
    CheckTrue(IsDarkGUIColor(GUIRGB(255, 0, 0)), __func__, __LINE__, "red, luma 76");
    CheckFalse(IsDarkGUIColor(GUIRGB(255, 255, 0)), __func__, __LINE__, "yellow, luma 225");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Palette::Test_TintedCarbonGUIPalette_TintsOnlyTheGraysAndTheAccent()
{
    // Arrange
    TGUIPalette const carbon = CarbonGUIPalette();

    // Act: grays toward blue, the accent green; and no tint, with the accent's own hue.
    TGUIPalette const tinted = TintedCarbonGUIPalette(240.0, 0.1, 120.0);
    TGUIPalette const untinted = TintedCarbonGUIPalette(240.0, 0.0, 120.0);

    // Assert
    CheckTrue(GUIBlue(tinted.Window) > GUIRed(tinted.Window) && GUIBlue(tinted.Window) > GUIGreen(tinted.Window),
        __func__, __LINE__, "a dark gray turns blue");
    // HSL's lightness, the highest channel and the lowest added, here blue and red.
    CheckTrue(std::abs(GUIBlue(tinted.Window) + GUIRed(tinted.Window) - 2 * GUIRed(carbon.Window)) <= 1,
        __func__, __LINE__, "keeping its lightness");
    CheckTrue(GUIBlue(tinted.WindowText) - GUIRed(tinted.WindowText) < GUIBlue(tinted.Window) -
        GUIRed(tinted.Window), __func__, __LINE__, "a light one less, so text stays nearly white");
    CheckEquals(GUIRGB(255, 255, 255), tinted.HotText, __func__, __LINE__, "white stays white");
    CheckTrue(GUIGreen(tinted.Highlight) > GUIRed(tinted.Highlight) && GUIGreen(tinted.Highlight) >
        GUIBlue(tinted.Highlight), __func__, __LINE__, "the accent takes its own hue");
    CheckEquals(carbon.Window, untinted.Window, __func__, __LINE__, "no tint leaves a gray as it is");
    CheckEquals(carbon.Panel, untinted.Panel, __func__, __LINE__, "every gray");

    // The colors that say how a test did.
    std::uint32_t TGUIPalette::* const kept[] = {
        &TGUIPalette::FailureText, &TGUIPalette::ProgressFailedBottom, &TGUIPalette::ProgressFailedTop,
        &TGUIPalette::SkipText, &TGUIPalette::TreeStatusFailed, &TGUIPalette::TreeStatusNotRun,
        &TGUIPalette::TreeStatusPassed, &TGUIPalette::TreeStatusRunning, &TGUIPalette::TreeStatusSkipped
    };
    bool statusKept = true;

    for (std::uint32_t TGUIPalette::* const color : kept)
        statusKept = statusKept && tinted.*color == carbon.*color;

    CheckTrue(statusKept, __func__, __LINE__, "the status colors stay Carbon's");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Palette)
