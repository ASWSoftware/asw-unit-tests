/* **************************************************************************
ASWUnitTests_GUI_Theme.cpp
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
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

char const BaseKey[] = "Base";
char const ColorsSection[] = "Colors";
char const DarkThemeKey[] = "DarkTheme";
char const ThemeKey[] = "Theme";
char const ThemeSection[] = "Theme";
char const ViewSection[] = "View";

//---------------------------------------------------------------------------

int HexDigitValue(char c);
bool LessText(std::string const& a, std::string const& b);
bool SameText(std::string const& a, std::string const& b);
char ToLowerASCII(char c);

//---------------------------------------------------------------------------
/*
    HexDigitValue

    The value of 'c' as a hexadecimal digit, in either case; -1 if it isn't one.
*/
int HexDigitValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return -1;
}
//---------------------------------------------------------------------------
/*
    LessText

    Whether 'a' sorts before 'b', ignoring ASCII case.
*/
bool LessText(std::string const& a, std::string const& b)
{
    return std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end(), [](char x, char y)
        {
            return ToLowerASCII(x) < ToLowerASCII(y);
        });
}
//---------------------------------------------------------------------------
/*
    SameText

    Whether 'a' and 'b' are the same, ignoring ASCII case, as the INI document matches section and key names.
*/
bool SameText(std::string const& a, std::string const& b)
{
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y)
        {
            return ToLowerASCII(x) == ToLowerASCII(y);
        });
}
//---------------------------------------------------------------------------
char ToLowerASCII(char c)
{
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::string FormatGUIThemeColor(std::uint32_t rgb)
{
    char const digits[] = "0123456789ABCDEF";
    std::string text = "#";

    for (int shift = 20; shift >= 0; shift -= 4)
        text += digits[(rgb >> shift) & 0xF];

    return text;
}
//---------------------------------------------------------------------------
std::string GUIDefaultDarkThemeName()
{
    return "Carbon";
}
//---------------------------------------------------------------------------
bool GUIThemeIsDark(EGUITheme theme, bool windowsAppsUseDark, bool highContrast)
{
    switch (theme)
    {
        case EGUITheme::Dark:
            return true;

        case EGUITheme::Light:
            return false;

        case EGUITheme::Windows:
            break;
    }

    return windowsAppsUseDark && !highContrast;
}
//---------------------------------------------------------------------------
std::vector<std::string> GUIThemeNames(std::vector<std::string> const& fileNames, std::string const& extension,
    std::vector<std::string> const& builtInNames)
{
    std::vector<std::string> names;

    for (std::string const& fileName : fileNames)
    {
        if (fileName.size() <= extension.size() ||
            !SameText(fileName.substr(fileName.size() - extension.size()), extension))
        {
            continue;
        }

        std::string const name = fileName.substr(0, fileName.size() - extension.size());
        auto const sameName = [&name](std::string const& other)
            {
                return SameText(other, name);
            };

        if (std::none_of(names.begin(), names.end(), sameName) &&
            std::none_of(builtInNames.begin(), builtInNames.end(), sameName))
        {
            names.push_back(name);
        }
    }

    std::stable_sort(names.begin(), names.end(), &LessText);
    return names;
}
//---------------------------------------------------------------------------
std::string GUIThemeProblemLine(std::string const& themeName, std::string const& problem)
{
    return "Dark theme \"" + themeName + "\": " + problem;
}
//---------------------------------------------------------------------------
std::vector<std::string> GUIVCLStyleDarkThemeNames()
{
    return { "Charcoal Dark Slate", "Cobalt XEMedia", "Onyx Blue", "Tablet Dark" };
}
//---------------------------------------------------------------------------
std::string LoadGUIDarkTheme(TGUIINIDocument const& ini)
{
    std::optional<std::string> const name = ini.ReadString(ViewSection, DarkThemeKey);

    if (!name.has_value() || name->empty() || SameText(*name, GUIDefaultDarkThemeName()))
        return GUIDefaultDarkThemeName();

    return *name;
}
//---------------------------------------------------------------------------
EGUITheme LoadGUITheme(TGUIINIDocument const& ini)
{
    std::optional<std::string> const name = ini.ReadString(ViewSection, ThemeKey);

    if (name.has_value())
    {
        if (SameText(*name, "Dark"))
            return EGUITheme::Dark;

        if (SameText(*name, "Light"))
            return EGUITheme::Light;
    }

    return EGUITheme::Windows;
}
//---------------------------------------------------------------------------
std::optional<std::uint32_t> ParseGUIThemeColor(std::string const& text)
{
    if (text.size() != 7 || text[0] != '#')
        return std::nullopt;

    std::uint32_t rgb = 0;

    for (std::size_t i = 1; i < text.size(); ++i)
    {
        int const digit = HexDigitValue(text[i]);

        if (digit < 0)
            return std::nullopt;

        rgb = (rgb << 4) | static_cast<std::uint32_t>(digit);
    }

    return rgb;
}
//---------------------------------------------------------------------------
/*
    ReadGUIThemeFile

    Every problem is reported, even in a file that can't be used, so one look at the log shows them all.
*/
TGUIThemeFile ReadGUIThemeFile(std::string const& text, std::vector<std::string> const& roles)
{
    TGUIINIDocument const ini(text);
    TGUIThemeFile file;

    for (std::string const& section : ini.SectionNames())
    {
        if (!SameText(section, ThemeSection) && !SameText(section, ColorsSection))
            file.Problems.push_back("[" + section + "] isn't a section of a theme file, so it's ignored.");
    }

    for (std::string const& key : ini.KeyNames(ThemeSection))
    {
        if (!SameText(key, BaseKey))
            file.Problems.push_back("[Theme] " + key + " isn't a setting of a theme file, so it's ignored.");
    }

    std::optional<std::string> const base = ini.ReadString(ThemeSection, BaseKey);

    if (!base.has_value())
    {
        file.Problems.push_back("[Theme] has no Base=Dark, which a theme file needs, so " + GUIDefaultDarkThemeName() +
            " is used instead.");
    }
    else if (!SameText(*base, "Dark"))
    {
        file.Problems.push_back("[Theme] Base=" + *base + " isn't supported, only Base=Dark, so " +
            GUIDefaultDarkThemeName() + " is used instead.");
    }

    // The roles met so far, as 'roles' names them, so a repeat is reported, even of an invalid color.
    std::vector<std::string> met;

    for (std::string const& key : ini.KeyNames(ColorsSection))
    {
        std::vector<std::string>::const_iterator const role = std::find_if(roles.begin(), roles.end(),
            [&key](std::string const& candidate)
            {
                return SameText(candidate, key);
            });

        if (role == roles.end())
        {
            file.Problems.push_back("[Colors] " + key + " isn't a role a theme can color, so it's ignored.");
            continue;
        }

        if (std::find(met.begin(), met.end(), *role) != met.end())
        {
            file.Problems.push_back("[Colors] " + key + " is set more than once, so only the first is used.");
            continue;
        }

        met.push_back(*role);

        // A repeated key reads as its first value, as TMemIniFile reads it.
        std::string const value = ini.ReadString(ColorsSection, key).value_or(std::string());
        std::optional<std::uint32_t> const rgb = ParseGUIThemeColor(value);

        if (!rgb.has_value())
        {
            file.Problems.push_back("[Colors] " + key + "=" + value + " isn't a color written as #RRGGBB, so it's "
                "ignored.");
            continue;
        }

        TGUIThemeColor color;
        color.RGB = *rgb;
        color.Role = *role;
        file.Colors.push_back(color);
    }

    file.Usable = base.has_value() && SameText(*base, "Dark");

    if (!file.Usable)
        file.Colors.clear();

    return file;
}
//---------------------------------------------------------------------------
void SaveGUIDarkTheme(TGUIINIDocument& ini, std::string const& name)
{
    ini.WriteString(ViewSection, DarkThemeKey, name);
}
//---------------------------------------------------------------------------
void SaveGUITheme(TGUIINIDocument& ini, EGUITheme theme)
{
    char const* name = "Windows";

    if (theme == EGUITheme::Dark)
        name = "Dark";
    else if (theme == EGUITheme::Light)
        name = "Light";

    ini.WriteString(ViewSection, ThemeKey, name);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
