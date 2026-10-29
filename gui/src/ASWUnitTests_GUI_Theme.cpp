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
#include <optional>
#include <string>
//---------------------------------------------------------------------------

namespace
{

char const ThemeKey[] = "Theme";
char const ViewSection[] = "View";

//---------------------------------------------------------------------------

bool SameText(std::string const& a, std::string const& b);
char ToLowerASCII(char c);

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
