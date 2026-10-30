/* **************************************************************************
Test_ASWUnitTests_GUI_Theme.cpp
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
#include "Test_ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

//---------------------------------------------------------------------------

std::string ColorsText(TGUIThemeFile const& file);
std::string JoinLines(std::vector<std::string> const& lines);

//---------------------------------------------------------------------------
/*
    ColorsText

    The file's colors as "Role=#RRGGBB", separated by commas, compared whole, so a failure shows them all.
*/
std::string ColorsText(TGUIThemeFile const& file)
{
    std::string text;

    for (TGUIThemeColor const& color : file.Colors)
        text += (text.empty() ? "" : ",") + color.Role + "=" + FormatGUIThemeColor(color.RGB);

    return text;
}
//---------------------------------------------------------------------------
/*
    JoinLines

    The lines, each followed by a line break, compared whole, so a failure shows them all.
*/
std::string JoinLines(std::vector<std::string> const& lines)
{
    std::string text;

    for (std::string const& line : lines)
        text += line + "\n";

    return text;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Theme
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Theme::TTest_ASWUnitTests_GUI_Theme()
    : inherited("ASWUnitTests_GUI_Theme_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_FormatGUIThemeColor_RoundTripsThroughParse,
        "FormatGUIThemeColor_RoundTripsThroughParse");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_GUIThemeIsDark_FollowsTheChoiceOrWindows,
        "GUIThemeIsDark_FollowsTheChoiceOrWindows");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_GUIThemeNames_ListsTheFilesOfOneKindSorted,
        "GUIThemeNames_ListsTheFilesOfOneKindSorted");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_LoadGUIDarkTheme_DefaultsToCarbon,
        "LoadGUIDarkTheme_DefaultsToCarbon");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_DefaultsToWindows, "LoadGUITheme_DefaultsToWindows");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_ReadsEachThemeIgnoringCase,
        "LoadGUITheme_ReadsEachThemeIgnoringCase");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_ParseGUIThemeColor_RejectsOtherForms,
        "ParseGUIThemeColor_RejectsOtherForms");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_NeedsBaseDark, "ReadGUIThemeFile_NeedsBaseDark");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_ReadsTheColorsItSets,
        "ReadGUIThemeFile_ReadsTheColorsItSets");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_ReportsEachProblem,
        "ReadGUIThemeFile_ReportsEachProblem");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_SaveGUIDarkTheme_RoundTripsBesideTheTheme,
        "SaveGUIDarkTheme_RoundTripsBesideTheTheme");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_SaveGUITheme_KeepsTheLayout, "SaveGUITheme_KeepsTheLayout");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_SaveGUITheme_RoundTrips, "SaveGUITheme_RoundTrips");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Theme::~TTest_ASWUnitTests_GUI_Theme()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_FormatGUIThemeColor_RoundTripsThroughParse()
{
    // Act and Assert
    CheckEquals(std::string("#1A2B3C"), FormatGUIThemeColor(0x1A2B3C), __func__, __LINE__, "red, green, blue");
    CheckEquals(std::string("#000000"), FormatGUIThemeColor(0), __func__, __LINE__, "with leading zeros");
    CheckEquals(std::string("#FFFFFF"), FormatGUIThemeColor(0xFFFFFF), __func__, __LINE__, "in uppercase");
    CheckEquals(std::string("#ABCDEF"), FormatGUIThemeColor(0xFFABCDEF), __func__, __LINE__,
        "ignoring bits above the blue, green and red bytes");

    for (std::uint32_t const rgb : { 0x000000u, 0x000001u, 0x0F1E2Du, 0x808080u, 0xC0FFEEu, 0xFFFFFFu })
    {
        CheckTrue(ParseGUIThemeColor(FormatGUIThemeColor(rgb)) == rgb, __func__, __LINE__,
            "read back: " + FormatGUIThemeColor(rgb));
    }

    CheckTrue(ParseGUIThemeColor("#c0ffee") == 0xC0FFEEu, __func__, __LINE__, "lowercase digits");
    CheckTrue(ParseGUIThemeColor("#C0ffEE") == 0xC0FFEEu, __func__, __LINE__, "mixed case digits");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_GUIThemeIsDark_FollowsTheChoiceOrWindows()
{
    // Act and Assert: GUIThemeIsDark(theme, windowsAppsUseDark, highContrast).
    CheckTrue(GUIThemeIsDark(EGUITheme::Dark, false, false), __func__, __LINE__, "Dark, with Windows light");
    CheckTrue(GUIThemeIsDark(EGUITheme::Dark, true, true), __func__, __LINE__, "Dark, even with high contrast");
    CheckFalse(GUIThemeIsDark(EGUITheme::Light, true, false), __func__, __LINE__, "Light, with Windows dark");
    CheckFalse(GUIThemeIsDark(EGUITheme::Light, false, true), __func__, __LINE__, "Light, with high contrast");
    CheckTrue(GUIThemeIsDark(EGUITheme::Windows, true, false), __func__, __LINE__, "Windows, set to dark");
    CheckFalse(GUIThemeIsDark(EGUITheme::Windows, false, false), __func__, __LINE__, "Windows, set to light");
    CheckFalse(GUIThemeIsDark(EGUITheme::Windows, true, true), __func__, __LINE__,
        "Windows, set to dark, with high contrast");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_GUIThemeNames_ListsTheFilesOfOneKindSorted()
{
    // Arrange: the Themes folder's files, in the order a listing might give them.
    std::vector<std::string> const files = {
        "ocean.ini", "Zest.ini", "Notes.txt", "Carbon.ini", "OCEAN.INI", ".ini", "apple.Ini", "carbon.INI",
        "Ocean.ini.bak", "Forest.vsf", "ini", "Two Words.ini",
    };

    // Act and Assert
    CheckEquals(std::string("apple\nocean\nTwo Words\nZest\n"), JoinLines(GUIThemeNames(files, ".ini")), __func__,
        __LINE__, "the .ini files' names, sorted ignoring case, a name once, without Carbon or a bare extension");
    CheckEquals(std::string("Forest\n"), JoinLines(GUIThemeNames(files, ".vsf")), __func__, __LINE__,
        "the VCL GUI runner's .vsf files");
    CheckEmpty(JoinLines(GUIThemeNames(std::vector<std::string>(), ".ini")), __func__, __LINE__,
        "an empty or missing folder");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_LoadGUIDarkTheme_DefaultsToCarbon()
{
    // Act and Assert
    CheckEquals(std::string("Carbon"), GUIDefaultDarkThemeName(), __func__, __LINE__, "the built-in dark theme");
    CheckEquals(std::string("Carbon"), LoadGUIDarkTheme(TGUIINIDocument()), __func__, __LINE__, "an empty file");
    CheckEquals(std::string("Carbon"), LoadGUIDarkTheme(TGUIINIDocument("[View]\r\nTheme=Dark\r\n")), __func__,
        __LINE__, "a layout file from before theme files");
    CheckEquals(std::string("Carbon"), LoadGUIDarkTheme(TGUIINIDocument("[View]\r\nDarkTheme=\r\n")), __func__,
        __LINE__, "an empty value");
    CheckEquals(std::string("Carbon"), LoadGUIDarkTheme(TGUIINIDocument("[View]\r\nDarkTheme=CARBON\r\n")), __func__,
        __LINE__, "Carbon in another case is the built-in one");
    CheckEquals(std::string("Carbon"), LoadGUIDarkTheme(TGUIINIDocument("[Window]\r\nDarkTheme=Ocean\r\n")),
        __func__, __LINE__, "a DarkTheme key in another section");
    CheckEquals(std::string("Deep Ocean"), LoadGUIDarkTheme(TGUIINIDocument("[view]\r\ndarktheme = Deep Ocean \r\n")),
        __func__, __LINE__, "a theme file's name, as written, trimmed by the INI document");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_DefaultsToWindows()
{
    // Act and Assert
    CheckTrue(LoadGUITheme(TGUIINIDocument()) == EGUITheme::Windows, __func__, __LINE__, "an empty file");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[Window]\r\nMaximized=1\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "a layout file from before 1.5");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nOther=Dark\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "no Theme key");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[Window]\r\nTheme=Dark\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "a Theme key in another section");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=Carbon\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "an unknown theme");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=\r\n")) == EGUITheme::Windows, __func__, __LINE__,
        "an empty value");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=Darker\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "a theme's name followed by more text");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_ReadsEachThemeIgnoringCase()
{
    // Act and Assert
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=Dark\r\n")) == EGUITheme::Dark, __func__, __LINE__,
        "Dark");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=Light\r\n")) == EGUITheme::Light, __func__, __LINE__,
        "Light");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme=Windows\r\n")) == EGUITheme::Windows, __func__,
        __LINE__, "Windows");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[view]\r\ntheme=DARK\r\n")) == EGUITheme::Dark, __func__, __LINE__,
        "names and value in another case");
    CheckTrue(LoadGUITheme(TGUIINIDocument("[View]\r\nTheme = light \r\n")) == EGUITheme::Light, __func__,
        __LINE__, "spaces around the value, which the INI document trims");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_ParseGUIThemeColor_RejectsOtherForms()
{
    // Act and Assert
    for (char const* const text : { "", "#", "#12345", "#1234567", "123456", "0x123456", "#12345G", " #123456",
                                    "#123456 ", "#-12345", "#+12345", "#12 456", "rgb(1,2,3)", "1, 2, 3", "#ABC" })
    {
        CheckFalse(ParseGUIThemeColor(text).has_value(), __func__, __LINE__, std::string("not a color: \"") + text +
            "\"");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_NeedsBaseDark()
{
    // Arrange
    std::vector<std::string> const roles = { "Window" };
    std::string const colors = "[Colors]\r\nWindow=#102030\r\n";

    // Act
    TGUIThemeFile const missing = ReadGUIThemeFile(colors, roles);
    TGUIThemeFile const light = ReadGUIThemeFile("[Theme]\r\nBase=Light\r\n" + colors, roles);
    TGUIThemeFile const empty = ReadGUIThemeFile(std::string(), roles);
    TGUIThemeFile const dark = ReadGUIThemeFile("[theme]\r\nbase=DARK\r\n" + colors, roles);

    // Assert
    CheckFalse(missing.Usable, __func__, __LINE__, "no Base");
    CheckEmpty(ColorsText(missing), __func__, __LINE__, "so no colors");
    CheckEquals(std::string("[Theme] has no Base=Dark, which a theme file needs, so Carbon is used instead.\n"),
        JoinLines(missing.Problems), __func__, __LINE__, "the problem");

    CheckFalse(light.Usable, __func__, __LINE__, "Base=Light");
    CheckEmpty(ColorsText(light), __func__, __LINE__, "so no colors either");
    CheckEquals(std::string("[Theme] Base=Light isn't supported, only Base=Dark, so Carbon is used instead.\n"),
        JoinLines(light.Problems), __func__, __LINE__, "its problem");

    CheckFalse(empty.Usable, __func__, __LINE__, "an empty file");
    CheckEquals(std::string("[Theme] has no Base=Dark, which a theme file needs, so Carbon is used instead.\n"),
        JoinLines(empty.Problems), __func__, __LINE__, "its problem");

    CheckTrue(dark.Usable, __func__, __LINE__, "Base=Dark, ignoring case");
    CheckEquals(std::string("Window=#102030"), ColorsText(dark), __func__, __LINE__, "its color");
    CheckEmpty(JoinLines(dark.Problems), __func__, __LINE__, "no problems");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_ReadsTheColorsItSets()
{
    // Arrange: comments and blank lines, as the template has, a byte order mark from an editor, LF line breaks
    // and roles written in another case.
    std::vector<std::string> const roles = { "Panel", "Window", "WindowText" };
    std::string const text = "\xEF\xBB\xBF; A theme\n"
        "\n"
        "[Theme]\n"
        "Base=Dark\n"
        "\n"
        "[Colors]\n"
        "; The window\n"
        "window=#102030\n"
        "  PANEL = #a0B0c0  \n";

    // Act
    TGUIThemeFile const file = ReadGUIThemeFile(text, roles);

    // Assert
    CheckTrue(file.Usable, __func__, __LINE__, "usable");
    CheckEquals(std::string("Window=#102030,Panel=#A0B0C0"), ColorsText(file), __func__, __LINE__,
        "in the file's order, each role named as the GUI names it, and WindowText left out");
    CheckEmpty(JoinLines(file.Problems), __func__, __LINE__, "no problems");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_ReadGUIThemeFile_ReportsEachProblem()
{
    // Arrange
    std::vector<std::string> const roles = { "Panel", "Window", "WindowText" };
    std::string const text = "[Theme]\r\n"
        "Base=Dark\r\n"
        "Name=Ocean\r\n"
        "[Colours]\r\n"
        "Window=#000000\r\n"
        "[Colors]\r\n"
        "Window=#102030\r\n"
        "Windw=#111111\r\n"
        "WindowText=white\r\n"
        "Panel=#123456 ; a comment\r\n"
        "WINDOW=#203040\r\n"
        "windowtext=#FFFFFF\r\n";

    // Act
    TGUIThemeFile const file = ReadGUIThemeFile(text, roles);

    // Assert
    CheckTrue(file.Usable, __func__, __LINE__, "still usable, with what it got right");
    CheckEquals(std::string("Window=#102030"), ColorsText(file), __func__, __LINE__, "the one valid color");
    CheckEquals(std::string("[Colours] isn't a section of a theme file, so it's ignored.\n"
        "[Theme] Name isn't a setting of a theme file, so it's ignored.\n"
        "[Colors] Windw isn't a role a theme can color, so it's ignored.\n"
        "[Colors] WindowText=white isn't a color written as #RRGGBB, so it's ignored.\n"
        "[Colors] Panel=#123456 ; a comment isn't a color written as #RRGGBB, so it's ignored.\n"
        "[Colors] WINDOW is set more than once, so only the first is used.\n"
        "[Colors] windowtext is set more than once, so only the first is used.\n"), JoinLines(file.Problems), __func__,
        __LINE__, "each problem, sections first, then the settings, then the colors in the file's order");
    CheckEquals(std::string("Dark theme \"Ocean\": a problem."), GUIThemeProblemLine("Ocean", "a problem."), __func__,
        __LINE__, "a problem as the log shows it");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_SaveGUIDarkTheme_RoundTripsBesideTheTheme()
{
    // Arrange
    TGUIINIDocument ini;
    SaveGUITheme(ini, EGUITheme::Dark);

    // Act and Assert: each overwrites the one before, and the theme stays.
    for (char const* const name : { "Ocean", "Two Words", "Carbon" })
    {
        SaveGUIDarkTheme(ini, name);
        TGUIINIDocument const readBack(ini.Text());
        CheckEquals(std::string(name), LoadGUIDarkTheme(readBack), __func__, __LINE__, "read back");
        CheckTrue(LoadGUITheme(readBack) == EGUITheme::Dark, __func__, __LINE__, "and the theme with it");
    }

    CheckEquals(std::string("[View]\r\nTheme=Dark\r\nDarkTheme=Carbon\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "both in [View]");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_SaveGUITheme_KeepsTheLayout()
{
    // Arrange: the theme is saved as soon as it's chosen, and the layout when the window closes, each keeping
    // the other.
    TGUIINIDocument ini;
    TGUILayout layout;
    layout.DetailHeight = 170;
    SaveGUILayout(ini, layout);

    // Act
    SaveGUITheme(ini, EGUITheme::Dark);
    SaveGUILayout(ini, layout);

    // Assert
    CheckEquals(std::string("[Panels]\r\nDetailHeight=170\r\n\r\n[Window]\r\nMaximized=0\r\n\r\n"
        "[View]\r\nTheme=Dark\r\n\r\n"), ini.Text(), __func__, __LINE__, "the theme after the layout");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Theme::Test_SaveGUITheme_RoundTrips()
{
    // Arrange
    TGUIINIDocument ini;

    // Act and Assert: each overwrites the one before.
    for (EGUITheme const theme : { EGUITheme::Dark, EGUITheme::Light, EGUITheme::Windows })
    {
        SaveGUITheme(ini, theme);
        CheckTrue(LoadGUITheme(TGUIINIDocument(ini.Text())) == theme, __func__, __LINE__, "read back");
    }

    CheckEquals(std::string("[View]\r\nTheme=Windows\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "one key, holding the last theme saved");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Theme)
