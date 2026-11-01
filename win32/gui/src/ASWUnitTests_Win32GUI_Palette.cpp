/* **************************************************************************
ASWUnitTests_Win32GUI_Palette.cpp
Author: Anthony S. West - ASW Software

The colors of the Win32 GUI runner's dark theme as COLORREFs.

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
#include <cstddef>
#include <cstdint>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::uint32_t FromCOLORREF(COLORREF color);
COLORREF ToCOLORREF(std::uint32_t rgb);

//---------------------------------------------------------------------------
/*
    FromCOLORREF

    A COLORREF, 0x00BBGGRR, as a theme file's color, 0xRRGGBB.
*/
std::uint32_t FromCOLORREF(COLORREF color)
{
    return ASWUnitTests::GUIRGB(GetRValue(color), GetGValue(color), GetBValue(color));
}
//---------------------------------------------------------------------------
/*
    ToCOLORREF

    A theme file's color, 0xRRGGBB, as a COLORREF, 0x00BBGGRR.
*/
COLORREF ToCOLORREF(std::uint32_t rgb)
{
    return RGB(ASWUnitTests::GUIRed(rgb), ASWUnitTests::GUIGreen(rgb), ASWUnitTests::GUIBlue(rgb));
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
COLORREF BlendColors(COLORREF from, COLORREF to, int numerator, int denominator)
{
    return ToCOLORREF(BlendGUIColors(FromCOLORREF(from), FromCOLORREF(to), numerator, denominator));
}
//---------------------------------------------------------------------------
COLORREF FailureTextColorOn(COLORREF background)
{
    return ToCOLORREF(GUIFailureTextColorOn(FromCOLORREF(background)));
}
//---------------------------------------------------------------------------
COLORREF SkipTextColorOn(COLORREF background)
{
    return ToCOLORREF(GUISkipTextColorOn(FromCOLORREF(background)));
}
//---------------------------------------------------------------------------
/*
    ToWin32Palette

    GUIPaletteRolesOf() lists the roles in the same order for both color types, so the two lists pair up.
*/
TWin32GUIPalette ToWin32Palette(TGUIPalette const& palette)
{
    std::vector<TGUIPaletteRole> const& from = GUIPaletteRoles();
    std::vector<TGUIPaletteRoleOf<COLORREF> > const& to = GUIPaletteRolesOf<COLORREF>();
    TWin32GUIPalette converted = {};

    for (std::size_t i = 0; i < from.size(); ++i)
        converted.*to[i].Color = ToCOLORREF(palette.*from[i].Color);

    return converted;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
