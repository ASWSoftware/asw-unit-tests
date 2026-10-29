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
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
#include "ASWUnitTests_GUI_Theme.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_MainWindow.h"
#include "ASWUnitTests_Win32GUI_System.h"
#include "ASWUnitTests_Win32GUI_TextDialog.h"
#include "ASWUnitTests_Win32GUI_Theme.h"
//---------------------------------------------------------------------------

#if defined(__MINGW32__)
// As in src/main.cpp: MinGW-w64's C runtime can expand wildcards in the command line into the names of matching
// files before WinMain() runs (WinLibs' builds do by default), and defining _dowildcard as 0 turns that off. The
// GUI reads its arguments from GetCommandLineW() rather than the runtime's argv, so they'd arrive as given anyway,
// but the expansion would still search the current folder at every start, and the self-tests check the setting.
extern "C"
{
int _dowildcard = 0;
}
#endif

//---------------------------------------------------------------------------

namespace
{

// The title of windows that have no other, as the VCL GUI runner's Application->Title.
char const ApplicationTitle[] = "ASWUnitTests";

//---------------------------------------------------------------------------

void ShowUnhandledException(ASWUnitTests::TWin32GUITheme& theme, std::string const& description);

//---------------------------------------------------------------------------
/*
    ShowUnhandledException

    Reports an exception that escaped to WinMain(), in the GUI's theme. Called from its catch blocks, so it
    throws nothing: if the text dialog can't be shown, a message box is.
*/
void ShowUnhandledException(ASWUnitTests::TWin32GUITheme& theme, std::string const& description)
{
    try
    {
        std::string const text = "Unhandled exception: " + description;

        try
        {
            ASWUnitTests::ShowTextDialog(nullptr, theme, ApplicationTitle, text);
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
    // Before any window, and destroyed after them.
    ASWUnitTests::TWin32GUITheme theme;

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

        // The saved theme, before any window, so the --help dialog has it too: Use Windows Setting with
        // --layout-ignore, which ignores the saved layout file, and --layout-reset, which deletes it.
        ASWUnitTests::EGUITheme const themeChoice = options.LayoutIgnore || options.LayoutReset ?
                ASWUnitTests::EGUITheme::Windows : ASWUnitTests::LoadGUIThemeFromLayoutFile();
        theme.SetDark(ASWUnitTests::GUIThemeIsDark(themeChoice, ASWUnitTests::WindowsAppsUseDarkTheme(),
            ASWUnitTests::WindowsHighContrast()));

        if (commandLine.EarlyExitCode.has_value())
        {
            ASWUnitTests::ShowTextDialog(nullptr, theme, ApplicationTitle, commandLine.Message);
            return *commandLine.EarlyExitCode;
        }

        ASWUnitTests::TWin32GUIMainWindow mainWindow(theme);
        mainWindow.Create();
        mainWindow.Start(options, themeChoice);
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
        ShowUnhandledException(theme, exception.what());
        exitCode = ASWUnitTests::ExitCode_UnhandledException;
    }
    catch (...)
    {
        ShowUnhandledException(theme, "Unknown");
        exitCode = ASWUnitTests::ExitCode_UnhandledExceptionUnknown;
    }

    return exitCode;
}
//---------------------------------------------------------------------------
