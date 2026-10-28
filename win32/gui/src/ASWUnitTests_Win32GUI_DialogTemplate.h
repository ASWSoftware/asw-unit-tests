/* **************************************************************************
ASWUnitTests_Win32GUI_DialogTemplate.h
Author: Anthony S. West - ASW Software

Builds the in-memory dialog templates of the Win32 GUI runner's dialogs.

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
#ifndef ASWUnitTests_Win32GUI_DialogTemplateH
#define ASWUnitTests_Win32GUI_DialogTemplateH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EWin32GUIDialogControlClass
//
// The predefined window classes of a dialog template's controls, as the
// atoms a DLGITEMTEMPLATE names them by.
/////////////////////////////////////////////////////////////////////////////
enum class EWin32GUIDialogControlClass : WORD
{
    Button = 0x0080,
    Edit = 0x0081,
    Static = 0x0082,
};


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIDialogTemplate
//
// An in-memory DLGTEMPLATE for DialogBoxIndirectParamW(): a dialog with the
// GUI's font, Segoe UI 9 pt, holding controls of the predefined classes.
// Its positions and sizes are placeholders, since the dialogs lay
// themselves out in pixels for their DPI once they're created. Windows
// scales a dialog and its controls to the DPI of the monitor it's on.
// Contains no window, so it's unit tested from the Windows console runners
// (see win32/tests/Test_ASWUnitTests_Win32GUI_DialogTemplate.cpp).
/////////////////////////////////////////////////////////////////////////////
class TWin32GUIDialogTemplate
{
private:
    std::vector<WORD> m_Words;

private:
    void AppendDWORD(DWORD value);
    // Appends 'text' and its terminating null.
    void AppendString(std::wstring const& text);

public:
    // A dialog with 'style', to which DS_SETFONT is added, titled 'caption', with no controls yet.
    TWin32GUIDialogTemplate(DWORD style, std::wstring const& caption);

    // Adds a control of 'controlClass' with the control ID 'id', 'style', 'exStyle' and 'text', after the
    // controls already added, so also after them in the tab order.
    void AddControl(EWin32GUIDialogControlClass controlClass, WORD id, DWORD style, DWORD exStyle,
        std::wstring const& text);
    // The number of controls added.
    WORD ControlCount() const;
    // The template, for DialogBoxIndirectParamW(); valid until this object is changed or destroyed.
    LPCDLGTEMPLATEW Get() const;
    // The template's contents, a WORD at a time.
    std::vector<WORD> const& Words() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_DialogTemplateH
