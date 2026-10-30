/* **************************************************************************
ASWUnitTests_Win32GUI_Palette.cpp
Author: Anthony S. West - ASW Software

The colors of the Win32 GUI runner's dark theme.

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
#include "ASWUnitTests_Win32GUI_Palette.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

COLORREF FromRGB(std::uint32_t rgb);
std::uint32_t ToRGB(COLORREF color);

//---------------------------------------------------------------------------
/*
    FromRGB

    A theme file's color, 0xRRGGBB, as a COLORREF, 0x00BBGGRR.
*/
COLORREF FromRGB(std::uint32_t rgb)
{
    return RGB((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}
//---------------------------------------------------------------------------
/*
    ToRGB

    A COLORREF as a theme file's color, 0xRRGGBB.
*/
std::uint32_t ToRGB(COLORREF color)
{
    return (static_cast<std::uint32_t>(GetRValue(color)) << 16) | (static_cast<std::uint32_t>(GetGValue(color)) << 8) |
        static_cast<std::uint32_t>(GetBValue(color));
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
COLORREF BlendColors(COLORREF from, COLORREF to, int numerator, int denominator)
{
    if (denominator <= 0)
        return from;

    int const part = (std::max)(0, (std::min)(numerator, denominator));
    auto const channel = [part, denominator](int a, int b)
        {
            return static_cast<BYTE>((a * (denominator - part) + b * part + denominator / 2) / denominator);
        };

    return RGB(channel(GetRValue(from), GetRValue(to)), channel(GetGValue(from), GetGValue(to)),
        channel(GetBValue(from), GetBValue(to)));
}
//---------------------------------------------------------------------------
/*
    CarbonPalette

    Most colors are Carbon.vsf's own, from TStyleManager::ActiveStyle's system, style and font colors and
    from pixels of the elements it draws; the selection, the tree's lines, the failed progress bar and the
    edit border are sampled from the VCL GUI runner under Carbon (plans/1.5/theme-win32-handoff.md).
*/
TWin32GUIPalette CarbonPalette()
{
    TWin32GUIPalette palette;

    palette.ButtonBorder = RGB(0x49, 0x49, 0x49);
    palette.ButtonDefaultFrame = RGB(0x24, 0x61, 0x7A);
    palette.ButtonDisabledBorder = RGB(0x3A, 0x3A, 0x3A);
    palette.ButtonDisabledFaceBottom = RGB(0x30, 0x30, 0x30);
    palette.ButtonDisabledFaceTop = RGB(0x3C, 0x3C, 0x3C);
    palette.ButtonDisabledOutline = RGB(0x2C, 0x2C, 0x2C);
    palette.ButtonFaceBottom = RGB(0x31, 0x31, 0x31);
    palette.ButtonFaceTop = RGB(0x43, 0x43, 0x43);
    palette.ButtonHotBorder = RGB(0x57, 0x57, 0x57);
    palette.ButtonHotFaceBottom = RGB(0x35, 0x35, 0x35);
    palette.ButtonHotFaceTop = RGB(0x47, 0x47, 0x47);
    palette.ButtonOutline = RGB(0x19, 0x19, 0x19);
    palette.ButtonPressedFaceBottom = RGB(0x48, 0x48, 0x48);
    palette.ButtonPressedFaceTop = RGB(0x37, 0x37, 0x37);
    palette.CheckBoxBorder = RGB(0x47, 0x47, 0x47);
    palette.CheckBoxFill = RGB(0x19, 0x19, 0x19);
    palette.CheckBoxHotBorder = RGB(0x5F, 0x5F, 0x5F);
    palette.CheckMark = RGB(0x84, 0x84, 0x84);
    palette.CheckMarkDisabled = RGB(0x53, 0x53, 0x53);
    palette.ControlText = RGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = RGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = RGB(0x4C, 0x4C, 0x4C);
    palette.EditBorderOuter = RGB(0x2E, 0x2E, 0x2E);
    palette.GripDot = RGB(0x1A, 0x1A, 0x1A);
    palette.HeaderBottom = RGB(0x35, 0x35, 0x35);
    palette.HeaderDivider = RGB(0x2E, 0x2E, 0x2E);
    palette.HeaderHot = RGB(0x4C, 0x4C, 0x4C);
    palette.HeaderTop = RGB(0x3D, 0x3D, 0x3D);
    palette.Highlight = RGB(0x26, 0x5A, 0x73);
    palette.HighlightFrame = RGB(0x20, 0x20, 0x20);
    palette.HotText = RGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = RGB(0x19, 0x19, 0x19);
    palette.MenuBarDisabledText = RGB(0x59, 0x59, 0x59);
    palette.MenuBorderInner = RGB(0x41, 0x41, 0x41);
    palette.MenuBorderOuter = RGB(0x2E, 0x2E, 0x2E);
    palette.MenuDisabledText = RGB(0x4F, 0x4F, 0x4F);
    palette.MenuSeparator = RGB(0x84, 0x84, 0x84);
    palette.Panel = RGB(0x33, 0x33, 0x33);
    palette.ProgressBorder = RGB(0x2B, 0x2B, 0x2B);
    palette.ProgressChunkBottom = RGB(0x27, 0x54, 0x6A);
    palette.ProgressChunkTop = RGB(0x24, 0x61, 0x7A);
    palette.ProgressFailedBottom = RGB(0x95, 0x29, 0x34);
    palette.ProgressFailedTop = RGB(0x94, 0x30, 0x3C);
    palette.ProgressTrack = RGB(0x19, 0x19, 0x19);
    palette.Selection = RGB(0x62, 0x62, 0x62);
    palette.SelectionFrame = RGB(0x38, 0x38, 0x38);
    palette.SpinArrow = RGB(0x1C, 0x1C, 0x1C);
    palette.SpinDisabledArrow = RGB(0x2A, 0x2A, 0x2A);
    palette.SpinDisabledFaceBottom = RGB(0x36, 0x38, 0x37);
    palette.SpinDisabledFaceTop = RGB(0x42, 0x42, 0x42);
    palette.SpinDisabledOutline = RGB(0x14, 0x14, 0x14);
    palette.SpinFaceBottom = RGB(0x4B, 0x4D, 0x4C);
    palette.SpinFaceTop = RGB(0x5B, 0x5B, 0x5B);
    palette.SpinHotFaceBottom = RGB(0x5D, 0x5E, 0x5E);
    palette.SpinHotFaceTop = RGB(0x6B, 0x6B, 0x6B);
    palette.SpinOutline = RGB(0x1B, 0x1B, 0x1B);
    palette.SpinPressedEdge = RGB(0x45, 0x45, 0x45);
    palette.StatusLine = RGB(0x2E, 0x2E, 0x2E);
    palette.TabActiveBottom = RGB(0x35, 0x35, 0x35);
    palette.TabActiveTop = RGB(0x3C, 0x3C, 0x3C);
    palette.TabBorder = RGB(0x41, 0x41, 0x41);
    palette.TabHot = RGB(0x33, 0x33, 0x33);
    palette.TabInactive = RGB(0x27, 0x27, 0x27);
    palette.TabInactiveBorder = RGB(0x2B, 0x2B, 0x2B);
    palette.TabOuter = RGB(0x14, 0x14, 0x14);
    palette.TooltipBackground = RGB(0x51, 0x51, 0x51);
    palette.TooltipText = RGB(0xEA, 0xEA, 0xEA);
    palette.TreeLine = RGB(0x6D, 0x6D, 0x6D);
    palette.Window = RGB(0x19, 0x19, 0x19);
    palette.WindowText = RGB(0xEA, 0xEA, 0xEA);
    // As the VCL GUI runner's on its dark window color.
    palette.FailureText = FailureTextColorOn(palette.Window);
    palette.SkipText = SkipTextColorOn(palette.Window);
    // The Win32 GUI runner's own, which the light look has too.
    palette.TreeStatusFailed = RGB(215, 40, 40);
    palette.TreeStatusNotRun = RGB(190, 190, 190);
    palette.TreeStatusPassed = RGB(16, 160, 16);
    palette.TreeStatusRunning = RGB(0, 120, 215);
    palette.TreeStatusSkipped = RGB(230, 165, 0);

    return palette;
}
//---------------------------------------------------------------------------
/*
    CarbonThemeFileText

    The roles' groups are separated by a blank line, each under its comment.
*/
std::string CarbonThemeFileText()
{
    TWin32GUIPalette const palette = CarbonPalette();
    std::string text = "; An ASWUnitTests theme file: the colors of the Win32 GUI runner's dark look.\r\n"
        ";\r\n"
        "; In the Themes folder (View > Theme > Dark Theme > Open Themes Folder), it's listed in View > Theme >\r\n"
        "; Dark Theme by its file name, and choosing it there uses it whenever the window is dark. Choosing it\r\n"
        "; again reads it again, so a change shows at once.\r\n"
        ";\r\n"
        "; Each line under [Colors] sets a role's color, as #RRGGBB. These are Carbon's, the built-in dark\r\n"
        "; theme's, and a role left out keeps Carbon's color, so the lines left as they are can be deleted.\r\n"
        "; Problems, such as a misspelled role, are reported in the Log tab.\r\n"
        "\r\n"
        "[Theme]\r\n"
        "Base=Dark\r\n"
        "\r\n"
        "[Colors]\r\n";
    bool first = true;

    for (TWin32GUIPaletteRole const& role : PaletteRoles())
    {
        if (role.Group != nullptr)
        {
            if (!first)
                text += "\r\n";

            first = false;
            std::string const group = role.Group;
            std::size_t start = 0;

            while (start <= group.size())
            {
                std::size_t end = group.find('\n', start);

                if (end == std::string::npos)
                    end = group.size();

                text += "; " + group.substr(start, end - start) + "\r\n";
                start = end + 1;
            }
        }

        text += std::string(role.Name) + "=" + FormatGUIThemeColor(ToRGB(palette.*role.Color)) + "\r\n";
    }

    return text;
}
//---------------------------------------------------------------------------
COLORREF FailureTextColorOn(COLORREF background)
{
    return IsDarkColor(background) ? RGB(255, 100, 100) : RGB(200, 0, 0);
}
//---------------------------------------------------------------------------
bool IsDarkColor(COLORREF color)
{
    int const luma = (GetRValue(color) * 299 + GetGValue(color) * 587 + GetBValue(color) * 114) / 1000;

    return luma < 128;
}
//---------------------------------------------------------------------------
std::vector<std::string> PaletteRoleNames()
{
    std::vector<std::string> names;

    for (TWin32GUIPaletteRole const& role : PaletteRoles())
        names.push_back(role.Name);

    return names;
}
//---------------------------------------------------------------------------
/*
    PaletteRoles

    Each role is named by the macro as its member, so a name can't differ from its member's. The groups follow
    TWin32GUIPalette's comments.
*/
std::vector<TWin32GUIPaletteRole> const& PaletteRoles()
{
#define ASWUNITTESTS_WIN32GUI_ROLE(member, group) \
    TWin32GUIPaletteRole{ &TWin32GUIPalette::member, group, #member }

    static std::vector<TWin32GUIPaletteRole> const roles = {
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonBorder,
            "Push buttons: the border, the default (or focused) button's line inside it, the face's gradient\n"
            "(bottom and top), normal, hot (under the mouse), pressed and disabled, and the outline around them."),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonDefaultFrame, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonDisabledBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonDisabledFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonDisabledFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonDisabledOutline, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonHotBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonHotFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonHotFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonOutline, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonPressedFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonPressedFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxBorder,
            "Check boxes (the test tree's too) and radio buttons: the box's or circle's border, its inside, its\n"
            "border under the mouse, and the mark (a cross, the partial state's square, a radio button's dot),\n"
            "normal and disabled."),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxFill, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxHotBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckMark, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckMarkDisabled, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ControlText,
            "The text of labels, buttons, radio buttons and check boxes, menus, the inactive tabs, the list's\n"
            "header and the status bar."),
        ASWUNITTESTS_WIN32GUI_ROLE(DisabledText, "Disabled text, but the menus' (see MenuBarDisabledText)."),
        ASWUNITTESTS_WIN32GUI_ROLE(EditBorderInner,
            "The two-pixel border of the edit boxes, the tree, the list and the log: inner and outer."),
        ASWUNITTESTS_WIN32GUI_ROLE(EditBorderOuter, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(FailureText, "A failed test's text in the log and the Failures & Skips list."),
        ASWUNITTESTS_WIN32GUI_ROLE(GripDot, "The status bar's size grip."),
        ASWUNITTESTS_WIN32GUI_ROLE(HeaderBottom,
            "The list's header: its gradient's bottom, the lines between columns, a column under the mouse, and\n"
            "the gradient's top."),
        ASWUNITTESTS_WIN32GUI_ROLE(HeaderDivider, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(HeaderHot, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(HeaderTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(Highlight, "The menu item under the mouse or the keyboard, and its frame."),
        ASWUNITTESTS_WIN32GUI_ROLE(HighlightFrame, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(HotText, "Text over Highlight or Selection, and a hot or pressed button's."),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuBarDisabledText, "A disabled item's text, in the menu bar and in a menu."),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuDisabledText, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuBackground,
            "A menu's background, its two-pixel frame (inner and outer) and its separators."),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuBorderInner, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuBorderOuter, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(MenuSeparator, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(Panel,
            "The background of the window, the menu bar, the toolbar, the tab strip, the status bar, the splitters\n"
            "and the dialogs."),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressBorder,
            "The progress bar: its border, its bar's gradient (bottom and top), normal and after a failure, and\n"
            "its track."),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressChunkBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressChunkTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressFailedBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressFailedTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ProgressTrack, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(Selection, "The selected node or row in the tree and the list, and its frame."),
        ASWUNITTESTS_WIN32GUI_ROLE(SelectionFrame, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SkipText, "A skipped test's text in the log and the Failures & Skips list."),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinArrow,
            "The results tabs' scroll arrows, when the tabs don't fit: the arrow, normal and disabled, each\n"
            "button's face gradient (bottom and top) and outline, disabled, normal and hot, and the edge a pressed\n"
            "one shows at its top and left."),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinDisabledArrow, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinDisabledFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinDisabledFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinDisabledOutline, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinHotFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinHotFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinOutline, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SpinPressedEdge, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(StatusLine, "The line along the status bar's top."),
        ASWUNITTESTS_WIN32GUI_ROLE(TabActiveBottom,
            "The results tabs: the selected one's gradient (bottom and top), the borders, a tab under the mouse,\n"
            "the other tabs and their borders, and the line around the strip and the pages."),
        ASWUNITTESTS_WIN32GUI_ROLE(TabActiveTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TabBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TabHot, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TabInactive, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TabInactiveBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TabOuter, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TooltipBackground, "Tooltips."),
        ASWUNITTESTS_WIN32GUI_ROLE(TooltipText, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeLine, "The test tree's dotted lines between nodes."),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeStatusFailed,
            "The test tree's status dots: failed, not run, passed, running and skipped. The light look keeps\n"
            "Carbon's."),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeStatusNotRun, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeStatusPassed, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeStatusRunning, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(TreeStatusSkipped, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(Window,
            "The background of the edit boxes, the detail pane, the tree, the list and the log, and their text."),
        ASWUNITTESTS_WIN32GUI_ROLE(WindowText, nullptr),
    };

#undef ASWUNITTESTS_WIN32GUI_ROLE

    return roles;
}
//---------------------------------------------------------------------------
TWin32GUIPalette PaletteWithThemeColors(TWin32GUIPalette palette, std::vector<TGUIThemeColor> const& colors)
{
    std::vector<TWin32GUIPaletteRole> const& roles = PaletteRoles();

    for (TGUIThemeColor const& color : colors)
    {
        std::vector<TWin32GUIPaletteRole>::const_iterator const role = std::find_if(roles.begin(), roles.end(),
            [&color](TWin32GUIPaletteRole const& candidate)
            {
                return color.Role == candidate.Name;
            });

        if (role != roles.end())
            palette.*(role->Color) = FromRGB(color.RGB);
    }

    return palette;
}
//---------------------------------------------------------------------------
COLORREF SkipTextColorOn(COLORREF background)
{
    return IsDarkColor(background) ? RGB(255, 190, 70) : RGB(190, 120, 0);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
