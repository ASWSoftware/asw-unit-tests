/* **************************************************************************
ASWUnitTests_GUI_Palette.h
Author: Anthony S. West - ASW Software

The colors of the GUI runners' dark looks, one per role, as RAD Studio's
Carbon style (the VCL GUI runner's dark theme) has them, and the dark
themes the runners have built in. The roles are named for what the Win32
GUI runner paints; another runner uses each role for its own counterpart
of that, or not at all. Contains no UI code, so it's unit tested from the
console runners (see gui/tests/Test_ASWUnitTests_GUI_Palette.cpp).

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
#ifndef ASWUnitTests_GUI_PaletteH
#define ASWUnitTests_GUI_PaletteH
//---------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIPaletteOf
//
// The colors the dark look paints with, one per role, each named for what
// it colors. CarbonGUIPalette() has Carbon's, and a theme file sets any of
// them by name (see GUIPaletteRolesOf()), so a member's name is part of
// that file's format: renaming one breaks the theme files that set it. The
// light look is each toolkit's standard look, so it has no palette.
//
// A template on the color type, so that a runner can hold the same roles in
// its toolkit's own color type and convert a whole palette in one place:
// TGUIPalette has a theme file's colors, 0xRRGGBB, and the Win32 GUI
// runner's TWin32GUIPalette has COLORREFs.
/////////////////////////////////////////////////////////////////////////////
template <typename TColor>
struct TGUIPaletteOf
{
    // Push buttons: the outline around them, the border, and the face's gradient from top to bottom, normal,
    // hot (under the mouse), pressed and disabled; the default button's (and the focused one's) line inside
    // its border; the text, and a hot or pressed button's.
    TColor ButtonBorder;
    TColor ButtonDefaultFrame;
    TColor ButtonDisabledBorder;
    TColor ButtonDisabledFaceBottom;
    TColor ButtonDisabledFaceTop;
    TColor ButtonDisabledOutline;
    TColor ButtonFaceBottom;
    TColor ButtonFaceTop;
    TColor ButtonHotBorder;
    TColor ButtonHotFaceBottom;
    TColor ButtonHotFaceTop;
    TColor ButtonHotText;
    TColor ButtonOutline;
    TColor ButtonPressedFaceBottom;
    TColor ButtonPressedFaceTop;
    TColor ButtonText;
    // Check boxes (the test tree's too) and radio buttons: the box's or circle's border, its inside, and the
    // mark (a cross, the partial state's square, the radio button's dot); the border under the mouse, and the
    // mark of a disabled one.
    TColor CheckBoxBorder;
    TColor CheckBoxFill;
    TColor CheckBoxHotBorder;
    TColor CheckMark;
    TColor CheckMarkDisabled;
    // The text of labels, radio buttons and check boxes, menus, the inactive tab, the list's header and the
    // status bar.
    TColor ControlText;
    // Disabled text, but the menus' (see MenuBarDisabledText and MenuDisabledText).
    TColor DisabledText;
    // The two-pixel border of the edit boxes, the tree, the list and the log: outer and inner.
    TColor EditBorderInner;
    TColor EditBorderOuter;
    // A failed and a skipped test's text in the log and the Failures & Skips list.
    TColor FailureText;
    // The status bar's size grip.
    TColor GripDot;
    // The list's header: its gradient from top to bottom, under the mouse, and the lines between columns.
    TColor HeaderBottom;
    TColor HeaderDivider;
    TColor HeaderHot;
    TColor HeaderTop;
    // The menu item under the mouse or the keyboard, and its frame.
    TColor Highlight;
    TColor HighlightFrame;
    // Text over Highlight.
    TColor HotText;
    // A disabled item's text, in the menu bar and in a menu.
    TColor MenuBarDisabledText;
    TColor MenuDisabledText;
    // A menu's background, its two-pixel frame (outer and inner) and its separators.
    TColor MenuBackground;
    TColor MenuBorderInner;
    TColor MenuBorderOuter;
    TColor MenuSeparator;
    // The window's background, and that of the menu bar, the toolbar, the tab strip, the status bar, the
    // splitters and the dialogs.
    TColor Panel;
    // The progress bar: its border, its track, and its bar's gradient from top to bottom, normal and failed.
    TColor ProgressBorder;
    TColor ProgressChunkBottom;
    TColor ProgressChunkTop;
    TColor ProgressFailedBottom;
    TColor ProgressFailedTop;
    TColor ProgressTrack;
    // The selected node or row in the tree and the list, its frame, and its text.
    TColor Selection;
    TColor SelectionFrame;
    TColor SelectionText;
    TColor SkipText;
    // Spin buttons, such as the results tabs' scroll arrows when the tabs don't fit: the outline around each, its
    // face's gradient from top to bottom, normal, hot and disabled, the edge a pressed one shows at its top and
    // left, and its arrow, normal and disabled.
    TColor SpinArrow;
    TColor SpinDisabledArrow;
    TColor SpinDisabledFaceBottom;
    TColor SpinDisabledFaceTop;
    TColor SpinDisabledOutline;
    TColor SpinFaceBottom;
    TColor SpinFaceTop;
    TColor SpinHotFaceBottom;
    TColor SpinHotFaceTop;
    TColor SpinOutline;
    TColor SpinPressedEdge;
    // The line along the status bar's top.
    TColor StatusLine;
    // The results tabs: the selected one's gradient from top to bottom, the others' color, one under the
    // mouse, their borders, and the line around the strip and the pages.
    TColor TabActiveBottom;
    TColor TabActiveTop;
    TColor TabBorder;
    TColor TabHot;
    TColor TabInactive;
    TColor TabInactiveBorder;
    TColor TabOuter;
    // Tooltips.
    TColor TooltipBackground;
    TColor TooltipText;
    // The test tree's dotted lines between nodes.
    TColor TreeLine;
    // The test tree's status dots: a test or group that failed, hasn't run, passed, is running, or was skipped.
    // Light draws them in Carbon's colors.
    TColor TreeStatusFailed;
    TColor TreeStatusNotRun;
    TColor TreeStatusPassed;
    TColor TreeStatusRunning;
    TColor TreeStatusSkipped;
    // The background of the edit boxes, the detail pane, the tree, the list and the log, and their text.
    TColor Window;
    TColor WindowText;
};

// The palette in a theme file's colors, 0xRRGGBB (see GUIRGB()).
using TGUIPalette = TGUIPaletteOf<std::uint32_t>;


/////////////////////////////////////////////////////////////////////////////
// TGUIPaletteRoleOf
//
// One of TGUIPaletteOf's colors, as a theme file sets it (see
// GUIPaletteRolesOf()).
/////////////////////////////////////////////////////////////////////////////
template <typename TColor>
struct TGUIPaletteRoleOf
{
    // The member it is.
    TColor TGUIPaletteOf<TColor>::* Color;
    // What this role and the ones after it, up to the next one with a Group, color, one line or more, for
    // CarbonGUIThemeFileText()'s comments; nullptr for those after it.
    char const* Group;
    // The member's name, which the theme file uses.
    char const* Name;
};

// A role of TGUIPalette.
using TGUIPaletteRole = TGUIPaletteRoleOf<std::uint32_t>;


/////////////////////////////////////////////////////////////////////////////
// TGUIBuiltInTheme
//
// A dark theme the GUI runners have built in, which View > Theme > Dark
// Theme lists before the theme files (see GUIBuiltInDarkThemes()).
/////////////////////////////////////////////////////////////////////////////
struct TGUIBuiltInTheme
{
    // Its name, as the menu lists it and the layout file saves it.
    std::string Name;
    TGUIPalette Palette;
};

//---------------------------------------------------------------------------

// 'from' mixed with 'to' in the proportion 'numerator' / 'denominator', each channel rounded to the nearest:
// 'from' at 0, 'to' at 'denominator', e.g. one row of a gradient. 'from' if 'denominator' isn't positive; the
// proportion is kept within 0 and 1.
std::uint32_t BlendGUIColors(std::uint32_t from, std::uint32_t to, int numerator, int denominator);
// The colors of RAD Studio's Carbon style, the VCL GUI runner's dark theme, read from its .vsf file and from
// the VCL GUI runner as it draws under it.
TGUIPalette CarbonGUIPalette();
// A theme file (see TGUIThemeFile) setting each of GUIPaletteRoles() to Carbon's color, in their order, with
// comments saying what the file is and what each group of roles colors, and CRLF line breaks: the template
// View > Theme > Dark Theme > Save Theme Template writes.
std::string CarbonGUIThemeFileText();
// The colors of RAD Studio's Charcoal Dark Slate VCL style, as CarbonGUIPalette() has Carbon's.
TGUIPalette CharcoalDarkSlateGUIPalette();
// The colors of RAD Studio's Cobalt XEMedia VCL style, as CarbonGUIPalette() has Carbon's.
TGUIPalette CobaltXEMediaGUIPalette();
// The built-in dark theme named 'name', ignoring (ASCII) case, e.g. as a layout file saved it; nullptr if
// there's none (see GUIBuiltInDarkThemes()).
TGUIBuiltInTheme const* FindGUIBuiltInDarkTheme(std::string const& name);
// The blue channel of 'color', 0xRRGGBB.
constexpr int GUIBlue(std::uint32_t color)
{
    return static_cast<int>(color & 0xFF);
}
// The dark themes built in: Carbon (GUIDefaultDarkThemeName(), CarbonGUIPalette()) first, then by name the
// others: RAD Studio's dark VCL styles the VCL GUI runner has too (GUIVCLStyleDarkThemeNames(), e.g.
// OnyxBlueGUIPalette()), and Carbon tinted (see TintedCarbonGUIPalette()): Ember (warm brown), Forest (green),
// Midnight (blue) and Plum (purple).
std::vector<TGUIBuiltInTheme> const& GUIBuiltInDarkThemes();
// The color of a failed test's text on 'background': GUIRGB(200, 0, 0), or GUIRGB(255, 100, 100) on a dark
// background (see IsDarkGUIColor()), as the VCL GUI runner's.
std::uint32_t GUIFailureTextColorOn(std::uint32_t background);
// The green channel of 'color', 0xRRGGBB.
constexpr int GUIGreen(std::uint32_t color)
{
    return static_cast<int>((color >> 8) & 0xFF);
}
// The names of GUIPaletteRoles(), in their order, for ReadGUIThemeFile().
std::vector<std::string> GUIPaletteRoleNames();
// GUIPaletteRolesOf() for TGUIPalette.
std::vector<TGUIPaletteRole> const& GUIPaletteRoles();
// Every member of TGUIPaletteOf<TColor>, once, with its name and group, in the same order for every TColor, so
// a palette converts to another color type role by role. Defined below.
template <typename TColor>
std::vector<TGUIPaletteRoleOf<TColor> > const& GUIPaletteRolesOf();
// 'palette' with each of 'colors' (a theme file's, see ReadGUIThemeFile()) in place of its role's color; one
// whose role isn't one of GUIPaletteRoles() is left out.
TGUIPalette GUIPaletteWithThemeColors(TGUIPalette palette, std::vector<TGUIThemeColor> const& colors);
// The red channel of 'color', 0xRRGGBB.
constexpr int GUIRed(std::uint32_t color)
{
    return static_cast<int>((color >> 16) & 0xFF);
}
// The color with these channels, 0 to 255 each, as 0xRRGGBB.
constexpr std::uint32_t GUIRGB(int red, int green, int blue)
{
    return (static_cast<std::uint32_t>(red & 0xFF) << 16) | (static_cast<std::uint32_t>(green & 0xFF) << 8) |
        static_cast<std::uint32_t>(blue & 0xFF);
}
// The color of a skipped test's text on 'background': GUIRGB(190, 120, 0), or GUIRGB(255, 190, 70) on a dark
// background (see IsDarkGUIColor()), as the VCL GUI runner's.
std::uint32_t GUISkipTextColorOn(std::uint32_t background);
// Whether 'color' is dark, by its luma (Rec. 601 weights) being below 128, as the VCL GUI runner decides
// whether a background needs lighter text colors.
bool IsDarkGUIColor(std::uint32_t color);
// The colors of RAD Studio's Onyx Blue VCL style, as CarbonGUIPalette() has Carbon's.
TGUIPalette OnyxBlueGUIPalette();
// The colors of RAD Studio's Tablet Dark VCL style, as CarbonGUIPalette() has Carbon's.
TGUIPalette TabletDarkGUIPalette();
// Carbon's palette tinted: each of its grays turned toward 'hue' (in degrees, 0 red, 120 green, 240 blue), by
// up to 'tint' of its chroma (0 to 1), most in the dark backgrounds and less as a gray gets lighter, so text
// stays nearly white; its accent (the highlight, the default button's frame and the progress bar) turned to
// 'accentHue'. The colors that say how a test did (the tree's status dots, the failed and skipped text and the
// failed progress bar) stay Carbon's.
TGUIPalette TintedCarbonGUIPalette(double hue, double tint, double accentHue);

//---------------------------------------------------------------------------
// GUIPaletteRolesOf()'s definition, here since it's a template. Each role is named by the macro as its member,
// so a name can't differ from its member's. The groups follow TGUIPaletteOf's comments.
template <typename TColor>
std::vector<TGUIPaletteRoleOf<TColor> > const& GUIPaletteRolesOf()
{
#define ASWUNITTESTS_GUI_ROLE(member, group) \
    TGUIPaletteRoleOf<TColor>{ &TGUIPaletteOf<TColor>::member, group, #member }

    static std::vector<TGUIPaletteRoleOf<TColor> > const roles = {
        ASWUNITTESTS_GUI_ROLE(ButtonBorder,
            "Push buttons: the border, the default (or focused) button's line inside it, the face's gradient\n"
            "(bottom and top), normal, hot (under the mouse), pressed and disabled, the outline around them, and\n"
            "the text, normal and hot or pressed."),
        ASWUNITTESTS_GUI_ROLE(ButtonDefaultFrame, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonDisabledBorder, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonDisabledFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonDisabledFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonDisabledOutline, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonHotBorder, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonHotFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonHotFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonHotText, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonOutline, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonPressedFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonPressedFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ButtonText, nullptr),
        ASWUNITTESTS_GUI_ROLE(CheckBoxBorder,
            "Check boxes (the test tree's too) and radio buttons: the box's or circle's border, its inside, its\n"
            "border under the mouse, and the mark (a cross, the partial state's square, a radio button's dot),\n"
            "normal and disabled."),
        ASWUNITTESTS_GUI_ROLE(CheckBoxFill, nullptr),
        ASWUNITTESTS_GUI_ROLE(CheckBoxHotBorder, nullptr),
        ASWUNITTESTS_GUI_ROLE(CheckMark, nullptr),
        ASWUNITTESTS_GUI_ROLE(CheckMarkDisabled, nullptr),
        ASWUNITTESTS_GUI_ROLE(ControlText,
            "The text of labels, radio buttons and check boxes, menus, the inactive tabs, the list's header and\n"
            "the status bar."),
        ASWUNITTESTS_GUI_ROLE(DisabledText, "Disabled text, but the menus' (see MenuBarDisabledText)."),
        ASWUNITTESTS_GUI_ROLE(EditBorderInner,
            "The two-pixel border of the edit boxes, the tree, the list and the log: inner and outer."),
        ASWUNITTESTS_GUI_ROLE(EditBorderOuter, nullptr),
        ASWUNITTESTS_GUI_ROLE(FailureText, "A failed test's text in the log and the Failures & Skips list."),
        ASWUNITTESTS_GUI_ROLE(GripDot, "The status bar's size grip."),
        ASWUNITTESTS_GUI_ROLE(HeaderBottom,
            "The list's header: its gradient's bottom, the lines between columns, a column under the mouse, and\n"
            "the gradient's top."),
        ASWUNITTESTS_GUI_ROLE(HeaderDivider, nullptr),
        ASWUNITTESTS_GUI_ROLE(HeaderHot, nullptr),
        ASWUNITTESTS_GUI_ROLE(HeaderTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(Highlight, "The menu item under the mouse or the keyboard, and its frame."),
        ASWUNITTESTS_GUI_ROLE(HighlightFrame, nullptr),
        ASWUNITTESTS_GUI_ROLE(HotText, "Text over Highlight."),
        ASWUNITTESTS_GUI_ROLE(MenuBarDisabledText, "A disabled item's text, in the menu bar and in a menu."),
        ASWUNITTESTS_GUI_ROLE(MenuDisabledText, nullptr),
        ASWUNITTESTS_GUI_ROLE(MenuBackground,
            "A menu's background, its two-pixel frame (inner and outer) and its separators."),
        ASWUNITTESTS_GUI_ROLE(MenuBorderInner, nullptr),
        ASWUNITTESTS_GUI_ROLE(MenuBorderOuter, nullptr),
        ASWUNITTESTS_GUI_ROLE(MenuSeparator, nullptr),
        ASWUNITTESTS_GUI_ROLE(Panel,
            "The background of the window, the menu bar, the toolbar, the tab strip, the status bar, the splitters\n"
            "and the dialogs."),
        ASWUNITTESTS_GUI_ROLE(ProgressBorder,
            "The progress bar: its border, its bar's gradient (bottom and top), normal and after a failure, and\n"
            "its track."),
        ASWUNITTESTS_GUI_ROLE(ProgressChunkBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ProgressChunkTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ProgressFailedBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(ProgressFailedTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(ProgressTrack, nullptr),
        ASWUNITTESTS_GUI_ROLE(Selection,
            "The selected node or row in the tree and the list, its frame, and its text."),
        ASWUNITTESTS_GUI_ROLE(SelectionFrame, nullptr),
        ASWUNITTESTS_GUI_ROLE(SelectionText, nullptr),
        ASWUNITTESTS_GUI_ROLE(SkipText, "A skipped test's text in the log and the Failures & Skips list."),
        ASWUNITTESTS_GUI_ROLE(SpinArrow,
            "The results tabs' scroll arrows, when the tabs don't fit: the arrow, normal and disabled, each\n"
            "button's face gradient (bottom and top) and outline, disabled, normal and hot, and the edge a pressed\n"
            "one shows at its top and left."),
        ASWUNITTESTS_GUI_ROLE(SpinDisabledArrow, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinDisabledFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinDisabledFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinDisabledOutline, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinHotFaceBottom, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinHotFaceTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinOutline, nullptr),
        ASWUNITTESTS_GUI_ROLE(SpinPressedEdge, nullptr),
        ASWUNITTESTS_GUI_ROLE(StatusLine, "The line along the status bar's top."),
        ASWUNITTESTS_GUI_ROLE(TabActiveBottom,
            "The results tabs: the selected one's gradient (bottom and top), the borders, a tab under the mouse,\n"
            "the other tabs and their borders, and the line around the strip and the pages."),
        ASWUNITTESTS_GUI_ROLE(TabActiveTop, nullptr),
        ASWUNITTESTS_GUI_ROLE(TabBorder, nullptr),
        ASWUNITTESTS_GUI_ROLE(TabHot, nullptr),
        ASWUNITTESTS_GUI_ROLE(TabInactive, nullptr),
        ASWUNITTESTS_GUI_ROLE(TabInactiveBorder, nullptr),
        ASWUNITTESTS_GUI_ROLE(TabOuter, nullptr),
        ASWUNITTESTS_GUI_ROLE(TooltipBackground, "Tooltips."),
        ASWUNITTESTS_GUI_ROLE(TooltipText, nullptr),
        ASWUNITTESTS_GUI_ROLE(TreeLine, "The test tree's dotted lines between nodes."),
        ASWUNITTESTS_GUI_ROLE(TreeStatusFailed,
            "The test tree's status dots: failed, not run, passed, running and skipped. The light look keeps\n"
            "Carbon's."),
        ASWUNITTESTS_GUI_ROLE(TreeStatusNotRun, nullptr),
        ASWUNITTESTS_GUI_ROLE(TreeStatusPassed, nullptr),
        ASWUNITTESTS_GUI_ROLE(TreeStatusRunning, nullptr),
        ASWUNITTESTS_GUI_ROLE(TreeStatusSkipped, nullptr),
        ASWUNITTESTS_GUI_ROLE(Window,
            "The background of the edit boxes, the detail pane, the tree, the list and the log, and their text."),
        ASWUNITTESTS_GUI_ROLE(WindowText, nullptr),
    };

#undef ASWUNITTESTS_GUI_ROLE

    return roles;
}

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_PaletteH
