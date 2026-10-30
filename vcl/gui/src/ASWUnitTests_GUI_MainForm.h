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
#include "ASWUnitTests_GUI_Shuffle.h"
#include "ASWUnitTests_GUI_TestList.h"
#include "ASWUnitTests_GUI_Theme.h"
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
    TAction* Act_CollapseAll;
    TAction* Act_CommandLineOptions;
    TAction* Act_CopyDetails;
    TAction* Act_Exit;
    TAction* Act_ExpandAll;
    TAction* Act_ExportJUnitReport;
    TAction* Act_FocusFilter;
    TAction* Act_OpenThemesFolder;
    TAction* Act_ResetLayout;
    TAction* Act_RunFailed;
    TAction* Act_RunSelected;
    TAction* Act_SelectAll;
    TAction* Act_SelectFailed;
    TAction* Act_SelectNone;
    TAction* Act_ShuffleOrder;
    TAction* Act_ShuffleSeed;
    TAction* Act_Stop;
    TAction* Act_ThemeDark;
    TAction* Act_ThemeLight;
    TAction* Act_ThemeWindows;
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
    TMenuItem* MI_Options;
    TMenuItem* MI_OptionsShuffleOrder;
    TMenuItem* MI_OptionsShuffleSeed;
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
    TMenuItem* MI_ViewCollapseAll;
    TMenuItem* MI_ViewDarkTheme;
    TMenuItem* MI_ViewDarkThemeOpenFolder;
    TMenuItem* MI_ViewDarkThemeSeparator1;
    TMenuItem* MI_ViewExpandAll;
    TMenuItem* MI_ViewResetLayout;
    TMenuItem* MI_ViewSeparator1;
    TMenuItem* MI_ViewSeparator2;
    TMenuItem* MI_ViewTheme;
    TMenuItem* MI_ViewThemeDark;
    TMenuItem* MI_ViewThemeLight;
    TMenuItem* MI_ViewThemeSeparator1;
    TMenuItem* MI_ViewThemeWindows;
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
    void __fastcall Act_CollapseAllExecute(TObject* Sender);
    void __fastcall Act_CommandLineOptionsExecute(TObject* Sender);
    void __fastcall Act_CopyDetailsExecute(TObject* Sender);
    void __fastcall Act_ExitExecute(TObject* Sender);
    void __fastcall Act_ExpandAllExecute(TObject* Sender);
    void __fastcall Act_ExportJUnitReportExecute(TObject* Sender);
    void __fastcall Act_FocusFilterExecute(TObject* Sender);
    void __fastcall Act_OpenThemesFolderExecute(TObject* Sender);
    void __fastcall Act_ResetLayoutExecute(TObject* Sender);
    void __fastcall Act_RunFailedExecute(TObject* Sender);
    void __fastcall Act_RunSelectedExecute(TObject* Sender);
    void __fastcall Act_SelectAllExecute(TObject* Sender);
    void __fastcall Act_SelectFailedExecute(TObject* Sender);
    void __fastcall Act_SelectNoneExecute(TObject* Sender);
    void __fastcall Act_ShuffleOrderExecute(TObject* Sender);
    void __fastcall Act_ShuffleSeedExecute(TObject* Sender);
    void __fastcall Act_StopExecute(TObject* Sender);
    void __fastcall Act_ThemeDarkExecute(TObject* Sender);
    void __fastcall Act_ThemeLightExecute(TObject* Sender);
    void __fastcall Act_ThemeWindowsExecute(TObject* Sender);
    void __fastcall Edt_FilterChange(TObject* Sender);
    void __fastcall FormClose(TObject* Sender, TCloseAction& Action);
    void __fastcall FormCloseQuery(TObject* Sender, bool& CanClose);
    void __fastcall LV_FailuresCustomDrawItem(TCustomListView* Sender, TListItem* Item, TCustomDrawState State,
        bool& DefaultDraw);
    void __fastcall LV_FailuresDblClick(TObject* Sender);
    void __fastcall MI_ViewDarkThemeClick(TObject* Sender);
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
    // The VCL style m_DarkTheme is (see LoadGUIDarkStyle()), which ApplyTheme() uses while the theme is dark.
    System::UnicodeString m_DarkStyle;
    // The View > Theme > Dark Theme choice: GUIDefaultDarkThemeName() or a VCL style file's name.
    std::string m_DarkTheme;
    // The items FillDarkThemeMenu() added to View > Theme > Dark Theme, one per dark theme, by index into
    // m_DarkThemeNames, which each one's Tag is.
    std::vector<TMenuItem*> m_DarkThemeItems;
    std::vector<std::string> m_DarkThemeNames;
    // The layout from the DFM, at 96 DPI, captured before the saved one is applied, for Reset Layout.
    ASWUnitTests::TGUILayout m_DefaultLayout;
    TWndMethod m_FormWindowProc; // The form's own WindowProc, which FormWindowProc() passes messages on to.
    // Set when the command line doesn't choose the tests itself (no --run, --filter, or partition options), so
    // the saved test selection is restored at start and saved again at close.
    bool m_KeepSelection;
    // The exit code the console runner would have returned for the latest run; see ExitCode().
    int m_LastRunExitCode;
    // The latest run's JUnit report entries, for --report-junit and Export JUnit Report. Unset until a run
    // finishes, and after a run an unhandled exception ended, which has no report, as in the console runner.
    std::optional<std::vector<ASWUnitTests::TJUnitTestCase> > m_LastRunJUnitTestCases;
    // Set when a run ends with a test that timed out or crashed, after which no further run can start; see
    // WasRunAborted().
    bool m_RunAborted;
    std::optional<std::chrono::steady_clock::time_point> m_RunEnd;
    size_t m_RunFinishedCount;
    bool m_Running;
    std::optional<std::chrono::steady_clock::time_point> m_RunStart;
    size_t m_RunTotalCount;
    // True while Start() puts --filter's pattern in the filter box, so Edt_FilterChange() leaves building the
    // tree to Start(), which then builds it once.
    bool m_SettingFilterText;
    // Whether runs are shuffled, and with which seed; starts from --shuffle and --shuffle-seed.
    ASWUnitTests::TGUIShuffle m_Shuffle;
    std::string m_StatusMessage;
    // True while the tree's check boxes are being set from m_TestList, so TV_TestsCheckStateChanging() lets
    // those changes through instead of treating them as clicks.
    bool m_SyncingChecks;
    // By index into m_TestList; nullptr while hidden. Read through TestNode(), which rebuilds it when the
    // tree's window has been recreated.
    std::vector<TTreeNode*> m_TestNodes;
    // The View > Theme menu's choice.
    ASWUnitTests::EGUITheme m_Theme;
    // Set when Windows' setting changed during a run, so ApplyTheme() applies it once the run has ended.
    bool m_ThemeChangePending;
    // Set when the tree's window is destroyed, which frees every TTreeNode, so m_TestNodes points at freed
    // nodes until RebuildNodeCache() finds the ones the recreated window loaded in their place.
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
    // Sets the style for m_Theme and Windows' current setting; see the definition.
    void ApplyTheme();
    void BuildTestTree();
    void CenterOnMonitor(int width, int height);
    void CreateStatusImages();
    // The window's layout as it is now, for saving.
    ASWUnitTests::TGUILayout CurrentLayout();
    // Chooses the dark theme of the View > Theme > Dark Theme item 'Sender'.
    void __fastcall DarkThemeItemClick(TObject* Sender);
    // Ends a run that an unhandled exception escaped from, described by 'description'.
    void FailRunOnUnhandledException(std::string const& description);
    // Lists Carbon and the VCL style files in View > Theme > Dark Theme, as it opens; see the definition.
    void FillDarkThemeMenu();
    // Follows the theme as it changes; see the definition.
    void __fastcall FormWindowProc(TMessage& message);
    // Restores the layout SaveLayout() saved, if any; see the definition.
    void LoadLayout();
    // Restores the test selection SaveSelection() saved, if any, noting what it restored in the log.
    void LoadSelection();
    void OnTestFinished(ASWUnitTests::TTestCaseRecord const& record, std::string const& testLog);
    void OnTestStarted(std::string const& groupName, std::string const& testName);
    // Refills m_TestNodes from the tree's test nodes, by the index each one holds in its Data.
    void RebuildNodeCache();
    // Colors the log's lines again, for the style now active.
    void RecolorLog();
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
    // Chooses View > Theme > Dark Theme's 'name', saving it, unless --layout-ignore, and applies it; see the
    // definition.
    void SetDarkTheme(std::string const& name);
    // Sets a tree node's status image from m_TestList: a test's own status, or a group's or suite's, from its
    // tests.
    void SetNodeImage(TTreeNode* node);
    // Sets the progress bar's position so it's also drawn in the error state; see the definition.
    void SetProgressPosition(int position);
    void SetRunning(bool running);
    // Chooses the theme, as the View > Theme menu does: saves it, unless --layout-ignore, and applies it.
    void SetTheme(ASWUnitTests::EGUITheme theme);
    // Shows m_Shuffle's state: Run in Shuffled Order's check mark, and the status bar.
    void ShowShuffleState();
    // Shows m_Theme as the View > Theme menu's checked item.
    void ShowThemeState();
    void SyncTreeChecks();
    // The test's tree node, or nullptr while it's hidden.
    TTreeNode* TestNode(size_t testIndex);
    // Watches for the tree's window being destroyed; see m_TreeNodesStale.
    void __fastcall TV_TestsWindowProc(TMessage& message);
    void UpdateCaption();
    // Enables or disables every action, and the filter box, by GUICommandStates()'s rules.
    void UpdateCommands();
    // Shows the selected test's, group's or suite's details in the detail pane.
    void UpdateDetail();
    void UpdateStatusBar();
    // Updates a test node's status image, and those of its group's and suites' nodes, if they're shown.
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
    // Loads the registered tests and applies 'options' (see WinMain()), and shows 'theme' and 'darkTheme', the
    // choices WinMain() applied, in the View > Theme menu, with 'darkStyle', the VCL style LoadGUIDarkStyle() gave
    // for 'darkTheme', and the problems it found in the log. Call once, right after creating the form.
    void Start(ASWUnitTests::TGUIOptions const& options, ASWUnitTests::EGUITheme theme, std::string const& darkTheme,
        System::UnicodeString const& darkStyle, std::vector<std::string> const& themeProblems);
    // True once a run has ended with a test that timed out or crashed. A timed-out test's abandoned thread may then
    // still be running, using its group, so WinMain() ends the process without destroying the groups, as the console
    // runner does.
    bool WasRunAborted() const;
};
//---------------------------------------------------------------------------
extern PACKAGE TFormASWUnitTestsGUIMain* FormASWUnitTestsGUIMain;
//---------------------------------------------------------------------------
#endif
