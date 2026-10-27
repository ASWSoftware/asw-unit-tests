/* **************************************************************************
ASWUnitTests_Win32GUI_MainLayout.h
Author: Anthony S. West - ASW Software

Where the Win32 GUI runner's main window puts its controls: the VCL GUI
runner's form layout, from its .dfm, at any DPI and window size. Contains no
window code, so it's unit tested from the Windows console runners (see
win32/tests/Test_ASWUnitTests_Win32GUI_MainLayout.cpp).

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
#ifndef ASWUnitTests_Win32GUI_MainLayoutH
#define ASWUnitTests_Win32GUI_MainLayoutH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWin32GUIMainSizes
//
// The main window's sizes at 96 DPI, from the VCL GUI runner's .dfm.
/////////////////////////////////////////////////////////////////////////////
struct TWin32GUIMainSizes
{
    static constexpr int DefaultClientHeight = 661;
    static constexpr int DefaultClientWidth = 1008;
    static constexpr int DefaultDetailHeight = 170;
    static constexpr int DefaultTestTreeWidth = 380;
    // Spl_Detail's MinSize: the least height of the detail pane, and of the results tabs below it.
    static constexpr int DetailMinHeight = 60;
    static constexpr int ProgressBarHeight = 20;
    static constexpr int SplitterSize = 5;
    // Fixed, as the VCL's TStatusBar is, rather than following its font as a Windows status bar does.
    static constexpr int StatusBarHeight = 19;
    // Spl_Tests' MinSize: the least width of the test tree, and of the results to its right.
    static constexpr int TestTreeMinWidth = 150;
    static constexpr int ToolbarHeight = 42;
};


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIMainLayout
//
// The main window's controls' bounds, in client pixels; see
// LayOutMainWindow(). The toolbar's controls have fixed positions, so
// aren't here.
/////////////////////////////////////////////////////////////////////////////
struct TWin32GUIMainLayout
{
    TGUIRect Detail; // Mem_Detail
    TGUIRect DetailSplitter; // Spl_Detail
    TGUIRect ProgressBar; // PB_Progress
    TGUIRect Results; // PC_Results
    TGUIRect StatusBar; // SB_Status
    TGUIRect TestTree; // TV_Tests
    TGUIRect TestsSplitter; // Spl_Tests
};

//---------------------------------------------------------------------------

// The detail pane's height kept within what its splitter allows, as the VCL's TSplitter keeps it: at least
// DetailMinHeight, and leaving at least that much for the results tabs below it, in a window with the given
// client height. In a window too small for both, the detail pane's minimum wins.
int ClampDetailHeight(int height, int clientHeight, UINT dpi);
// The test tree's width kept within what its splitter allows, as the VCL's TSplitter keeps it: at least
// TestTreeMinWidth, and leaving at least that much for the results to its right, in a window with the given
// client width. In a window too small for both, the tree's minimum wins.
int ClampTestTreeWidth(int width, int clientWidth, UINT dpi);
// The controls' bounds for a client area of the given size, as the VCL lays out the .dfm's aligned controls:
// the toolbar strip at the top, the status bar and the progress bar above it at the bottom, the test tree at
// the left ('testTreeWidth' wide) with its splitter, and to its right the detail pane ('detailHeight' tall), its
// splitter and the results tabs. The tree's width and the detail pane's height are used as given, as the VCL
// keeps an aligned control's size when the window is resized; only what's left for the results shrinks, never
// below zero.
TWin32GUIMainLayout LayOutMainWindow(int clientWidth, int clientHeight, int testTreeWidth, int detailHeight, UINT dpi);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_MainLayoutH
