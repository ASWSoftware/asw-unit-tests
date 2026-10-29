/* **************************************************************************
ASWUnitTests_Win32GUI_MessageDialog.h
Author: Anthony S. West - ASW Software

The Win32 GUI runner's message box: an icon, a message and an OK button, as
MessageBoxW() shows them, but in the GUI's theme.

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
#ifndef ASWUnitTests_Win32GUI_MessageDialogH
#define ASWUnitTests_Win32GUI_MessageDialogH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Theme.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EWin32GUIMessageIcon
//
// The icon ShowMessageDialog() shows, and the sound it plays, as
// MessageBoxW()'s MB_ICONERROR and MB_ICONWARNING.
/////////////////////////////////////////////////////////////////////////////
enum class EWin32GUIMessageIcon
{
    Error,
    Warning,
};

//---------------------------------------------------------------------------

// Shows 'text' (UTF-8) in a modal dialog titled 'caption', owned by 'owner' (nullptr for none), with 'icon' and
// an OK button, in 'theme''s look, and waits until it's closed. As in a MessageBoxW(), its sound plays, the
// message wraps, and Ctrl+C copies the caption, the message and the button's text. Throws nothing: if the
// dialog can't be shown, a MessageBoxW() is.
void ShowMessageDialog(HWND owner, TWin32GUITheme& theme, std::string const& caption, std::string const& text,
    EWin32GUIMessageIcon icon);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_MessageDialogH
