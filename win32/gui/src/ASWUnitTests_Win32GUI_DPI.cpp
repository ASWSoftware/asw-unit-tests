/* **************************************************************************
ASWUnitTests_Win32GUI_DPI.cpp
Author: Anthony S. West - ASW Software

Sizes and fonts for a DPI, for the Win32 GUI runner.

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
#include "ASWUnitTests_Win32GUI_DPI.h"
//---------------------------------------------------------------------------
#include <stdexcept>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
TWin32GUIFont CreateMonospaceFont(UINT dpi)
{
    LOGFONTW font = {};
    font.lfHeight = -MulDiv(10, static_cast<int>(dpi), 72);
    font.lfWeight = FW_NORMAL;
    font.lfCharSet = DEFAULT_CHARSET;
    font.lfQuality = CLEARTYPE_QUALITY;
    font.lfPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lstrcpynW(font.lfFaceName, L"Consolas", LF_FACESIZE);

    HFONT const handle = CreateFontIndirectW(&font);

    if (handle == nullptr)
        throw std::runtime_error("Couldn't create the monospace font");

    return TWin32GUIFont(handle);
}
//---------------------------------------------------------------------------
TWin32GUIFont CreateUIFont(UINT dpi)
{
    NONCLIENTMETRICSW metrics = {};
    metrics.cbSize = sizeof(metrics);

    if (SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi) == 0)
        throw std::runtime_error("Couldn't get the message font");

    HFONT const handle = CreateFontIndirectW(&metrics.lfMessageFont);

    if (handle == nullptr)
        throw std::runtime_error("Couldn't create the message font");

    return TWin32GUIFont(handle);
}
//---------------------------------------------------------------------------
int ScaleForDPI(int value, UINT dpi)
{
    return MulDiv(value, static_cast<int>(dpi), USER_DEFAULT_SCREEN_DPI);
}
//---------------------------------------------------------------------------
UINT WindowDPI(HWND window)
{
    UINT const dpi = GetDpiForWindow(window);

    return dpi != 0 ? dpi : USER_DEFAULT_SCREEN_DPI;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIFont
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWin32GUIFont::TWin32GUIFont()
    : m_Font(nullptr)
{
}
//---------------------------------------------------------------------------
TWin32GUIFont::TWin32GUIFont(HFONT font)
    : m_Font(font)
{
}
//---------------------------------------------------------------------------
TWin32GUIFont::TWin32GUIFont(TWin32GUIFont&& other) noexcept
    : m_Font(other.m_Font)
{
    other.m_Font = nullptr;
}
//---------------------------------------------------------------------------
TWin32GUIFont::~TWin32GUIFont()
{
    if (m_Font != nullptr)
        DeleteObject(m_Font);
}
//---------------------------------------------------------------------------
TWin32GUIFont& TWin32GUIFont::operator=(TWin32GUIFont&& other) noexcept
{
    if (this != &other)
    {
        if (m_Font != nullptr)
            DeleteObject(m_Font);

        m_Font = other.m_Font;
        other.m_Font = nullptr;
    }

    return *this;
}
//---------------------------------------------------------------------------
HFONT TWin32GUIFont::Get() const
{
    return m_Font;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
