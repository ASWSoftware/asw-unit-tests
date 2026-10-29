/* **************************************************************************
ASWUnitTests_Win32GUI_MessageDialog.cpp
Author: Anthony S. West - ASW Software

The Win32 GUI runner's message box.

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
#include "ASWUnitTests_Win32GUI_MessageDialog.h"
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

// Posted to the dialog when it moves to a monitor with another DPI, so its icon is reloaded and it's laid out
// again after Windows has rescaled it and its controls' fonts.
UINT const ApplyDPIMessage = WM_APP + 1;
// The controls' IDs; the OK button's is IDOK.
WORD const IconID = 100;
WORD const MessageID = 101;
// The OK button's subclass, for Ctrl+C.
UINT_PTR const OKButtonSubclassID = 1;

/////////////////////////////////////////////////////////////////////////////
// TMessageDialogState
//
// What the dialog shows, and the icon it loads for its DPI.
/////////////////////////////////////////////////////////////////////////////
struct TMessageDialogState
{
    std::wstring Caption;
    HICON Icon = nullptr;
    EWin32GUIMessageIcon IconKind = EWin32GUIMessageIcon::Error;
    std::wstring Text;
    TWin32GUITheme* Theme = nullptr;
};

//---------------------------------------------------------------------------

void ApplyDPI(HWND dialog);
TWin32GUIDialogTemplate BuildDialogTemplate(TMessageDialogState const& state);
void CenterOnOwner(HWND dialog);
INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR HandleDialogMessage(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
void LayOutDialog(HWND dialog);
LRESULT CALLBACK OKButtonProc(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id,
    DWORD_PTR state);
UINT SoundOf(EWin32GUIMessageIcon icon);

//---------------------------------------------------------------------------
/*
    ApplyDPI

    Loads the icon at the size Windows' icons have at the dialog's DPI, scaled down from a larger one if
    needed, then lays the dialog out again.
*/
void ApplyDPI(HWND dialog)
{
    TMessageDialogState* const state = reinterpret_cast<TMessageDialogState*>(GetWindowLongPtrW(dialog, DWLP_USER));
    UINT const dpi = WindowDPI(dialog);
    int const size = GetSystemMetricsForDpi(SM_CXICON, dpi);

    // IDI_ERROR and IDI_WARNING; the W versions of those macros need UNICODE.
    HICON icon = nullptr;
    LoadIconWithScaleDown(nullptr, MAKEINTRESOURCEW(state->IconKind == EWin32GUIMessageIcon::Error ? 32513 : 32515),
        size, size, &icon);

    if (icon != nullptr)
    {
        SendDlgItemMessageW(dialog, IconID, STM_SETICON, reinterpret_cast<WPARAM>(icon), 0);

        if (state->Icon != nullptr)
            DestroyIcon(state->Icon);

        state->Icon = icon;
    }

    LayOutDialog(dialog);
}
//---------------------------------------------------------------------------
/*
    BuildDialogTemplate

    LayOutDialog() sets the positions and sizes.
*/
TWin32GUIDialogTemplate BuildDialogTemplate(TMessageDialogState const& state)
{
    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME, state.Caption);

    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, IconID, WS_CHILD | WS_VISIBLE | SS_ICON |
        SS_REALSIZECONTROL, 0, std::wstring());
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, MessageID, WS_CHILD | WS_VISIBLE | SS_LEFT |
        SS_NOPREFIX, 0, state.Text);
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDOK, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_DEFPUSHBUTTON, 0, L"OK");

    return dialogTemplate;
}
//---------------------------------------------------------------------------
/*
    CenterOnOwner

    As MessageBoxW() places itself: centered on its owner, or on the work area of its monitor if it has none,
    kept within that work area.
*/
void CenterOnOwner(HWND dialog)
{
    HWND const owner = GetWindow(dialog, GW_OWNER);
    MONITORINFO monitor = {};
    monitor.cbSize = sizeof(monitor);
    GetMonitorInfoW(MonitorFromWindow(owner != nullptr ? owner : dialog, MONITOR_DEFAULTTONEAREST), &monitor);

    RECT const& work = monitor.rcWork;
    RECT around = work;

    if (owner != nullptr)
        GetWindowRect(owner, &around);

    RECT bounds;
    GetWindowRect(dialog, &bounds);
    int const width = bounds.right - bounds.left;
    int const height = bounds.bottom - bounds.top;

    int const left = (std::max)(static_cast<int>(work.left), (std::min)(static_cast<int>(work.right) - width,
        static_cast<int>(around.left + (around.right - around.left - width) / 2)));
    int const top = (std::max)(static_cast<int>(work.top), (std::min)(static_cast<int>(work.bottom) - height,
        static_cast<int>(around.top + (around.bottom - around.top - height) / 2)));

    SetWindowPos(dialog, nullptr, left, top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------
/*
    DialogProc

    No exception may leave a dialog procedure, since unwinding through Windows' own frames is undefined. One
    that would ends the dialog with -1, which ShowMessageDialog() answers with a MessageBoxW().
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
            TMessageDialogState* const state = reinterpret_cast<TMessageDialogState*>(lParam);

            state->Theme->AttachWindow(dialog);
            SetWindowSubclass(GetDlgItem(dialog, IDOK), &OKButtonProc, OKButtonSubclassID,
                reinterpret_cast<DWORD_PTR>(state));

            // Shown before the main window exists, it has a taskbar button of its own.
            if (GetWindow(dialog, GW_OWNER) == nullptr)
                SendMessageW(dialog, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(MainIcon()));

            ApplyDPI(dialog);
            CenterOnOwner(dialog);
            MessageBeep(SoundOf(state->IconKind));

            SetFocus(GetDlgItem(dialog, IDOK));
            return FALSE; // The focus is set.
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
            {
                EndDialog(dialog, IDOK);
                return TRUE;
            }

            return FALSE;

        case WM_DPICHANGED:
            PostMessageW(dialog, ApplyDPIMessage, 0, 0);
            return FALSE; // Windows moves and rescales the dialog.

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

    As a MessageBoxW() at the dialog's DPI: the icon at the top left, the message to its right, wrapped at 400
    pixels at 96 DPI and centered on the icon if it's shorter, and the OK button centered below both, as the
    VCL GUI runner's message dialogs have it. Then sizes the dialog to fit, keeping its position.
*/
void LayOutDialog(HWND dialog)
{
    UINT const dpi = WindowDPI(dialog);
    int const margin = ScaleForDPI(12, dpi);
    int const iconSize = GetSystemMetricsForDpi(SM_CXICON, dpi);
    HWND const messageLabel = GetDlgItem(dialog, MessageID);

    std::wstring const text = UTF8ToWide(WindowText(messageLabel));
    RECT bounds = { 0, 0, ScaleForDPI(400, dpi), 0 };
    HDC const context = GetDC(messageLabel);

    if (context == nullptr)
        throw std::runtime_error("Couldn't measure the message");

    HGDIOBJ const oldFont = SelectObject(context, reinterpret_cast<HGDIOBJ>(SendMessageW(messageLabel, WM_GETFONT, 0,
        0)));
    DrawTextW(context, text.c_str(), -1, &bounds, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
    SelectObject(context, oldFont);
    ReleaseDC(messageLabel, context);

    int const textWidth = bounds.right;
    int const textHeight = bounds.bottom;
    int const textLeft = margin + iconSize + margin;
    int const textTop = margin + (std::max)(0, (iconSize - textHeight) / 2);

    MoveWindow(GetDlgItem(dialog, IconID), margin, margin, iconSize, iconSize, TRUE);
    MoveWindow(messageLabel, textLeft, textTop, textWidth, textHeight, TRUE);

    int const buttonWidth = ScaleForDPI(80, dpi);
    int const buttonHeight = ScaleForDPI(26, dpi);
    int const width = (std::max)(textLeft + textWidth + margin, buttonWidth + 2 * margin);
    int const buttonTop = (std::max)(margin + iconSize, textTop + textHeight) + margin + ScaleForDPI(4, dpi);
    MoveWindow(GetDlgItem(dialog, IDOK), (width - buttonWidth) / 2, buttonTop, buttonWidth, buttonHeight, TRUE);

    RECT frame = { 0, 0, width, buttonTop + buttonHeight + margin };
    AdjustWindowRectExForDpi(&frame, static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_STYLE)), FALSE,
        static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_EXSTYLE)), dpi);
    SetWindowPos(dialog, nullptr, 0, 0, frame.right - frame.left, frame.bottom - frame.top,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------
/*
    OKButtonProc

    The OK button, which has the focus, copies the dialog as text on Ctrl+C, laid out as a MessageBoxW()
    copies itself. No exception may leave a window procedure; a failure to copy is ignored, as it would be in
    a MessageBoxW().
*/
LRESULT CALLBACK OKButtonProc(HWND button, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR id, DWORD_PTR state)
{
    if (message == WM_NCDESTROY)
    {
        RemoveWindowSubclass(button, &OKButtonProc, id);
    }
    else if (message == WM_KEYDOWN && (wParam == 'C' || wParam == VK_INSERT) &&
             (GetKeyState(VK_CONTROL) & 0x8000) != 0)
    {
        try
        {
            TMessageDialogState const* const dialog = reinterpret_cast<TMessageDialogState const*>(state);
            std::string const line = "---------------------------\r\n";
            SetClipboardText(button, line + WideToUTF8(dialog->Caption) + "\r\n" + line + WideToUTF8(dialog->Text) +
                "\r\n" + line + "OK\r\n" + line);
        }
        catch (...)
        {
        }

        return 0;
    }

    return DefSubclassProc(button, message, wParam, lParam);
}
//---------------------------------------------------------------------------
UINT SoundOf(EWin32GUIMessageIcon icon)
{
    return icon == EWin32GUIMessageIcon::Error ? MB_ICONERROR : MB_ICONWARNING;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void ShowMessageDialog(HWND owner, TWin32GUITheme& theme, std::string const& caption, std::string const& text,
    EWin32GUIMessageIcon icon)
{
    try
    {
        TMessageDialogState state;
        state.Caption = UTF8ToWide(caption);
        state.IconKind = icon;
        state.Text = UTF8ToWide(AdjustLineBreaks(text));
        state.Theme = &theme;

        TWin32GUIDialogTemplate const dialogTemplate = BuildDialogTemplate(state);
        INT_PTR const result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), dialogTemplate.Get(), owner,
            &DialogProc, reinterpret_cast<LPARAM>(&state));

        if (state.Icon != nullptr)
            DestroyIcon(state.Icon);

        if (result != -1)
            return;
    }
    catch (...)
    {
    }

    try
    {
        MessageBoxW(owner, UTF8ToWide(text).c_str(), UTF8ToWide(caption).c_str(), MB_OK | SoundOf(icon));
    }
    catch (...)
    {
    }
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
