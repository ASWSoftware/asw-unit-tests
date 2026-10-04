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
#include <cmath>
//---------------------------------------------------------------------------
#include <System.IOUtils.hpp>
#include <System.SysUtils.hpp>
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

std::optional<int> ReadInt(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident);
std::optional<int> ReadSize(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident, int minimum);
void WriteOrDelete(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident,
    std::optional<int> const& value);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// The value, or empty if it's missing or isn't a whole number.
std::optional<int> ReadInt(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident)
{
    int value = 0;

    if (!System::Sysutils::TryStrToInt(ini.ReadString(section, ident, System::UnicodeString()), value))
        return std::nullopt;

    return value;
}
//---------------------------------------------------------------------------
// The value, or empty if it's missing, isn't a whole number, or is less than 'minimum'.
std::optional<int> ReadSize(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident, int minimum)
{
    std::optional<int> const value = ReadInt(ini, section, ident);

    if (!value.has_value() || *value < minimum)
        return std::nullopt;

    return value;
}
//---------------------------------------------------------------------------
void WriteOrDelete(System::Inifiles::TCustomIniFile& ini, char const* section, char const* ident,
    std::optional<int> const& value)
{
    if (value.has_value())
        ini.WriteInteger(section, ident, *value);
    else
        ini.DeleteKey(section, ident);
}
//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

//---------------------------------------------------------------------------
System::UnicodeString GUILayoutFilePath()
{
    // GetHomePath() is %APPDATA% on Windows.
    System::UnicodeString const folder = System::Ioutils::TPath::Combine(System::Ioutils::TPath::GetHomePath(),
        "ASWUnitTests");
    System::UnicodeString const fileName = System::Sysutils::ChangeFileExt(
        System::Sysutils::ExtractFileName(System::ParamStr(0)), ".ini");

    return System::Ioutils::TPath::Combine(folder, fileName);
}
//---------------------------------------------------------------------------
bool IsWindowReachable(System::Types::TRect const& bounds, std::vector<System::Types::TRect> const& workAreas)
{
    if (bounds.Width() <= 0 || bounds.Height() <= 0)
        return false;

    int const titleBarBottom = bounds.Top + std::min(bounds.Height(), TitleBarHeight);
    int const minWidth = std::min(bounds.Width(), MinGrabWidth);

    for (System::Types::TRect const& workArea : workAreas)
    {
        int const overlapWidth = std::min(bounds.Right, workArea.Right) - std::max(bounds.Left, workArea.Left);
        bool const titleBarInside = bounds.Top >= workArea.Top && titleBarBottom <= workArea.Bottom;

        if (titleBarInside && overlapWidth >= minWidth)
            return true;
    }

    return false;
}
//---------------------------------------------------------------------------
TGUILayout LoadGUILayout(System::Inifiles::TCustomIniFile& ini)
{
    TGUILayout layout;

    layout.DetailHeight = ReadSize(ini, PanelsSection, "DetailHeight", MinPanelSize);
    layout.Maximized = ini.ReadBool(WindowSection, "Maximized", false);
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
void SaveGUILayout(System::Inifiles::TCustomIniFile& ini, TGUILayout const& layout)
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

    ini.WriteBool(WindowSection, "Maximized", layout.Maximized);
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
System::Types::TRect ScreenToWorkspace(System::Types::TRect const& rect, System::Types::TRect const& primaryMonitor,
    System::Types::TRect const& primaryWorkArea)
{
    int const dx = primaryWorkArea.Left - primaryMonitor.Left;
    int const dy = primaryWorkArea.Top - primaryMonitor.Top;

    return System::Types::TRect(rect.Left - dx, rect.Top - dy, rect.Right - dx, rect.Bottom - dy);
}
//---------------------------------------------------------------------------
System::Types::TRect WorkspaceToScreen(System::Types::TRect const& rect, System::Types::TRect const& primaryMonitor,
    System::Types::TRect const& primaryWorkArea)
{
    int const dx = primaryWorkArea.Left - primaryMonitor.Left;
    int const dy = primaryWorkArea.Top - primaryMonitor.Top;

    return System::Types::TRect(rect.Left + dx, rect.Top + dy, rect.Right + dx, rect.Bottom + dy);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
