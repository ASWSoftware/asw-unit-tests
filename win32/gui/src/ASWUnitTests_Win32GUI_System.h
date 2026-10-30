/* **************************************************************************
ASWUnitTests_Win32GUI_System.h
Author: Anthony S. West - ASW Software

What the Win32 GUI runner needs from its process and system: text
conversion between the framework's UTF-8 and Windows' UTF-16, its
command-line arguments, where it keeps its settings, file access through
wide paths, and the clipboard. The Win32 counterpart of the VCL GUI runner's
ASWUnitTests_GUI_VCLSystem, with the same function names, so the two GUIs
can be compared side by side.

Text and paths are UTF-8 std::string throughout, converted to UTF-16 only
where they reach Windows, so a path outside the ANSI code page works.

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
#ifndef ASWUnitTests_Win32GUI_SystemH
#define ASWUnitTests_Win32GUI_SystemH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// The text with every line break made CRLF, which an EDIT control needs: a lone LF or CR becomes CRLF, much as
// the VCL's AdjustLineBreaks() does for the VCL GUI runner.
std::string AdjustLineBreaks(std::string const& text);
// Creates the folder, and any of its parent folders that don't exist yet; succeeds if it already exists. Throws
// std::runtime_error if it can't be created. 'folderPath' must be absolute.
void CreateFolderPath(std::string const& folderPath);
// This executable's file name, without its folder, e.g. "ASWUnitTests_Win32_GUI.exe".
std::string ExeFileName();
// The path made absolute, relative to the current folder, with any "." and ".." folders resolved, much as the
// VCL's ExpandFileName() does for the VCL GUI runner. The file needn't exist. Throws std::runtime_error if the
// path isn't valid.
std::string ExpandFileName(std::string const& path);
// Whether a file (not a folder) exists at the path.
bool FileExists(std::string const& path);
// The names of the files (not folders) in the folder, without the folder, in the order Windows lists them;
// empty if the folder doesn't exist or can't be listed.
std::vector<std::string> FileNamesInFolder(std::string const& folderPath);
// This executable's layout file, %APPDATA%\ASWUnitTests\<exe name>.ini, so each test app built on the GUI
// keeps its own layout, shared by all of its build configurations.
std::string GUILayoutFilePath();
// This executable's selection file, beside its layout file (see GUILayoutFilePath()): its name with a
// ".selection" extension.
std::string GUISelectionFilePath();
// The folder that holds the GUI runners' layout and selection files, %APPDATA%\ASWUnitTests. It may not exist
// yet (see CreateFolderPath()).
std::string GUISettingsFolderPath();
// The folder that holds the theme files both GUI runners share (see TGUIThemeFile), whatever executable runs
// them: %APPDATA%\ASWUnitTests\Themes. It may not exist yet (see CreateFolderPath()).
std::string GUIThemesFolderPath();
// Whether a WM_SETTINGCHANGE message's parameters are about a setting that Use Windows Setting follows: the
// app mode (Windows names its change "ImmersiveColorSet") or high contrast.
bool IsThemeSettingChange(WPARAM wParam, LPARAM lParam);
// The dark theme saved in the layout file at 'layoutPath', e.g. GUILayoutFilePath()'s (see LoadGUIDarkTheme());
// GUIDefaultDarkThemeName() when there's no file or it can't be read.
std::string LoadGUIDarkThemeFromLayoutFile(std::string const& layoutPath);
// The theme saved in the layout file at 'layoutPath', e.g. GUILayoutFilePath()'s (see LoadGUITheme()); Windows
// when there's no file or it can't be read.
EGUITheme LoadGUIThemeFromLayoutFile(std::string const& layoutPath);
// This executable's icon, its MAINICON resource (resource/ASWUnitTests_Icon.rc, or Icon_MainIcon in a RAD Studio
// project), at the standard icon size; nullptr if it has none. Shared, so it's never destroyed.
HICON MainIcon();
// Opens the folder in a File Explorer window, for 'owner' (a window of this process, or nullptr). Throws
// std::runtime_error if it can't, e.g. because the folder doesn't exist.
void OpenFolderInExplorer(HWND owner, std::string const& folderPath);
// This process's command-line arguments, without the program name, converted to UTF-8, for
// TGUICommandLine::Parse().
std::vector<std::string> ProcessArguments();
// The file's bytes, unchanged, e.g. UTF-8 text for TGUIINIDocument. Throws std::runtime_error if it can't be
// read.
std::string ReadFileBytes(std::string const& path);
// Deletes the file. False if it couldn't be deleted, e.g. because it doesn't exist.
bool RemoveFile(std::string const& path);
// Saves 'name', the dark theme's, in the layout file at 'layoutPath', e.g. GUILayoutFilePath() (see
// SaveGUIDarkTheme()), keeping the layout and any other values in it, and creating the file and its folder if
// needed. Not saving it (e.g. a read-only profile) only means the next start uses the default.
void SaveGUIDarkThemeToLayoutFile(std::string const& layoutPath, std::string const& name);
// Saves 'theme' in the layout file at 'layoutPath', e.g. GUILayoutFilePath() (see SaveGUITheme()), keeping the
// layout and any other values in it, and creating the file and its folder if needed. Not saving it (e.g. a
// read-only profile) only means the next start uses the default.
void SaveGUIThemeToLayoutFile(std::string const& layoutPath, EGUITheme theme);
// Replaces the clipboard's content with the text, for 'owner' (a window of this process). Throws
// std::runtime_error if it can't, e.g. while another program has the clipboard open.
void SetClipboardText(HWND owner, std::string const& text);
// The window's text, e.g. a control's, converted to UTF-8.
std::string WindowText(HWND window);
// Whether Windows' setting for apps is dark: AppsUseLightTheme is 0 under
// HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize. False if it can't be read.
bool WindowsAppsUseDarkTheme();
// Whether a high contrast theme is on.
bool WindowsHighContrast();
// Replaces the file's content with 'bytes', unchanged, creating the file if needed (but not its folder).
// Throws std::runtime_error if it can't be written.
void WriteFileBytes(std::string const& path, std::string const& bytes);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_SystemH
