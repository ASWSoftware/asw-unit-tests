/* **************************************************************************
Test_ASWUnitTests_Win32GUI_DialogTemplate.cpp
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
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_Win32GUI_DialogTemplate.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DialogTemplate.h"
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

std::string ClassName(HWND window);
INT_PTR CALLBACK IgnoringDialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
std::string WordsText(std::vector<WORD> const& words, std::size_t first, std::size_t count);

//---------------------------------------------------------------------------
std::string ClassName(HWND window)
{
    wchar_t name[64] = {};
    GetClassNameW(window, name, 64);

    return WideToUTF8(name);
}
//---------------------------------------------------------------------------
/*
    IgnoringDialogProc

    A dialog procedure that leaves every message to the dialog's defaults.
*/
INT_PTR CALLBACK IgnoringDialogProc(HWND /*dialog*/, UINT /*message*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    return FALSE;
}
//---------------------------------------------------------------------------
/*
    WordsText

    'count' of the words from 'first' as "w1,w2,...", in hex, to compare them with a single CheckEquals(). Stops
    early at the end of 'words'.
*/
std::string WordsText(std::vector<WORD> const& words, std::size_t first, std::size_t count)
{
    static char const digits[] = "0123456789ABCDEF";
    std::string text;

    for (std::size_t i = first; i < first + count && i < words.size(); ++i)
    {
        if (!text.empty())
            text += ",";

        for (int shift = 12; shift >= 0; shift -= 4)
            text += digits[(words[i] >> shift) & 0xF];
    }

    return text;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_DialogTemplate
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_DialogTemplate::TTest_ASWUnitTests_Win32GUI_DialogTemplate()
    : inherited("ASWUnitTests_Win32GUI_DialogTemplate_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_AddControl_AlignsEachControlOnADWORD,
        "AddControl_AlignsEachControlOnADWORD");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_AddControl_CountsTheControls,
        "AddControl_CountsTheControls");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_Constructor_WritesTheHeaderWithTheGUIFont,
        "Constructor_WritesTheHeaderWithTheGUIFont");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_Get_IsATemplateWindowsCreatesTheDialogFrom,
        "Get_IsATemplateWindowsCreatesTheDialogFrom");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_DialogTemplate::~TTest_ASWUnitTests_Win32GUI_DialogTemplate()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_AddControl_AlignsEachControlOnADWORD()
{
    // Arrange: a three-letter caption leaves the header 25 words long, so the first control needs a padding word.
    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP, L"Abc");

    // Act
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, 7, WS_CHILD | WS_VISIBLE, WS_EX_CLIENTEDGE,
        L"OK");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, 8, WS_CHILD, 0, L"X");

    // Assert
    std::vector<WORD> const& words = dialogTemplate.Words();
    CheckEquals(std::string("0000"), WordsText(words, 25, 1), __func__, __LINE__, "a padding word after the header");
    CheckEquals(std::string("0000,5000,0200,0000,0000,0000,000A,000A,0007,FFFF,0080,004F,004B,0000,0000"),
        WordsText(words, 26, 15), __func__, __LINE__,
        "the button: style, extended style, placeholder bounds, ID, class, text, no creation data");
    CheckEquals(std::string("0000"), WordsText(words, 41, 1), __func__, __LINE__, "a padding word after the button");
    CheckEquals(std::string("0000,4000,0000,0000,0000,0000,000A,000A,0008,FFFF,0082,0058,0000,0000"),
        WordsText(words, 42, 14), __func__, __LINE__, "the static control, from the next DWORD");
    CheckEquals(56, static_cast<int>(words.size()), __func__, __LINE__, "nothing after it");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_AddControl_CountsTheControls()
{
    // Arrange
    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP, L"A");

    // Act, Assert
    CheckEquals(0, static_cast<int>(dialogTemplate.ControlCount()), __func__, __LINE__, "none at first");

    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDOK, WS_CHILD, 0, L"OK");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Edit, 100, WS_CHILD, 0, std::wstring());

    CheckEquals(2, static_cast<int>(dialogTemplate.ControlCount()), __func__, __LINE__, "two added");
    CheckEquals(std::string("0002"), WordsText(dialogTemplate.Words(), 4, 1), __func__, __LINE__,
        "in the header's cdit");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_Constructor_WritesTheHeaderWithTheGUIFont()
{
    // Act
    TWin32GUIDialogTemplate const dialogTemplate(WS_POPUP | WS_CAPTION, L"Ab");

    // Assert: WS_POPUP | WS_CAPTION | DS_SETFONT is 0x80C00040.
    CheckEquals(std::string("0040,80C0,0000,0000,0000,0000,0000,00C8,0064,0000,0000,0041,0062,0000,"
        "0009,0053,0065,0067,006F,0065,0020,0055,0049,0000"), WordsText(dialogTemplate.Words(), 0, 100), __func__,
        __LINE__, "style with DS_SETFONT, no extended style or controls, placeholder bounds, no menu, the default "
        "class, the caption, then Segoe UI 9 pt");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DialogTemplate::Test_Get_IsATemplateWindowsCreatesTheDialogFrom()
{
    // Arrange: a hidden dialog with a caption outside ASCII (U+00E9) and one control of each class, the last
    // after a padding word.
    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP | WS_CAPTION, L"Caf\x00E9");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDOK, WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        0, L"OK!");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Edit, 101, WS_CHILD | WS_VISIBLE | ES_NUMBER,
        WS_EX_CLIENTEDGE, L"42");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, 102, WS_CHILD | WS_VISIBLE, 0, L"\x4E2D");

    // Act
    HWND const dialog = CreateDialogIndirectParamW(GetModuleHandleW(nullptr), dialogTemplate.Get(), nullptr,
        &IgnoringDialogProc, 0);

    std::string const caption = WindowText(dialog);
    HWND const button = GetDlgItem(dialog, IDOK);
    HWND const edit = GetDlgItem(dialog, 101);
    HWND const label = GetDlgItem(dialog, 102);
    std::string const controls = ClassName(button) + " " + WindowText(button) + ", " + ClassName(edit) + " " +
        WindowText(edit) + ", " + ClassName(label) + " " + WindowText(label);
    bool const editStyled = (GetWindowLongPtrW(edit, GWL_STYLE) & ES_NUMBER) != 0 &&
        (GetWindowLongPtrW(edit, GWL_EXSTYLE) & WS_EX_CLIENTEDGE) != 0;

    LOGFONTW font = {};
    HFONT const fontHandle = reinterpret_cast<HFONT>(SendMessageW(dialog, WM_GETFONT, 0, 0));

    if (fontHandle != nullptr)
        GetObjectW(fontHandle, sizeof(font), &font);

    if (dialog != nullptr)
        DestroyWindow(dialog);

    // Assert
    CheckTrue(dialog != nullptr, __func__, __LINE__, "Windows created the dialog");
    CheckEquals(std::string("Caf\xC3\xA9"), caption, __func__, __LINE__, "its caption");
    CheckEquals(std::string("Button OK!, Edit 42, Static \xE4\xB8\xAD"), controls, __func__, __LINE__,
        "each control's class and text");
    CheckTrue(editStyled, __func__, __LINE__, "the edit's style and extended style");
    CheckEquals(std::wstring(L"Segoe UI"), std::wstring(font.lfFaceName), __func__, __LINE__, "the dialog's font");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_DialogTemplate)
