/* **************************************************************************
ASWUnitTests_GUI_Theme.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_GUI_ThemeH
#define ASWUnitTests_GUI_ThemeH
//---------------------------------------------------------------------------
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EGUITheme
//
// A GUI runner's theme, as chosen in its View > Theme menu and saved in its
// layout file. Contains no UI code, so it's unit tested from the console
// runners (see gui/tests/Test_ASWUnitTests_GUI_Theme.cpp).
/////////////////////////////////////////////////////////////////////////////
enum class EGUITheme
{
    Windows, // Follows Windows' setting for apps, light or dark; the default.
    Light,
    Dark
};


/////////////////////////////////////////////////////////////////////////////
// TGUIThemeColor
//
// One color a theme file sets: the role it colors, as the GUI runner names
// it, and the color, as 0xRRGGBB.
/////////////////////////////////////////////////////////////////////////////
struct TGUIThemeColor
{
    std::uint32_t RGB = 0;
    std::string Role;
};


/////////////////////////////////////////////////////////////////////////////
// TGUIThemeFile
//
// A theme file as ReadGUIThemeFile() read it. A theme file is an INI file
// in the GUI runners' Themes folder, named for its theme, that sets the
// dark look's colors (the Win32 GUI runner's; the VCL GUI runner's dark
// themes are VCL style files instead):
//   [Theme]
//   Base=Dark
//   [Colors]
//   Window=#202020
// Base=Dark is required, so that a later version can add light themes. Each
// [Colors] key is a role's name, ignoring (ASCII) case, and its value a
// color as #RRGGBB, in either case; a role left out keeps the built-in
// dark theme's color.
/////////////////////////////////////////////////////////////////////////////
struct TGUIThemeFile
{
    // The colors the file sets, in its order, each role once, named as the GUI runner names it.
    std::vector<TGUIThemeColor> Colors;
    // What's wrong with the file, one sentence each, for the log (see GUIThemeProblemLine()): an unknown
    // section, setting or role, a color that isn't one, a role set twice, or a missing or unknown Base.
    std::vector<std::string> Problems;
    // Whether the file can be used, which it can't without Base=Dark; Colors is then empty.
    bool Usable = false;
};

//---------------------------------------------------------------------------

// 'rgb', 0xRRGGBB, as a theme file writes a color: "#RRGGBB", in uppercase.
std::string FormatGUIThemeColor(std::uint32_t rgb);
// The built-in dark theme's name, "Carbon": the default for LoadGUIDarkTheme(), and a name GUIThemeNames()
// leaves out, so a theme file can't take it.
std::string GUIDefaultDarkThemeName();
// Whether 'theme' is dark: Light and Dark as chosen, and Windows as 'windowsAppsUseDark' (Windows' app mode
// setting) says, except that it's light while 'highContrast' is on, so the window shows high contrast's own
// colors.
bool GUIThemeIsDark(EGUITheme theme, bool windowsAppsUseDark, bool highContrast);
// The names of the themes among 'fileNames', the files in the Themes folder: each one that ends in
// 'extension' (e.g. ".ini"), ignoring (ASCII) case, without it, sorted ignoring case, and each name once.
// Leaves out GUIDefaultDarkThemeName(), ignoring case, and a file named just 'extension'.
std::vector<std::string> GUIThemeNames(std::vector<std::string> const& fileNames, std::string const& extension);
// A line for the log about one of the dark theme 'themeName''s problems (e.g. one of TGUIThemeFile's), as both
// GUI runners word it: Dark theme "<name>": <problem>
std::string GUIThemeProblemLine(std::string const& themeName, std::string const& problem);
// Reads the dark theme SaveGUIDarkTheme() wrote to 'ini', its [View] section's DarkTheme key: the name of a
// theme file, or GUIDefaultDarkThemeName() when it's missing or empty, or is that name in another case.
std::string LoadGUIDarkTheme(TGUIINIDocument const& ini);
// Reads the theme SaveGUITheme() wrote to 'ini', its [View] section's Theme key, ignoring (ASCII) case.
// Windows when it's missing or isn't a theme's name.
EGUITheme LoadGUITheme(TGUIINIDocument const& ini);
// The color 'text' is, as a theme file writes it (see TGUIThemeFile), as 0xRRGGBB; nullopt if it isn't one.
std::optional<std::uint32_t> ParseGUIThemeColor(std::string const& text);
// Reads a theme file's 'text' (see TGUIThemeFile), UTF-8, whose roles are 'roles', the names the GUI runner
// gives them.
TGUIThemeFile ReadGUIThemeFile(std::string const& text, std::vector<std::string> const& roles);
// Writes 'name', a dark theme's (see LoadGUIDarkTheme()), to 'ini'. The file itself is written from the
// document's Text() afterward.
void SaveGUIDarkTheme(TGUIINIDocument& ini, std::string const& name);
// Writes 'theme' to 'ini', as "Windows", "Light" or "Dark". The file itself is written from the document's
// Text() afterward.
void SaveGUITheme(TGUIINIDocument& ini, EGUITheme theme);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_ThemeH
