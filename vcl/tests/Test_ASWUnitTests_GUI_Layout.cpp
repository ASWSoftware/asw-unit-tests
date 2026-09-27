/* **************************************************************************
Test_ASWUnitTests_GUI_Layout.cpp
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
#include "Test_ASWUnitTests_GUI_Layout.h"
//---------------------------------------------------------------------------
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.IniFiles.hpp>
#include <System.SysUtils.hpp>
#include <System.Types.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace
{

using System::Types::TRect;

// An empty, in-memory INI file; nothing is read from or written to disk.
std::unique_ptr<System::Inifiles::TMemIniFile> NewMemIniFile();

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
std::unique_ptr<System::Inifiles::TMemIniFile> NewMemIniFile()
{
    return std::unique_ptr<System::Inifiles::TMemIniFile>(new System::Inifiles::TMemIniFile(System::UnicodeString()));
}
//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Layout
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Layout::TTest_ASWUnitTests_GUI_Layout()
    : inherited("ASWUnitTests_GUI_Layout_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_GUILayoutFilePath_IsPerExeUnderAppData,
        "GUILayoutFilePath_IsPerExeUnderAppData");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_FalseOffScreen, "IsWindowReachable_FalseOffScreen");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_FalseWhenTitleBarIsHidden,
        "IsWindowReachable_FalseWhenTitleBarIsHidden");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_TrueOnAWorkArea, "IsWindowReachable_TrueOnAWorkArea");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_LoadGUILayout_EmptyIniHasNoValues, "LoadGUILayout_EmptyIniHasNoValues");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_LoadGUILayout_IgnoresInvalidValues, "LoadGUILayout_IgnoresInvalidValues");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_SaveGUILayout_RemovesMissingValues, "SaveGUILayout_RemovesMissingValues");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_SaveGUILayout_RoundTrips, "SaveGUILayout_RoundTrips");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_ScaleFrom96_ScalesAndRounds, "ScaleFrom96_ScalesAndRounds");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_ScaleTo96_ReversesScaleFrom96, "ScaleTo96_ReversesScaleFrom96");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_ScreenToWorkspace_ReversesWorkspaceToScreen,
        "ScreenToWorkspace_ReversesWorkspaceToScreen");
    RegisterTest(&TTest_ASWUnitTests_GUI_Layout::Test_WorkspaceToScreen_OffsetsByPrimaryWorkArea,
        "WorkspaceToScreen_OffsetsByPrimaryWorkArea");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Layout::~TTest_ASWUnitTests_GUI_Layout()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_GUILayoutFilePath_IsPerExeUnderAppData()
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
void TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_FalseOffScreen()
{
    // Arrange
    std::vector<TRect> const workAreas{ TRect(0, 0, 1920, 1040) };

    // Act and Assert
    CheckFalse(IsWindowReachable(TRect(2000, 100, 2800, 700), workAreas), __func__, __LINE__,
        "a window past a disconnected monitor's edge");
    CheckFalse(IsWindowReachable(TRect(1870, 100, 2670, 700), workAreas), __func__, __LINE__,
        "only 50 pixels of its title bar on screen");
    CheckFalse(IsWindowReachable(TRect(100, 100, 900, 700), std::vector<TRect>()), __func__, __LINE__,
        "no work areas at all");
    CheckFalse(IsWindowReachable(TRect(100, 100, 100, 700), workAreas), __func__, __LINE__, "an empty rectangle");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_FalseWhenTitleBarIsHidden()
{
    // Arrange
    std::vector<TRect> const workAreas{ TRect(0, 0, 1920, 1040) };

    // Act and Assert
    CheckFalse(IsWindowReachable(TRect(100, -20, 900, 580), workAreas), __func__, __LINE__,
        "its top above the work area");
    CheckFalse(IsWindowReachable(TRect(100, 1020, 900, 1620), workAreas), __func__, __LINE__,
        "its title bar below the work area's bottom, e.g. behind the taskbar");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_IsWindowReachable_TrueOnAWorkArea()
{
    // Arrange: a second monitor to the left of the primary one, at negative screen coordinates.
    std::vector<TRect> const workAreas{ TRect(0, 0, 1920, 1040), TRect(-1600, 0, 0, 860) };

    // Act and Assert
    CheckTrue(IsWindowReachable(TRect(100, 100, 900, 700), workAreas), __func__, __LINE__, "on the primary monitor");
    CheckTrue(IsWindowReachable(TRect(-1200, 100, -400, 700), workAreas), __func__, __LINE__,
        "on the second monitor");
    CheckTrue(IsWindowReachable(TRect(1770, 100, 2570, 700), workAreas), __func__, __LINE__,
        "150 pixels of its title bar on screen");
    CheckTrue(IsWindowReachable(TRect(100, 800, 900, 1400), workAreas), __func__, __LINE__,
        "only its title bar and a little more on screen");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_LoadGUILayout_EmptyIniHasNoValues()
{
    // Arrange
    std::unique_ptr<System::Inifiles::TMemIniFile> const ini = NewMemIniFile();

    // Act
    TGUILayout const layout = LoadGUILayout(*ini);

    // Assert
    CheckFalse(layout.DetailHeight.has_value(), __func__, __LINE__, "no detail pane height");
    CheckFalse(layout.Maximized, __func__, __LINE__, "not maximized");
    CheckFalse(layout.TestTreeWidth.has_value(), __func__, __LINE__, "no test tree width");
    CheckFalse(layout.Window.has_value(), __func__, __LINE__, "no window bounds");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_LoadGUILayout_IgnoresInvalidValues()
{
    // Arrange
    std::unique_ptr<System::Inifiles::TMemIniFile> const ini = NewMemIniFile();
    ini->WriteString("Panels", "DetailHeight", "0");
    ini->WriteString("Panels", "TestTreeWidth", "wide");
    ini->WriteString("Window", "Height", "600");
    ini->WriteString("Window", "Left", "10");
    ini->WriteString("Window", "Maximized", "yes");
    ini->WriteString("Window", "Top", "");
    ini->WriteString("Window", "Width", "800");

    // Act
    TGUILayout const layout = LoadGUILayout(*ini);

    // Assert
    CheckFalse(layout.DetailHeight.has_value(), __func__, __LINE__, "a zero size is ignored");
    CheckFalse(layout.Maximized, __func__, __LINE__, "a flag that isn't a number reads as not maximized");
    CheckFalse(layout.TestTreeWidth.has_value(), __func__, __LINE__, "a size that isn't a number is ignored");
    CheckFalse(layout.Window.has_value(), __func__, __LINE__, "the window isn't placed without all four values");

    // Arrange: every window value valid except a height below the minimum.
    ini->WriteString("Window", "Height", "40");
    ini->WriteString("Window", "Top", "20");

    // Act and Assert
    CheckFalse(LoadGUILayout(*ini).Window.has_value(), __func__, __LINE__, "nor with a height too small to use");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_SaveGUILayout_RemovesMissingValues()
{
    // Arrange
    std::unique_ptr<System::Inifiles::TMemIniFile> const ini = NewMemIniFile();
    TGUILayout full;
    full.DetailHeight = 170;
    full.TestTreeWidth = 360;
    TGUIWindowBounds window;
    window.Height = 700;
    window.Left = 50;
    window.Top = 60;
    window.Width = 1000;
    full.Window = window;
    SaveGUILayout(*ini, full);

    // Act
    SaveGUILayout(*ini, TGUILayout());

    // Assert
    CheckFalse(ini->ValueExists("Panels", "DetailHeight"), __func__, __LINE__, "the detail pane height is removed");
    CheckFalse(ini->ValueExists("Panels", "TestTreeWidth"), __func__, __LINE__, "the test tree width is removed");
    CheckFalse(ini->ValueExists("Window", "Height"), __func__, __LINE__, "and so are the window's height,");
    CheckFalse(ini->ValueExists("Window", "Left"), __func__, __LINE__, "left,");
    CheckFalse(ini->ValueExists("Window", "Top"), __func__, __LINE__, "top,");
    CheckFalse(ini->ValueExists("Window", "Width"), __func__, __LINE__, "and width");
    CheckTrue(ini->ValueExists("Window", "Maximized"), __func__, __LINE__, "the maximized flag is always written");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_SaveGUILayout_RoundTrips()
{
    // Arrange: a window on a monitor left of the primary one, so its position is negative.
    std::unique_ptr<System::Inifiles::TMemIniFile> const ini = NewMemIniFile();
    TGUILayout saved;
    saved.DetailHeight = 170;
    saved.Maximized = true;
    saved.TestTreeWidth = 360;
    TGUIWindowBounds window;
    window.Height = 700;
    window.Left = -1500;
    window.Top = 60;
    window.Width = 1024;
    saved.Window = window;

    // Act
    SaveGUILayout(*ini, saved);
    TGUILayout const loaded = LoadGUILayout(*ini);

    // Assert
    AssertTrue(loaded.DetailHeight.has_value(), __func__, __LINE__, "the detail pane height is kept");
    CheckEquals(170, *loaded.DetailHeight, __func__, __LINE__, "unchanged");
    CheckTrue(loaded.Maximized, __func__, __LINE__, "the maximized flag is kept");
    AssertTrue(loaded.TestTreeWidth.has_value(), __func__, __LINE__, "the test tree width is kept");
    CheckEquals(360, *loaded.TestTreeWidth, __func__, __LINE__, "unchanged");
    AssertTrue(loaded.Window.has_value(), __func__, __LINE__, "the window bounds are kept");
    CheckEquals(700, loaded.Window->Height, __func__, __LINE__, "height");
    CheckEquals(-1500, loaded.Window->Left, __func__, __LINE__, "left");
    CheckEquals(60, loaded.Window->Top, __func__, __LINE__, "top");
    CheckEquals(1024, loaded.Window->Width, __func__, __LINE__, "width");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_ScaleFrom96_ScalesAndRounds()
{
    // Act and Assert
    CheckEquals(100, ScaleFrom96(100, 96), __func__, __LINE__, "100% scaling");
    CheckEquals(125, ScaleFrom96(100, 120), __func__, __LINE__, "125% scaling");
    CheckEquals(150, ScaleFrom96(100, 144), __func__, __LINE__, "150% scaling");
    CheckEquals(200, ScaleFrom96(100, 192), __func__, __LINE__, "200% scaling");
    CheckEquals(152, ScaleFrom96(101, 144), __func__, __LINE__, "151.5 rounds to the nearest pixel");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_ScaleTo96_ReversesScaleFrom96()
{
    // Act and Assert
    CheckEquals(100, ScaleTo96(125, 120), __func__, __LINE__, "from 125% scaling");
    CheckEquals(100, ScaleTo96(150, 144), __func__, __LINE__, "from 150% scaling");

    for (int value : { 60, 150, 333, 1008 })
    {
        CheckEquals(value, ScaleTo96(ScaleFrom96(value, 144), 144), __func__, __LINE__,
            "a size survives being scaled to 150% and back: " + std::to_string(value));
    }

    CheckEquals(360, ScaleTo96(360, 0), __func__, __LINE__, "an unknown DPI leaves the size as it is");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_ScreenToWorkspace_ReversesWorkspaceToScreen()
{
    // Arrange: a taskbar docked at the left of the primary monitor pushes its work area right 60 pixels.
    TRect const primaryMonitor(0, 0, 1920, 1080);
    TRect const leftTaskbarWorkArea(60, 0, 1920, 1080);
    TRect const screenRect(150, 90, 1350, 910);

    // Act
    TRect const workspaceRect = ScreenToWorkspace(screenRect, primaryMonitor, leftTaskbarWorkArea);
    TRect const roundTrip = WorkspaceToScreen(workspaceRect, primaryMonitor, leftTaskbarWorkArea);

    // Assert
    CheckEquals(90, workspaceRect.Left, __func__, __LINE__, "left moves by the work area's offset");
    CheckEquals(90, workspaceRect.Top, __func__, __LINE__, "top is unchanged");
    CheckEquals(1290, workspaceRect.Right, __func__, __LINE__, "right moves by the work area's offset");
    CheckEquals(910, workspaceRect.Bottom, __func__, __LINE__, "bottom is unchanged");
    CheckEquals(150, roundTrip.Left, __func__, __LINE__, "converting back restores left");
    CheckEquals(90, roundTrip.Top, __func__, __LINE__, "top");
    CheckEquals(1350, roundTrip.Right, __func__, __LINE__, "right");
    CheckEquals(910, roundTrip.Bottom, __func__, __LINE__, "and bottom");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Layout::Test_WorkspaceToScreen_OffsetsByPrimaryWorkArea()
{
    // Arrange: a taskbar docked at the top of the primary monitor pushes its work area down 40 pixels.
    TRect const primaryMonitor(0, 0, 1920, 1080);
    TRect const topTaskbarWorkArea(0, 40, 1920, 1080);

    // Act
    TRect const shifted = WorkspaceToScreen(TRect(10, 10, 810, 610), primaryMonitor, topTaskbarWorkArea);
    TRect const unshifted = WorkspaceToScreen(TRect(10, 10, 810, 610), primaryMonitor, TRect(0, 0, 1920, 1040));

    // Assert
    CheckEquals(10, shifted.Left, __func__, __LINE__, "left is unchanged");
    CheckEquals(50, shifted.Top, __func__, __LINE__, "top moves down with the work area");
    CheckEquals(810, shifted.Right, __func__, __LINE__, "right is unchanged");
    CheckEquals(650, shifted.Bottom, __func__, __LINE__, "bottom moves down with the work area");
    CheckEquals(10, unshifted.Top, __func__, __LINE__, "with the taskbar at the bottom, nothing moves");
    CheckEquals(610, unshifted.Bottom, __func__, __LINE__, "bottom neither");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Layout)
