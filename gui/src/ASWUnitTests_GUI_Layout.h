/* **************************************************************************
ASWUnitTests_GUI_Layout.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_GUI_LayoutH
#define ASWUnitTests_GUI_LayoutH
//---------------------------------------------------------------------------
#include <optional>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIRect
//
// A rectangle in pixels, like the Windows API's RECT: Right and Bottom are
// just outside it, so Width() is Right - Left.
/////////////////////////////////////////////////////////////////////////////
struct TGUIRect
{
    int Bottom = 0;
    int Left = 0;
    int Right = 0;
    int Top = 0;

    TGUIRect();
    TGUIRect(int left, int top, int right, int bottom);

    int Height() const;
    int Width() const;
};


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
// A GUI runner's window layout, saved when the window closes and restored
// when it next opens. Sizes are kept at 96 DPI (100% display scaling),
// converted with ScaleFrom96() and ScaleTo96(), so a layout saved at one
// display scaling still fits at another. A value that's missing, or wasn't
// valid when loaded, is empty, and the window keeps its default for it.
// Contains no UI code, so it's unit tested from the console runners (see
// gui/tests/Test_ASWUnitTests_GUI_Layout.cpp). Each GUI reads and writes the
// file itself, e.g. the VCL GUI's at GUILayoutFilePath() in
// vcl/gui/src/ASWUnitTests_GUI_VCLSystem.h.
/////////////////////////////////////////////////////////////////////////////
struct TGUILayout
{
    std::optional<int> DetailHeight; // The detail pane's height, at 96 DPI.
    bool Maximized = false;
    std::optional<int> TestTreeWidth; // The test tree's width, at 96 DPI.
    std::optional<TGUIWindowBounds> Window;
};

//---------------------------------------------------------------------------

// Whether enough of a window's top edge, where its title bar is, lies on one of 'workAreas' for the window to
// be dragged; e.g. false once the monitor it was on has been disconnected. All in screen pixels.
bool IsWindowReachable(TGUIRect const& bounds, std::vector<TGUIRect> const& workAreas);
// Reads the layout SaveGUILayout() wrote to 'ini'. A size or position is read as a whole decimal number, and
// the maximized flag as maximized when it's a whole number other than 0, as the VCL's TCustomIniFile read them
// (though it also read hexadecimal, which a layout file never has).
TGUILayout LoadGUILayout(TGUIINIDocument const& ini);
// Writes 'layout' to 'ini', removing each value it doesn't have. The file itself is written from the
// document's Text() afterward.
void SaveGUILayout(TGUIINIDocument& ini, TGUILayout const& layout);
// Converts a size at 96 DPI to one at 'pixelsPerInch', rounded to the nearest pixel.
int ScaleFrom96(int value, int pixelsPerInch);
// Converts a size at 'pixelsPerInch' to one at 96 DPI, rounded to the nearest pixel.
int ScaleTo96(int value, int pixelsPerInch);
// Converts 'rect' from screen coordinates to workspace coordinates, which SetWindowPlacement() uses; the
// reverse of WorkspaceToScreen().
TGUIRect ScreenToWorkspace(TGUIRect const& rect, TGUIRect const& primaryMonitor, TGUIRect const& primaryWorkArea);
// Converts 'rect' from workspace coordinates, which GetWindowPlacement() uses and which are relative to the
// primary monitor's work area, to screen coordinates.
TGUIRect WorkspaceToScreen(TGUIRect const& rect, TGUIRect const& primaryMonitor, TGUIRect const& primaryWorkArea);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_LayoutH
