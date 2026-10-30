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

COLORREF FromHSL(double hue, double saturation, double lightness);
COLORREF FromRGB(std::uint32_t rgb);
bool SameText(std::string const& a, std::string const& b);
void ToHSL(COLORREF color, double& hue, double& saturation, double& lightness);
std::uint32_t ToRGB(COLORREF color);

//---------------------------------------------------------------------------
/*
    FromHSL

    The color with 'hue' (in degrees, any number of turns), 'saturation' and 'lightness' (0 to 1 each), each
    channel rounded to the nearest.
*/
COLORREF FromHSL(double hue, double saturation, double lightness)
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
            return static_cast<BYTE>(std::lround((std::min)(1.0, (std::max)(0.0, value + lowest)) * 255.0));
        };

    return RGB(channel(red), channel(green), channel(blue));
}
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
void ToHSL(COLORREF color, double& hue, double& saturation, double& lightness)
{
    double const red = GetRValue(color) / 255.0;
    double const green = GetGValue(color) / 255.0;
    double const blue = GetBValue(color) / 255.0;
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
    BuiltInDarkThemes

    Each tint's hue and strength, and its accent's hue, chosen by eye in the runner. The VCL styles' names are
    GUIVCLStyleDarkThemeNames()'s.
*/
std::vector<TWin32GUIBuiltInTheme> const& BuiltInDarkThemes()
{
    static std::vector<TWin32GUIBuiltInTheme> const themes = {
        { GUIDefaultDarkThemeName(), CarbonPalette() },
        { "Charcoal Dark Slate", CharcoalDarkSlatePalette() },
        { "Cobalt XEMedia", CobaltXEMediaPalette() },
        { "Ember", TintedCarbonPalette(25.0, 0.09, 30.0) },
        { "Forest", TintedCarbonPalette(140.0, 0.08, 155.0) },
        { "Midnight", TintedCarbonPalette(220.0, 0.10, 205.0) },
        { "Onyx Blue", OnyxBluePalette() },
        { "Plum", TintedCarbonPalette(285.0, 0.09, 295.0) },
        { "Tablet Dark", TabletDarkPalette() }
    };

    return themes;
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
    palette.ButtonHotText = RGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = RGB(0x19, 0x19, 0x19);
    palette.ButtonPressedFaceBottom = RGB(0x48, 0x48, 0x48);
    palette.ButtonPressedFaceTop = RGB(0x37, 0x37, 0x37);
    palette.ButtonText = RGB(0xC0, 0xC0, 0xC0);
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
    palette.SelectionText = RGB(0xFF, 0xFF, 0xFF);
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
/*
    CharcoalDarkSlatePalette

    The colors of RAD Studio's Charcoal Dark Slate style, read from its .vsf file as CarbonPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TWin32GUIPalette CharcoalDarkSlatePalette()
{
    TWin32GUIPalette palette = CarbonPalette();

    palette.ButtonBorder = RGB(0x43, 0x43, 0x43);
    palette.ButtonDefaultFrame = RGB(0x32, 0x32, 0x32);
    palette.ButtonDisabledBorder = RGB(0x45, 0x45, 0x45);
    palette.ButtonDisabledFaceBottom = RGB(0x39, 0x39, 0x39);
    palette.ButtonDisabledFaceTop = RGB(0x4C, 0x4C, 0x4C);
    palette.ButtonDisabledOutline = RGB(0x2F, 0x2F, 0x2F);
    palette.ButtonFaceBottom = RGB(0x28, 0x28, 0x28);
    palette.ButtonFaceTop = RGB(0x40, 0x40, 0x40);
    palette.ButtonHotBorder = RGB(0x4A, 0x4A, 0x4A);
    palette.ButtonHotFaceBottom = RGB(0x1F, 0x1F, 0x1F);
    palette.ButtonHotFaceTop = RGB(0x32, 0x32, 0x32);
    palette.ButtonHotText = RGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = RGB(0x27, 0x27, 0x27);
    palette.ButtonPressedFaceBottom = RGB(0x20, 0x20, 0x20);
    palette.ButtonPressedFaceTop = RGB(0x14, 0x14, 0x14);
    palette.ButtonText = RGB(0xA3, 0xA3, 0xA3);
    palette.CheckBoxBorder = RGB(0x5D, 0x5D, 0x5D);
    palette.CheckBoxFill = RGB(0x1F, 0x1F, 0x1F);
    palette.CheckBoxHotBorder = RGB(0x70, 0x70, 0x70);
    palette.CheckMark = RGB(0x7B, 0x7B, 0x7B);
    palette.CheckMarkDisabled = RGB(0x4D, 0x4D, 0x4D);
    palette.ControlText = RGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = RGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = RGB(0x5E, 0x5E, 0x5E);
    palette.EditBorderOuter = RGB(0x36, 0x36, 0x36);
    palette.GripDot = RGB(0x41, 0x41, 0x41);
    palette.HeaderBottom = RGB(0x3E, 0x3E, 0x3E);
    palette.HeaderDivider = RGB(0x2D, 0x2D, 0x2D);
    palette.HeaderHot = RGB(0x43, 0x43, 0x43);
    palette.HeaderTop = RGB(0x49, 0x49, 0x49);
    palette.Highlight = RGB(0x5F, 0x5F, 0x5F);
    palette.HighlightFrame = RGB(0x8A, 0x8A, 0x8A);
    palette.HotText = RGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = RGB(0x1F, 0x1F, 0x1F);
    palette.MenuBarDisabledText = RGB(0x93, 0x93, 0x93);
    palette.MenuBorderInner = RGB(0x58, 0x58, 0x58);
    palette.MenuBorderOuter = RGB(0x36, 0x36, 0x36);
    palette.MenuDisabledText = RGB(0x69, 0x69, 0x69);
    palette.MenuSeparator = RGB(0x3B, 0x3B, 0x3B);
    palette.Panel = RGB(0x3E, 0x3E, 0x3E);
    palette.ProgressBorder = RGB(0x29, 0x29, 0x29);
    palette.ProgressChunkBottom = RGB(0x5C, 0x5C, 0x5C);
    palette.ProgressChunkTop = RGB(0x5C, 0x5C, 0x5C);
    palette.ProgressTrack = RGB(0x31, 0x31, 0x31);
    palette.SelectionText = RGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = RGB(0x7D, 0x7D, 0x7D);
    palette.SpinDisabledArrow = RGB(0x40, 0x40, 0x40);
    palette.SpinDisabledFaceBottom = RGB(0x23, 0x23, 0x23);
    palette.SpinDisabledFaceTop = RGB(0x23, 0x23, 0x23);
    palette.SpinDisabledOutline = RGB(0x23, 0x23, 0x23);
    palette.SpinFaceBottom = RGB(0x23, 0x23, 0x23);
    palette.SpinFaceTop = RGB(0x23, 0x23, 0x23);
    palette.SpinHotFaceBottom = RGB(0x23, 0x23, 0x23);
    palette.SpinHotFaceTop = RGB(0x23, 0x23, 0x23);
    palette.SpinOutline = RGB(0x23, 0x23, 0x23);
    palette.SpinPressedEdge = RGB(0x23, 0x23, 0x23);
    palette.StatusLine = RGB(0x40, 0x40, 0x40);
    palette.TabActiveBottom = RGB(0x3E, 0x3E, 0x3E);
    palette.TabActiveTop = RGB(0x57, 0x57, 0x57);
    palette.TabBorder = RGB(0x40, 0x40, 0x40);
    palette.TabHot = RGB(0x25, 0x25, 0x25);
    palette.TabInactive = RGB(0x25, 0x25, 0x25);
    palette.TabInactiveBorder = RGB(0x23, 0x23, 0x23);
    palette.TabOuter = RGB(0x23, 0x23, 0x23);
    palette.TooltipBackground = RGB(0x5B, 0x5B, 0x5B);
    palette.TooltipText = RGB(0xFF, 0xFF, 0xFF);
    palette.Window = RGB(0x23, 0x23, 0x23);
    palette.WindowText = RGB(0xC0, 0xC0, 0xC0);

    palette.FailureText = FailureTextColorOn(palette.Window);
    palette.SkipText = SkipTextColorOn(palette.Window);
    palette.TreeLine = BlendColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
/*
    CobaltXEMediaPalette

    The colors of RAD Studio's Cobalt XEMedia style, read from its .vsf file as CarbonPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TWin32GUIPalette CobaltXEMediaPalette()
{
    TWin32GUIPalette palette = CarbonPalette();

    palette.ButtonBorder = RGB(0x07, 0x0B, 0x11);
    palette.ButtonDefaultFrame = RGB(0x5A, 0x77, 0x9B);
    palette.ButtonDisabledBorder = RGB(0x09, 0x0A, 0x0B);
    palette.ButtonDisabledFaceBottom = RGB(0x16, 0x18, 0x1C);
    palette.ButtonDisabledFaceTop = RGB(0x2B, 0x2E, 0x32);
    palette.ButtonDisabledOutline = RGB(0x14, 0x17, 0x1A);
    palette.ButtonFaceBottom = RGB(0x1A, 0x33, 0x53);
    palette.ButtonFaceTop = RGB(0x46, 0x53, 0x66);
    palette.ButtonHotBorder = RGB(0x07, 0x0F, 0x1C);
    palette.ButtonHotFaceBottom = RGB(0x2A, 0x79, 0xB0);
    palette.ButtonHotFaceTop = RGB(0x5A, 0x77, 0x9B);
    palette.ButtonHotText = RGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = RGB(0x11, 0x1A, 0x28);
    palette.ButtonPressedFaceBottom = RGB(0x15, 0x4D, 0x75);
    palette.ButtonPressedFaceTop = RGB(0x38, 0x4C, 0x66);
    palette.ButtonText = RGB(0xC0, 0xC0, 0xC0);
    palette.CheckBoxBorder = RGB(0x39, 0x57, 0x8B);
    palette.CheckBoxFill = RGB(0x17, 0x2E, 0x4D);
    palette.CheckBoxHotBorder = RGB(0x54, 0x6E, 0x9C);
    palette.CheckMark = RGB(0x27, 0x75, 0xB8);
    palette.CheckMarkDisabled = RGB(0x1B, 0x49, 0x74);
    palette.ControlText = RGB(0xC0, 0xC0, 0xC0);
    palette.DisabledText = RGB(0x41, 0x41, 0x41);
    palette.EditBorderInner = RGB(0x04, 0x06, 0x0C);
    palette.EditBorderOuter = RGB(0x18, 0x31, 0x53);
    palette.GripDot = RGB(0x1B, 0x2B, 0x45);
    palette.HeaderBottom = RGB(0x10, 0x17, 0x24);
    palette.HeaderDivider = RGB(0x00, 0x00, 0x00);
    palette.HeaderHot = RGB(0x0B, 0x11, 0x19);
    palette.HeaderTop = RGB(0x19, 0x21, 0x2D);
    palette.Highlight = RGB(0x0B, 0x36, 0x65);
    palette.HighlightFrame = RGB(0x29, 0x45, 0x69);
    palette.HotText = RGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = RGB(0x00, 0x00, 0x00);
    palette.MenuBarDisabledText = RGB(0x41, 0x41, 0x41);
    palette.MenuBorderInner = RGB(0x0D, 0x18, 0x25);
    palette.MenuBorderOuter = RGB(0x21, 0x39, 0x50);
    palette.MenuDisabledText = RGB(0x41, 0x41, 0x41);
    palette.MenuSeparator = RGB(0x0B, 0x36, 0x65);
    palette.Panel = RGB(0x01, 0x01, 0x01);
    palette.ProgressBorder = RGB(0x18, 0x31, 0x53);
    palette.ProgressChunkBottom = RGB(0x24, 0x8B, 0xD1);
    palette.ProgressChunkTop = RGB(0x86, 0xB1, 0xCC);
    palette.ProgressTrack = RGB(0x00, 0x00, 0x00);
    palette.SelectionText = RGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = RGB(0x4C, 0x6D, 0x8F);
    palette.SpinDisabledArrow = RGB(0x2D, 0x41, 0x56);
    palette.SpinDisabledFaceBottom = RGB(0x02, 0x04, 0x08);
    palette.SpinDisabledFaceTop = RGB(0x02, 0x04, 0x08);
    palette.SpinDisabledOutline = RGB(0x02, 0x04, 0x08);
    palette.SpinFaceBottom = RGB(0x02, 0x04, 0x08);
    palette.SpinFaceTop = RGB(0x02, 0x04, 0x08);
    palette.SpinHotFaceBottom = RGB(0x02, 0x04, 0x08);
    palette.SpinHotFaceTop = RGB(0x02, 0x04, 0x08);
    palette.SpinOutline = RGB(0x02, 0x04, 0x08);
    palette.SpinPressedEdge = RGB(0x02, 0x04, 0x08);
    palette.StatusLine = RGB(0x03, 0x05, 0x07);
    palette.TabActiveBottom = RGB(0x0E, 0x13, 0x1C);
    palette.TabActiveTop = RGB(0x21, 0x2F, 0x45);
    palette.TabBorder = RGB(0x0A, 0x0E, 0x15);
    palette.TabHot = RGB(0x05, 0x07, 0x0A);
    palette.TabInactive = RGB(0x05, 0x07, 0x0A);
    palette.TabInactiveBorder = RGB(0x0D, 0x0D, 0x0D);
    palette.TabOuter = RGB(0x32, 0x43, 0x5E);
    palette.TooltipBackground = RGB(0x26, 0x46, 0x6C);
    palette.TooltipText = RGB(0xC0, 0xC0, 0xC0);
    palette.Window = RGB(0x04, 0x07, 0x0E);
    palette.WindowText = RGB(0xFF, 0xFF, 0xFF);

    palette.FailureText = FailureTextColorOn(palette.Window);
    palette.SkipText = SkipTextColorOn(palette.Window);
    palette.TreeLine = BlendColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
COLORREF FailureTextColorOn(COLORREF background)
{
    return IsDarkColor(background) ? RGB(255, 100, 100) : RGB(200, 0, 0);
}
//---------------------------------------------------------------------------
TWin32GUIBuiltInTheme const* FindBuiltInDarkTheme(std::string const& name)
{
    for (TWin32GUIBuiltInTheme const& theme : BuiltInDarkThemes())
    {
        if (SameText(theme.Name, name))
            return &theme;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
bool IsDarkColor(COLORREF color)
{
    int const luma = (GetRValue(color) * 299 + GetGValue(color) * 587 + GetBValue(color) * 114) / 1000;

    return luma < 128;
}
//---------------------------------------------------------------------------
/*
    OnyxBluePalette

    The colors of RAD Studio's Onyx Blue style, read from its .vsf file as CarbonPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TWin32GUIPalette OnyxBluePalette()
{
    TWin32GUIPalette palette = CarbonPalette();

    palette.ButtonBorder = RGB(0xAD, 0xB0, 0xB4);
    palette.ButtonDefaultFrame = RGB(0x74, 0xA0, 0xC4);
    palette.ButtonDisabledBorder = RGB(0x36, 0x39, 0x3E);
    palette.ButtonDisabledFaceBottom = RGB(0x23, 0x25, 0x28);
    palette.ButtonDisabledFaceTop = RGB(0x42, 0x45, 0x49);
    palette.ButtonDisabledOutline = RGB(0x2A, 0x2C, 0x30);
    palette.ButtonFaceBottom = RGB(0x8C, 0x90, 0x95);
    palette.ButtonFaceTop = RGB(0xD8, 0xDA, 0xDC);
    palette.ButtonHotBorder = RGB(0x42, 0x72, 0x9A);
    palette.ButtonHotFaceBottom = RGB(0x27, 0x41, 0x57);
    palette.ButtonHotFaceTop = RGB(0x74, 0xA0, 0xC4);
    palette.ButtonHotText = RGB(0xFF, 0xFF, 0xFF);
    palette.ButtonOutline = RGB(0x2A, 0x2C, 0x30);
    palette.ButtonPressedFaceBottom = RGB(0x3B, 0x5C, 0x77);
    palette.ButtonPressedFaceTop = RGB(0x6D, 0x8C, 0xA6);
    palette.ButtonText = RGB(0x18, 0x1A, 0x1D);
    palette.CheckBoxBorder = RGB(0x6A, 0x6D, 0x72);
    palette.CheckBoxFill = RGB(0x23, 0x25, 0x29);
    palette.CheckBoxHotBorder = RGB(0x8C, 0x8E, 0x92);
    palette.CheckMark = RGB(0xCC, 0xD0, 0xD4);
    palette.CheckMarkDisabled = RGB(0x66, 0x69, 0x6E);
    palette.ControlText = RGB(0xFF, 0xFF, 0xFF);
    palette.DisabledText = RGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = RGB(0x29, 0x2C, 0x31);
    palette.EditBorderOuter = RGB(0x1C, 0x1C, 0x1C);
    palette.GripDot = RGB(0x4E, 0x4F, 0x57);
    palette.HeaderBottom = RGB(0x3E, 0x41, 0x46);
    palette.HeaderDivider = RGB(0x44, 0x47, 0x4B);
    palette.HeaderHot = RGB(0x60, 0x63, 0x67);
    palette.HeaderTop = RGB(0x4B, 0x4E, 0x52);
    palette.Highlight = RGB(0x44, 0x75, 0x9D);
    palette.HighlightFrame = RGB(0x41, 0x70, 0x96);
    palette.HotText = RGB(0xFF, 0xFF, 0xFF);
    palette.MenuBackground = RGB(0x31, 0x34, 0x39);
    palette.MenuBarDisabledText = RGB(0x80, 0x80, 0x80);
    palette.MenuBorderInner = RGB(0x3E, 0x41, 0x45);
    palette.MenuBorderOuter = RGB(0x18, 0x1A, 0x1D);
    palette.MenuDisabledText = RGB(0x80, 0x80, 0x80);
    palette.MenuSeparator = RGB(0x3E, 0x40, 0x45);
    palette.Panel = RGB(0x31, 0x34, 0x39);
    palette.ProgressBorder = RGB(0x1C, 0x1C, 0x1C);
    palette.ProgressChunkBottom = RGB(0x23, 0x3F, 0x57);
    palette.ProgressChunkTop = RGB(0x69, 0x97, 0xBE);
    palette.ProgressTrack = RGB(0x29, 0x2C, 0x31);
    palette.SelectionText = RGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = RGB(0xDD, 0xDE, 0xDF);
    palette.SpinDisabledArrow = RGB(0x63, 0x67, 0x6B);
    palette.SpinDisabledFaceBottom = RGB(0x26, 0x29, 0x2D);
    palette.SpinDisabledFaceTop = RGB(0x26, 0x29, 0x2D);
    palette.SpinDisabledOutline = RGB(0x26, 0x29, 0x2D);
    palette.SpinFaceBottom = RGB(0x26, 0x29, 0x2D);
    palette.SpinFaceTop = RGB(0x26, 0x29, 0x2D);
    palette.SpinHotFaceBottom = RGB(0x26, 0x29, 0x2D);
    palette.SpinHotFaceTop = RGB(0x26, 0x29, 0x2D);
    palette.SpinOutline = RGB(0x26, 0x29, 0x2D);
    palette.SpinPressedEdge = RGB(0x26, 0x29, 0x2D);
    palette.StatusLine = RGB(0x29, 0x2C, 0x33);
    palette.TabActiveBottom = RGB(0x2B, 0x2E, 0x33);
    palette.TabActiveTop = RGB(0x35, 0x38, 0x3D);
    palette.TabBorder = RGB(0x2C, 0x2F, 0x34);
    palette.TabHot = RGB(0x21, 0x24, 0x28);
    palette.TabInactive = RGB(0x21, 0x24, 0x28);
    palette.TabInactiveBorder = RGB(0x20, 0x23, 0x27);
    palette.TabOuter = RGB(0x20, 0x23, 0x27);
    palette.TooltipBackground = RGB(0x12, 0x12, 0x12);
    palette.TooltipText = RGB(0xFF, 0xFF, 0xFF);
    palette.Window = RGB(0x29, 0x2C, 0x31);
    palette.WindowText = RGB(0xFF, 0xFF, 0xFF);

    palette.FailureText = FailureTextColorOn(palette.Window);
    palette.SkipText = SkipTextColorOn(palette.Window);
    palette.TreeLine = BlendColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
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
            "(bottom and top), normal, hot (under the mouse), pressed and disabled, the outline around them, and\n"
            "the text, normal and hot or pressed."),
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
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonHotText, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonOutline, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonPressedFaceBottom, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonPressedFaceTop, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ButtonText, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxBorder,
            "Check boxes (the test tree's too) and radio buttons: the box's or circle's border, its inside, its\n"
            "border under the mouse, and the mark (a cross, the partial state's square, a radio button's dot),\n"
            "normal and disabled."),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxFill, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckBoxHotBorder, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckMark, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(CheckMarkDisabled, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(ControlText,
            "The text of labels, radio buttons and check boxes, menus, the inactive tabs, the list's header and\n"
            "the status bar."),
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
        ASWUNITTESTS_WIN32GUI_ROLE(HotText, "Text over Highlight."),
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
        ASWUNITTESTS_WIN32GUI_ROLE(Selection,
            "The selected node or row in the tree and the list, its frame, and its text."),
        ASWUNITTESTS_WIN32GUI_ROLE(SelectionFrame, nullptr),
        ASWUNITTESTS_WIN32GUI_ROLE(SelectionText, nullptr),
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
/*
    TabletDarkPalette

    The colors of RAD Studio's Tablet Dark style, read from its .vsf file as CarbonPalette()'s were
    (plans/1.5/experiments/style_palette.dpr), with the values its elements don't give taken from the VCL GUI
    runner under it; the selection and the colors that say how a test did are Carbon's, as the VCL GUI
    runner's are under every style.
*/
TWin32GUIPalette TabletDarkPalette()
{
    TWin32GUIPalette palette = CarbonPalette();

    palette.ButtonBorder = RGB(0x3D, 0x4A, 0x79);
    palette.ButtonDefaultFrame = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonDisabledBorder = RGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledFaceBottom = RGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledFaceTop = RGB(0x30, 0x33, 0x4B);
    palette.ButtonDisabledOutline = RGB(0x30, 0x33, 0x4B);
    palette.ButtonFaceBottom = RGB(0x3D, 0x4A, 0x79);
    palette.ButtonFaceTop = RGB(0x3D, 0x4A, 0x79);
    palette.ButtonHotBorder = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotFaceBottom = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotFaceTop = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonHotText = RGB(0x00, 0x00, 0x00);
    palette.ButtonOutline = RGB(0x3D, 0x4A, 0x79);
    palette.ButtonPressedFaceBottom = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonPressedFaceTop = RGB(0x1A, 0xBC, 0x9C);
    palette.ButtonText = RGB(0xFF, 0xFF, 0xFF);
    palette.CheckBoxBorder = RGB(0x6B, 0x73, 0x94);
    palette.CheckBoxFill = RGB(0x2F, 0x36, 0x55);
    palette.CheckBoxHotBorder = RGB(0x1A, 0xBC, 0x9C);
    palette.CheckMark = RGB(0xE6, 0xE6, 0xE6);
    palette.CheckMarkDisabled = RGB(0x6D, 0x74, 0x8B);
    palette.ControlText = RGB(0xFF, 0xFF, 0xFF);
    palette.DisabledText = RGB(0x80, 0x80, 0x80);
    palette.EditBorderInner = RGB(0x0F, 0x0F, 0x11);
    palette.EditBorderOuter = RGB(0x2B, 0x29, 0x37);
    palette.GripDot = RGB(0x6B, 0x6B, 0x6B);
    palette.HeaderBottom = RGB(0x2F, 0x2D, 0x3C);
    palette.HeaderDivider = RGB(0x2F, 0x2D, 0x3C);
    palette.HeaderHot = RGB(0x36, 0x34, 0x43);
    palette.HeaderTop = RGB(0x2F, 0x2D, 0x3C);
    palette.Highlight = RGB(0x1A, 0xBC, 0x9C);
    palette.HighlightFrame = RGB(0x1A, 0xBC, 0x9C);
    palette.HotText = RGB(0x00, 0x00, 0x00);
    palette.MenuBackground = RGB(0x39, 0x42, 0x64);
    palette.MenuBarDisabledText = RGB(0x80, 0x80, 0x80);
    palette.MenuBorderInner = RGB(0x39, 0x42, 0x64);
    palette.MenuBorderOuter = RGB(0x00, 0x00, 0x00);
    palette.MenuDisabledText = RGB(0x80, 0x80, 0x80);
    palette.MenuSeparator = RGB(0x1F, 0x1D, 0x28);
    palette.Panel = RGB(0x2B, 0x29, 0x37);
    palette.ProgressBorder = RGB(0x1A, 0x18, 0x22);
    palette.ProgressChunkBottom = RGB(0x1A, 0xBC, 0x9C);
    palette.ProgressChunkTop = RGB(0x1A, 0xBC, 0x9C);
    palette.ProgressTrack = RGB(0x1A, 0x18, 0x22);
    palette.SelectionText = RGB(0xFF, 0xFF, 0xFF);
    palette.SpinArrow = RGB(0x30, 0x2F, 0x3D);
    palette.SpinDisabledArrow = RGB(0x2E, 0x2C, 0x3A);
    palette.SpinDisabledFaceBottom = RGB(0x2B, 0x29, 0x37);
    palette.SpinDisabledFaceTop = RGB(0x2B, 0x29, 0x37);
    palette.SpinDisabledOutline = RGB(0x2B, 0x29, 0x37);
    palette.SpinFaceBottom = RGB(0x2B, 0x29, 0x37);
    palette.SpinFaceTop = RGB(0x2B, 0x29, 0x37);
    palette.SpinHotFaceBottom = RGB(0x1D, 0x9F, 0x88);
    palette.SpinHotFaceTop = RGB(0x27, 0x4E, 0x50);
    palette.SpinOutline = RGB(0x2B, 0x29, 0x37);
    palette.SpinPressedEdge = RGB(0x27, 0x4E, 0x50);
    palette.StatusLine = RGB(0x38, 0x37, 0x46);
    palette.TabActiveBottom = RGB(0x39, 0x42, 0x64);
    palette.TabActiveTop = RGB(0x39, 0x42, 0x64);
    palette.TabBorder = RGB(0x39, 0x42, 0x64);
    palette.TabHot = RGB(0x2B, 0x29, 0x37);
    palette.TabInactive = RGB(0x2B, 0x29, 0x37);
    palette.TabInactiveBorder = RGB(0x49, 0x4A, 0x5A);
    palette.TabOuter = RGB(0x3D, 0x4A, 0x79);
    palette.TooltipBackground = RGB(0x39, 0x42, 0x64);
    palette.TooltipText = RGB(0xFF, 0xFF, 0xFF);
    palette.Window = RGB(0x39, 0x42, 0x64);
    palette.WindowText = RGB(0xFF, 0xFF, 0xFF);

    // Lighter than FailureTextColorOn()'s red, which is too dark to read on the indigo window.
    palette.FailureText = RGB(255, 150, 150);
    palette.SkipText = SkipTextColorOn(palette.Window);
    palette.TreeLine = BlendColors(palette.Window, palette.WindowText, 2, 5);
    return palette;
}
//---------------------------------------------------------------------------
/*
    TintedCarbonPalette

    A gray (a color whose HSL saturation is under 0.1) keeps its lightness and gets the chroma 'tint' times
    (1 - its lightness); a colored one keeps its saturation and lightness and gets 'accentHue'.
*/
TWin32GUIPalette TintedCarbonPalette(double hue, double tint, double accentHue)
{
    // The colors that say how a test did.
    COLORREF TWin32GUIPalette::* const kept[] = {
        &TWin32GUIPalette::FailureText, &TWin32GUIPalette::ProgressFailedBottom, &TWin32GUIPalette::ProgressFailedTop,
        &TWin32GUIPalette::SkipText, &TWin32GUIPalette::TreeStatusFailed, &TWin32GUIPalette::TreeStatusNotRun,
        &TWin32GUIPalette::TreeStatusPassed, &TWin32GUIPalette::TreeStatusRunning, &TWin32GUIPalette::TreeStatusSkipped
    };

    TWin32GUIPalette palette = CarbonPalette();

    for (TWin32GUIPaletteRole const& role : PaletteRoles())
    {
        if (std::find(std::begin(kept), std::end(kept), role.Color) != std::end(kept))
            continue;

        COLORREF& color = palette.*role.Color;
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
