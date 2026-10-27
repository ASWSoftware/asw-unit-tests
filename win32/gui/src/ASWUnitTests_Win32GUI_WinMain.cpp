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
#include <exception>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_System.h"
#include "ASWUnitTests_Win32GUI_TextDialog.h"
//---------------------------------------------------------------------------

namespace
{

// The title of windows that have no other, as the VCL GUI runner's Application->Title.
char const ApplicationTitle[] = "ASWUnitTests";
wchar_t const MainWindowClassName[] = L"ASWUnitTests_Win32GUI_Main";

//---------------------------------------------------------------------------

LRESULT CALLBACK MainWindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);
void ShowUnhandledException(std::string const& description);

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
/*
    ShowUnhandledException

    Reports an exception that escaped to WinMain(). Called from its catch blocks, so it throws nothing: if the
    text dialog can't be shown, a message box is.
*/
void ShowUnhandledException(std::string const& description)
{
    try
    {
        std::string const text = "Unhandled exception: " + description;

        try
        {
            ASWUnitTests::ShowTextDialog(nullptr, ApplicationTitle, text);
        }
        catch (...)
        {
            MessageBoxW(nullptr, ASWUnitTests::UTF8ToWide(text).c_str(), L"ASWUnitTests", MB_OK | MB_ICONERROR);
        }
    }
    catch (...)
    {
        MessageBoxW(nullptr, L"Unhandled exception", L"ASWUnitTests", MB_OK | MB_ICONERROR);
    }
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

int WINAPI WinMain(HINSTANCE instance, HINSTANCE, LPSTR, int showCommand)
{
    int exitCode = ASWUnitTests::ExitCode_Success;

    try
    {
        INITCOMMONCONTROLSEX commonControls = {};
        commonControls.dwSize = sizeof(commonControls);
        commonControls.dwICC = ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS | ICC_STANDARD_CLASSES |
            ICC_TAB_CLASSES | ICC_TREEVIEW_CLASSES;
        InitCommonControlsEx(&commonControls);

        // Parsed before the main window exists, so --help, --version, and argument errors are shown on their own
        // and the process exits with the same code the console runner would use, without opening the GUI.
        ASWUnitTests::TGUIOptions options;
        ASWUnitTests::TGUICommandLineResult const commandLine =
            ASWUnitTests::TGUICommandLine::Parse(ASWUnitTests::ProcessArguments(), options);

        if (commandLine.EarlyExitCode.has_value())
        {
            ASWUnitTests::ShowTextDialog(nullptr, ApplicationTitle, commandLine.Message);
            return *commandLine.EarlyExitCode;
        }

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
    }
    catch (std::exception const& exception)
    {
        ShowUnhandledException(exception.what());
        exitCode = ASWUnitTests::ExitCode_UnhandledException;
    }
    catch (...)
    {
        ShowUnhandledException("Unknown");
        exitCode = ASWUnitTests::ExitCode_UnhandledExceptionUnknown;
    }

    return exitCode;
}
//---------------------------------------------------------------------------
