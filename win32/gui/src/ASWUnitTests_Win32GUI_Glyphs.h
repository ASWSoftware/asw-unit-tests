/* **************************************************************************
ASWUnitTests_Win32GUI_Glyphs.h
Author: Anthony S. West - ASW Software

The small shapes the Win32 GUI runner's dark theme draws, such as a check
box's cross and a menu's submenu arrow, as Carbon (the VCL GUI runner's dark
style) has them, antialiased at any size. Contains no window code, so it's
unit tested from the Windows console runners (see
win32/tests/Test_ASWUnitTests_Win32GUI_Glyphs.cpp).

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
#ifndef ASWUnitTests_Win32GUI_GlyphsH
#define ASWUnitTests_Win32GUI_GlyphsH
//---------------------------------------------------------------------------
#include <cstdint>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EWin32GUIGlyph
//
// A shape RenderWin32GUIGlyph() draws, in a square cell. The check box's
// and radio button's are Carbon's at 13 pixels; the menus' at 16.
/////////////////////////////////////////////////////////////////////////////
enum class EWin32GUIGlyph
{
    // A menu's submenu arrow: a triangle pointing right, in the cell's middle.
    ArrowRight,
    // A radio button's circle, filling the cell; drawn in the border color, with InnerCircle over it in the
    // fill color.
    Circle,
    // A checked check box's mark: a cross.
    Cross,
    // A checked radio button's dot, and a checked radio menu item's.
    Dot,
    // The inside of a radio button's circle, one pixel in from Circle at 13 pixels.
    InnerCircle,
    // A check box's partial state: a square in the middle.
    Square,
    // A checked menu item's mark: a check mark.
    Tick,
};

//---------------------------------------------------------------------------

// How much of each pixel 'glyph' covers in a 'size' x 'size' cell, row by row from the top left: 0 where it's
// absent, up to 255 where it covers the whole pixel, antialiased by sampling each pixel at 4 x 4 points. Empty
// if 'size' isn't positive.
std::vector<std::uint8_t> RenderWin32GUIGlyph(EWin32GUIGlyph glyph, int size);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_GlyphsH
