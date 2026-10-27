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

namespace ASWUnitTests
{

// The text with every line break made CRLF, which an EDIT control needs: a lone LF or CR becomes CRLF, much as
// the VCL's AdjustLineBreaks() does for the VCL GUI runner.
std::string AdjustLineBreaks(std::string const& text);
// The arguments in a Windows command line such as GetCommandLineW()'s, without the program name, converted to
// UTF-8. Split by CommandLineToArgvW(), whose quoting rules are those of the C runtime's argv in the console
// runner.
std::vector<std::string> CommandLineArguments(std::wstring const& commandLine);
// Creates the folder, and any of its parent folders that don't exist yet; succeeds if it already exists. Throws
// std::runtime_error if it can't be created. 'folderPath' must be absolute.
void CreateFolderPath(std::string const& folderPath);
// This executable's file name, without its folder, e.g. "ASWUnitTests_Win32_GUI.exe".
std::string ExeFileName();
// Whether a file (not a folder) exists at the path.
bool FileExists(std::string const& path);
// This executable's layout file, %APPDATA%\ASWUnitTests\<exe name>.ini, so each test app built on the GUI
// keeps its own layout, shared by all of its build configurations.
std::string GUILayoutFilePath();
// This executable's selection file, beside its layout file (see GUILayoutFilePath()): its name with a
// ".selection" extension.
std::string GUISelectionFilePath();
// The folder that holds the GUI runners' layout and selection files, %APPDATA%\ASWUnitTests. It may not exist
// yet (see CreateFolderPath()).
std::string GUISettingsFolderPath();
// This process's command-line arguments, without the program name, converted to UTF-8, for
// TGUICommandLine::Parse().
std::vector<std::string> ProcessArguments();
// The file's bytes, unchanged, e.g. UTF-8 text for TGUIINIDocument. Throws std::runtime_error if it can't be
// read.
std::string ReadFileBytes(std::string const& path);
// Deletes the file. False if it couldn't be deleted, e.g. because it doesn't exist.
bool RemoveFile(std::string const& path);
// Replaces the clipboard's content with the text, for 'owner' (a window of this process). Throws
// std::runtime_error if it can't, e.g. while another program has the clipboard open.
void SetClipboardText(HWND owner, std::string const& text);
// Converts UTF-8 text to UTF-16 for Windows. Invalid UTF-8 becomes U+FFFD. The framework's WideToUTF8()
// (ASWUnitTests_Utils.h) converts back.
std::wstring UTF8ToWide(std::string const& text);
// Replaces the file's content with 'bytes', unchanged, creating the file if needed (but not its folder).
// Throws std::runtime_error if it can't be written.
void WriteFileBytes(std::string const& path, std::string const& bytes);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_SystemH
