/* **************************************************************************
ASWUnitTests_Win32GUI_TextDialog.cpp
Author: Anthony S. West - ASW Software

The Win32 GUI runner's text dialog.

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
#include "ASWUnitTests_Win32GUI_TextDialog.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <stdexcept>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DialogTemplate.h"
#include "ASWUnitTests_Win32GUI_DPI.h"
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The text box's control ID; the OK button's is IDOK.
WORD const TextBoxID = 100;
// Posted to the dialog when it moves to a monitor with another DPI, so its monospace font is recreated after
// Windows has rescaled the dialog and its controls.
UINT const ApplyDPIMessage = WM_APP + 1;

/////////////////////////////////////////////////////////////////////////////
// TTextDialogState
//
// What the dialog shows, and the monospace font it creates for its DPI.
/////////////////////////////////////////////////////////////////////////////
struct TTextDialogState
{
    std::wstring Text;
    TWin32GUIFont MonospaceFont;
};

//---------------------------------------------------------------------------

void ApplyDPI(HWND dialog);
TWin32GUIDialogTemplate BuildDialogTemplate(std::wstring const& caption);
INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR HandleDialogMessage(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
void LayOutDialog(HWND dialog);
void SizeAndCenterDialog(HWND dialog);

//---------------------------------------------------------------------------
/*
    ApplyDPI

    Sets the text box's monospace font for the dialog's DPI, then lays the dialog out again.
*/
void ApplyDPI(HWND dialog)
{
    TTextDialogState* const state = reinterpret_cast<TTextDialogState*>(GetWindowLongPtrW(dialog, DWLP_USER));

    state->MonospaceFont = CreateMonospaceFont(WindowDPI(dialog));
    SendMessageW(GetDlgItem(dialog, TextBoxID), WM_SETFONT, reinterpret_cast<WPARAM>(state->MonospaceFont.Get()),
        TRUE);
    LayOutDialog(dialog);
}
//---------------------------------------------------------------------------
/*
    BuildDialogTemplate

    A resizable window holding the OK button and the text box. SizeAndCenterDialog() and LayOutDialog() set
    their positions and sizes in pixels.
*/
TWin32GUIDialogTemplate BuildDialogTemplate(std::wstring const& caption)
{
    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_CLIPCHILDREN,
        caption);

    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDOK, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_DEFPUSHBUTTON, 0, L"OK");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Edit, TextBoxID, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE,
        std::wstring());

    return dialogTemplate;
}
//---------------------------------------------------------------------------
/*
    DialogProc

    No exception may leave a dialog procedure, since unwinding through Windows' own frames is undefined. One
    that would ends the dialog with -1, which ShowTextDialog() reports as a failure.
*/
INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    try
    {
        return HandleDialogMessage(dialog, message, wParam, lParam);
    }
    catch (...)
    {
        EndDialog(dialog, -1);
        return TRUE;
    }
}
//---------------------------------------------------------------------------
INT_PTR HandleDialogMessage(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtrW(dialog, DWLP_USER, lParam);
            TTextDialogState const* const state = reinterpret_cast<TTextDialogState const*>(lParam);

            // Without a limit, an EDIT control holds only 32K characters.
            HWND const textBox = GetDlgItem(dialog, TextBoxID);
            SendMessageW(textBox, EM_SETLIMITTEXT, 0, 0);
            SetWindowTextW(textBox, state->Text.c_str());

            // Shown before the main window exists (e.g. for --help), it has a taskbar button of its own.
            if (GetWindow(dialog, GW_OWNER) == nullptr)
                SendMessageW(dialog, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(MainIcon()));

            SizeAndCenterDialog(dialog);
            ApplyDPI(dialog);

            SetFocus(GetDlgItem(dialog, IDOK));
            return FALSE; // The focus is set.
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(dialog, LOWORD(wParam));
                return TRUE;
            }

            return FALSE;

        case WM_CTLCOLORSTATIC:
            // A read-only text box is drawn like an editable one, as the VCL GUI runner's is, rather than gray.
            if (reinterpret_cast<HWND>(lParam) == GetDlgItem(dialog, TextBoxID))
            {
                HDC const context = reinterpret_cast<HDC>(wParam);
                SetTextColor(context, GetSysColor(COLOR_WINDOWTEXT));
                SetBkColor(context, GetSysColor(COLOR_WINDOW));
                return reinterpret_cast<INT_PTR>(GetSysColorBrush(COLOR_WINDOW));
            }

            return FALSE;

        case WM_DPICHANGED:
            PostMessageW(dialog, ApplyDPIMessage, 0, 0);
            return FALSE; // Windows moves and rescales the dialog.

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* const info = reinterpret_cast<MINMAXINFO*>(lParam);
            UINT const dpi = WindowDPI(dialog);
            info->ptMinTrackSize.x = ScaleForDPI(320, dpi);
            info->ptMinTrackSize.y = ScaleForDPI(200, dpi);
            return TRUE;
        }

        case WM_SIZE:
            LayOutDialog(dialog);
            return TRUE;

        default:
            if (message == ApplyDPIMessage)
            {
                ApplyDPI(dialog);
                return TRUE;
            }

            return FALSE;
    }
}
//---------------------------------------------------------------------------
/*
    LayOutDialog

    As the VCL GUI runner's text dialog: the text box fills the window above a 44-pixel strip (at 96 DPI)
    that holds the OK button at its right.
*/
void LayOutDialog(HWND dialog)
{
    RECT client;
    GetClientRect(dialog, &client);

    UINT const dpi = WindowDPI(dialog);
    int const stripHeight = ScaleForDPI(44, dpi);
    int const textHeight = (std::max)(0, static_cast<int>(client.bottom) - stripHeight);

    MoveWindow(GetDlgItem(dialog, TextBoxID), 0, 0, client.right, textHeight, TRUE);
    MoveWindow(GetDlgItem(dialog, IDOK), client.right - ScaleForDPI(96, dpi), textHeight + ScaleForDPI(9, dpi),
        ScaleForDPI(84, dpi), ScaleForDPI(26, dpi), TRUE);
}
//---------------------------------------------------------------------------
/*
    SizeAndCenterDialog

    Gives the dialog the VCL GUI runner's text dialog's client size, 760 by 520 at 96 DPI, centered on the
    work area of its owner's monitor, or of the one it opened on if it has no owner.
*/
void SizeAndCenterDialog(HWND dialog)
{
    UINT const dpi = WindowDPI(dialog);
    RECT frame = { 0, 0, ScaleForDPI(760, dpi), ScaleForDPI(520, dpi) };
    AdjustWindowRectExForDpi(&frame, static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_STYLE)), FALSE,
        static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_EXSTYLE)), dpi);

    HWND const owner = GetWindow(dialog, GW_OWNER);
    MONITORINFO monitor = {};
    monitor.cbSize = sizeof(monitor);
    GetMonitorInfoW(MonitorFromWindow(owner != nullptr ? owner : dialog, MONITOR_DEFAULTTONEAREST), &monitor);

    RECT const& work = monitor.rcWork;
    int const width = (std::min)(static_cast<int>(frame.right - frame.left), static_cast<int>(work.right - work.left));
    int const height = (std::min)(static_cast<int>(frame.bottom - frame.top), static_cast<int>(work.bottom - work.top));

    SetWindowPos(dialog, nullptr, work.left + (work.right - work.left - width) / 2,
        work.top + (work.bottom - work.top - height) / 2, width, height, SWP_NOZORDER | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void ShowTextDialog(HWND owner, std::string const& caption, std::string const& text)
{
    TTextDialogState state;
    state.Text = UTF8ToWide(AdjustLineBreaks(text));

    TWin32GUIDialogTemplate const dialogTemplate = BuildDialogTemplate(UTF8ToWide(caption));

    INT_PTR const result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), dialogTemplate.Get(), owner,
        &DialogProc, reinterpret_cast<LPARAM>(&state));

    if (result == -1)
        throw std::runtime_error("Couldn't show the \"" + caption + "\" dialog");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
