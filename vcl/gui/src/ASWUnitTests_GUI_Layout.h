/* **************************************************************************
ASWUnitTests_GUI_Layout.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_GUI_LayoutH
#define ASWUnitTests_GUI_LayoutH
//---------------------------------------------------------------------------
#include <optional>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
#include <System.IniFiles.hpp>
#include <System.Types.hpp>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIWindowBounds
//
// The main window's restored (not maximized) bounds, as saved between
// sessions: its position in screen pixels, and its size at 96 DPI (see
// TGUILayout).
/////////////////////////////////////////////////////////////////////////////
struct TGUIWindowBounds
{
    int Height = 0;
    int Left = 0;
    int Top = 0;
    int Width = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TGUILayout
//
// The VCL GUI runner's window layout, saved when the window closes and
// restored when it next opens. Sizes are kept at 96 DPI (100% display
// scaling), converted with ScaleFrom96() and ScaleTo96(), so a layout saved
// at one display scaling still fits at another. A value that's missing, or
// wasn't valid when loaded, is empty, and the window keeps its default for
// it. Contains no VCL UI code, so it's unit tested from the VCL console
// runner (see vcl/tests/Test_ASWUnitTests_GUI_Layout.cpp).
/////////////////////////////////////////////////////////////////////////////
struct TGUILayout
{
    std::optional<int> DetailHeight; // The detail pane's height, at 96 DPI.
    bool Maximized = false;
    std::optional<int> TestTreeWidth; // The test tree's width, at 96 DPI.
    std::optional<TGUIWindowBounds> Window;
};

//---------------------------------------------------------------------------

// This executable's layout file, %APPDATA%\ASWUnitTests\<exe name>.ini, so each test app built on the GUI
// keeps its own layout, shared by all of its build configurations.
System::UnicodeString GUILayoutFilePath();
// Whether enough of a window's top edge, where its title bar is, lies on one of 'workAreas' for the window to
// be dragged; e.g. false once the monitor it was on has been disconnected. All in screen pixels.
bool IsWindowReachable(System::Types::TRect const& bounds, std::vector<System::Types::TRect> const& workAreas);
// Reads the layout SaveGUILayout() wrote to 'ini'.
TGUILayout LoadGUILayout(System::Inifiles::TCustomIniFile& ini);
// Writes 'layout' to 'ini', removing each value it doesn't have. For a TMemIniFile, the file itself is only
// written by a later UpdateFile().
void SaveGUILayout(System::Inifiles::TCustomIniFile& ini, TGUILayout const& layout);
// Converts a size at 96 DPI to one at 'pixelsPerInch', rounded to the nearest pixel.
int ScaleFrom96(int value, int pixelsPerInch);
// Converts a size at 'pixelsPerInch' to one at 96 DPI, rounded to the nearest pixel.
int ScaleTo96(int value, int pixelsPerInch);
// Converts 'rect' from screen coordinates to workspace coordinates, which SetWindowPlacement() uses; the
// reverse of WorkspaceToScreen().
System::Types::TRect ScreenToWorkspace(System::Types::TRect const& rect, System::Types::TRect const& primaryMonitor,
    System::Types::TRect const& primaryWorkArea);
// Converts 'rect' from workspace coordinates, which GetWindowPlacement() uses and which are relative to the
// primary monitor's work area, to screen coordinates.
System::Types::TRect WorkspaceToScreen(System::Types::TRect const& rect, System::Types::TRect const& primaryMonitor,
    System::Types::TRect const& primaryWorkArea);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_LayoutH
