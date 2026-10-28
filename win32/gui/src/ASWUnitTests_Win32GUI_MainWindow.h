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
#include <chrono>
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
#include "ASWUnitTests_GUI_Commands.h"
#include "ASWUnitTests_GUI_Layout.h"
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
    // A toolbar button: its command, caption, left edge and width at 96 DPI, and tooltip, from the .dfm.
    struct TToolbarButton
    {
        EWin32GUICommand Command;
        wchar_t const* Caption;
        int Left;
        int Width;
        wchar_t const* Hint;
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
    // The tooltips of the toolbar's buttons and the filter box, the .dfm's Hints.
    HWND m_TT_Hints;
    HWND m_TV_Tests;

    HACCEL m_Accelerators;
    // Set when the window was asked to close during a run, which then closes it once the run has stopped.
    bool m_CloseAfterRun;
    // Set by WM_CLOSE, so destroying the window ends the message loop; destroying it otherwise (e.g. as this
    // object is destroyed after an error) doesn't.
    bool m_Closing;
    TGUICommandStates m_CommandStates;
    // The test that has started but not yet finished, if any.
    std::optional<std::size_t> m_CurrentTestIndex;
    // The detail pane's height and the test tree's width, in pixels at m_DPI.
    int m_DetailHeight;
    UINT m_DPI;
    // The path last chosen in Export JUnit Report's dialog, where it starts next time.
    std::string m_ExportJUnitReportPath;
    // The control that had the focus as the current or latest run started; see SetRunning().
    HWND m_FocusBeforeRun;
    // The control that had the focus when the window was last deactivated, given it back on activation.
    HWND m_FocusedControl;
    // Holds the registered tests; created by Start(). Reports to m_Observer, so the destructor destroys it first.
    std::unique_ptr<TTestHandler> m_Handler;
    // Set when the selection is restored at start and saved at close: only when the command line chooses no
    // tests, so an automated --run or a --filter launch never replaces the selection saved interactively.
    bool m_KeepSelection;
    // The exit code the console runner would have returned for the latest run; see ExitCode().
    int m_LastRunExitCode;
    // The latest run's JUnit report entries, for --report-junit and Export JUnit Report. Unset until a run
    // finishes, and after a run an unhandled exception ended, which has no report, as in the console runner.
    std::optional<std::vector<TJUnitTestCase> > m_LastRunJUnitTestCases;
    TWin32GUIFont m_MonospaceFont;
    TGUIRunObserver m_Observer;
    // Set by LoadLayout() when the saved layout was maximized, so Show() opens the window maximized.
    bool m_OpenMaximized;
    TGUIOptions m_Options;
    // The exit code of a WM_QUIT that ProcessMessages() took from the queue during a run, posted again when the
    // run ends.
    std::optional<int> m_PendingQuitCode;
    // Actions PostToUI() queued, run by RunQueued().
    std::deque<std::function<void()> > m_Queue;
    std::mutex m_QueueMutex;
    // Msftedit.dll, which provides the rich edit control the log uses.
    HMODULE m_RichEditLibrary;
    // Set when a run ends with a test that timed out or crashed, after which no further run can start; see
    // WasRunAborted().
    bool m_RunAborted;
    std::optional<std::chrono::steady_clock::time_point> m_RunEnd;
    std::size_t m_RunFinishedCount;
    bool m_Running;
    std::optional<std::chrono::steady_clock::time_point> m_RunStart;
    std::size_t m_RunTotalCount;
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
    // Each test's tree node, by index into m_TestList; nullptr while the filter box hides it. Set by
    // BuildTestTree(); a Win32 tree's nodes stay valid until they're deleted.
    std::vector<HTREEITEM> m_TestNodes;
    int m_TestTreeWidth;
    TWin32GUIFont m_UIFont;
    HWND m_Window;

private:
    static LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam);

