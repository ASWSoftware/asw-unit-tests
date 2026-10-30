/* **************************************************************************
ASWUnitTests_Win32GUI_Palette.h
Author: Anthony S. West - ASW Software

The colors of the Win32 GUI runner's dark theme, one per role, as RAD
Studio's Carbon style (the VCL GUI runner's dark theme) has them. Contains
no window code, so it's unit tested from the Windows console runners (see
win32/tests/Test_ASWUnitTests_Win32GUI_Palette.cpp).

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
#ifndef ASWUnitTests_Win32GUI_PaletteH
#define ASWUnitTests_Win32GUI_PaletteH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWin32GUIPalette
//
// The colors the dark theme paints with, one per role, each named for what
// it colors. CarbonPalette() has Carbon's, and a theme file sets any of
// them by name (see PaletteRoles()), so a member's name is part of that
// file's format: renaming one breaks the theme files that set it. The light
// theme is Windows' standard look, in the system colors, so it has no
// palette.
/////////////////////////////////////////////////////////////////////////////
struct TWin32GUIPalette
{
    // Push buttons: the outline around them, the border, and the face's gradient from top to bottom, normal,
    // hot (under the mouse), pressed and disabled; the default button's (and the focused one's) line inside
    // its border.
    COLORREF ButtonBorder;
    COLORREF ButtonDefaultFrame;
    COLORREF ButtonDisabledBorder;
    COLORREF ButtonDisabledFaceBottom;
    COLORREF ButtonDisabledFaceTop;
    COLORREF ButtonDisabledOutline;
    COLORREF ButtonFaceBottom;
    COLORREF ButtonFaceTop;
    COLORREF ButtonHotBorder;
    COLORREF ButtonHotFaceBottom;
    COLORREF ButtonHotFaceTop;
    COLORREF ButtonOutline;
    COLORREF ButtonPressedFaceBottom;
    COLORREF ButtonPressedFaceTop;
    // Check boxes (the test tree's too) and radio buttons: the box's or circle's border, its inside, and the
    // mark (a cross, the partial state's square, the radio button's dot); the border under the mouse, and the
    // mark of a disabled one.
    COLORREF CheckBoxBorder;
    COLORREF CheckBoxFill;
    COLORREF CheckBoxHotBorder;
    COLORREF CheckMark;
    COLORREF CheckMarkDisabled;
    // The text of labels, buttons, radio buttons and check boxes, menus, the inactive tab, the list's header
    // and the status bar.
    COLORREF ControlText;
    // Disabled text, but the menus' (see MenuBarDisabledText and MenuDisabledText).
    COLORREF DisabledText;
    // The two-pixel border of the edit boxes, the tree, the list and the log: outer and inner.
    COLORREF EditBorderInner;
    COLORREF EditBorderOuter;
    // A failed and a skipped test's text in the log and the Failures & Skips list.
    COLORREF FailureText;
    // The status bar's size grip.
    COLORREF GripDot;
    // The list's header: its gradient from top to bottom, under the mouse, and the lines between columns.
    COLORREF HeaderBottom;
    COLORREF HeaderDivider;
    COLORREF HeaderHot;
    COLORREF HeaderTop;
    // The menu item under the mouse or the keyboard, and its frame.
    COLORREF Highlight;
    COLORREF HighlightFrame;
    // Text over Highlight or Selection, and a hot or pressed button's.
    COLORREF HotText;
    // A disabled item's text, in the menu bar and in a menu.
    COLORREF MenuBarDisabledText;
    COLORREF MenuDisabledText;
    // A menu's background, its two-pixel frame (outer and inner) and its separators.
    COLORREF MenuBackground;
    COLORREF MenuBorderInner;
    COLORREF MenuBorderOuter;
    COLORREF MenuSeparator;
    // The window's background, and that of the menu bar, the toolbar, the tab strip, the status bar, the
    // splitters and the dialogs.
    COLORREF Panel;
    // The progress bar: its border, its track, and its bar's gradient from top to bottom, normal and failed.
    COLORREF ProgressBorder;
    COLORREF ProgressChunkBottom;
    COLORREF ProgressChunkTop;
    COLORREF ProgressFailedBottom;
    COLORREF ProgressFailedTop;
    COLORREF ProgressTrack;
    // The selected node or row in the tree and the list, and its frame.
    COLORREF Selection;
    COLORREF SelectionFrame;
    COLORREF SkipText;
    // Spin buttons, such as the results tabs' scroll arrows when the tabs don't fit: the outline around each, its
    // face's gradient from top to bottom, normal, hot and disabled, the edge a pressed one shows at its top and
    // left, and its arrow, normal and disabled.
    COLORREF SpinArrow;
    COLORREF SpinDisabledArrow;
    COLORREF SpinDisabledFaceBottom;
    COLORREF SpinDisabledFaceTop;
    COLORREF SpinDisabledOutline;
    COLORREF SpinFaceBottom;
    COLORREF SpinFaceTop;
    COLORREF SpinHotFaceBottom;
    COLORREF SpinHotFaceTop;
    COLORREF SpinOutline;
    COLORREF SpinPressedEdge;
    // The line along the status bar's top.
    COLORREF StatusLine;
    // The results tabs: the selected one's gradient from top to bottom, the others' color, one under the
    // mouse, their borders, and the line around the strip and the pages.
    COLORREF TabActiveBottom;
    COLORREF TabActiveTop;
    COLORREF TabBorder;
    COLORREF TabHot;
    COLORREF TabInactive;
    COLORREF TabInactiveBorder;
    COLORREF TabOuter;
    // Tooltips.
    COLORREF TooltipBackground;
    COLORREF TooltipText;
    // The test tree's dotted lines between nodes.
    COLORREF TreeLine;
    // The test tree's status dots: a test or group that failed, hasn't run, passed, is running, or was skipped.
    // Light draws them in Carbon's colors.
    COLORREF TreeStatusFailed;
    COLORREF TreeStatusNotRun;
    COLORREF TreeStatusPassed;
    COLORREF TreeStatusRunning;
    COLORREF TreeStatusSkipped;
    // The background of the edit boxes, the detail pane, the tree, the list and the log, and their text.
    COLORREF Window;
    COLORREF WindowText;
};


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIPaletteRole
//
// One of TWin32GUIPalette's colors, as a theme file sets it (see
// PaletteRoles()).
/////////////////////////////////////////////////////////////////////////////
struct TWin32GUIPaletteRole
{
    // The member it is.
    COLORREF TWin32GUIPalette::* Color;
    // What this role and the ones after it, up to the next one with a Group, color, one line or more, for
    // CarbonThemeFileText()'s comments; nullptr for those after it.
    char const* Group;
    // The member's name, which the theme file uses.
    char const* Name;
};

//---------------------------------------------------------------------------

// 'from' mixed with 'to' in the proportion 'numerator' / 'denominator', each channel rounded to the nearest:
// 'from' at 0, 'to' at 'denominator', e.g. one row of a gradient. 'from' if 'denominator' isn't positive; the
// proportion is kept within 0 and 1.
COLORREF BlendColors(COLORREF from, COLORREF to, int numerator, int denominator);
// The colors of RAD Studio's Carbon style, the VCL GUI runner's dark theme, read from its .vsf file and from
// the VCL GUI runner as it draws under it.
TWin32GUIPalette CarbonPalette();
// A theme file (see TGUIThemeFile) setting each of PaletteRoles() to Carbon's color, in their order, with
// comments saying what the file is and what each group of roles colors, and CRLF line breaks: the template
// View > Theme > Dark Theme > Save Theme Template writes.
std::string CarbonThemeFileText();
// The color of a failed test's text on 'background': RGB(200, 0, 0), or RGB(255, 100, 100) on a dark
// background (see IsDarkColor()), as the VCL GUI runner's.
COLORREF FailureTextColorOn(COLORREF background);
// Whether 'color' is dark, by its luma (Rec. 601 weights) being below 128, as the VCL GUI runner decides
// whether a background needs lighter text colors.
bool IsDarkColor(COLORREF color);
// The names of PaletteRoles(), in their order, for ReadGUIThemeFile().
std::vector<std::string> PaletteRoleNames();
// Every member of TWin32GUIPalette, once, in the struct's order, with its name and group.
std::vector<TWin32GUIPaletteRole> const& PaletteRoles();
// 'palette' with each of 'colors' (a theme file's, see ReadGUIThemeFile()) in place of its role's color; one
// whose role isn't one of PaletteRoles() is left out.
TWin32GUIPalette PaletteWithThemeColors(TWin32GUIPalette palette, std::vector<TGUIThemeColor> const& colors);
// The color of a skipped test's text on 'background': RGB(190, 120, 0), or RGB(255, 190, 70) on a dark
// background (see IsDarkColor()), as the VCL GUI runner's.
COLORREF SkipTextColorOn(COLORREF background);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_PaletteH
