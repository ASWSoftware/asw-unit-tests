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
#include <initializer_list>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
#include "ASWUnitTests_GUI_Theme.h"
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
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_GUIThemeIsDark_FollowsTheChoiceOrWindows,
        "GUIThemeIsDark_FollowsTheChoiceOrWindows");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_DefaultsToWindows, "LoadGUITheme_DefaultsToWindows");
    RegisterTest(&TTest_ASWUnitTests_GUI_Theme::Test_LoadGUITheme_ReadsEachThemeIgnoringCase,
        "LoadGUITheme_ReadsEachThemeIgnoringCase");
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
