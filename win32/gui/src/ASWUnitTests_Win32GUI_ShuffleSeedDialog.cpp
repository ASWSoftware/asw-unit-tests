/* **************************************************************************
ASWUnitTests_Win32GUI_ShuffleSeedDialog.cpp
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
#include "ASWUnitTests_Win32GUI_ShuffleSeedDialog.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DialogTemplate.h"
#include "ASWUnitTests_Win32GUI_DPI.h"
#include "ASWUnitTests_Win32GUI_MessageDialog.h"
#include "ASWUnitTests_Win32GUI_System.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// Posted to the dialog when it moves to a monitor with another DPI, so it's laid out again after Windows has
// rescaled it and its controls' fonts.
UINT const ApplyDPIMessage = WM_APP + 1;
// The dialog's title, and its error message's, as the VCL GUI runner's.
wchar_t const DialogCaption[] = L"Shuffle Seed";
// The controls' IDs, in the dialog's order; the buttons' are IDOK and IDCANCEL. The radio buttons' must be
// consecutive, for CheckRadioButton().
WORD const FixedSeedID = 102;
WORD const IntroLabelID = 100;
WORD const LastSeedLabelID = 104;
WORD const NewSeedID = 101;
WORD const SeedBoxID = 103;

/////////////////////////////////////////////////////////////////////////////
// TShuffleSeedDialogState
//
// The choice the dialog edits, and the theme it's shown in.
/////////////////////////////////////////////////////////////////////////////
struct TShuffleSeedDialogState
{
    TGUIShuffle* Shuffle = nullptr;
    TWin32GUITheme* Theme = nullptr;
};

//---------------------------------------------------------------------------

void ApplyChoice(HWND dialog);
TWin32GUIDialogTemplate BuildDialogTemplate(TGUIShuffle const& shuffle);
void CenterOnOwner(HWND dialog);
INT_PTR CALLBACK DialogProc(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR HandleDialogMessage(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam);
void LayOutDialog(HWND dialog);
void SelectSeedChoice(HWND dialog, WORD radioButtonID);
int TextHeight(HWND control, int width);

//---------------------------------------------------------------------------
/*
    ApplyChoice

    OK's click: applies the choice and closes the dialog, or, if the seed isn't valid, says why and leaves the
    dialog open with the seed selected, as the VCL GUI runner's does.
*/
void ApplyChoice(HWND dialog)
{
    TShuffleSeedDialogState const* const state = reinterpret_cast<TShuffleSeedDialogState const*>(
        GetWindowLongPtrW(dialog, DWLP_USER));
    HWND const seedBox = GetDlgItem(dialog, SeedBoxID);
    std::string error;

    if (!state->Shuffle->ApplySeedChoice(IsDlgButtonChecked(dialog, NewSeedID) == BST_CHECKED, WindowText(seedBox),
        error))
    {
        ShowMessageDialog(dialog, *state->Theme, WideToUTF8(DialogCaption), error, EWin32GUIMessageIcon::Warning);
        // Focuses the seed box as tabbing to it does, which also selects its text.
        SendMessageW(dialog, WM_NEXTDLGCTL, reinterpret_cast<WPARAM>(seedBox), TRUE);
        return;
    }

    EndDialog(dialog, IDOK);
}
//---------------------------------------------------------------------------
/*
    BuildDialogTemplate

    The VCL GUI runner's dialog, with its captions, & letters and tab order, and the seed box already holding
    the seed to show, so only the user's own typing selects "same seed" (see HandleDialogMessage()).
    LayOutDialog() sets the positions and sizes.
*/
TWin32GUIDialogTemplate BuildDialogTemplate(TGUIShuffle const& shuffle)
{
    std::optional<unsigned int> const lastSeed = shuffle.LastSeed();
    std::string const lastSeedText = lastSeed.has_value() ?
            "The latest shuffled run's seed was " + std::to_string(*lastSeed) + "." :
            std::string("No run has been shuffled yet.");

    TWin32GUIDialogTemplate dialogTemplate(WS_POPUP | WS_CAPTION | WS_SYSMENU | DS_MODALFRAME, DialogCaption);

    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, IntroLabelID, WS_CHILD | WS_VISIBLE | SS_LEFT |
        SS_NOPREFIX, 0, L"A shuffled run runs the groups, and each group's tests, in an order its seed decides, so "
        L"running again with the same seed repeats that order. Clicking OK also turns on Run in Shuffled Order.");
    // SelectSeedChoice() gives the checked one the tab stop.
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, NewSeedID, WS_CHILD | WS_VISIBLE | WS_GROUP |
        BS_AUTORADIOBUTTON, 0, L"Use a &new random seed for each run");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, FixedSeedID, WS_CHILD | WS_VISIBLE |
        BS_AUTORADIOBUTTON, 0, L"Use the same &seed for every run:");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Edit, SeedBoxID, WS_CHILD | WS_VISIBLE | WS_GROUP |
        WS_TABSTOP | ES_NUMBER | ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, UTF8ToWide(std::to_string(shuffle.SeedToShow())));
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Static, LastSeedLabelID, WS_CHILD | WS_VISIBLE |
        WS_GROUP | SS_LEFT | SS_NOPREFIX, 0, UTF8ToWide(lastSeedText));
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDCANCEL, WS_CHILD | WS_VISIBLE | WS_GROUP |
        WS_TABSTOP | BS_PUSHBUTTON, 0, L"Cancel");
    dialogTemplate.AddControl(EWin32GUIDialogControlClass::Button, IDOK, WS_CHILD | WS_VISIBLE | WS_TABSTOP |
        BS_DEFPUSHBUTTON, 0, L"OK");

    return dialogTemplate;
}
//---------------------------------------------------------------------------
/*
    CenterOnOwner

    As the VCL's poMainFormCenter, but kept within the work area of the owner's monitor.
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
    that would ends the dialog with -1, which ShowShuffleSeedDialog() reports as a failure.
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
/*
    HandleDialogMessage

    The seed box's EN_CHANGE selects "same seed". The template gives the box its first text, which sends none.
*/
INT_PTR HandleDialogMessage(HWND dialog, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
        {
            SetWindowLongPtrW(dialog, DWLP_USER, lParam);
            TShuffleSeedDialogState const* const state = reinterpret_cast<TShuffleSeedDialogState const*>(lParam);

            HWND const seedBox = GetDlgItem(dialog, SeedBoxID);
            SendMessageW(seedBox, EM_SETLIMITTEXT, 10, 0); // 4294967295, the largest seed.
            state->Theme->AttachWindow(dialog);
            state->Theme->Attach(seedBox, EWin32GUIThemedControl::Edit);

            bool const fixedSeed = state->Shuffle->FixedSeed().has_value();
            SelectSeedChoice(dialog, fixedSeed ? FixedSeedID : NewSeedID);

            LayOutDialog(dialog);
            CenterOnOwner(dialog);

            if (fixedSeed)
            {
                SetFocus(seedBox);
                SendMessageW(seedBox, EM_SETSEL, 0, -1);
            }
            else
            {
                SetFocus(GetDlgItem(dialog, NewSeedID));
            }

            return FALSE; // The focus is set.
        }

        case WM_COMMAND:
            switch (LOWORD(wParam))
            {
                case FixedSeedID:
                case NewSeedID:
                    if (HIWORD(wParam) == BN_CLICKED)
                        SelectSeedChoice(dialog, LOWORD(wParam));

                    return TRUE;

                case IDCANCEL:
                    EndDialog(dialog, IDCANCEL);
                    return TRUE;

                case IDOK:
                    ApplyChoice(dialog);
                    return TRUE;

                case SeedBoxID:
                    if (HIWORD(wParam) == EN_CHANGE)
                        SelectSeedChoice(dialog, FixedSeedID);

                    return TRUE;

                default:
                    return FALSE;
            }

        case WM_DPICHANGED:
            PostMessageW(dialog, ApplyDPIMessage, 0, 0);
            return FALSE; // Windows moves and rescales the dialog.

        default:
            if (message == ApplyDPIMessage)
            {
                LayOutDialog(dialog);
                return TRUE;
            }

            return FALSE;
    }
}
//---------------------------------------------------------------------------
/*
    LayOutDialog

    Places the controls as the VCL GUI runner's dialog does, scaled to the dialog's DPI: 420 pixels wide at
    96 DPI, as tall as the wrapped introduction needs. Then sizes the dialog to fit, keeping its position.
*/
void LayOutDialog(HWND dialog)
{
    UINT const dpi = WindowDPI(dialog);
    int const margin = ScaleForDPI(12, dpi);
    int const width = ScaleForDPI(420, dpi);
    int const textWidth = width - 2 * margin;

    HWND const introLabel = GetDlgItem(dialog, IntroLabelID);
    int const introHeight = TextHeight(introLabel, textWidth);
    MoveWindow(introLabel, margin, margin, textWidth, introHeight, TRUE);
    int top = margin + introHeight + margin;

    MoveWindow(GetDlgItem(dialog, NewSeedID), margin, top, textWidth, ScaleForDPI(20, dpi), TRUE);
    top += ScaleForDPI(28, dpi);

    int const fixedSeedWidth = ScaleForDPI(220, dpi);
    MoveWindow(GetDlgItem(dialog, FixedSeedID), margin, top, fixedSeedWidth, ScaleForDPI(20, dpi), TRUE);
    MoveWindow(GetDlgItem(dialog, SeedBoxID), margin + fixedSeedWidth + ScaleForDPI(8, dpi),
        top - ScaleForDPI(2, dpi), ScaleForDPI(120, dpi), ScaleForDPI(23, dpi), TRUE);
    top += ScaleForDPI(32, dpi);

    HWND const lastSeedLabel = GetDlgItem(dialog, LastSeedLabelID);
    int const lastSeedHeight = TextHeight(lastSeedLabel, textWidth);
    MoveWindow(lastSeedLabel, margin, top, textWidth, lastSeedHeight, TRUE);
    top += lastSeedHeight + margin + ScaleForDPI(4, dpi);

    int const buttonWidth = ScaleForDPI(80, dpi);
    int const buttonHeight = ScaleForDPI(26, dpi);
    int const cancelLeft = width - margin - buttonWidth;
    MoveWindow(GetDlgItem(dialog, IDCANCEL), cancelLeft, top, buttonWidth, buttonHeight, TRUE);
    MoveWindow(GetDlgItem(dialog, IDOK), cancelLeft - ScaleForDPI(8, dpi) - buttonWidth, top, buttonWidth,
        buttonHeight, TRUE);

    RECT frame = { 0, 0, width, top + buttonHeight + margin };
    AdjustWindowRectExForDpi(&frame, static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_STYLE)), FALSE,
        static_cast<DWORD>(GetWindowLongPtrW(dialog, GWL_EXSTYLE)), dpi);
    SetWindowPos(dialog, nullptr, 0, 0, frame.right - frame.left, frame.bottom - frame.top,
        SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------
/*
    SelectSeedChoice

    Checks one of the two radio buttons and gives it the group's tab stop, so Tab goes to the checked one, as
    in the VCL, whose radio buttons are tab stops only while checked.
*/
void SelectSeedChoice(HWND dialog, WORD radioButtonID)
{
    CheckRadioButton(dialog, NewSeedID, FixedSeedID, radioButtonID);

    for (WORD const id : { NewSeedID, FixedSeedID })
    {
        HWND const radioButton = GetDlgItem(dialog, id);
        LONG_PTR const style = GetWindowLongPtrW(radioButton, GWL_STYLE);
        SetWindowLongPtrW(radioButton, GWL_STYLE, id == radioButtonID ? (style | WS_TABSTOP) : (style & ~WS_TABSTOP));
    }
}
//---------------------------------------------------------------------------
/*
    TextHeight

    The height of the control's text in its font, wrapped at 'width' pixels, as a word-wrapping label's.
*/
int TextHeight(HWND control, int width)
{
    std::wstring const text = UTF8ToWide(WindowText(control));
    RECT bounds = { 0, 0, width, 0 };

    HDC const context = GetDC(control);

    if (context == nullptr)
        throw std::runtime_error("Couldn't measure the Shuffle Seed dialog's text");

    HGDIOBJ const oldFont = SelectObject(context, reinterpret_cast<HGDIOBJ>(SendMessageW(control, WM_GETFONT, 0,
        0)));
    DrawTextW(context, text.c_str(), -1, &bounds, DT_CALCRECT | DT_WORDBREAK | DT_NOPREFIX | DT_EXPANDTABS);
    SelectObject(context, oldFont);
    ReleaseDC(control, context);

    return bounds.bottom;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
bool ShowShuffleSeedDialog(HWND owner, TWin32GUITheme& theme, TGUIShuffle& shuffle)
{
    TWin32GUIDialogTemplate const dialogTemplate = BuildDialogTemplate(shuffle);

    TShuffleSeedDialogState state;
    state.Shuffle = &shuffle;
    state.Theme = &theme;

    INT_PTR const result = DialogBoxIndirectParamW(GetModuleHandleW(nullptr), dialogTemplate.Get(), owner,
        &DialogProc, reinterpret_cast<LPARAM>(&state));

    if (result == -1)
        throw std::runtime_error("Couldn't show the \"Shuffle Seed\" dialog");

    return result == IDOK;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
