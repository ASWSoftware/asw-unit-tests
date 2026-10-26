/* **************************************************************************
ASWUnitTests_GUI_Layout.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
#include "ASWUnitTests_GUI_Layout.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <climits>
#include <cmath>
#include <string>
//---------------------------------------------------------------------------

namespace
{

char const PanelsSection[] = "Panels";
char const WindowSection[] = "Window";

// Smallest sizes accepted from a layout file, at 96 DPI; anything smaller is treated as invalid.
int const MinPanelSize = 1;
int const MinWindowHeight = 150;
int const MinWindowWidth = 200;

// How much of a window's top edge must be on a work area for IsWindowReachable(), in pixels: the top
// TitleBarHeight pixels, at least MinGrabWidth of them across (or the window's whole width, if narrower).
int const MinGrabWidth = 100;
int const TitleBarHeight = 30;

//---------------------------------------------------------------------------

std::optional<int> ParseInt(std::string const& text);
std::optional<int> ReadInt(ASWUnitTests::TGUIINIDocument const& ini, char const* section, char const* ident);
std::optional<int> ReadSize(ASWUnitTests::TGUIINIDocument const& ini, char const* section, char const* ident,
    int minimum);
void WriteOrDelete(ASWUnitTests::TGUIINIDocument& ini, char const* section, char const* ident,
    std::optional<int> const& value);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
/*
    ParseInt

    'text' as a whole decimal number, with an optional sign, or empty if it isn't one or doesn't fit in an int.
*/
std::optional<int> ParseInt(std::string const& text)
{
    size_t start = 0;
    bool const negative = !text.empty() && text[0] == '-';

    if (!text.empty() && (text[0] == '-' || text[0] == '+'))
        start = 1;

    if (start == text.size())
        return std::nullopt;

    // Accumulated as a negative number, whose range includes INT_MIN.
    long long value = 0;

    for (size_t i = start; i < text.size(); ++i)
    {
        if (text[i] < '0' || text[i] > '9')
            return std::nullopt;

        value = value * 10 - (text[i] - '0');

        if (value < INT_MIN)
            return std::nullopt;
    }

    if (!negative && -value > INT_MAX)
        return std::nullopt;

    return static_cast<int>(negative ? value : -value);
}
//---------------------------------------------------------------------------
/*
    ReadInt

    The value, or empty if it's missing or isn't a whole number.
*/
std::optional<int> ReadInt(ASWUnitTests::TGUIINIDocument const& ini, char const* section, char const* ident)
{
    std::optional<std::string> const text = ini.ReadString(section, ident);

    if (!text.has_value())
        return std::nullopt;

    return ParseInt(*text);
}
//---------------------------------------------------------------------------
/*
    ReadSize

    The value, or empty if it's missing, isn't a whole number, or is less than 'minimum'.
*/
std::optional<int> ReadSize(ASWUnitTests::TGUIINIDocument const& ini, char const* section, char const* ident,
    int minimum)
{
    std::optional<int> const value = ReadInt(ini, section, ident);

    if (!value.has_value() || *value < minimum)
        return std::nullopt;

    return value;
}
//---------------------------------------------------------------------------
void WriteOrDelete(ASWUnitTests::TGUIINIDocument& ini, char const* section, char const* ident,
    std::optional<int> const& value)
{
    if (value.has_value())
        ini.WriteString(section, ident, std::to_string(*value));
    else
        ini.DeleteKey(section, ident);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
bool IsWindowReachable(TGUIRect const& bounds, std::vector<TGUIRect> const& workAreas)
{
    if (bounds.Width() <= 0 || bounds.Height() <= 0)
        return false;

    int const titleBarBottom = bounds.Top + std::min(bounds.Height(), TitleBarHeight);
    int const minWidth = std::min(bounds.Width(), MinGrabWidth);

    for (TGUIRect const& workArea : workAreas)
    {
        int const overlapWidth = std::min(bounds.Right, workArea.Right) - std::max(bounds.Left, workArea.Left);
        bool const titleBarInside = bounds.Top >= workArea.Top && titleBarBottom <= workArea.Bottom;

        if (titleBarInside && overlapWidth >= minWidth)
            return true;
    }

    return false;
}
//---------------------------------------------------------------------------
TGUILayout LoadGUILayout(TGUIINIDocument const& ini)
{
    TGUILayout layout;

    layout.DetailHeight = ReadSize(ini, PanelsSection, "DetailHeight", MinPanelSize);
    layout.Maximized = ReadInt(ini, WindowSection, "Maximized").value_or(0) != 0;
    layout.TestTreeWidth = ReadSize(ini, PanelsSection, "TestTreeWidth", MinPanelSize);

    std::optional<int> const height = ReadSize(ini, WindowSection, "Height", MinWindowHeight);
    std::optional<int> const left = ReadInt(ini, WindowSection, "Left");
    std::optional<int> const top = ReadInt(ini, WindowSection, "Top");
    std::optional<int> const width = ReadSize(ini, WindowSection, "Width", MinWindowWidth);

    // Only all four together place the window.
    if (height.has_value() && left.has_value() && top.has_value() && width.has_value())
    {
        TGUIWindowBounds window;
        window.Height = *height;
        window.Left = *left;
        window.Top = *top;
        window.Width = *width;
        layout.Window = window;
    }

    return layout;
}
//---------------------------------------------------------------------------
void SaveGUILayout(TGUIINIDocument& ini, TGUILayout const& layout)
{
    WriteOrDelete(ini, PanelsSection, "DetailHeight", layout.DetailHeight);
    WriteOrDelete(ini, PanelsSection, "TestTreeWidth", layout.TestTreeWidth);

    std::optional<int> height;
    std::optional<int> left;
    std::optional<int> top;
    std::optional<int> width;

    if (layout.Window.has_value())
    {
        height = layout.Window->Height;
        left = layout.Window->Left;
        top = layout.Window->Top;
        width = layout.Window->Width;
    }

    WriteOrDelete(ini, WindowSection, "Height", height);
    WriteOrDelete(ini, WindowSection, "Left", left);
    WriteOrDelete(ini, WindowSection, "Top", top);
    WriteOrDelete(ini, WindowSection, "Width", width);

    ini.WriteString(WindowSection, "Maximized", layout.Maximized ? "1" : "0");
}
//---------------------------------------------------------------------------
int ScaleFrom96(int value, int pixelsPerInch)
{
    return static_cast<int>(std::lround(static_cast<double>(value) * pixelsPerInch / 96.0));
}
//---------------------------------------------------------------------------
int ScaleTo96(int value, int pixelsPerInch)
{
    if (pixelsPerInch <= 0)
        return value;

    return static_cast<int>(std::lround(static_cast<double>(value) * 96.0 / pixelsPerInch));
}
//---------------------------------------------------------------------------
TGUIRect ScreenToWorkspace(TGUIRect const& rect, TGUIRect const& primaryMonitor, TGUIRect const& primaryWorkArea)
{
    int const dx = primaryWorkArea.Left - primaryMonitor.Left;
    int const dy = primaryWorkArea.Top - primaryMonitor.Top;

    return TGUIRect(rect.Left - dx, rect.Top - dy, rect.Right - dx, rect.Bottom - dy);
}
//---------------------------------------------------------------------------
TGUIRect WorkspaceToScreen(TGUIRect const& rect, TGUIRect const& primaryMonitor, TGUIRect const& primaryWorkArea)
{
    int const dx = primaryWorkArea.Left - primaryMonitor.Left;
    int const dy = primaryWorkArea.Top - primaryMonitor.Top;

    return TGUIRect(rect.Left + dx, rect.Top + dy, rect.Right + dx, rect.Bottom + dy);
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TGUIRect
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TGUIRect::TGUIRect()
{
}
//---------------------------------------------------------------------------
TGUIRect::TGUIRect(int left, int top, int right, int bottom)
    : Bottom(bottom),
      Left(left),
      Right(right),
      Top(top)
{
}
//---------------------------------------------------------------------------
int TGUIRect::Height() const
{
    return Bottom - Top;
}
//---------------------------------------------------------------------------
int TGUIRect::Width() const
{
    return Right - Left;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