private:
    // Adds a failed or skipped test to the Failures & Skips list.
    void AddFailureItem(std::size_t testIndex);
    // Appends the framework's log output to the log, each line colored as ClassifyGUILogLine() says.
    void AppendLog(std::string const& text);
    // Recreates the fonts and status images for m_DPI and applies them, with everything else sized by the DPI:
    // the tree's indent, the status bar's parts and the failure list's columns.
    void ApplyDPI();
    // Shows only the tests the filter box's text matches, and rebuilds the tree for them.
    void ApplyFilterBox();
    // Sets the test tree's width and the detail pane's height that 'layout' has, within what the splitters
    // allow.
    void ApplyPanelSizes(TGUILayout const& layout);
    // Rebuilds the test tree from m_TestList's visible tests, fully expanded; see the definition.
    void BuildTestTree();
    void CenterOnMonitor();
    HWND CreateChild(DWORD exStyle, wchar_t const* className, wchar_t const* text, DWORD style, WORD id = 0);
    void CreateControls();
    void CreateMenuBar();
    void CreateStatusImages();
    void CreateTooltips();
    // The window's layout as it would be saved; see the definition.
    TGUILayout CurrentLayout() const;
    // Runs a command from its menu item, toolbar button or hot key, if it's enabled; see the definition.
    void DoCommand(EWin32GUICommand command);
    // Saves the latest run's JUnit report where the user chooses; see the definition.
    void ExportJUnitReport();
    // Ends the run on an unhandled exception that escaped TTestHandler::Run(), described by 'description'.
    void FailRunOnUnhandledException(std::string const& description);
    LRESULT HandleMessage(UINT message, WPARAM wParam, LPARAM lParam);
    bool IsToolbarButton(HWND control) const;
    void LayOut();
    // Restores the layout saved when the window last closed; see the definition.
    void LoadLayout();
    // Restores the test selection saved when the window last closed, noting what it did in the log.
    void LoadSelection();
    // Moves the focus to 'control' as Tab does; see the definition.
    void MoveFocusTo(HWND control);
    // The control Tab (or Shift+Tab, if not 'forward') moves the focus to from 'from'; see the definition.
    HWND NextTabStop(HWND from, bool forward) const;
    // Handles WM_CLOSE; see the definition.
    void OnClose();
    void OnDetailSplitterDrag(int position);
    void OnDPIChanged(UINT dpi, RECT const& suggested);
    // The Failures & Skips list's NM_CUSTOMDRAW: each row's text in its result's color.
    LRESULT OnFailuresCustomDraw(NMLVCUSTOMDRAW& draw) const;
    // Selects the double-clicked failure's test in the tree; see the definition.
    void OnFailuresDoubleClick();
    // Handles Tab, Shift+Tab and Ctrl+Tab before the dialog keyboard handling does; see the definition.
    bool OnTabKey(MSG const& message);
    void OnTestFinished(TTestCaseRecord const& record, std::string const& testLog);
    void OnTestStarted(std::string const& groupName, std::string const& testName);
    void OnTestsSplitterDrag(int position);
    // Handles TVN_ITEMCHANGINGW; true vetoes the change. See the definition.
    bool OnTreeItemChanging(NMTVITEMCHANGE const& change);
    // Handles one message from the queue, as the message loop does: hot keys first, then a dialog's keyboard
    // handling, then the window it's for.
    void ProcessMessage(MSG& message);
    // Handles the messages waiting in the queue, so the window stays responsive while tests run; see the
    // definition.
    void ProcessMessages();
    // Sets every test tree node's status image from m_TestList.
    void RefreshAllNodeImages();
    // Reports an error that escaped from 'where' in a message box.
    void ReportUnexpectedError(std::string const& where, std::string const& description);
    // Gives the window its default size, centered on its monitor, and the panels their default sizes.
    void ResetLayout();
    // Deletes the saved layout, for --layout-reset; see the definition.
    void ResetSavedLayout();
    void RunCheckedTests();
    // Runs an event handler's body, reporting any exception from it by name; see the definition.
    void RunGuarded(std::string const& where, std::function<void()> const& body);
    // Runs the actions PostToUI() queued.
    void RunQueued();
    // Runs the tests 'filter' matches; see the definition.
    void RunTests(TestFilter const& filter, std::string const& filterDescription);
    // Saves the window's layout to GUILayoutFilePath(), keeping any other values in the file. Not saving it
    // (e.g. a read-only profile) only means the next start uses the defaults.
    void SaveLayout();
    // Saves the test selection to GUISelectionFilePath(). Not saving it only means the next start checks every
    // test.
    void SaveSelection();
    // Shows the results page 'page' (0 Failures & Skips, 1 Log), as clicking its tab does.
    void SelectResultsPage(int page);
    // The test tree's selected node, or nullptr if none is.
    HTREEITEM SelectedTreeItem() const;
    // Sets a test tree node's status image from m_TestList.
    void SetNodeImage(HTREEITEM item);
    // Sets the progress bar's position; see the definition.
    void SetProgressPosition(int position);
    // Enables or disables the commands for a run starting or ending; see the definition.
    void SetRunning(bool running);
    // Expands or collapses every node of the test tree.
    void SetTreeExpanded(bool expanded);
    // Shows the results page whose tab is selected; see the definition.
    void ShowResultsPage();
    // Shows whether shuffling is on, in the Options menu's check mark and the status bar.
    void ShowShuffleState();
    // Sets every test tree node's check box from m_TestList; see the definition.
    void SyncTreeChecks();
    // A test's tree node, or nullptr while the filter box hides it.
    HTREEITEM TestNode(std::size_t testIndex) const;
    void UpdateCaption();
    // Enables or disables every command, and the filter box, by GUICommandStates()'s rules.
    void UpdateCommands();
    // Shows the selected node's details in the detail pane.
    void UpdateDetail();
    void UpdateStatusBar();
    // Sets the status images of a test's node and of the group and suite nodes holding it.
    void UpdateTestNodeImages(std::size_t testIndex);
    // Writes the latest run's JUnit report to 'path', noting the outcome in the log. False, with 'error' set to
    // why, if it couldn't be written. See the definition.
    bool WriteJUnitReport(std::string const& path, std::string& error);

