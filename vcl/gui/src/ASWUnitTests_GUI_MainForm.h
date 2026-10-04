/* **************************************************************************
ASWUnitTests_GUI_MainForm.h
Author: Anthony S. West - ASW Software

The VCL GUI runner's main window.

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

#ifndef ASWUnitTests_GUI_MainFormH
#define ASWUnitTests_GUI_MainFormH
//---------------------------------------------------------------------------
#include <System.Actions.hpp>
#include <System.Classes.hpp>
#include <System.ImageList.hpp>
#include <Vcl.ActnList.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ImgList.hpp>
#include <Vcl.Menus.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
// IDE includes above here
//---------------------------------------------------------------------------
#include <chrono>
#include <cstddef>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_CommandLine.h"
#include "ASWUnitTests_GUI_Layout.h"
#include "ASWUnitTests_GUI_RunObserver.h"
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFormASWUnitTestsGUIMain
/////////////////////////////////////////////////////////////////////////////
class TFormASWUnitTestsGUIMain : public TForm
{
private:
    typedef TForm inherited;

__published: // IDE-managed Components
    TAction* Act_About;
    TAction* Act_CommandLineOptions;
    TAction* Act_CopyDetails;
    TAction* Act_Exit;
    TAction* Act_ExportJUnitReport;
    TAction* Act_FocusFilter;
    TAction* Act_ResetLayout;
    TAction* Act_RunFailed;
    TAction* Act_RunSelected;
    TAction* Act_SelectAll;
    TAction* Act_SelectFailed;
    TAction* Act_SelectNone;
    TAction* Act_Stop;
    TActionList* AL_Main;
    TButton* Btn_CopyDetails;
    TButton* Btn_RunFailed;
    TButton* Btn_RunSelected;
    TButton* Btn_SelectAll;
    TButton* Btn_SelectFailed;
    TButton* Btn_SelectNone;
    TButton* Btn_Stop;
    TEdit* Edt_Filter;
    TImageList* IL_Status;
    TLabel* Lbl_Filter;
    TListView* LV_Failures;
    TMemo* Mem_Detail;
    TMenuItem* MI_File;
    TMenuItem* MI_FileExit;
    TMenuItem* MI_FileExportJUnitReport;
    TMenuItem* MI_FileSeparator1;
    TMenuItem* MI_Help;
    TMenuItem* MI_HelpAbout;
    TMenuItem* MI_HelpCommandLineOptions;
    TMenuItem* MI_Run;
    TMenuItem* MI_RunFailed;
    TMenuItem* MI_RunSelected;
    TMenuItem* MI_RunStop;
    TMenuItem* MI_Tests;
    TMenuItem* MI_TestsCopyDetails;
    TMenuItem* MI_TestsFilter;
    TMenuItem* MI_TestsSelectAll;
    TMenuItem* MI_TestsSelectFailed;
    TMenuItem* MI_TestsSelectNone;
    TMenuItem* MI_TestsSeparator1;
    TMenuItem* MI_View;
    TMenuItem* MI_ViewResetLayout;
    TMainMenu* MM_Main;
    TProgressBar* PB_Progress;
    TPageControl* PC_Results;
    TPanel* Pnl_Results;
    TPanel* Pnl_Toolbar;
    TRichEdit* RE_Log;
    TStatusBar* SB_Status;
    TSaveDialog* SD_JUnitReport;
    TSplitter* Spl_Detail;
    TSplitter* Spl_Tests;
    TTabSheet* TS_Failures;
    TTabSheet* TS_Log;
    TTreeView* TV_Tests;
    void __fastcall Act_AboutExecute(TObject* Sender);
    void __fastcall Act_CommandLineOptionsExecute(TObject* Sender);
    void __fastcall Act_CopyDetailsExecute(TObject* Sender);
    void __fastcall Act_ExitExecute(TObject* Sender);
    void __fastcall Act_ExportJUnitReportExecute(TObject* Sender);
    void __fastcall Act_FocusFilterExecute(TObject* Sender);
    void __fastcall Act_ResetLayoutExecute(TObject* Sender);
    void __fastcall Act_RunFailedExecute(TObject* Sender);
    void __fastcall Act_RunSelectedExecute(TObject* Sender);
    void __fastcall Act_SelectAllExecute(TObject* Sender);
    void __fastcall Act_SelectFailedExecute(TObject* Sender);
    void __fastcall Act_SelectNoneExecute(TObject* Sender);
    void __fastcall Act_StopExecute(TObject* Sender);
    void __fastcall Edt_FilterChange(TObject* Sender);
    void __fastcall FormClose(TObject* Sender, TCloseAction& Action);
    void __fastcall FormCloseQuery(TObject* Sender, bool& CanClose);
    void __fastcall LV_FailuresCustomDrawItem(TCustomListView* Sender, TListItem* Item, TCustomDrawState State,
        bool& DefaultDraw);
    void __fastcall LV_FailuresDblClick(TObject* Sender);
    void __fastcall TV_TestsChange(TObject* Sender, TTreeNode* Node);
    void __fastcall TV_TestsCheckStateChanging(TCustomTreeView* Sender, TTreeNode* Node,
        TNodeCheckState NewCheckState, TNodeCheckState OldCheckState, bool& AllowChange);

private: // User declarations
    // Declared before m_Handler, so it's destroyed after the handler that points at it.
    ASWUnitTests::TGUIRunObserver m_Observer;
    std::unique_ptr<ASWUnitTests::TTestHandler> m_Handler;
    ASWUnitTests::TGUIOptions m_Options;
    ASWUnitTests::TGUITestList m_TestList;
    // Set when the window was asked to close during a run, which then closes it once the run has stopped.
    bool m_CloseAfterRun;
    // The test that has started but not yet finished, if any.
    std::optional<size_t> m_CurrentTestIndex;
    // The layout from the DFM, at 96 DPI, captured before the saved one is applied, for Reset Layout.
    ASWUnitTests::TGUILayout m_DefaultLayout;
    // By index into m_TestList.GroupNames(); nullptr while hidden. Read through GroupNode(), which rebuilds it
    // when the tree's window has been recreated.
    std::vector<TTreeNode*> m_GroupNodes;
    // Set when the command line doesn't choose the tests itself (no --run, --filter, or partition options), so
    // the saved test selection is restored at start and saved again at close.
    bool m_KeepSelection;
    // The exit code the console runner would have returned for the latest run; see ExitCode().
    int m_LastRunExitCode;
    // The latest run's JUnit report entries, for --report-junit and Export JUnit Report. Unset until a run
    // finishes, and after a run an unhandled exception ended, which has no report, as in the console runner.
    std::optional<std::vector<ASWUnitTests::TJUnitTestCase> > m_LastRunJUnitTestCases;
    std::optional<std::chrono::steady_clock::time_point> m_RunEnd;
    size_t m_RunFinishedCount;
    bool m_Running;
    std::optional<std::chrono::steady_clock::time_point> m_RunStart;
    size_t m_RunTotalCount;
    std::string m_StatusMessage;
    // True while the tree's check boxes are being set from m_TestList, so TV_TestsCheckStateChanging() lets
    // those changes through instead of treating them as clicks.
    bool m_SyncingChecks;
    // By index into m_TestList; nullptr while hidden. Read through TestNode(), like m_GroupNodes.
    std::vector<TTreeNode*> m_TestNodes;
    // Set when the tree's window is destroyed, which frees every TTreeNode, so m_GroupNodes and m_TestNodes
    // point at freed nodes until RebuildNodeCache() finds the ones the recreated window loaded in their place.
    bool m_TreeNodesStale;
    TWndMethod m_TreeViewWindowProc; // TV_Tests' own WindowProc, which TV_TestsWindowProc() passes messages on to.

private:
    // Adds a failed or skipped test to the Failures & Skips list.
    void AddFailureItem(size_t testIndex);
    // Appends log text to the Log tab, coloring failure and skip lines.
    void AppendLog(std::string const& text);
    // Shows only the tests matching the filter box, then rebuilds the tree.
    void ApplyFilterBox();
    // Sets the test tree width and detail pane height that 'layout' has, fitted to the window's current size.
    void ApplyPanelSizes(ASWUnitTests::TGUILayout const& layout);
    void BuildTestTree();
    void CenterOnMonitor(int width, int height);
    void CreateStatusImages();
    // The window's layout as it is now, for saving.
    ASWUnitTests::TGUILayout CurrentLayout();
    // Ends a run that an unhandled exception escaped from, described by 'description'.
    void FailRunOnUnhandledException(std::string const& description);
    // The group's tree node, or nullptr while it's hidden.
    TTreeNode* GroupNode(size_t groupIndex);
    // Restores the layout SaveLayout() saved, if any; see the definition.
    void LoadLayout();
    // Restores the test selection SaveSelection() saved, if any, noting what it restored in the log.
    void LoadSelection();
    void OnTestFinished(ASWUnitTests::TTestCaseRecord const& record, std::string const& testLog);
    void OnTestStarted(std::string const& groupName, std::string const& testName);
    // Refills m_GroupNodes and m_TestNodes from the tree's nodes, by the index each one holds in its Data.
    void RebuildNodeCache();
    void RefreshAllNodeImages();
    // Deletes the saved layout, for --layout-reset, noting the outcome in the log.
    void ResetSavedLayout();
    // Runs the checked tests that are shown, as Run Selected and --run do.
    void RunCheckedTests();
    // Runs an event handler's body, reporting any C++ exception from it by name; see the definition.
    void RunGuarded(char const* where, std::function<void()> const& body);
    // Runs the tests 'filter' matches, updating the tree and status as each one starts and finishes.
    void RunTests(ASWUnitTests::TestFilter const& filter, std::string const& filterDescription);
    // Saves the window's layout for the next start, ignoring any error.
    void SaveLayout();
    // Saves which tests are checked and shown for the next start, ignoring any error.
    void SaveSelection();
    void SetRunning(bool running);
    void SyncTreeChecks();
    // The test's tree node, or nullptr while it's hidden.
    TTreeNode* TestNode(size_t testIndex);
    // Watches for the tree's window being destroyed; see m_TreeNodesStale.
    void __fastcall TV_TestsWindowProc(TMessage& message);
    void UpdateCaption();
    // Shows the selected test's or group's details in the detail pane.
    void UpdateDetail();
    void UpdateStatusBar();
    // Updates a test node's status image, and its group node's, if they're shown.
    void UpdateTestNodeImages(size_t testIndex);
    // Writes the latest run's JUnit report to 'path', noting the outcome in the log. Returns false, with
    // 'error' saying why, if it couldn't be written.
    bool WriteJUnitReport(System::UnicodeString const& path, std::string& error);

public: // User declarations
    __fastcall TFormASWUnitTestsGUIMain(TComponent* Owner);
    __fastcall ~TFormASWUnitTestsGUIMain();

    // The process exit code: with --exit, what the console runner would have returned for the --run run;
    // otherwise ExitCode_Success, since the GUI was used interactively.
    int ExitCode() const;
    // Loads the registered tests and applies 'options' (see WinMain()). Call once, right after creating the form.
    void Start(ASWUnitTests::TGUIOptions const& options);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormASWUnitTestsGUIMain* FormASWUnitTestsGUIMain;
//---------------------------------------------------------------------------
#endif
