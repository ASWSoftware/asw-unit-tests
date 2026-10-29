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

    return palette;
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
COLORREF SkipTextColorOn(COLORREF background)
{
    return IsDarkColor(background) ? RGB(255, 190, 70) : RGB(190, 120, 0);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