public:
    TWin32GUIMainWindow();
    ~TWin32GUIMainWindow();

    TWin32GUIMainWindow(TWin32GUIMainWindow const&) = delete;
    TWin32GUIMainWindow& operator=(TWin32GUIMainWindow const&) = delete;

    // Creates the window, hidden, at its default size, centered on its monitor. Throws std::runtime_error if it
    // can't be created.
    void Create();
    // The process exit code: with --exit, the one the console runner would have returned for the --run run;
    // otherwise ExitCode_Success, since the GUI was used interactively.
    int ExitCode() const;
    HWND Handle() const;
    // Queues 'action' to run on the window's thread from its message loop, after the messages already waiting:
    // the counterpart of the VCL's TThread::ForceQueue(). Safe to call from any thread.
    void PostToUI(std::function<void()> action);
    // Runs the window's message loop until it's closed.
    void RunMessageLoop();
    // Shows the window as WinMain()'s 'showCommand' says, but maximized if the restored layout was and nothing
    // else was asked for.
    void Show(int showCommand);
    // Loads the registered tests, applies the command line's options, and restores the saved layout and selection;
    // see the definition. Call after Create(), before Show().
    void Start(TGUIOptions const& options);
    // Whether a run ended with a test that timed out or crashed. A timed-out test's abandoned thread may still be
    // running, using one of the test groups, and after a crash their state may be corrupted, so the caller should
    // then end the process, as the console runner does, without destroying this window and its tests.
    bool WasRunAborted() const;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_Win32GUI_MainWindowH
