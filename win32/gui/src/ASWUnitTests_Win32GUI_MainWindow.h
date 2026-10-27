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
#include <memory>
#include <mutex>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
#include "ASWUnitTests_GUI_Commands.h"
#include "ASWUnitTests_GUI_RunObserver.h"
#include "ASWUnitTests_GUI_Shuffle.h"
#include "ASWUnitTests_GUI_TestList.h"
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
    // Holds the registered tests; created by Start(). Reports to m_Observer, so the destructor destroys it first.
    std::unique_ptr<TTestHandler> m_Handler;
    TWin32GUIFont m_MonospaceFont;
    TGUIRunObserver m_Observer;
    TGUIOptions m_Options;
    // Actions PostToUI() queued, run by RunQueued().
    std::deque<std::function<void()> > m_Queue;
    std::mutex m_QueueMutex;
    // Msftedit.dll, which provides the rich edit control the log uses.
    HMODULE m_RichEditLibrary;
    // Set while Start() puts --filter's pattern in the filter box, so its EN_CHANGE leaves building the tree to
    // Start(), which then builds it once.
    bool m_SettingFilterText;
    TGUIShuffle m_Shuffle;
    // The test tree's status dots, one per TGUITestStatus, drawn for m_DPI by CreateStatusImages().
    HIMAGELIST m_StatusImages;
    // The status bar's last panel.
    std::string m_StatusMessage;
    // Set while SyncTreeChecks() sets the tree's check boxes, so OnTreeItemChanging() lets the changes through.
    bool m_SyncingChecks;
    TGUITestList m_TestList;
    int m_TestTreeWidth;
    TWin32GUIFont m_UIFont;
    HWND m_Window;

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Appends the framework's log output to the log, each line colored as ClassifyGUILogLine() says.
    void AppendLog(std::string const& text);
    // Recreates the fonts and status images for m_DPI and applies them, with everything else sized by the DPI:
    // the tree's indent, the status bar's parts and the failure list's columns.
    void ApplyDPI();
    // Shows only the tests the filter box's text matches, and rebuilds the tree for them.
    void ApplyFilterBox();
    // Rebuilds the test tree from m_TestList's visible tests, fully expanded; see the definition.
    void BuildTestTree();
    void CenterOnMonitor();
    HWND CreateChild(DWORD exStyle, wchar_t const* className, wchar_t const* text, DWORD style, WORD id = 0);
    void CreateControls();
    void CreateMenuBar();
    void CreateStatusImages();
    // Runs a command from its menu item, toolbar button or hot key, if it's enabled; see the definition.
    void DoCommand(EWin32GUICommand command);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    void LayOut();
    void OnDetailSplitterDrag(int position);
    void OnDPIChanged(UINT dpi, RECT const& suggested);
    void OnTestsSplitterDrag(int position);
    // Handles TVN_ITEMCHANGINGW; true vetoes the change. See the definition.
    bool OnTreeItemChanging(NMTVITEMCHANGE const& change);
    // Sets every test tree node's status image from m_TestList.
    void RefreshAllNodeImages();
    // Reports an error that escaped from 'where' in a message box.
    void ReportUnexpectedError(std::string const& where, std::string const& description);
    // Runs an event handler's body, reporting any exception from it by name; see the definition.
    void RunGuarded(std::string const& where, std::function<void()> const& body);
    // Runs the actions PostToUI() queued.
    void RunQueued();
    // The test tree's selected node, or nullptr if none is.
    HTREEITEM SelectedTreeItem() const;
    // Sets a test tree node's status image from m_TestList.
    void SetNodeImage(HTREEITEM item);
    // Expands or collapses every node of the test tree.
    void SetTreeExpanded(bool expanded);
    // Shows the results page whose tab is selected.
    void ShowResultsPage();
    // Sets every test tree node's check box from m_TestList; see the definition.
    void SyncTreeChecks();
    void UpdateCaption();
    // Enables or disables every command, and the filter box, by GUICommandStates()'s rules.
    void UpdateCommands();
    // Shows the selected node's details in the detail pane.
    void UpdateDetail();
    void UpdateStatusBar();

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
    // Loads the registered tests and applies the command line's options; see the definition. Call after
    // Create(), before Show().
    void Start(TGUIOptions const& options);
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_MainWindowH
