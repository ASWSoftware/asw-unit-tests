/* **************************************************************************
ASWUnitTests_GUI_VCLSystem.h
Author: Anthony S. West - ASW Software

What the VCL GUI runner needs from its process and system through the
VCL/RTL, kept out of the shared units in gui/src, which contain no VCL code.

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
#ifndef ASWUnitTests_GUI_VCLSystemH
#define ASWUnitTests_GUI_VCLSystemH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// Sets the VCL style for 'theme' (see GUIThemeIsDark()): 'darkStyle' (see LoadGUIDarkStyle()) when it's dark,
// otherwise the default unstyled look. Returns whether the active style changed. Call it on the main thread;
// before any form exists, each form is created in that style, and afterward it restyles the open ones.
bool ApplyGUITheme(EGUITheme theme, System::UnicodeString const& darkStyle);
// The names of the files (not folders) in the folder, without the folder, converted to UTF-8; empty if the
// folder doesn't exist or can't be listed.
std::vector<std::string> FileNamesInFolder(System::UnicodeString const& folderPath);
// This executable's layout file, %APPDATA%\ASWUnitTests\<exe name>.ini, so each test app built on the GUI
// keeps its own layout, shared by all of its build configurations.
System::UnicodeString GUILayoutFilePath();
// This executable's selection file, beside its layout file (see GUILayoutFilePath()): its name with a
// ".selection" extension.
System::UnicodeString GUISelectionFilePath();
// The folder that holds the theme files both GUI runners share, whatever executable runs them:
// %APPDATA%\ASWUnitTests\Themes. The VCL GUI runner's are VCL style files, "<name>.vsf". It may not exist yet.
System::UnicodeString GUIThemesFolderPath();
// The VCL style the dark theme 'darkTheme' is, for ApplyGUITheme(): Carbon, which the GUI project links in, for
// GUIDefaultDarkThemeName(); otherwise the style in "<darkTheme>.vsf" in 'themesFolder', e.g.
// GUIThemesFolderPath(), loaded the first time it's needed. Adds a line for the log to 'problems' (see
// GUIThemeProblemLine()), and gives Carbon, when the file is missing, isn't a VCL style, or has the name of a
// style already loaded from elsewhere.
System::UnicodeString LoadGUIDarkStyle(System::UnicodeString const& themesFolder, std::string const& darkTheme,
    std::vector<std::string>& problems);
// The dark theme SaveGUIDarkThemeToLayoutFile() saved in the layout file at 'layoutPath', e.g.
// GUILayoutFilePath(), or GUIDefaultDarkThemeName() if there's none or it can't be read.
std::string LoadGUIDarkThemeFromLayoutFile(System::UnicodeString const& layoutPath);
// The theme SaveGUIThemeToLayoutFile() saved in the layout file at 'layoutPath', e.g. GUILayoutFilePath(), or
// Windows if there's none or it can't be read.
EGUITheme LoadGUIThemeFromLayoutFile(System::UnicodeString const& layoutPath);
// Opens the folder in a File Explorer window. Throws an RTL exception if it can't, e.g. because the folder
// doesn't exist.
void OpenFolderInExplorer(System::UnicodeString const& folderPath);
// This process's command-line arguments, without the program name, converted to UTF-8, for
// TGUICommandLine::Parse().
std::vector<std::string> ProcessArguments();
// The file's bytes, unchanged, e.g. UTF-8 text for TGUIINIDocument. Throws an RTL exception if it can't be
// read.
std::string ReadFileBytes(System::UnicodeString const& path);
// Saves 'name', the dark theme's, in the layout file at 'layoutPath', e.g. GUILayoutFilePath(), keeping its other
// values, and creating it and its folder if needed. Ignores any error, which only means the next start uses the
// default dark theme.
void SaveGUIDarkThemeToLayoutFile(System::UnicodeString const& layoutPath, std::string const& name);
// Saves 'theme' in the layout file at 'layoutPath', e.g. GUILayoutFilePath(), keeping its other values, and
// creating it and its folder if needed. Ignores any error, which only means the next start uses the default theme.
void SaveGUIThemeToLayoutFile(System::UnicodeString const& layoutPath, EGUITheme theme);
// Whether Windows' app mode setting is dark; false if it can't be read, as on a Windows version without it.
bool WindowsAppsUseDarkTheme();
// Whether Windows' high contrast is on.
bool WindowsHighContrast();
// Replaces the file's content with 'bytes', unchanged, creating the file if needed (but not its folder).
// Throws an RTL exception if it can't be written.
void WriteFileBytes(System::UnicodeString const& path, std::string const& bytes);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_VCLSystemH
