/* **************************************************************************
ASWUnitTests_Win32GUI_DPI.h
Author: Anthony S. West - ASW Software

Sizes and fonts for a DPI, for the Win32 GUI runner, which is per-monitor
DPI aware: its sizes are given at 96 DPI, as in the VCL GUI runner's .dfm,
and scaled to the DPI of the monitor its window is on.

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
#ifndef ASWUnitTests_Win32GUI_DPIH
#define ASWUnitTests_Win32GUI_DPIH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWin32GUIFont
//
// Owns a font handle, deleting it when replaced or destroyed.
/////////////////////////////////////////////////////////////////////////////
class TWin32GUIFont
{
private:
    HFONT m_Font;

public:
    TWin32GUIFont();
    explicit TWin32GUIFont(HFONT font);
    TWin32GUIFont(TWin32GUIFont&& other) noexcept;
    ~TWin32GUIFont();

    TWin32GUIFont(TWin32GUIFont const&) = delete;
    TWin32GUIFont& operator=(TWin32GUIFont const&) = delete;
    TWin32GUIFont& operator=(TWin32GUIFont&& other) noexcept;

    // The font, or nullptr if there's none.
    HFONT Get() const;
};

//---------------------------------------------------------------------------

// The GUI's monospace font, Consolas 10 pt, for 'dpi', as the VCL GUI runner's detail pane, log and text
// dialog use. Throws std::runtime_error if it can't be created.
TWin32GUIFont CreateMonospaceFont(UINT dpi);
// Windows' message font (Segoe UI 9 pt by default) for 'dpi', the font of the GUI's controls. Throws
// std::runtime_error if it can't be created.
TWin32GUIFont CreateUIFont(UINT dpi);
// 'value', a size at 96 DPI, at 'dpi', rounded to the nearest pixel (halves away from zero).
int ScaleForDPI(int value, UINT dpi);
// The DPI 'window' is shown at: its monitor's, for a per-monitor DPI aware window.
UINT WindowDPI(HWND window);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_DPIH
