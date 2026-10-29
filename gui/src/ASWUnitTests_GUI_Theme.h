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
#include "ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EGUITheme
//
// A GUI runner's theme, as chosen in the VCL GUI runner's View > Theme menu
// and saved in its layout file. Contains no UI code, so it's unit tested
// from the console runners (see gui/tests/Test_ASWUnitTests_GUI_Theme.cpp).
// The Win32 GUI runner has no themes yet.
/////////////////////////////////////////////////////////////////////////////
enum class EGUITheme
{
    Windows, // Follows Windows' setting for apps, light or dark; the default.
    Light,
    Dark
};

//---------------------------------------------------------------------------

// Whether 'theme' is dark: Light and Dark as chosen, and Windows as 'windowsAppsUseDark' (Windows' app mode
// setting) says, except that it's light while 'highContrast' is on, so the window shows high contrast's own
// colors.
bool GUIThemeIsDark(EGUITheme theme, bool windowsAppsUseDark, bool highContrast);
// Reads the theme SaveGUITheme() wrote to 'ini', its [View] section's Theme key, ignoring (ASCII) case.
// Windows when it's missing or isn't a theme's name.
EGUITheme LoadGUITheme(TGUIINIDocument const& ini);
// Writes 'theme' to 'ini', as "Windows", "Light" or "Dark". The file itself is written from the document's
// Text() afterward.
void SaveGUITheme(TGUIINIDocument& ini, EGUITheme theme);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_ThemeH
