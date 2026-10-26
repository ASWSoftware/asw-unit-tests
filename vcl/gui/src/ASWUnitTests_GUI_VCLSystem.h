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

namespace ASWUnitTests
{

// This process's command-line arguments, without the program name, converted to UTF-8, for
// TGUICommandLine::Parse().
std::vector<std::string> ProcessArguments();

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_VCLSystemH
