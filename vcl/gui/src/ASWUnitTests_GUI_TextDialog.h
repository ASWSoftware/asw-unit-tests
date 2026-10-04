/* **************************************************************************
ASWUnitTests_GUI_TextDialog.h
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
#ifndef ASWUnitTests_GUI_TextDialogH
#define ASWUnitTests_GUI_TextDialogH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

// Shows 'text' (UTF-8) in a modal, resizable dialog with a read-only, scrollable, monospace text box, for
// console-style output such as --help, whose aligned columns a regular message box's proportional font would
// scramble. 'caption' is the dialog's title.
void ShowTextDialog(System::UnicodeString const& caption, std::string const& text);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_TextDialogH
