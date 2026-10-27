/* **************************************************************************
ASWUnitTests_Win32GUI_MainWindow.h
Author: Anthony S. West - ASW Software

The Win32 GUI runner's main window, the counterpart of the VCL GUI runner's
main form (vcl/gui/src/ASWUnitTests_GUI_MainForm): the same controls, menu,
hot keys and layout, built with the Win32 API. Its controls are named as
the .dfm's components and its commands as the form's actions, so the two
can be compared side by side.

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
#ifndef ASWUnitTests_Win32GUI_MainWindowH
#define ASWUnitTests_Win32GUI_MainWindowH
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
#include <deque>
#include <functional>
#include <mutex>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Commands.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DPI.h"
#include "ASWUnitTests_Win32GUI_Splitter.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// EWin32GUICommand
//
// The main window's commands, one per VCL GUI runner action, named as in
// TGUICommandStates (e.g. RunSelected for Act_RunSelected). Each is also the
// control ID of its menu item and toolbar button.
/////////////////////////////////////////////////////////////////////////////
enum class EWin32GUICommand : WORD
{
    About = 1000,
    CollapseAll,
    CommandLineOptions,
    CopyDetails,
    Exit,
    ExpandAll,
    ExportJUnitReport,
    FocusFilter,
    ResetLayout,
    RunFailed,
    RunSelected,
    SelectAll,
    SelectFailed,
    SelectNone,
    ShuffleOrder,
    ShuffleSeed,
    Stop,
};


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIMainWindow
/////////////////////////////////////////////////////////////////////////////
class TWin32GUIMainWindow
{
private:
    // A toolbar button: its command, caption, and left edge and width at 96 DPI, from the .dfm.
    struct TToolbarButton
    {
        EWin32GUICommand Command;
        wchar_t const* Caption;
        int Left;
        int Width;
        HWND TWin32GUIMainWindow::* Control;
    };

private:
    static TToolbarButton const ToolbarButtons[7];

private:
    // Controls, named as the VCL GUI runner's .dfm components.
    HWND m_Btn_CopyDetails;
    HWND m_Btn_RunFailed;
    HWND m_Btn_RunSelected;
    HWND m_Btn_SelectAll;
    HWND m_Btn_SelectFailed;
    HWND m_Btn_SelectNone;
    HWND m_Btn_Stop;
    HWND m_Edt_Filter;
    HWND m_Lbl_Filter;
    HWND m_LV_Failures;
    HWND m_Mem_Detail;
    HMENU m_MM_Main;
    HWND m_PB_Progress;
    HWND m_PC_Results;
    HWND m_RE_Log;
    HWND m_SB_Status;
    TWin32GUISplitter m_Spl_Detail;
    TWin32GUISplitter m_Spl_Tests;
    HWND m_TV_Tests;

    HACCEL m_Accelerators;
    // Set by WM_CLOSE, so destroying the window ends the message loop; destroying it otherwise (e.g. as this
    // object is destroyed after an error) doesn't.
    bool m_Closing;
    TGUICommandStates m_CommandStates;
    // The detail pane's height and the test tree's width, in pixels at m_DPI.
    int m_DetailHeight;
    UINT m_DPI;
    // The control that had the focus when the window was last deactivated, given it back on activation.
    HWND m_FocusedControl;
    TWin32GUIFont m_MonospaceFont;
    // Actions PostToUI() queued, run by RunQueued().
    std::deque<std::function<void()> > m_Queue;
    std::mutex m_QueueMutex;
    // Msftedit.dll, which provides the rich edit control the log uses.
    HMODULE m_RichEditLibrary;
    int m_TestTreeWidth;
    TWin32GUIFont m_UIFont;
    HWND m_Window;

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Recreates the fonts for m_DPI and applies them, with everything else sized by the DPI: the status bar's
    // parts and the failure list's columns.
    void ApplyDPI();
    void CenterOnMonitor();
    HWND CreateChild(DWORD exStyle, wchar_t const* className, wchar_t const* text, DWORD style, WORD id = 0);
    void CreateControls();
    void CreateMenuBar();
    // Runs a command from its menu item, toolbar button or hot key, if it's enabled; see the definition.
    void DoCommand(EWin32GUICommand command);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void LayOut();
    void OnDetailSplitterDrag(int position);
    void OnDPIChanged(UINT dpi, RECT const& suggested);
    void OnTestsSplitterDrag(int position);
    // Reports an error that escaped from 'where' in a message box.
    void ReportUnexpectedError(std::string const& where, std::string const& description);
    // Runs an event handler's body, reporting any exception from it by name; see the definition.
    void RunGuarded(std::string const& where, std::function<void()> const& body);
    // Runs the actions PostToUI() queued.
    void RunQueued();
    // Shows the results page whose tab is selected.
    void ShowResultsPage();
    // Enables or disables every command, and the filter box, by GUICommandStates()'s rules.
    void UpdateCommands();

public:
    TWin32GUIMainWindow();
    ~TWin32GUIMainWindow();

    TWin32GUIMainWindow(TWin32GUIMainWindow const&) = delete;
    TWin32GUIMainWindow& operator=(TWin32GUIMainWindow const&) = delete;

    // Creates the window, hidden, at its default size, centered on its monitor. Throws std::runtime_error if it
    // can't be created.
    void Create();
    // The process exit code: ExitCode_Success, since the GUI was used interactively.
    int ExitCode() const;
    HWND Handle() const;
    // Queues 'action' to run on the window's thread from its message loop, after the messages already waiting:
    // the counterpart of the VCL's TThread::ForceQueue(). Safe to call from any thread.
    void PostToUI(std::function<void()> action);
    // Runs the window's message loop until it's closed.
    void RunMessageLoop();
    void Show(int showCommand);
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_MainWindowH
