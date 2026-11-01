/* **************************************************************************
ASWUnitTests_GUI_Palette.cpp
Author: Anthony S. West - ASW Software

The colors of the GUI runners' dark looks, and their built-in dark themes.

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
#include "ASWUnitTests_GUI_Palette.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::uint32_t FromHSL(double hue, double saturation, double lightness);
bool SameText(std::string const& a, std::string const& b);
void ToHSL(std::uint32_t color, double& hue, double& saturation, double& lightness);

//---------------------------------------------------------------------------
/*
    FromHSL

    The color with 'hue' (in degrees, any number of turns), 'saturation' and 'lightness' (0 to 1 each), each
    channel rounded to the nearest.
*/
std::uint32_t FromHSL(double hue, double saturation, double lightness)
{
    double const chroma = (1.0 - std::fabs(2.0 * lightness - 1.0)) * saturation;
    double const sector = std::fmod(std::fmod(hue, 360.0) + 360.0, 360.0) / 60.0;
    double const second = chroma * (1.0 - std::fabs(std::fmod(sector, 2.0) - 1.0));
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;

    switch (static_cast<int>(sector))
    {
        case 0:
            red = chroma;
            green = second;
            break;

        case 1:
            red = second;
            green = chroma;
            break;

        case 2:
            green = chroma;
            blue = second;
            break;

        case 3:
            green = second;
            blue = chroma;
            break;

        case 4:
            red = second;
            blue = chroma;
            break;

        default:
            red = chroma;
            blue = second;
            break;
    }

    double const lowest = lightness - chroma / 2.0;
    auto const channel = [lowest](double value)
        {
            return static_cast<int>(std::lround((std::min)(1.0, (std::max)(0.0, value + lowest)) * 255.0));
        };

    return ASWUnitTests::GUIRGB(channel(red), channel(green), channel(blue));
}
//---------------------------------------------------------------------------
/*
    SameText

    Whether 'a' and 'b' are the same, ignoring the case of ASCII letters.
*/
bool SameText(std::string const& a, std::string const& b)
{
    auto const lower = [](char c)
        {
            return c >= 'A' && c <= 'Z' ? static_cast<char>(c - 'A' + 'a') : c;
        };

    if (a.size() != b.size())
        return false;

    for (std::size_t i = 0; i < a.size(); ++i)
    {
        if (lower(a[i]) != lower(b[i]))
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
/*
    ToHSL

    The color's hue (in degrees, 0 up to 360; 0 for a gray), saturation and lightness (0 to 1 each).
*/
void ToHSL(std::uint32_t color, double& hue, double& saturation, double& lightness)
{
    double const red = ASWUnitTests::GUIRed(color) / 255.0;
    double const green = ASWUnitTests::GUIGreen(color) / 255.0;
    double const blue = ASWUnitTests::GUIBlue(color) / 255.0;
    double const highest = (std::max)(red, (std::max)(green, blue));
    double const lowest = (std::min)(red, (std::min)(green, blue));
    double const chroma = highest - lowest;

    lightness = (highest + lowest) / 2.0;
    saturation = chroma <= 0.0 ? 0.0 : chroma / (1.0 - std::fabs(2.0 * lightness - 1.0));

    if (chroma <= 0.0)
        hue = 0.0;
    else if (highest == red)
        hue = 60.0 * std::fmod((green - blue) / chroma + 6.0, 6.0);
    else if (highest == green)
        hue = 60.0 * ((blue - red) / chroma + 2.0);
    else
        hue = 60.0 * ((red - green) / chroma + 4.0);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::uint32_t BlendGUIColors(std::uint32_t from, std::uint32_t to, int numerator, int denominator)
{
    if (denominator <= 0)
        return from;

    int const part = (std::max)(0, (std::min)(numerator, denominator));
    auto const channel = [part, denominator](int a, int b)
        {
            return static_cast<int>((a * (denominator - part) + b * part + denominator / 2) / denominator);
        };

    return GUIRGB(channel(GUIRed(from), GUIRed(to)), channel(GUIGreen(from), GUIGreen(to)),
        channel(GUIBlue(from), GUIBlue(to)));
}
//---------------------------------------------------------------------------
/*
    CarbonGUIPalette

    Most colors are Carbon.vsf's own, from TStyleManager::ActiveStyle's system, style and font colors and
    from pixels of the elements it draws; the selection, the tree's lines, the failed progress bar and the
    edit border are sampled from the VCL GUI runner under Carbon (plans/1.5/theme-win32-handoff.md).
*/
TGUIPalette CarbonGUIPalette()
{
    TGUIPalette palette;

    palette.ButtonBorder = GUIRGB(0x49, 0x49, 0x49);
    palette.ButtonDefaultFrame = GUIRGB(0x24, 0x61, 0x7A);
    palette.ButtonDisabledBorder = GUIRGB(0x3A, 0x3A, 0x3A);
    palette.ButtonDisabledFaceBottom = GUIRGB(0x30, 0x30, 0x30);
    palette.ButtonDisabledFaceTop = GUIRGB(0x3C, 0x3C, 0x3C);
    palette.ButtonDisabledOutline = GUIRGB(0x2C, 0x2C, 0x2C);
    palette.ButtonFaceBottom = GUIRGB(0x31, 0x31, 0x31);
    palette.ButtonFaceTop = GUIRGB(0x43, 0x43, 0x43);
    palette.ButtonHotBorder = GUIRGB(0x57, 0x57, 0x57);
    palette.ButtonHotFaceBottom = GUIRGB(0x35, 0x35, 0x35);
    palette.ButtonHotFaceTop = GUIRGB(0x47, 0x47, 0x47);
    palette.ButtonHotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = GUIRGB(0x19, 0x19, 0x19);
    palette.ButtonPressedFaceBottom = GUIRGB(0x48, 0x48, 0x48);
    palette.ButtonPressedFaceTop = GUIRGB(0x37, 0x37, 0x37);
    palette.ButtonText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.CheckBoxBorder = GUIRGB(0x47, 0x47, 0x47);
    palette.CheckBoxFill = GUIRGB(0x19, 0x19, 0x19);
    palette.CheckBoxHotBorder = GUIRGB(0x5F, 0x5F, 0x5F);
    palette.CheckMark = GUIRGB(0x84, 0x84, 0x84);
    palette.CheckMarkDisabled = GUIRGB(0x53, 0x53, 0x53);
    palette.ControlText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = GUIRGB(0x4C, 0x4C, 0x4C);
    palette.EditBorderOuter = GUIRGB(0x2E, 0x2E, 0x2E);
    palette.GripDot = GUIRGB(0x1A, 0x1A, 0x1A);
    palette.HeaderBottom = GUIRGB(0x35, 0x35, 0x35);
    palette.HeaderDivider = GUIRGB(0x2E, 0x2E, 0x2E);
    palette.HeaderHot = GUIRGB(0x4C, 0x4C, 0x4C);
    palette.HeaderTop = GUIRGB(0x3D, 0x3D, 0x3D);
    palette.Highlight = GUIRGB(0x26, 0x5A, 0x73);
    palette.HighlightFrame = GUIRGB(0x20, 0x20, 0x20);
    palette.HotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = GUIRGB(0x19, 0x19, 0x19);
    palette.MenuBarDisabledText = GUIRGB(0x59, 0x59, 0x59);
    palette.MenuBorderInner = GUIRGB(0x41, 0x41, 0x41);
    palette.MenuBorderOuter = GUIRGB(0x2E, 0x2E, 0x2E);
    palette.MenuDisabledText = GUIRGB(0x4F, 0x4F, 0x4F);
    palette.MenuSeparator = GUIRGB(0x84, 0x84, 0x84);
    palette.Panel = GUIRGB(0x33, 0x33, 0x33);
    palette.ProgressBorder = GUIRGB(0x2B, 0x2B, 0x2B);
    palette.ProgressChunkBottom = GUIRGB(0x27, 0x54, 0x6A);
    palette.ProgressChunkTop = GUIRGB(0x24, 0x61, 0x7A);
    palette.ProgressFailedBottom = GUIRGB(0x95, 0x29, 0x34);
    palette.ProgressFailedTop = GUIRGB(0x94, 0x30, 0x3C);
    palette.ProgressTrack = GUIRGB(0x19, 0x19, 0x19);
    palette.Selection = GUIRGB(0x62, 0x62, 0x62);
    palette.SelectionFrame = GUIRGB(0x38, 0x38, 0x38);
    palette.SelectionText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = GUIRGB(0x1C, 0x1C, 0x1C);
    palette.SpinDisabledArrow = GUIRGB(0x2A, 0x2A, 0x2A);
    palette.SpinDisabledFaceBottom = GUIRGB(0x36, 0x38, 0x37);
    palette.SpinDisabledFaceTop = GUIRGB(0x42, 0x42, 0x42);
    palette.SpinDisabledOutline = GUIRGB(0x14, 0x14, 0x14);
    palette.SpinFaceBottom = GUIRGB(0x4B, 0x4D, 0x4C);
    palette.SpinFaceTop = GUIRGB(0x5B, 0x5B, 0x5B);
    palette.SpinHotFaceBottom = GUIRGB(0x5D, 0x5E, 0x5E);
    palette.SpinHotFaceTop = GUIRGB(0x6B, 0x6B, 0x6B);
    palette.SpinOutline = GUIRGB(0x1B, 0x1B, 0x1B);
    palette.SpinPressedEdge = GUIRGB(0x45, 0x45, 0x45);
    palette.StatusLine = GUIRGB(0x2E, 0x2E, 0x2E);
    palette.TabActiveBottom = GUIRGB(0x35, 0x35, 0x35);
    palette.TabActiveTop = GUIRGB(0x3C, 0x3C, 0x3C);
    palette.TabBorder = GUIRGB(0x41, 0x41, 0x41);
    palette.TabHot = GUIRGB(0x33, 0x33, 0x33);
    palette.TabInactive = GUIRGB(0x27, 0x27, 0x27);
    palette.TabInactiveBorder = GUIRGB(0x2B, 0x2B, 0x2B);
    palette.TabOuter = GUIRGB(0x14, 0x14, 0x14);
    palette.TooltipBackground = GUIRGB(0x51, 0x51, 0x51);
    palette.TooltipText = GUIRGB(0xEA, 0xEA, 0xEA);
    palette.TreeLine = GUIRGB(0x6D, 0x6D, 0x6D);
    palette.Window = GUIRGB(0x19, 0x19, 0x19);
    palette.WindowText = GUIRGB(0xEA, 0xEA, 0xEA);
    // As the VCL GUI runner's on its dark window color.
    palette.FailureText = GUIFailureTextColorOn(palette.Window);
    palette.SkipText = GUISkipTextColorOn(palette.Window);
    // The Win32 GUI runner's own, which the light look has too.
    palette.TreeStatusFailed = GUIRGB(215, 40, 40);
    palette.TreeStatusNotRun = GUIRGB(190, 190, 190);
    palette.TreeStatusPassed = GUIRGB(16, 160, 16);
    palette.TreeStatusRunning = GUIRGB(0, 120, 215);
    palette.TreeStatusSkipped = GUIRGB(230, 165, 0);

    return palette;
}
//---------------------------------------------------------------------------
/*
    CarbonGUIThemeFileText

    The roles' groups are separated by a blank line, each under its comment.
*/
std::string CarbonGUIThemeFileText()
{
    TGUIPalette const palette = CarbonGUIPalette();
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

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
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

        text += std::string(role.Name) + "=" + FormatGUIThemeColor(palette.*role.Color) + "\r\n";
    }

    return text;
}
//---------------------------------------------------------------------------
/*
    CharcoalDarkSlateGUIPalette

    The colors of RAD Studio's Charcoal Dark Slate style, read from its .vsf file as CarbonGUIPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TGUIPalette CharcoalDarkSlateGUIPalette()
{
    TGUIPalette palette = CarbonGUIPalette();

    palette.ButtonBorder = GUIRGB(0x43, 0x43, 0x43);
    palette.ButtonDefaultFrame = GUIRGB(0x32, 0x32, 0x32);
    palette.ButtonDisabledBorder = GUIRGB(0x45, 0x45, 0x45);
    palette.ButtonDisabledFaceBottom = GUIRGB(0x39, 0x39, 0x39);
    palette.ButtonDisabledFaceTop = GUIRGB(0x4C, 0x4C, 0x4C);
    palette.ButtonDisabledOutline = GUIRGB(0x2F, 0x2F, 0x2F);
    palette.ButtonFaceBottom = GUIRGB(0x28, 0x28, 0x28);
    palette.ButtonFaceTop = GUIRGB(0x40, 0x40, 0x40);
    palette.ButtonHotBorder = GUIRGB(0x4A, 0x4A, 0x4A);
    palette.ButtonHotFaceBottom = GUIRGB(0x1F, 0x1F, 0x1F);
    palette.ButtonHotFaceTop = GUIRGB(0x32, 0x32, 0x32);
    palette.ButtonHotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = GUIRGB(0x27, 0x27, 0x27);
    palette.ButtonPressedFaceBottom = GUIRGB(0x20, 0x20, 0x20);
    palette.ButtonPressedFaceTop = GUIRGB(0x14, 0x14, 0x14);
    palette.ButtonText = GUIRGB(0xA3, 0xA3, 0xA3);
    palette.CheckBoxBorder = GUIRGB(0x5D, 0x5D, 0x5D);
    palette.CheckBoxFill = GUIRGB(0x1F, 0x1F, 0x1F);
    palette.CheckBoxHotBorder = GUIRGB(0x70, 0x70, 0x70);
    palette.CheckMark = GUIRGB(0x7B, 0x7B, 0x7B);
    palette.CheckMarkDisabled = GUIRGB(0x4D, 0x4D, 0x4D);
    palette.ControlText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = GUIRGB(0x5E, 0x5E, 0x5E);
    palette.EditBorderOuter = GUIRGB(0x36, 0x36, 0x36);
    palette.GripDot = GUIRGB(0x41, 0x41, 0x41);
    palette.HeaderBottom = GUIRGB(0x3E, 0x3E, 0x3E);
    palette.HeaderDivider = GUIRGB(0x2D, 0x2D, 0x2D);
    palette.HeaderHot = GUIRGB(0x43, 0x43, 0x43);
    palette.HeaderTop = GUIRGB(0x49, 0x49, 0x49);
    palette.Highlight = GUIRGB(0x5F, 0x5F, 0x5F);
    palette.HighlightFrame = GUIRGB(0x8A, 0x8A, 0x8A);
    palette.HotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = GUIRGB(0x1F, 0x1F, 0x1F);
    palette.MenuBarDisabledText = GUIRGB(0x93, 0x93, 0x93);
    palette.MenuBorderInner = GUIRGB(0x58, 0x58, 0x58);
    palette.MenuBorderOuter = GUIRGB(0x36, 0x36, 0x36);
    palette.MenuDisabledText = GUIRGB(0x69, 0x69, 0x69);
    palette.MenuSeparator = GUIRGB(0x3B, 0x3B, 0x3B);
    palette.Panel = GUIRGB(0x3E, 0x3E, 0x3E);
    palette.ProgressBorder = GUIRGB(0x29, 0x29, 0x29);
    palette.ProgressChunkBottom = GUIRGB(0x5C, 0x5C, 0x5C);
    palette.ProgressChunkTop = GUIRGB(0x5C, 0x5C, 0x5C);
    palette.ProgressTrack = GUIRGB(0x31, 0x31, 0x31);
    palette.SelectionText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = GUIRGB(0x7D, 0x7D, 0x7D);
    palette.SpinDisabledArrow = GUIRGB(0x40, 0x40, 0x40);
    palette.SpinDisabledFaceBottom = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinDisabledFaceTop = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinDisabledOutline = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinFaceBottom = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinFaceTop = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinHotFaceBottom = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinHotFaceTop = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinOutline = GUIRGB(0x23, 0x23, 0x23);
    palette.SpinPressedEdge = GUIRGB(0x23, 0x23, 0x23);
    palette.StatusLine = GUIRGB(0x40, 0x40, 0x40);
    palette.TabActiveBottom = GUIRGB(0x3E, 0x3E, 0x3E);
    palette.TabActiveTop = GUIRGB(0x57, 0x57, 0x57);
    palette.TabBorder = GUIRGB(0x40, 0x40, 0x40);
    palette.TabHot = GUIRGB(0x25, 0x25, 0x25);
    palette.TabInactive = GUIRGB(0x25, 0x25, 0x25);
    palette.TabInactiveBorder = GUIRGB(0x23, 0x23, 0x23);
    palette.TabOuter = GUIRGB(0x23, 0x23, 0x23);
    palette.TooltipBackground = GUIRGB(0x5B, 0x5B, 0x5B);
    palette.TooltipText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.Window = GUIRGB(0x23, 0x23, 0x23);
    palette.WindowText = GUIRGB(0xC0, 0xC0, 0xC0);

    palette.FailureText = GUIFailureTextColorOn(palette.Window);
    palette.SkipText = GUISkipTextColorOn(palette.Window);
    palette.TreeLine = BlendGUIColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
/*
    CobaltXEMediaGUIPalette

    The colors of RAD Studio's Cobalt XEMedia style, read from its .vsf file as CarbonGUIPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TGUIPalette CobaltXEMediaGUIPalette()
{
    TGUIPalette palette = CarbonGUIPalette();

    palette.ButtonBorder = GUIRGB(0x07, 0x0B, 0x11);
    palette.ButtonDefaultFrame = GUIRGB(0x5A, 0x77, 0x9B);
    palette.ButtonDisabledBorder = GUIRGB(0x09, 0x0A, 0x0B);
    palette.ButtonDisabledFaceBottom = GUIRGB(0x16, 0x18, 0x1C);
    palette.ButtonDisabledFaceTop = GUIRGB(0x2B, 0x2E, 0x32);
    palette.ButtonDisabledOutline = GUIRGB(0x14, 0x17, 0x1A);
    palette.ButtonFaceBottom = GUIRGB(0x1A, 0x33, 0x53);
    palette.ButtonFaceTop = GUIRGB(0x46, 0x53, 0x66);
    palette.ButtonHotBorder = GUIRGB(0x07, 0x0F, 0x1C);
    palette.ButtonHotFaceBottom = GUIRGB(0x2A, 0x79, 0xB0);
    palette.ButtonHotFaceTop = GUIRGB(0x5A, 0x77, 0x9B);
    palette.ButtonHotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = GUIRGB(0x11, 0x1A, 0x28);
    palette.ButtonPressedFaceBottom = GUIRGB(0x15, 0x4D, 0x75);
    palette.ButtonPressedFaceTop = GUIRGB(0x38, 0x4C, 0x66);
    palette.ButtonText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.CheckBoxBorder = GUIRGB(0x39, 0x57, 0x8B);
    palette.CheckBoxFill = GUIRGB(0x17, 0x2E, 0x4D);
    palette.CheckBoxHotBorder = GUIRGB(0x54, 0x6E, 0x9C);
    palette.CheckMark = GUIRGB(0x27, 0x75, 0xB8);
    palette.CheckMarkDisabled = GUIRGB(0x1B, 0x49, 0x74);
    palette.ControlText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = GUIRGB(0x41, 0x41, 0x41);
    palette.EditBorderInner = GUIRGB(0x04, 0x06, 0x0C);
    palette.EditBorderOuter = GUIRGB(0x18, 0x31, 0x53);
    palette.GripDot = GUIRGB(0x1B, 0x2B, 0x45);
    palette.HeaderBottom = GUIRGB(0x10, 0x17, 0x24);
    palette.HeaderDivider = GUIRGB(0x00, 0x00, 0x00);
    palette.HeaderHot = GUIRGB(0x0B, 0x11, 0x19);
    palette.HeaderTop = GUIRGB(0x19, 0x21, 0x2D);
    palette.Highlight = GUIRGB(0x0B, 0x36, 0x65);
    palette.HighlightFrame = GUIRGB(0x29, 0x45, 0x69);
    palette.HotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = GUIRGB(0x00, 0x00, 0x00);
    palette.MenuBarDisabledText = GUIRGB(0x41, 0x41, 0x41);
    palette.MenuBorderInner = GUIRGB(0x0D, 0x18, 0x25);
    palette.MenuBorderOuter = GUIRGB(0x21, 0x39, 0x50);
    palette.MenuDisabledText = GUIRGB(0x41, 0x41, 0x41);
    palette.MenuSeparator = GUIRGB(0x0B, 0x36, 0x65);
    palette.Panel = GUIRGB(0x01, 0x01, 0x01);
    palette.ProgressBorder = GUIRGB(0x18, 0x31, 0x53);
    palette.ProgressChunkBottom = GUIRGB(0x24, 0x8B, 0xD1);
    palette.ProgressChunkTop = GUIRGB(0x86, 0xB1, 0xCC);
    palette.ProgressTrack = GUIRGB(0x00, 0x00, 0x00);
    palette.SelectionText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = GUIRGB(0x4C, 0x6D, 0x8F);
    palette.SpinDisabledArrow = GUIRGB(0x2D, 0x41, 0x56);
    palette.SpinDisabledFaceBottom = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinDisabledFaceTop = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinDisabledOutline = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinFaceBottom = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinFaceTop = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinHotFaceBottom = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinHotFaceTop = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinOutline = GUIRGB(0x02, 0x04, 0x08);
    palette.SpinPressedEdge = GUIRGB(0x02, 0x04, 0x08);
    palette.StatusLine = GUIRGB(0x03, 0x05, 0x07);
    palette.TabActiveBottom = GUIRGB(0x0E, 0x13, 0x1C);
    palette.TabActiveTop = GUIRGB(0x21, 0x2F, 0x45);
    palette.TabBorder = GUIRGB(0x0A, 0x0E, 0x15);
    palette.TabHot = GUIRGB(0x05, 0x07, 0x0A);
    palette.TabInactive = GUIRGB(0x05, 0x07, 0x0A);
    palette.TabInactiveBorder = GUIRGB(0x0D, 0x0D, 0x0D);
    palette.TabOuter = GUIRGB(0x32, 0x43, 0x5E);
    palette.TooltipBackground = GUIRGB(0x26, 0x46, 0x6C);
    palette.TooltipText = GUIRGB(0xC0, 0xC0, 0xC0);
    palette.Window = GUIRGB(0x04, 0x07, 0x0E);
    palette.WindowText = GUIRGB(0xFF, 0xFF, 0xFF);

    palette.FailureText = GUIFailureTextColorOn(palette.Window);
    palette.SkipText = GUISkipTextColorOn(palette.Window);
    palette.TreeLine = BlendGUIColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
TGUIBuiltInTheme const* FindGUIBuiltInDarkTheme(std::string const& name)
{
    for (TGUIBuiltInTheme const& theme : GUIBuiltInDarkThemes())
    {
        if (SameText(theme.Name, name))
            return &theme;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
/*
    GUIBuiltInDarkThemes

    Each tint's hue and strength, and its accent's hue, chosen by eye in the runner. The VCL styles' names are
    GUIVCLStyleDarkThemeNames()'s.
*/
std::vector<TGUIBuiltInTheme> const& GUIBuiltInDarkThemes()
{
    static std::vector<TGUIBuiltInTheme> const themes = {
        { GUIDefaultDarkThemeName(), CarbonGUIPalette() },
        { "Charcoal Dark Slate", CharcoalDarkSlateGUIPalette() },
        { "Cobalt XEMedia", CobaltXEMediaGUIPalette() },
        { "Ember", TintedCarbonGUIPalette(25.0, 0.09, 30.0) },
        { "Forest", TintedCarbonGUIPalette(140.0, 0.08, 155.0) },
        { "Midnight", TintedCarbonGUIPalette(220.0, 0.10, 205.0) },
        { "Onyx Blue", OnyxBlueGUIPalette() },
        { "Plum", TintedCarbonGUIPalette(285.0, 0.09, 295.0) },
        { "Tablet Dark", TabletDarkGUIPalette() }
    };

    return themes;
}
//---------------------------------------------------------------------------
std::uint32_t GUIFailureTextColorOn(std::uint32_t background)
{
    return IsDarkGUIColor(background) ? GUIRGB(255, 100, 100) : GUIRGB(200, 0, 0);
}
//---------------------------------------------------------------------------
std::vector<std::string> GUIPaletteRoleNames()
{
    std::vector<std::string> names;

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
        names.push_back(role.Name);

    return names;
}
//---------------------------------------------------------------------------
std::vector<TGUIPaletteRole> const& GUIPaletteRoles()
{
    return GUIPaletteRolesOf<std::uint32_t>();
}
//---------------------------------------------------------------------------
TGUIPalette GUIPaletteWithThemeColors(TGUIPalette palette, std::vector<TGUIThemeColor> const& colors)
{
    std::vector<TGUIPaletteRole> const& roles = GUIPaletteRoles();

    for (TGUIThemeColor const& color : colors)
    {
        std::vector<TGUIPaletteRole>::const_iterator const role = std::find_if(roles.begin(), roles.end(),
            [&color](TGUIPaletteRole const& candidate)
            {
                return color.Role == candidate.Name;
            });

        if (role != roles.end())
            palette.*(role->Color) = color.RGB;
    }

    return palette;
}
//---------------------------------------------------------------------------
std::uint32_t GUISkipTextColorOn(std::uint32_t background)
{
    return IsDarkGUIColor(background) ? GUIRGB(255, 190, 70) : GUIRGB(190, 120, 0);
}
//---------------------------------------------------------------------------
bool IsDarkGUIColor(std::uint32_t color)
{
    int const luma = (GUIRed(color) * 299 + GUIGreen(color) * 587 + GUIBlue(color) * 114) / 1000;

    return luma < 128;
}
//---------------------------------------------------------------------------
/*
    OnyxBlueGUIPalette

    The colors of RAD Studio's Onyx Blue style, read from its .vsf file as CarbonGUIPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TGUIPalette OnyxBlueGUIPalette()
{
    TGUIPalette palette = CarbonGUIPalette();

    palette.ButtonBorder = GUIRGB(0xAD, 0xB0, 0xB4);
    palette.ButtonDefaultFrame = GUIRGB(0x74, 0xA0, 0xC4);
    palette.ButtonDisabledBorder = GUIRGB(0x36, 0x39, 0x3E);
    palette.ButtonDisabledFaceBottom = GUIRGB(0x23, 0x25, 0x28);
    palette.ButtonDisabledFaceTop = GUIRGB(0x42, 0x45, 0x49);
    palette.ButtonDisabledOutline = GUIRGB(0x2A, 0x2C, 0x30);
    palette.ButtonFaceBottom = GUIRGB(0x8C, 0x90, 0x95);
    palette.ButtonFaceTop = GUIRGB(0xD8, 0xDA, 0xDC);
    palette.ButtonHotBorder = GUIRGB(0x42, 0x72, 0x9A);
    palette.ButtonHotFaceBottom = GUIRGB(0x27, 0x41, 0x57);
    palette.ButtonHotFaceTop = GUIRGB(0x74, 0xA0, 0xC4);
    palette.ButtonHotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = GUIRGB(0x2A, 0x2C, 0x30);
    palette.ButtonPressedFaceBottom = GUIRGB(0x3B, 0x5C, 0x77);
    palette.ButtonPressedFaceTop = GUIRGB(0x6D, 0x8C, 0xA6);
    palette.ButtonText = GUIRGB(0x18, 0x1A, 0x1D);
    palette.CheckBoxBorder = GUIRGB(0x6A, 0x6D, 0x72);
    palette.CheckBoxFill = GUIRGB(0x23, 0x25, 0x29);
    palette.CheckBoxHotBorder = GUIRGB(0x8C, 0x8E, 0x92);
    palette.CheckMark = GUIRGB(0xCC, 0xD0, 0xD4);
    palette.CheckMarkDisabled = GUIRGB(0x66, 0x69, 0x6E);
    palette.ControlText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.DisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = GUIRGB(0x29, 0x2C, 0x31);
    palette.EditBorderOuter = GUIRGB(0x1C, 0x1C, 0x1C);
    palette.GripDot = GUIRGB(0x4E, 0x4F, 0x57);
    palette.HeaderBottom = GUIRGB(0x3E, 0x41, 0x46);
    palette.HeaderDivider = GUIRGB(0x44, 0x47, 0x4B);
    palette.HeaderHot = GUIRGB(0x60, 0x63, 0x67);
    palette.HeaderTop = GUIRGB(0x4B, 0x4E, 0x52);
    palette.Highlight = GUIRGB(0x44, 0x75, 0x9D);
    palette.HighlightFrame = GUIRGB(0x41, 0x70, 0x96);
    palette.HotText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = GUIRGB(0x31, 0x34, 0x39);
    palette.MenuBarDisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.MenuBorderInner = GUIRGB(0x3E, 0x41, 0x45);
    palette.MenuBorderOuter = GUIRGB(0x18, 0x1A, 0x1D);
    palette.MenuDisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.MenuSeparator = GUIRGB(0x3E, 0x40, 0x45);
    palette.Panel = GUIRGB(0x31, 0x34, 0x39);
    palette.ProgressBorder = GUIRGB(0x1C, 0x1C, 0x1C);
    palette.ProgressChunkBottom = GUIRGB(0x23, 0x3F, 0x57);
    palette.ProgressChunkTop = GUIRGB(0x69, 0x97, 0xBE);
    palette.ProgressTrack = GUIRGB(0x29, 0x2C, 0x31);
    palette.SelectionText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = GUIRGB(0xDD, 0xDE, 0xDF);
    palette.SpinDisabledArrow = GUIRGB(0x63, 0x67, 0x6B);
    palette.SpinDisabledFaceBottom = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinDisabledFaceTop = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinDisabledOutline = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinFaceBottom = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinFaceTop = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinHotFaceBottom = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinHotFaceTop = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinOutline = GUIRGB(0x26, 0x29, 0x2D);
    palette.SpinPressedEdge = GUIRGB(0x26, 0x29, 0x2D);
    palette.StatusLine = GUIRGB(0x29, 0x2C, 0x33);
    palette.TabActiveBottom = GUIRGB(0x2B, 0x2E, 0x33);
    palette.TabActiveTop = GUIRGB(0x35, 0x38, 0x3D);
    palette.TabBorder = GUIRGB(0x2C, 0x2F, 0x34);
    palette.TabHot = GUIRGB(0x21, 0x24, 0x28);
    palette.TabInactive = GUIRGB(0x21, 0x24, 0x28);
    palette.TabInactiveBorder = GUIRGB(0x20, 0x23, 0x27);
    palette.TabOuter = GUIRGB(0x20, 0x23, 0x27);
    palette.TooltipBackground = GUIRGB(0x12, 0x12, 0x12);
    palette.TooltipText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.Window = GUIRGB(0x29, 0x2C, 0x31);
    palette.WindowText = GUIRGB(0xFF, 0xFF, 0xFF);

    palette.FailureText = GUIFailureTextColorOn(palette.Window);
    palette.SkipText = GUISkipTextColorOn(palette.Window);
    palette.TreeLine = BlendGUIColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
/*
    TabletDarkGUIPalette

    The colors of RAD Studio's Tablet Dark style, read from its .vsf file as CarbonGUIPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TGUIPalette TabletDarkGUIPalette()
{
    TGUIPalette palette = CarbonGUIPalette();

    palette.ButtonBorder = GUIRGB(0x3D, 0x4A, 0x79);
    palette.ButtonDefaultFrame = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonDisabledBorder = GUIRGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledFaceBottom = GUIRGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledFaceTop = GUIRGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledOutline = GUIRGB(0x30, 0x33, 0x4B);
    palette.ButtonFaceBottom = GUIRGB(0x3D, 0x4A, 0x79);
    palette.ButtonFaceTop = GUIRGB(0x3D, 0x4A, 0x79);
    palette.ButtonHotBorder = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotFaceBottom = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotFaceTop = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotText = GUIRGB(0x00, 0x00, 0x00);
    palette.ButtonOutline = GUIRGB(0x3D, 0x4A, 0x79);
    palette.ButtonPressedFaceBottom = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonPressedFaceTop = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ButtonText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.CheckBoxBorder = GUIRGB(0x6B, 0x73, 0x94);
    palette.CheckBoxFill = GUIRGB(0x2F, 0x36, 0x55);
    palette.CheckBoxHotBorder = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.CheckMark = GUIRGB(0xE6, 0xE6, 0xE6);
    palette.CheckMarkDisabled = GUIRGB(0x6D, 0x74, 0x8B);
    palette.ControlText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.DisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = GUIRGB(0x0F, 0x0F, 0x11);
    palette.EditBorderOuter = GUIRGB(0x2B, 0x29, 0x37);
    palette.GripDot = GUIRGB(0x6B, 0x6B, 0x6B);
    palette.HeaderBottom = GUIRGB(0x2F, 0x2D, 0x3C);
    palette.HeaderDivider = GUIRGB(0x2F, 0x2D, 0x3C);
    palette.HeaderHot = GUIRGB(0x36, 0x34, 0x43);
    palette.HeaderTop = GUIRGB(0x2F, 0x2D, 0x3C);
    palette.Highlight = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.HighlightFrame = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.HotText = GUIRGB(0x00, 0x00, 0x00);
    palette.MenuBackground = GUIRGB(0x39, 0x42, 0x64);
    palette.MenuBarDisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.MenuBorderInner = GUIRGB(0x39, 0x42, 0x64);
    palette.MenuBorderOuter = GUIRGB(0x00, 0x00, 0x00);
    palette.MenuDisabledText = GUIRGB(0x80, 0x80, 0x80);
    palette.MenuSeparator = GUIRGB(0x1F, 0x1D, 0x28);
    palette.Panel = GUIRGB(0x2B, 0x29, 0x37);
    palette.ProgressBorder = GUIRGB(0x1A, 0x18, 0x22);
    palette.ProgressChunkBottom = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ProgressChunkTop = GUIRGB(0x1A, 0xBC, 0x9C);
    palette.ProgressTrack = GUIRGB(0x1A, 0x18, 0x22);
    palette.SelectionText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = GUIRGB(0x30, 0x2F, 0x3D);
    palette.SpinDisabledArrow = GUIRGB(0x2E, 0x2C, 0x3A);
    palette.SpinDisabledFaceBottom = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinDisabledFaceTop = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinDisabledOutline = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinFaceBottom = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinFaceTop = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinHotFaceBottom = GUIRGB(0x1D, 0x9F, 0x88);
    palette.SpinHotFaceTop = GUIRGB(0x27, 0x4E, 0x50);
    palette.SpinOutline = GUIRGB(0x2B, 0x29, 0x37);
    palette.SpinPressedEdge = GUIRGB(0x27, 0x4E, 0x50);
    palette.StatusLine = GUIRGB(0x38, 0x37, 0x46);
    palette.TabActiveBottom = GUIRGB(0x39, 0x42, 0x64);
    palette.TabActiveTop = GUIRGB(0x39, 0x42, 0x64);
    palette.TabBorder = GUIRGB(0x39, 0x42, 0x64);
    palette.TabHot = GUIRGB(0x2B, 0x29, 0x37);
    palette.TabInactive = GUIRGB(0x2B, 0x29, 0x37);
    palette.TabInactiveBorder = GUIRGB(0x49, 0x4A, 0x5A);
    palette.TabOuter = GUIRGB(0x3D, 0x4A, 0x79);
    palette.TooltipBackground = GUIRGB(0x39, 0x42, 0x64);
    palette.TooltipText = GUIRGB(0xFF, 0xFF, 0xFF);
    palette.Window = GUIRGB(0x39, 0x42, 0x64);
    palette.WindowText = GUIRGB(0xFF, 0xFF, 0xFF);

    // Lighter than GUIFailureTextColorOn()'s red, which is too dark to read on the indigo window.
    palette.FailureText = GUIRGB(255, 150, 150);
    palette.SkipText = GUISkipTextColorOn(palette.Window);
    palette.TreeLine = BlendGUIColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
/*
    TintedCarbonGUIPalette

    A gray (a color whose HSL saturation is under 0.1) keeps its lightness and gets the chroma 'tint' times
    (1 - its lightness); a colored one keeps its saturation and lightness and gets 'accentHue'.
*/
TGUIPalette TintedCarbonGUIPalette(double hue, double tint, double accentHue)
{
    // The colors that say how a test did.
    std::uint32_t TGUIPalette::* const kept[] = {
        &TGUIPalette::FailureText, &TGUIPalette::ProgressFailedBottom, &TGUIPalette::ProgressFailedTop,
        &TGUIPalette::SkipText, &TGUIPalette::TreeStatusFailed, &TGUIPalette::TreeStatusNotRun,
        &TGUIPalette::TreeStatusPassed, &TGUIPalette::TreeStatusRunning, &TGUIPalette::TreeStatusSkipped
    };

    TGUIPalette palette = CarbonGUIPalette();

    for (TGUIPaletteRole const& role : GUIPaletteRoles())
    {
        if (std::find(std::begin(kept), std::end(kept), role.Color) != std::end(kept))
            continue;

        std::uint32_t& color = palette.*role.Color;
        double colorHue = 0.0;
        double saturation = 0.0;
        double lightness = 0.0;
        ToHSL(color, colorHue, saturation, lightness);

        if (saturation < 0.1)
        {
            // The chroma as HSL's saturation, which is relative to the most chroma the lightness allows.
            double const room = 1.0 - std::fabs(2.0 * lightness - 1.0);
            double const chroma = tint * (1.0 - lightness);
            color = FromHSL(hue, room > 0.0 ? (std::min)(1.0, chroma / room) : 0.0, lightness);
        }
        else
        {
            color = FromHSL(accentHue, saturation, lightness);
        }
    }

    return palette;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
