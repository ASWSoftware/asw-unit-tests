/* **************************************************************************
ASWUnitTests_Win32GUI_DialogTemplate.cpp
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
#include "ASWUnitTests_Win32GUI_DialogTemplate.h"
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The index of DLGTEMPLATE's cdit, the number of controls, after its style and extended style.
std::size_t const ControlCountIndex = 4;

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIDialogTemplate
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWin32GUIDialogTemplate::TWin32GUIDialogTemplate(DWORD style, std::wstring const& caption)
{
    AppendDWORD(style | DS_SETFONT);
    AppendDWORD(0); // Extended style
    m_Words.push_back(0); // Number of controls, counted by AddControl().
    m_Words.insert(m_Words.end(), { 0, 0, 200, 100 }); // x, y, cx, cy
    m_Words.push_back(0); // No menu
    m_Words.push_back(0); // The predefined dialog class
    AppendString(caption);
    m_Words.push_back(9); // Point size
    AppendString(L"Segoe UI");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIDialogTemplate::AddControl

    Appends a DLGITEMTEMPLATE, DWORD-aligned as each must be.
*/
void TWin32GUIDialogTemplate::AddControl(EWin32GUIDialogControlClass controlClass, WORD id, DWORD style,
    DWORD exStyle, std::wstring const& text)
{
    if (m_Words.size() % 2 != 0)
        m_Words.push_back(0);

    AppendDWORD(style);
    AppendDWORD(exStyle);
    m_Words.insert(m_Words.end(), { 0, 0, 10, 10 }); // x, y, cx, cy
    m_Words.push_back(id);
    m_Words.push_back(0xFFFF); // A predefined class's atom follows.
    m_Words.push_back(static_cast<WORD>(controlClass));
    AppendString(text);
    m_Words.push_back(0); // No creation data

    ++m_Words[ControlCountIndex];
}
//---------------------------------------------------------------------------
void TWin32GUIDialogTemplate::AppendDWORD(DWORD value)
{
    m_Words.push_back(LOWORD(value));
    m_Words.push_back(HIWORD(value));
}
//---------------------------------------------------------------------------
void TWin32GUIDialogTemplate::AppendString(std::wstring const& text)
{
    for (wchar_t const character : text)
        m_Words.push_back(static_cast<WORD>(character));

    m_Words.push_back(0);
}
//---------------------------------------------------------------------------
WORD TWin32GUIDialogTemplate::ControlCount() const
{
    return m_Words[ControlCountIndex];
}
//---------------------------------------------------------------------------
LPCDLGTEMPLATEW TWin32GUIDialogTemplate::Get() const
{
    // A std::vector's storage is aligned for any fundamental type, so the template starts on a DWORD boundary,
    // as it must.
    return reinterpret_cast<LPCDLGTEMPLATEW>(m_Words.data());
}
//---------------------------------------------------------------------------
std::vector<WORD> const& TWin32GUIDialogTemplate::Words() const
{
    return m_Words;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
