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
#include "ASWUnitTests_Win32GUI_MainWindow.h"
#include "ASWUnitTests_Win32GUI_System.h"
#include "ASWUnitTests_Win32GUI_TextDialog.h"
//---------------------------------------------------------------------------

namespace
{

// The title of windows that have no other, as the VCL GUI runner's Application->Title.
char const ApplicationTitle[] = "ASWUnitTests";

//---------------------------------------------------------------------------

void ShowUnhandledException(std::string const& description);

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

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int showCommand)
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

        ASWUnitTests::TWin32GUIMainWindow mainWindow;
        mainWindow.Create();
        mainWindow.Start(options);
        mainWindow.Show(showCommand);
        mainWindow.RunMessageLoop();

        exitCode = mainWindow.ExitCode();

        // After a timeout, the abandoned test's thread may still be running, using one of the window's test groups,
        // and after a crash their state may be corrupted, so the process ends here, as the console runner's does,
        // without destroying them.
        if (mainWindow.WasRunAborted())
            ExitProcess(static_cast<UINT>(exitCode));
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
