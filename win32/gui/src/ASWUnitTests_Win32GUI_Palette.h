/* **************************************************************************
ASWUnitTests_Win32GUI_Palette.h
Author: Anthony S. West - ASW Software

The colors of the Win32 GUI runner's dark theme as COLORREFs, converted
from the GUI runners' shared palette (gui/src/ASWUnitTests_GUI_Palette.h).
Contains no window code, so it's unit tested from the Windows console
runners (see win32/tests/Test_ASWUnitTests_Win32GUI_Palette.cpp).

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
#include "ASWUnitTests_GUI_Palette.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWin32GUIPalette
//
// The palette the Win32 GUI runner paints its dark theme with: the shared
// palette's roles (see TGUIPaletteOf) as COLORREFs, 0x00BBGGRR, made from a
// TGUIPalette by ToWin32Palette().
/////////////////////////////////////////////////////////////////////////////
using TWin32GUIPalette = TGUIPaletteOf<COLORREF>;

//---------------------------------------------------------------------------

// BlendGUIColors() for COLORREFs.
COLORREF BlendColors(COLORREF from, COLORREF to, int numerator, int denominator);
// GUIFailureTextColorOn() for COLORREFs.
COLORREF FailureTextColorOn(COLORREF background);
// GUISkipTextColorOn() for COLORREFs.
COLORREF SkipTextColorOn(COLORREF background);
// 'palette' with each role's color converted from 0xRRGGBB to a COLORREF.
TWin32GUIPalette ToWin32Palette(TGUIPalette const& palette);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_PaletteH
