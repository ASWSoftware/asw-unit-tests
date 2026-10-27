/* **************************************************************************
ASWUnitTests_Win32GUI_WinMain.cpp
Author: Anthony S. West - ASW Software

Win32 GUI app that runs the ASWUnitTests framework's tests.

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
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

wchar_t const MainWindowClassName[] = L"ASWUnitTests_Win32GUI_Main";

//---------------------------------------------------------------------------

LRESULT CALLBACK MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

//---------------------------------------------------------------------------
LRESULT CALLBACK MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.lpfnWndProc = MainWindowProc;
    windowClass.hInstance = instance;
    windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
    windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    windowClass.lpszClassName = MainWindowClassName;

    if (RegisterClassExW(&windowClass) == 0)
        return ASWUnitTests::ExitCode_UnhandledException;

    HWND const mainWindow = CreateWindowExW(0, MainWindowClassName, L"ASWUnitTests", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, instance, nullptr);

    if (mainWindow == nullptr)
        return ASWUnitTests::ExitCode_UnhandledException;

    ShowWindow(mainWindow, showCommand);

    MSG message;
    while (GetMessageW(&message, nullptr, 0, 0) > 0)
    {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    return ASWUnitTests::ExitCode_Success;
}
//---------------------------------------------------------------------------
