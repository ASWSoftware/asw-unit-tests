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

namespace ASWUnitTests
{

// This executable's layout file, %APPDATA%\ASWUnitTests\<exe name>.ini, so each test app built on the GUI
// keeps its own layout, shared by all of its build configurations.
System::UnicodeString GUILayoutFilePath();
// This executable's selection file, beside its layout file (see GUILayoutFilePath()): its name with a
// ".selection" extension.
System::UnicodeString GUISelectionFilePath();
// This process's command-line arguments, without the program name, converted to UTF-8, for
// TGUICommandLine::Parse().
std::vector<std::string> ProcessArguments();
// The file's bytes, unchanged, e.g. UTF-8 text for TGUIINIDocument. Throws an RTL exception if it can't be
// read.
std::string ReadFileBytes(System::UnicodeString const& path);
// Replaces the file's content with 'bytes', unchanged, creating the file if needed (but not its folder).
// Throws an RTL exception if it can't be written.
void WriteFileBytes(System::UnicodeString const& path, std::string const& bytes);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_VCLSystemH
