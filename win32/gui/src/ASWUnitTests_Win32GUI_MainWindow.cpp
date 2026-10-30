/* **************************************************************************
ASWUnitTests_Win32GUI_MainWindow.cpp
Author: Anthony S. West - ASW Software

The Win32 GUI runner's main window.

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
#include "ASWUnitTests_Win32GUI_MainWindow.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_INIDocument.h"
#include "ASWUnitTests_GUI_Selection.h"
#include "ASWUnitTests_GUI_Theme.h"
#include "ASWUnitTests_GUI_Tree.h"
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_MainLayout.h"
#include "ASWUnitTests_Win32GUI_MessageDialog.h"
#include "ASWUnitTests_Win32GUI_ShuffleSeedDialog.h"
#include "ASWUnitTests_Win32GUI_System.h"
#include "ASWUnitTests_Win32GUI_TextDialog.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The command ID of View > Theme > Dark Theme's first theme, Carbon, after the EWin32GUICommand IDs; each of the
// others' is the next (see TWin32GUIMainWindow::m_DarkThemeNames), up to MaxDarkThemes of them.
WORD const FirstDarkThemeCommand = 2000;
wchar_t const MainWindowClassName[] = L"ASWUnitTests_Win32GUI_Main";
// The most themes View > Theme > Dark Theme lists, Carbon included.
std::size_t const MaxDarkThemes = 1000;
// Posted by PostToUI(), so the message loop runs the queued actions.
UINT const RunQueuedMessage = WM_APP + 1;
// The title of the GUI's windows, as the VCL GUI runner's Application->Title.
char const Title[] = "ASWUnitTests";

// Every command, for enabling them all.
EWin32GUICommand const AllCommands[] = {
    EWin32GUICommand::About,
    EWin32GUICommand::CollapseAll,
    EWin32GUICommand::CommandLineOptions,
    EWin32GUICommand::CopyDetails,
    EWin32GUICommand::Exit,
    EWin32GUICommand::ExpandAll,
    EWin32GUICommand::ExportJUnitReport,
    EWin32GUICommand::FocusFilter,
    EWin32GUICommand::OpenThemesFolder,
    EWin32GUICommand::ResetLayout,
    EWin32GUICommand::RunFailed,
    EWin32GUICommand::RunSelected,
    EWin32GUICommand::SaveThemeTemplate,
    EWin32GUICommand::SelectAll,
    EWin32GUICommand::SelectFailed,
    EWin32GUICommand::SelectNone,
    EWin32GUICommand::ShuffleOrder,
    EWin32GUICommand::ShuffleSeed,
    EWin32GUICommand::Stop,
    EWin32GUICommand::ThemeDark,
    EWin32GUICommand::ThemeLight,
    EWin32GUICommand::ThemeWindows,
};

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TRedrawScope
//
// Stops a control from redrawing for the lifetime of this object, e.g.
// while the test tree is rebuilt, then redraws it once, even if an
// exception leaves the scope.
/////////////////////////////////////////////////////////////////////////////
class TRedrawScope
{
private:
    HWND const m_Control;

public:
    explicit TRedrawScope(HWND control);
    ~TRedrawScope();

    TRedrawScope(TRedrawScope const&) = delete;
    TRedrawScope& operator=(TRedrawScope const&) = delete;
};


/////////////////////////////////////////////////////////////////////////////
// TScopedFlag
//
// Sets a bool for the lifetime of this object, clearing it again even if
// an exception leaves the scope.
/////////////////////////////////////////////////////////////////////////////
class TScopedFlag
{
private:
    bool& m_Flag;

public:
    explicit TScopedFlag(bool& flag);
    ~TScopedFlag();

    TScopedFlag(TScopedFlag const&) = delete;
    TScopedFlag& operator=(TScopedFlag const&) = delete;
};

//---------------------------------------------------------------------------

BOOL CALLBACK AddMonitorWorkArea(HMONITOR monitor, HDC context, LPRECT bounds, LPARAM workAreas);
void AppendCommand(HMENU menu, EWin32GUICommand command, wchar_t const* caption);
HMENU AppendSubmenu(HMENU menuBar, wchar_t const* caption);
std::optional<std::string> AskSaveFilePath(HWND owner, wchar_t const* title, wchar_t const* filter,
    wchar_t const* defaultExtension, std::string const& initialPath);
char const* CommandName(EWin32GUICommand command);
bool IsCommandEnabled(TGUICommandStates const& states, EWin32GUICommand command);
COLORREF LogLineColor(std::string const& line, TWin32GUITheme const& theme);
std::vector<TGUIRect> MonitorWorkAreas();
HTREEITEM NextTreeItem(HWND tree, HTREEITEM item);
MONITORINFO PrimaryMonitorInfo();
HTREEITEM RelatedTreeItem(HWND tree, HTREEITEM item, UINT relation);
UINT StateImageIndex(TGUICheckState state);
TGUIRect ToGUIRect(RECT const& rect);
TGUINodeRef TreeItemRef(HWND tree, HTREEITEM item);

//---------------------------------------------------------------------------
/*
    AddMonitorWorkArea

    EnumDisplayMonitors()'s callback for MonitorWorkAreas(): adds the monitor's work area to 'workAreas', a
    std::vector<TGUIRect>. No exception may leave it, so one that would ends the enumeration.
*/
BOOL CALLBACK AddMonitorWorkArea(HMONITOR monitor, HDC /*context*/, LPRECT /*bounds*/, LPARAM workAreas)
{
    try
    {
        MONITORINFO info = {};
        info.cbSize = sizeof(info);

        if (GetMonitorInfoW(monitor, &info) != 0)
            reinterpret_cast<std::vector<TGUIRect>*>(workAreas)->push_back(ToGUIRect(info.rcWork));

        return TRUE;
    }
    catch (...)
    {
        return FALSE;
    }
}
//---------------------------------------------------------------------------
void AppendCommand(HMENU menu, EWin32GUICommand command, wchar_t const* caption)
{
    AppendMenuW(menu, MF_STRING, static_cast<UINT_PTR>(command), caption);
}
//---------------------------------------------------------------------------
HMENU AppendSubmenu(HMENU menuBar, wchar_t const* caption)
{
    HMENU const submenu = CreatePopupMenu();

    if (submenu == nullptr)
        throw std::runtime_error("Couldn't create the menu");

    AppendMenuW(menuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(submenu), caption);
    return submenu;
}
//---------------------------------------------------------------------------
/*
    AskSaveFilePath

    The path chosen in Windows' Save dialog, or nullopt if it was canceled. The dialog starts at 'initialPath',
    given its folder and its file name separately, so a folder that doesn't exist (any more) only means it starts
    in its default folder. Throws std::runtime_error if the dialog can't be shown.
*/
std::optional<std::string> AskSaveFilePath(HWND owner, wchar_t const* title, wchar_t const* filter,
    wchar_t const* defaultExtension, std::string const& initialPath)
{
    std::wstring const initial = UTF8ToWide(initialPath);
    std::size_t const separator = initial.find_last_of(L"\\/");
    std::wstring const initialFolder = separator == std::wstring::npos ? std::wstring() :
            initial.substr(0, separator + 1);
    std::wstring const initialName = separator == std::wstring::npos ? initial : initial.substr(separator + 1);

    // Room for the longest path Windows allows, plus the terminating null.
    std::vector<wchar_t> fileName(32768, L'\0');
    std::copy(initialName.begin(), initialName.begin() +
        static_cast<std::ptrdiff_t>((std::min)(initialName.size(), fileName.size() - 1)), fileName.begin());

    OPENFILENAMEW dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = filter;
    dialog.lpstrFile = fileName.data();
    dialog.nMaxFile = static_cast<DWORD>(fileName.size());
    dialog.lpstrInitialDir = initialFolder.empty() ? nullptr : initialFolder.c_str();
    dialog.lpstrTitle = title;
    dialog.Flags = OFN_ENABLESIZING | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
    dialog.lpstrDefExt = defaultExtension;

    if (GetSaveFileNameW(&dialog) == 0)
    {
        DWORD const error = CommDlgExtendedError();

        if (error != 0)
        {
            throw std::runtime_error("Couldn't show the " + WideToUTF8(title) + " dialog (error " +
                std::to_string(error) + ")");
        }

        return std::nullopt; // Canceled.
    }

    return WideToUTF8(fileName.data());
}
//---------------------------------------------------------------------------
/*
    CommandName

    The command's name in an error report, as the VCL GUI runner's RunGuarded() calls it.
*/
char const* CommandName(EWin32GUICommand command)
{
    switch (command)
    {
        case EWin32GUICommand::About:
            return "About";

        case EWin32GUICommand::CollapseAll:
            return "Collapse All";

        case EWin32GUICommand::CommandLineOptions:
            return "Command Line Options";

        case EWin32GUICommand::CopyDetails:
            return "Copy Details";

        case EWin32GUICommand::Exit:
            return "Exit";

        case EWin32GUICommand::ExpandAll:
            return "Expand All";

        case EWin32GUICommand::ExportJUnitReport:
            return "Export JUnit Report";

        case EWin32GUICommand::FocusFilter:
            return "Filter";

        case EWin32GUICommand::OpenThemesFolder:
            return "Theme > Dark Theme > Open Themes Folder";

        case EWin32GUICommand::ResetLayout:
            return "Reset Layout";

        case EWin32GUICommand::RunFailed:
            return "Run Failed";

        case EWin32GUICommand::RunSelected:
            return "Run Selected";

        case EWin32GUICommand::SaveThemeTemplate:
            return "Theme > Dark Theme > Save Theme Template";

        case EWin32GUICommand::SelectAll:
            return "Select All";

        case EWin32GUICommand::SelectFailed:
            return "Select Failed";

        case EWin32GUICommand::SelectNone:
            return "Select None";

        case EWin32GUICommand::ShuffleOrder:
            return "Run in Shuffled Order";

        case EWin32GUICommand::ShuffleSeed:
            return "Shuffle Seed";

        case EWin32GUICommand::Stop:
            return "Stop";

        case EWin32GUICommand::ThemeDark:
            return "Theme > Dark";

        case EWin32GUICommand::ThemeLight:
            return "Theme > Light";

        case EWin32GUICommand::ThemeWindows:
            return "Theme > Use Windows Setting";
    }

    return "a command";
}
//---------------------------------------------------------------------------
bool IsCommandEnabled(TGUICommandStates const& states, EWin32GUICommand command)
{
    switch (command)
    {
        case EWin32GUICommand::About:
            return states.About;

        case EWin32GUICommand::CollapseAll:
            return states.CollapseAll;

        case EWin32GUICommand::CommandLineOptions:
            return states.CommandLineOptions;

        case EWin32GUICommand::CopyDetails:
            return states.CopyDetails;

        case EWin32GUICommand::Exit:
            return states.Exit;

        case EWin32GUICommand::ExpandAll:
            return states.ExpandAll;

        case EWin32GUICommand::ExportJUnitReport:
            return states.ExportJUnitReport;

        case EWin32GUICommand::FocusFilter:
            return states.FocusFilter;

        case EWin32GUICommand::OpenThemesFolder:
            return states.Theme;

        case EWin32GUICommand::ResetLayout:
            return states.ResetLayout;

        case EWin32GUICommand::RunFailed:
            return states.RunFailed;

        case EWin32GUICommand::RunSelected:
            return states.RunSelected;

        case EWin32GUICommand::SaveThemeTemplate:
            return states.Theme;

        case EWin32GUICommand::SelectAll:
            return states.SelectAll;

        case EWin32GUICommand::SelectFailed:
            return states.SelectFailed;

        case EWin32GUICommand::SelectNone:
            return states.SelectNone;

        case EWin32GUICommand::ShuffleOrder:
            return states.ShuffleOrder;

        case EWin32GUICommand::ShuffleSeed:
            return states.ShuffleSeed;

        case EWin32GUICommand::Stop:
            return states.Stop;

        case EWin32GUICommand::ThemeDark:
        case EWin32GUICommand::ThemeLight:
        case EWin32GUICommand::ThemeWindows:
            return states.Theme;
    }

    return false;
}
//---------------------------------------------------------------------------
/*
    LogLineColor

    The log's color for one line of the framework's log output (see ClassifyGUILogLine()) in 'theme', as the
    VCL GUI runner's: normal lines in the text color, explicitly, so a dark log's aren't black.
*/
COLORREF LogLineColor(std::string const& line, TWin32GUITheme const& theme)
{
    switch (ClassifyGUILogLine(line))
    {
        case TGUILogLineKind::Failure:
            return theme.FailureTextColor();

        case TGUILogLineKind::Skip:
            return theme.SkipTextColor();

        case TGUILogLineKind::Normal:
            break;
    }

    return theme.WindowTextColor();
}
//---------------------------------------------------------------------------
/*
    MonitorWorkAreas

    Every monitor's work area (the part the taskbar and other docked bars leave free), in screen pixels.
*/
std::vector<TGUIRect> MonitorWorkAreas()
{
    std::vector<TGUIRect> workAreas;
    EnumDisplayMonitors(nullptr, nullptr, &AddMonitorWorkArea, reinterpret_cast<LPARAM>(&workAreas));

    return workAreas;
}
//---------------------------------------------------------------------------
/*
    NextTreeItem

    The node after 'item' in the tree's full order, expanded or not: its first child, or else the next sibling
    of it or of the nearest of its parents that has one; nullptr after the last node. The first node is
    RelatedTreeItem(tree, nullptr, TVGN_ROOT).
*/
HTREEITEM NextTreeItem(HWND tree, HTREEITEM item)
{
    HTREEITEM const child = RelatedTreeItem(tree, item, TVGN_CHILD);

    if (child != nullptr)
        return child;

    for (HTREEITEM node = item; node != nullptr; node = RelatedTreeItem(tree, node, TVGN_PARENT))
    {
        HTREEITEM const sibling = RelatedTreeItem(tree, node, TVGN_NEXT);

        if (sibling != nullptr)
            return sibling;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
/*
    PrimaryMonitorInfo

    The primary monitor's bounds and work area, which window placements' workspace coordinates are relative to.
*/
MONITORINFO PrimaryMonitorInfo()
{
    MONITORINFO info = {};
    info.cbSize = sizeof(info);
    POINT const origin = { 0, 0 };
    GetMonitorInfoW(MonitorFromPoint(origin, MONITOR_DEFAULTTOPRIMARY), &info);

    return info;
}
//---------------------------------------------------------------------------
/*
    RelatedTreeItem

    TVM_GETNEXTITEM: the node 'relation' (e.g. TVGN_CHILD) names, relative to 'item'; nullptr if there's none.
*/
HTREEITEM RelatedTreeItem(HWND tree, HTREEITEM item, UINT relation)
{
    return reinterpret_cast<HTREEITEM>(SendMessageW(tree, TVM_GETNEXTITEM, relation, reinterpret_cast<LPARAM>(item)));
}
//---------------------------------------------------------------------------
/*
    StateImageIndex

    A test tree check box's state image, as TVS_EX_PARTIALCHECKBOXES numbers them.
*/
UINT StateImageIndex(TGUICheckState state)
{
    switch (state)
    {
        case TGUICheckState::Checked:
            return 2;

        case TGUICheckState::Partial:
            return 3;

        case TGUICheckState::Unchecked:
            break;
    }

    return 1;
}
//---------------------------------------------------------------------------
TGUIRect ToGUIRect(RECT const& rect)
{
    return TGUIRect(rect.left, rect.top, rect.right, rect.bottom);
}
//---------------------------------------------------------------------------
/*
    TreeItemRef

    Which suite, group or test a test tree node stands for, from the data BuildTestTree() gave it.
*/
TGUINodeRef TreeItemRef(HWND tree, HTREEITEM item)
{
    TVITEMW data = {};
    data.mask = TVIF_HANDLE | TVIF_PARAM;
    data.hItem = item;
    SendMessageW(tree, TVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&data));

    return DecodeGUINodeRef(static_cast<std::uintptr_t>(data.lParam));
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TRedrawScope
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TRedrawScope::TRedrawScope(HWND control)
    : m_Control(control)
{
    SendMessageW(m_Control, WM_SETREDRAW, FALSE, 0);
}
//---------------------------------------------------------------------------
TRedrawScope::~TRedrawScope()
{
    SendMessageW(m_Control, WM_SETREDRAW, TRUE, 0);
    RedrawWindow(m_Control, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TScopedFlag
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TScopedFlag::TScopedFlag(bool& flag)
    : m_Flag(flag)
{
    m_Flag = true;
}
//---------------------------------------------------------------------------
TScopedFlag::~TScopedFlag()
{
    m_Flag = false;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWin32GUIMainWindow
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
// The hints are the actions', with their shortcut added as the VCL adds it to an action's control's hint.
TWin32GUIMainWindow::TToolbarButton const TWin32GUIMainWindow::ToolbarButtons[7] = {
    { EWin32GUICommand::RunSelected, L"R&un Selected", 8, 110, L"Run the checked tests that are shown (F9)",
      &TWin32GUIMainWindow::m_Btn_RunSelected },
    { EWin32GUICommand::RunFailed, L"Run Faile&d", 124, 100, L"Rerun the tests that failed in the last run",
      &TWin32GUIMainWindow::m_Btn_RunFailed },
    { EWin32GUICommand::Stop, L"&Stop", 230, 80, L"Stop the run after the current test",
      &TWin32GUIMainWindow::m_Btn_Stop },
    { EWin32GUICommand::SelectAll, L"Select &All", 328, 90, L"Check every shown test",
      &TWin32GUIMainWindow::m_Btn_SelectAll },
    { EWin32GUICommand::SelectNone, L"Select &None", 424, 90, L"Uncheck every shown test",
      &TWin32GUIMainWindow::m_Btn_SelectNone },
    { EWin32GUICommand::SelectFailed, L"Select Fai&led", 520, 90,
      L"Check only the shown tests that failed in the last run", &TWin32GUIMainWindow::m_Btn_SelectFailed },
    { EWin32GUICommand::CopyDetails, L"&Copy Details", 628, 110,
      L"Copy the selected test's details to the clipboard (Ctrl+Shift+C)", &TWin32GUIMainWindow::m_Btn_CopyDetails },
};

//---------------------------------------------------------------------------
TWin32GUIMainWindow::TWin32GUIMainWindow(TWin32GUITheme& theme)
    : m_Btn_CopyDetails(nullptr),
      m_Btn_RunFailed(nullptr),
      m_Btn_RunSelected(nullptr),
      m_Btn_SelectAll(nullptr),
      m_Btn_SelectFailed(nullptr),
      m_Btn_SelectNone(nullptr),
      m_Btn_Stop(nullptr),
      m_Edt_Filter(nullptr),
      m_Lbl_Filter(nullptr),
      m_LV_Failures(nullptr),
      m_Mem_Detail(nullptr),
      m_MI_ViewDarkTheme(nullptr),
      m_MM_Main(nullptr),
      m_PB_Progress(nullptr),
      m_PC_Results(nullptr),
      m_RE_Log(nullptr),
      m_SB_Status(nullptr),
      m_TT_Hints(nullptr),
      m_TV_Tests(nullptr),
      m_Accelerators(nullptr),
      m_CloseAfterRun(false),
      m_Closing(false),
      m_DarkTheme(GUIDefaultDarkThemeName()),
      m_DarkThemeNames(1, GUIDefaultDarkThemeName()),
      m_DetailHeight(TWin32GUIMainSizes::DefaultDetailHeight),
      m_DPI(USER_DEFAULT_SCREEN_DPI),
      m_FocusBeforeRun(nullptr),
      m_FocusedControl(nullptr),
      m_KeepSelection(false),
      m_LastRunExitCode(ExitCode_Success),
      m_OpenMaximized(false),
      m_RichEditLibrary(nullptr),
      m_RunAborted(false),
      m_RunFinishedCount(0),
      m_Running(false),
      m_RunTotalCount(0),
      m_SettingFilterText(false),
      m_StatusImages(nullptr),
      m_StatusMessage("Ready."),
      m_SyncingChecks(false),
      m_TestTreeWidth(TWin32GUIMainSizes::DefaultTestTreeWidth),
      m_Theme(theme),
      m_ThemeChangePending(false),
      m_ThemeChoice(EGUITheme::Windows),
      m_Window(nullptr)
{
}
//---------------------------------------------------------------------------
TWin32GUIMainWindow::~TWin32GUIMainWindow()
{
    // Its tests' groups are destroyed while m_Observer, declared after it, still exists.
    m_Handler.reset();

    if (m_Window != nullptr)
        DestroyWindow(m_Window);
    else if (m_MM_Main != nullptr)
        DestroyMenu(m_MM_Main); // Created, but never given to a window, which would have destroyed it.

    // After the tree that used it.
    if (m_StatusImages != nullptr)
        ImageList_Destroy(m_StatusImages);

    if (m_Accelerators != nullptr)
        DestroyAcceleratorTable(m_Accelerators);

    if (m_RichEditLibrary != nullptr)
        FreeLibrary(m_RichEditLibrary);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::AddFailureItem

    Each row keeps its test's index into m_TestList, for its color and for a double-click.
*/
void TWin32GUIMainWindow::AddFailureItem(std::size_t testIndex)
{
    TTestId const& test = m_TestList.Test(testIndex);
    TGUITestResult const* const result = m_TestList.Result(testIndex);

    std::wstring const columns[] = {
        UTF8ToWide(StatusName(m_TestList.Status(testIndex))),
        UTF8ToWide(test.GroupName + "." + test.TestName),
        UTF8ToWide(result != nullptr ? GUIFirstLine(result->Record.Message) : std::string()),
    };

    LVITEMW item = {};
    item.mask = LVIF_TEXT | LVIF_PARAM;
    item.iItem = static_cast<int>(SendMessageW(m_LV_Failures, LVM_GETITEMCOUNT, 0, 0));
    item.pszText = const_cast<wchar_t*>(columns[0].c_str());
    item.lParam = static_cast<LPARAM>(testIndex);

    int const row = static_cast<int>(SendMessageW(m_LV_Failures, LVM_INSERTITEMW, 0, reinterpret_cast<LPARAM>(&item)));

    if (row < 0)
        throw std::runtime_error("Couldn't add a test to the Failures & Skips list");

    for (int column = 1; column < 3; ++column)
    {
        LVITEMW text = {};
        text.iSubItem = column;
        text.pszText = const_cast<wchar_t*>(columns[column].c_str());
        SendMessageW(m_LV_Failures, LVM_SETITEMTEXTW, static_cast<WPARAM>(row), reinterpret_cast<LPARAM>(&text));
    }
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::AppendLog(std::string const& text)
{
    // One line at a time, so each can have its own color. A trailing piece with no line break (from
    // LogAppend()) is appended as-is, and the next piece continues its line.
    std::size_t start = 0;

    while (start < text.size())
    {
        std::size_t const end = text.find('\n', start);
        bool const endsLine = end != std::string::npos;
        std::string const line = text.substr(start, endsLine ? end - start : std::string::npos);

        // Past the end is clamped to the end, which sidesteps the rich edit counting a line break as one
        // character here but two in GetWindowTextLengthW().
        int const length = GetWindowTextLengthW(m_RE_Log);
        SendMessageW(m_RE_Log, EM_SETSEL, static_cast<WPARAM>(length), length);

        CHARFORMAT2W format = {};
        format.cbSize = sizeof(format);
        format.dwMask = CFM_COLOR;
        format.crTextColor = LogLineColor(line, m_Theme);
        SendMessageW(m_RE_Log, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));

        SendMessageW(m_RE_Log, EM_REPLACESEL, FALSE, reinterpret_cast<LPARAM>(UTF8ToWide(line +
            (endsLine ? "\r\n" : "")).c_str()));

        start = endsLine ? end + 1 : text.size();
    }

    SendMessageW(m_RE_Log, WM_VSCROLL, SB_BOTTOM, 0);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ApplyDPI()
{
    m_UIFont = CreateUIFont(m_DPI);
    m_MonospaceFont = CreateMonospaceFont(m_DPI);

    for (HWND child = GetWindow(m_Window, GW_CHILD); child != nullptr; child = GetWindow(child, GW_HWNDNEXT))
    {
        HFONT const font = child == m_Mem_Detail || child == m_RE_Log ? m_MonospaceFont.Get() : m_UIFont.Get();
        SendMessageW(child, WM_SETFONT, reinterpret_cast<WPARAM>(font), FALSE);
    }

    CreateStatusImages();
    SendMessageW(m_TV_Tests, TVM_SETINDENT, static_cast<WPARAM>(ScaleForDPI(19, m_DPI)), 0);
    // e.g. the dark tree's check boxes and the menus' sizes, for the new DPI.
    m_Theme.Refresh();

    // The status bar's parts' widths, from the .dfm's panels; the last, the status message's, takes the rest of
    // the bar, as VCL's TStatusBar gives its last panel.
    std::vector<int> rightEdges;
    int right = 0;

    for (int const width : GUIStatusPanelWidths())
    {
        right += ScaleForDPI(width, m_DPI);
        rightEdges.push_back(right);
    }

    if (!rightEdges.empty())
        rightEdges.back() = -1;

    SendMessageW(m_SB_Status, SB_SETPARTS, rightEdges.size(), reinterpret_cast<LPARAM>(rightEdges.data()));

    // The Detail column's width is set by LayOut(), to fill the list.
    SendMessageW(m_LV_Failures, LVM_SETCOLUMNWIDTH, 0, ScaleForDPI(70, m_DPI));
    SendMessageW(m_LV_Failures, LVM_SETCOLUMNWIDTH, 1, ScaleForDPI(240, m_DPI));

    LayOut();
    // Every control too, in its new font: resizing one keeps what it had drawn and redraws only the part it
    // gained, which would leave e.g. the filter label's old, smaller text showing.
    RedrawWindow(m_Window, nullptr, nullptr, RDW_ERASE | RDW_FRAME | RDW_INVALIDATE | RDW_ALLCHILDREN);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ApplyFilterBox

    The text matches as GUIFilterBoxFilter() describes.
*/
void TWin32GUIMainWindow::ApplyFilterBox()
{
    m_TestList.SetVisibleFilter(GUIFilterBoxFilter(WindowText(m_Edt_Filter)));

    BuildTestTree();
    UpdateDetail();
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ApplyPanelSizes(TGUILayout const& layout)
{
    // Kept within what the splitters allow, so each can still be dragged and nothing is squeezed out of view.
    RECT client;
    GetClientRect(m_Window, &client);
    int const pixelsPerInch = static_cast<int>(m_DPI);

    if (layout.TestTreeWidth.has_value())
        m_TestTreeWidth = ClampTestTreeWidth(ScaleFrom96(*layout.TestTreeWidth, pixelsPerInch), client.right, m_DPI);

    if (layout.DetailHeight.has_value())
        m_DetailHeight = ClampDetailHeight(ScaleFrom96(*layout.DetailHeight, pixelsPerInch), client.bottom, m_DPI);

    LayOut();
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ApplyTheme

    Dark or light as View > Theme says, or with Use Windows Setting, as Windows' app mode does, but light while
    a high contrast theme is on (see GUIThemeIsDark()). Not during a run, as in the VCL GUI runner; SetRunning()
    calls it again once the run has ended.
*/
void TWin32GUIMainWindow::ApplyTheme()
{
    if (m_Running)
    {
        m_ThemeChangePending = true;
        return;
    }

    m_ThemeChangePending = false;
    bool const dark = GUIThemeIsDark(m_ThemeChoice, WindowsAppsUseDarkTheme(), WindowsHighContrast());

    if (dark == m_Theme.IsDark())
        return;

    m_Theme.SetDark(dark);
    // The tree's status dots are the dark theme's own colors while dark.
    CreateStatusImages();
    RecolorLog();
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::BuildTestTree

    Rebuilds the tree from m_TestList: one node per group, in order, each with one child node per test, and
    one node per suite, holding the nodes of the groups and suites in it. A group's and a suite's node show
    just its own name.
*/
void TWin32GUIMainWindow::BuildTestTree()
{
    {
        // Each step redraws the tree once, when it's done: WM_SETREDRAW doesn't nest, so one step can't
        // bracket the others. Nothing is painted until the window processes messages again anyway.
        TRedrawScope const updating(m_TV_Tests);
        // Each new node starts unchecked, which SyncTreeChecks() below then corrects.
        TScopedFlag const syncing(m_SyncingChecks);

        SendMessageW(m_TV_Tests, TVM_DELETEITEM, 0, reinterpret_cast<LPARAM>(TVI_ROOT));
        m_TestNodes.assign(m_TestList.Count(), nullptr);

        // Each node comes after its parent, so its parent's tree node already exists.
        std::vector<TGUITreeNode> const nodes = BuildGUITree(m_TestList);
        std::vector<HTREEITEM> treeItems;
        treeItems.reserve(nodes.size());

        for (TGUITreeNode const& node : nodes)
        {
            std::wstring text = UTF8ToWide(node.Text);

            TVINSERTSTRUCTW insert = {};
            insert.hParent = node.Parent.has_value() ? treeItems[*node.Parent] : TVI_ROOT;
            insert.hInsertAfter = TVI_LAST;
            insert.item.mask = TVIF_TEXT | TVIF_PARAM;
            insert.item.pszText = &text[0];
            insert.item.lParam = static_cast<LPARAM>(EncodeGUINodeRef(node.Ref));

            HTREEITEM const item = reinterpret_cast<HTREEITEM>(SendMessageW(m_TV_Tests, TVM_INSERTITEMW, 0,
                reinterpret_cast<LPARAM>(&insert)));

            if (item == nullptr)
                throw std::runtime_error("Couldn't add a node to the test tree");

            treeItems.push_back(item);

            if (node.Ref.Kind == TGUINodeKind::Test)
                m_TestNodes[node.Ref.Index] = item;
        }
    }

    SetTreeExpanded(true);
    RefreshAllNodeImages();
    SyncTreeChecks();

    // Expand All and Collapse All, e.g. while the filter hides every test.
    UpdateCommands();

    HTREEITEM const first = RelatedTreeItem(m_TV_Tests, nullptr, TVGN_ROOT);

    if (first != nullptr)
        SendMessageW(m_TV_Tests, TVM_ENSUREVISIBLE, 0, reinterpret_cast<LPARAM>(first));
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CenterOnMonitor

    Gives the window the .dfm's client size at its DPI, centered on its monitor's work area, as the VCL GUI
    runner's form opens when it has no saved layout.
*/
void TWin32GUIMainWindow::CenterOnMonitor()
{
    RECT frame = { 0, 0, ScaleForDPI(TWin32GUIMainSizes::DefaultClientWidth, m_DPI),
                   ScaleForDPI(TWin32GUIMainSizes::DefaultClientHeight, m_DPI) };
    AdjustWindowRectExForDpi(&frame, static_cast<DWORD>(GetWindowLongPtrW(m_Window, GWL_STYLE)), TRUE,
        static_cast<DWORD>(GetWindowLongPtrW(m_Window, GWL_EXSTYLE)), m_DPI);

    MONITORINFO monitor = {};
    monitor.cbSize = sizeof(monitor);
    GetMonitorInfoW(MonitorFromWindow(m_Window, MONITOR_DEFAULTTONEAREST), &monitor);

    RECT const& work = monitor.rcWork;
    int const width = frame.right - frame.left;
    int const height = frame.bottom - frame.top;
    int const left = work.left + (work.right - work.left - width) / 2;
    int const top = work.top + (work.bottom - work.top - height) / 2;

    SetWindowPos(m_Window, nullptr, left > work.left ? left : work.left, top > work.top ? top : work.top, width,
        height, SWP_NOZORDER | SWP_NOACTIVATE);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::Create()
{
    HINSTANCE const instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW windowClass = {};

    if (GetClassInfoExW(instance, MainWindowClassName, &windowClass) == 0)
    {
        windowClass.cbSize = sizeof(windowClass);
        windowClass.lpfnWndProc = &WindowProc;
        windowClass.hInstance = instance;
        // With no small icon given, Windows takes the title bar's from the same resource, at the size it needs.
        windowClass.hIcon = MainIcon();
        windowClass.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512)); // IDC_ARROW
        windowClass.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
        windowClass.lpszClassName = MainWindowClassName;

        if (RegisterClassExW(&windowClass) == 0)
            throw std::runtime_error("Couldn't register the main window class");
    }

    // The rich edit control the log uses comes from this library.
    m_RichEditLibrary = LoadLibraryW(L"Msftedit.dll");

    if (m_RichEditLibrary == nullptr)
        throw std::runtime_error("Couldn't load Msftedit.dll, which has the log's rich edit control");

    CreateMenuBar();

    if (CreateWindowExW(WS_EX_CONTROLPARENT, MainWindowClassName, UTF8ToWide(Title).c_str(),
        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr,
        m_MM_Main, instance, this) == nullptr)
    {
        throw std::runtime_error("Couldn't create the main window");
    }

    m_Theme.AttachWindow(m_Window, m_MM_Main);
    m_DPI = WindowDPI(m_Window);
    m_TestTreeWidth = ScaleForDPI(TWin32GUIMainSizes::DefaultTestTreeWidth, m_DPI);
    m_DetailHeight = ScaleForDPI(TWin32GUIMainSizes::DefaultDetailHeight, m_DPI);

    CreateControls();
    CreateTooltips();
    CenterOnMonitor();
    ApplyDPI();
    UpdateCommands();

    // The .dfm's action shortcuts: Run Selected F9, Copy Details Ctrl+Shift+C, Filter Ctrl+F.
    ACCEL accelerators[] = {
        { FVIRTKEY, VK_F9, static_cast<WORD>(EWin32GUICommand::RunSelected) },
        { FVIRTKEY | FCONTROL | FSHIFT, 'C', static_cast<WORD>(EWin32GUICommand::CopyDetails) },
        { FVIRTKEY | FCONTROL, 'F', static_cast<WORD>(EWin32GUICommand::FocusFilter) },
    };
    m_Accelerators = CreateAcceleratorTableW(accelerators, static_cast<int>(sizeof(accelerators) / sizeof(accelerators[0])));

    if (m_Accelerators == nullptr)
        throw std::runtime_error("Couldn't create the hot keys");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CreateChild

    Creates a child control of the main window, at no size until LayOut() places it. Throws
    std::runtime_error if it can't be created.
*/
HWND TWin32GUIMainWindow::CreateChild(DWORD exStyle, wchar_t const* className, wchar_t const* text, DWORD style,
    WORD id)
{
    HWND const control = CreateWindowExW(exStyle, className, text, WS_CHILD | WS_VISIBLE | style, 0, 0, 0, 0, m_Window,
        reinterpret_cast<HMENU>(static_cast<UINT_PTR>(id)), GetModuleHandleW(nullptr), nullptr);

    if (control == nullptr)
        throw std::runtime_error("Couldn't create a control");

    return control;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CreateControls

    Creates the .dfm's controls, empty. They're created in tab order, which is also their z-order, the first
    on top; so the results pages come before the tab control they sit on, which would otherwise hide them.
*/
void TWin32GUIMainWindow::CreateControls()
{
    for (TToolbarButton const& button : ToolbarButtons)
    {
        this->*button.Control = CreateChild(0, L"BUTTON", button.Caption, WS_TABSTOP | BS_PUSHBUTTON,
            static_cast<WORD>(button.Command));
    }

    // Created just before the filter box, so its hot key moves the focus there, as FocusControl does.
    m_Lbl_Filter = CreateChild(0, L"STATIC", L"F&ilter:", SS_LEFT);
    m_Edt_Filter = CreateChild(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_TABSTOP | ES_AUTOHSCROLL);
    SendMessageW(m_Edt_Filter, EM_SETCUEBANNER, FALSE, reinterpret_cast<LPARAM>(L"e.g. String or *Handler*.Run_*"));
    m_Theme.Attach(m_Edt_Filter, EWin32GUIThemedControl::Edit);

    m_TV_Tests = CreateChild(WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"", WS_TABSTOP | TVS_HASBUTTONS | TVS_HASLINES |
        TVS_LINESATROOT | TVS_SHOWSELALWAYS);
    // The partial state, for a group or suite with only some of its tests checked, has to be asked for before
    // the check boxes, which create the check box images. The check boxes only once the tree exists, as
    // TVS_CHECKBOXES' documentation says, or they may not show their state. Painted through a buffer, so it
    // doesn't flicker.
    SendMessageW(m_TV_Tests, TVM_SETEXTENDEDSTYLE, TVS_EX_PARTIALCHECKBOXES | TVS_EX_DOUBLEBUFFER,
        TVS_EX_PARTIALCHECKBOXES | TVS_EX_DOUBLEBUFFER);
    SetWindowLongPtrW(m_TV_Tests, GWL_STYLE, GetWindowLongPtrW(m_TV_Tests, GWL_STYLE) | TVS_CHECKBOXES);
    // Drawn like File Explorer's tree (arrows to expand, a light selection) while light, as the VCL draws its
    // trees; see TWin32GUITheme.
    m_Theme.Attach(m_TV_Tests, EWin32GUIThemedControl::TreeView);
    m_Spl_Tests.Create(m_Window, TWin32GUISplitter::EOrientation::Vertical, [this](int position)
        {
            OnTestsSplitterDrag(position);
        });
    m_Theme.Attach(m_Spl_Tests.Handle(), EWin32GUIThemedControl::Splitter);

    m_Mem_Detail = CreateChild(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_READONLY |
        ES_AUTOVSCROLL);
    SendMessageW(m_Mem_Detail, EM_SETLIMITTEXT, 0, 0); // Otherwise 32K characters at most.
    m_Theme.Attach(m_Mem_Detail, EWin32GUIThemedControl::Edit);
    m_Spl_Detail.Create(m_Window, TWin32GUISplitter::EOrientation::Horizontal, [this](int position)
        {
            OnDetailSplitterDrag(position);
        });
    m_Theme.Attach(m_Spl_Detail.Handle(), EWin32GUIThemedControl::Splitter);

    m_LV_Failures = CreateChild(WS_EX_CLIENTEDGE, WC_LISTVIEWW, L"", WS_TABSTOP | LVS_REPORT | LVS_SINGLESEL |
        LVS_SHOWSELALWAYS);
    SendMessageW(m_LV_Failures, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER,
        LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

    wchar_t const* const columns[] = { L"Result", L"Test", L"Detail" };

    for (int i = 0; i < 3; ++i)
    {
        LVCOLUMNW column = {};
        column.mask = LVCF_TEXT;
        column.pszText = const_cast<wchar_t*>(columns[i]);
        SendMessageW(m_LV_Failures, LVM_INSERTCOLUMNW, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(&column));
    }

    // As the VCL draws its list views while light; it also keeps a selected row's text in its result's color.
    m_Theme.Attach(m_LV_Failures, EWin32GUIThemedControl::ListView);

    m_RE_Log = CreateChild(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"", WS_TABSTOP | WS_VSCROLL | WS_HSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL);
    SendMessageW(m_RE_Log, EM_EXLIMITTEXT, 0, 0x7FFFFFFE); // Otherwise 32K characters at most.
    SendMessageW(m_RE_Log, EM_SETTARGETDEVICE, 0, 1); // No word wrap.
    ShowWindow(m_RE_Log, SW_HIDE);
    m_Theme.Attach(m_RE_Log, EWin32GUIThemedControl::RichEdit);

    m_PC_Results = CreateChild(0, WC_TABCONTROLW, L"", WS_TABSTOP | WS_CLIPSIBLINGS);
    // "&&" for a literal '&', as in the .dfm: a tab control takes '&' as a hot key's prefix.
    wchar_t const* const pages[] = { L"Failures && Skips", L"Log" };

    for (int i = 0; i < 2; ++i)
    {
        TCITEMW item = {};
        item.mask = TCIF_TEXT;
        item.pszText = const_cast<wchar_t*>(pages[i]);
        SendMessageW(m_PC_Results, TCM_INSERTITEMW, static_cast<WPARAM>(i), reinterpret_cast<LPARAM>(&item));
    }

    m_Theme.Attach(m_PC_Results, EWin32GUIThemedControl::TabControl);

    m_PB_Progress = CreateChild(0, PROGRESS_CLASSW, L"", 0);
    m_Theme.Attach(m_PB_Progress, EWin32GUIThemedControl::ProgressBar);
    // Placed and sized by LayOut(), at the .dfm's height, rather than by itself at its font's.
    m_SB_Status = CreateChild(0, STATUSCLASSNAMEW, L"", SBARS_SIZEGRIP | CCS_NORESIZE | CCS_NOPARENTALIGN);
    m_Theme.Attach(m_SB_Status, EWin32GUIThemedControl::StatusBar);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CreateMenuBar

    The .dfm's main menu, with each action's caption and shortcut.
*/
void TWin32GUIMainWindow::CreateMenuBar()
{
    m_MM_Main = CreateMenu();

    if (m_MM_Main == nullptr)
        throw std::runtime_error("Couldn't create the menu");

    HMENU const file = AppendSubmenu(m_MM_Main, L"&File");
    AppendCommand(file, EWin32GUICommand::ExportJUnitReport, L"&Export JUnit Report...");
    AppendMenuW(file, MF_SEPARATOR, 0, nullptr);
    AppendCommand(file, EWin32GUICommand::Exit, L"E&xit");

    HMENU const run = AppendSubmenu(m_MM_Main, L"&Run");
    AppendCommand(run, EWin32GUICommand::RunSelected, L"&Run Selected\tF9");
    AppendCommand(run, EWin32GUICommand::RunFailed, L"Run &Failed");
    AppendCommand(run, EWin32GUICommand::Stop, L"&Stop");

    HMENU const tests = AppendSubmenu(m_MM_Main, L"&Tests");
    AppendCommand(tests, EWin32GUICommand::SelectAll, L"Select &All");
    AppendCommand(tests, EWin32GUICommand::SelectNone, L"Select &None");
    AppendCommand(tests, EWin32GUICommand::SelectFailed, L"Select Fai&led");
    AppendCommand(tests, EWin32GUICommand::FocusFilter, L"&Filter\tCtrl+F");
    AppendMenuW(tests, MF_SEPARATOR, 0, nullptr);
    AppendCommand(tests, EWin32GUICommand::CopyDetails, L"&Copy Details\tCtrl+Shift+C");

    HMENU const view = AppendSubmenu(m_MM_Main, L"&View");
    AppendCommand(view, EWin32GUICommand::ExpandAll, L"&Expand All");
    AppendCommand(view, EWin32GUICommand::CollapseAll, L"&Collapse All");
    AppendMenuW(view, MF_SEPARATOR, 0, nullptr);
    AppendCommand(view, EWin32GUICommand::ResetLayout, L"&Reset Layout");
    AppendMenuW(view, MF_SEPARATOR, 0, nullptr);
    HMENU const theme = AppendSubmenu(view, L"&Theme");
    AppendCommand(theme, EWin32GUICommand::ThemeWindows, L"Use &Windows Setting");
    AppendCommand(theme, EWin32GUICommand::ThemeLight, L"&Light");
    AppendCommand(theme, EWin32GUICommand::ThemeDark, L"&Dark");
    AppendMenuW(theme, MF_SEPARATOR, 0, nullptr);
    // Filled with the theme files each time it opens; Carbon until then.
    m_MI_ViewDarkTheme = AppendSubmenu(theme, L"Dark &Theme");
    AppendMenuW(m_MI_ViewDarkTheme, MF_STRING, FirstDarkThemeCommand, L"&Carbon");
    AppendMenuW(m_MI_ViewDarkTheme, MF_SEPARATOR, 0, nullptr);
    AppendCommand(m_MI_ViewDarkTheme, EWin32GUICommand::SaveThemeTemplate, L"&Save Theme Template...");
    AppendCommand(m_MI_ViewDarkTheme, EWin32GUICommand::OpenThemesFolder, L"&Open Themes Folder");

    HMENU const options = AppendSubmenu(m_MM_Main, L"&Options");
    AppendCommand(options, EWin32GUICommand::ShuffleOrder, L"Run in &Shuffled Order");
    AppendCommand(options, EWin32GUICommand::ShuffleSeed, L"Shuffle S&eed...");

    HMENU const help = AppendSubmenu(m_MM_Main, L"&Help");
    AppendCommand(help, EWin32GUICommand::CommandLineOptions, L"&Command Line Options...");
    AppendCommand(help, EWin32GUICommand::About, L"&About...");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CreateStatusImages

    Draws one colored dot per TGUITestStatus into a new image list for the test tree, in the enum's order, so
    a status converts directly to its image index. Drawn at runtime, scaled to m_DPI, as the VCL GUI runner
    draws its own, rather than stored as images. In the dark theme's colors while dark, and else in Carbon's,
    which are the light look's too.
*/
void TWin32GUIMainWindow::CreateStatusImages()
{
    TWin32GUIPalette const palette = m_Theme.IsDark() ? m_Theme.Palette() : CarbonPalette();

    // In TGUITestStatus order: NotRun, Running, Passed, Failed, Skipped.
    COLORREF const colors[] = {
        palette.TreeStatusNotRun,
        palette.TreeStatusRunning,
        palette.TreeStatusPassed,
        palette.TreeStatusFailed,
        palette.TreeStatusSkipped,
    };
    // Each dot's background, left out by ImageList_AddMasked(): a color none of the dots has, as a theme file may
    // give one any color.
    COLORREF transparent = RGB(255, 0, 255);

    for (COLORREF const candidate : { RGB(255, 0, 255), RGB(0, 255, 255), RGB(255, 255, 0), RGB(1, 2, 3),
                                      RGB(254, 0, 254), RGB(0, 254, 254) })
    {
        transparent = candidate;

        if (std::find(std::begin(colors), std::end(colors), candidate) == std::end(colors))
            break;
    }

    int const size = ScaleForDPI(16, m_DPI);
    int const inset = ScaleForDPI(3, m_DPI);

    HIMAGELIST const images = ImageList_Create(size, size, ILC_COLOR24 | ILC_MASK, 5, 0);

    if (images == nullptr)
        throw std::runtime_error("Couldn't create the test tree's status images");

    HDC const screen = GetDC(nullptr);
    HDC const context = CreateCompatibleDC(screen);
    HBITMAP const bitmap = CreateCompatibleBitmap(screen, size, size);
    ReleaseDC(nullptr, screen);

    bool drawn = context != nullptr && bitmap != nullptr;

    if (drawn)
    {
        HGDIOBJ const oldPen = SelectObject(context, GetStockObject(DC_PEN));
        HGDIOBJ const oldBrush = SelectObject(context, GetStockObject(DC_BRUSH));
        RECT const whole = { 0, 0, size, size };

        for (COLORREF const color : colors)
        {
            HGDIOBJ const oldBitmap = SelectObject(context, bitmap);

            SetDCBrushColor(context, transparent);
            FillRect(context, &whole, static_cast<HBRUSH>(GetStockObject(DC_BRUSH)));
            SetDCBrushColor(context, color);
            SetDCPenColor(context, color);
            Ellipse(context, inset, inset, size - inset, size - inset);

            // A bitmap selected into a device context can't be added to an image list.
            SelectObject(context, oldBitmap);
            drawn = ImageList_AddMasked(images, bitmap, transparent) >= 0 && drawn;
        }

        SelectObject(context, oldBrush);
        SelectObject(context, oldPen);
    }

    if (bitmap != nullptr)
        DeleteObject(bitmap);

    if (context != nullptr)
        DeleteDC(context);

    if (!drawn)
    {
        ImageList_Destroy(images);
        throw std::runtime_error("Couldn't draw the test tree's status images");
    }

    SendMessageW(m_TV_Tests, TVM_SETIMAGELIST, TVSIL_NORMAL, reinterpret_cast<LPARAM>(images));

    if (m_StatusImages != nullptr)
        ImageList_Destroy(m_StatusImages);

    m_StatusImages = images;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CreateTooltips

    The VCL shows a control's Hint as its tooltip while the window is active, and not for a disabled control,
    which a Windows tooltip that subclasses its control doesn't either, since the control gets no mouse
    messages.
*/
void TWin32GUIMainWindow::CreateTooltips()
{
    m_TT_Hints = CreateWindowExW(WS_EX_TOPMOST, TOOLTIPS_CLASSW, nullptr, WS_POPUP | TTS_NOPREFIX, CW_USEDEFAULT,
        CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, m_Window, nullptr, GetModuleHandleW(nullptr), nullptr);

    if (m_TT_Hints == nullptr)
        throw std::runtime_error("Couldn't create the tooltips");

    m_Theme.Attach(m_TT_Hints, EWin32GUIThemedControl::Tooltip);

    auto const addTool = [this](HWND control, wchar_t const* text)
        {
            TTTOOLINFOW tool = {};
            tool.cbSize = sizeof(tool);
            tool.uFlags = TTF_IDISHWND | TTF_SUBCLASS;
            tool.hwnd = m_Window;
            tool.uId = reinterpret_cast<UINT_PTR>(control);
            tool.lpszText = const_cast<wchar_t*>(text);
            SendMessageW(m_TT_Hints, TTM_ADDTOOLW, 0, reinterpret_cast<LPARAM>(&tool));
        };

    for (TToolbarButton const& button : ToolbarButtons)
        addTool(this->*button.Control, button.Hint);

    addTool(m_Edt_Filter, L"Show only tests whose \"Group.Test\" name contains this text, which may use the * and ? "
        L"wildcards of --filter; not case-sensitive");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::CurrentLayout

    The window's restored bounds come from GetWindowPlacement(), so a maximized (or minimized) window still
    saves the size it restores to.
*/
TGUILayout TWin32GUIMainWindow::CurrentLayout() const
{
    TGUILayout layout;
    int const pixelsPerInch = static_cast<int>(m_DPI);

    layout.DetailHeight = ScaleTo96(m_DetailHeight, pixelsPerInch);
    layout.TestTreeWidth = ScaleTo96(m_TestTreeWidth, pixelsPerInch);

    WINDOWPLACEMENT placement = {};
    placement.length = sizeof(placement);

    if (GetWindowPlacement(m_Window, &placement) != 0)
    {
        MONITORINFO const primary = PrimaryMonitorInfo();
        TGUIRect const bounds = WorkspaceToScreen(ToGUIRect(placement.rcNormalPosition), ToGUIRect(primary.rcMonitor),
            ToGUIRect(primary.rcWork));

        TGUIWindowBounds window;
        window.Height = ScaleTo96(bounds.Height(), pixelsPerInch);
        window.Left = bounds.Left;
        window.Top = bounds.Top;
        window.Width = ScaleTo96(bounds.Width(), pixelsPerInch);
        layout.Window = window;

        layout.Maximized = placement.showCmd == SW_SHOWMAXIMIZED ||
            (placement.showCmd == SW_SHOWMINIMIZED && (placement.flags & WPF_RESTORETOMAXIMIZED) != 0);
    }

    return layout;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::DoCommand

    A disabled command does nothing, since a hot key still sends it when its menu item and button are disabled.
*/
void TWin32GUIMainWindow::DoCommand(EWin32GUICommand command)
{
    if (!IsCommandEnabled(m_CommandStates, command))
        return;

    RunGuarded(CommandName(command), [this, command]()
        {
            switch (command)
            {
                case EWin32GUICommand::About:
                    ShowTextDialog(m_Window, m_Theme, std::string("About ") + Title, GUIAboutText("Win32 GUI runner"));
                    break;

                case EWin32GUICommand::CollapseAll:
                    SetTreeExpanded(false);
                    // A selected node that's now hidden passes the selection to the top-level node above it.
                    UpdateDetail();
                    break;

                case EWin32GUICommand::CommandLineOptions:
                    ShowTextDialog(m_Window, m_Theme, std::string(Title) + " - Command Line Options",
                        TGUICommandLine::HelpText());
                    break;

                case EWin32GUICommand::CopyDetails:
                    SetClipboardText(m_Window, WindowText(m_Mem_Detail));
                    m_StatusMessage = "Details copied to the clipboard.";
                    UpdateStatusBar();
                    break;

                case EWin32GUICommand::Exit:
                    PostMessageW(m_Window, WM_CLOSE, 0, 0);
                    break;

                case EWin32GUICommand::ExpandAll:
                {
                    SetTreeExpanded(true);
                    HTREEITEM const selected = SelectedTreeItem();

                    if (selected != nullptr)
                        SendMessageW(m_TV_Tests, TVM_ENSUREVISIBLE, 0, reinterpret_cast<LPARAM>(selected));

                    break;
                }

                case EWin32GUICommand::ExportJUnitReport:
                    ExportJUnitReport();
                    break;

                case EWin32GUICommand::FocusFilter:
                    SetFocus(m_Edt_Filter);
                    SendMessageW(m_Edt_Filter, EM_SETSEL, 0, -1);
                    break;

                case EWin32GUICommand::OpenThemesFolder:
                    CreateFolderPath(GUIThemesFolderPath());
                    OpenFolderInExplorer(m_Window, GUIThemesFolderPath());
                    break;

                case EWin32GUICommand::ResetLayout:
                    ResetLayout();
                    break;

                case EWin32GUICommand::RunFailed:
                    if (m_TestList.StatusCount(TGUITestStatus::Failed) == 0)
                        break;

                    // Built before RunTests() resets every result, while the failures are still known.
                    RunTests(m_TestList.FailedFilter(), "the previous run's failures");
                    break;

                case EWin32GUICommand::RunSelected:
                    if (m_TestList.CheckedCount() == 0)
                    {
                        m_StatusMessage = "No tests are checked.";
                        UpdateStatusBar();
                        break;
                    }

                    RunCheckedTests();
                    break;

                case EWin32GUICommand::SaveThemeTemplate:
                    SaveThemeTemplate();
                    break;

                case EWin32GUICommand::SelectAll:
                    m_TestList.SetAllChecked(true);
                    SyncTreeChecks();
                    break;

                case EWin32GUICommand::SelectFailed:
                    m_TestList.CheckOnlyFailed();
                    SyncTreeChecks();
                    break;

                case EWin32GUICommand::SelectNone:
                    m_TestList.SetAllChecked(false);
                    SyncTreeChecks();
                    break;

                case EWin32GUICommand::ShuffleOrder:
                    m_Shuffle.SetEnabled(!m_Shuffle.Enabled());
                    ShowShuffleState();
                    break;

                case EWin32GUICommand::ShuffleSeed:
                    if (ShowShuffleSeedDialog(m_Window, m_Theme, m_Shuffle))
                        ShowShuffleState();

                    break;

                case EWin32GUICommand::Stop:
                    m_Observer.RequestStop();
                    m_StatusMessage = "Stopping after the current test...";
                    UpdateStatusBar();
                    break;

                case EWin32GUICommand::ThemeDark:
                    SetTheme(EGUITheme::Dark);
                    break;

                case EWin32GUICommand::ThemeLight:
                    SetTheme(EGUITheme::Light);
                    break;

                case EWin32GUICommand::ThemeWindows:
                    SetTheme(EGUITheme::Windows);
                    break;

                default:
                    break;
            }
        });
}
//---------------------------------------------------------------------------
int TWin32GUIMainWindow::ExitCode() const
{
    return m_Options.ExitAfterRun ? m_LastRunExitCode : ExitCode_Success;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ExportJUnitReport

    Saves the latest run's JUnit report, the same one --report-junit writes, e.g. when --report-junit wasn't
    given. The dialog starts at --report-junit's path if there is one, and then at the last one chosen.
*/
void TWin32GUIMainWindow::ExportJUnitReport()
{
    if (m_Running || !m_LastRunJUnitTestCases.has_value())
        return;

    if (m_ExportJUnitReportPath.empty() && !m_Options.CLI.JunitReportPath.empty())
    {
        m_ExportJUnitReportPath = ExpandFileName(m_Options.CLI.JunitReportPath);
    }
    else if (m_ExportJUnitReportPath.empty())
    {
        // e.g. "ASWUnitTests_Win32_GUI_JUnit.xml", in the dialog's default folder.
        m_ExportJUnitReportPath = GUIDefaultJUnitReportFileName(ExeFileName());
    }

    // The .dfm's SD_JUnitReport: its title, filter and default extension.
    std::optional<std::string> const path = AskSaveFilePath(m_Window, L"Export JUnit Report",
        L"JUnit XML reports (*.xml)\0*.xml\0All files (*.*)\0*.*\0", L"xml", m_ExportJUnitReportPath);

    if (!path.has_value())
        return; // Canceled.

    m_ExportJUnitReportPath = *path;

    std::string error;

    if (!WriteJUnitReport(m_ExportJUnitReportPath, error))
    {
        std::string const message = "The JUnit report couldn't be written to:\n" + m_ExportJUnitReportPath + "\n\n" +
            error;
        ShowMessageDialog(m_Window, m_Theme, Title, message, EWin32GUIMessageIcon::Error);
        return;
    }

    m_StatusMessage = "JUnit report exported.";
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::FailRunOnUnhandledException(std::string const& description)
{
    m_Observer.Flush();

    // The test that was running when the exception escaped never finished, so it's the one that failed.
    if (m_CurrentTestIndex.has_value())
    {
        m_TestList.SetResult(*m_CurrentTestIndex,
            GUIUnhandledExceptionRecord(m_TestList.Test(*m_CurrentTestIndex), description), std::string());
        UpdateTestNodeImages(*m_CurrentTestIndex);
        AddFailureItem(*m_CurrentTestIndex);
        m_CurrentTestIndex.reset();
    }

    SendMessageW(m_PB_Progress, PBM_SETSTATE, PBST_ERROR, 0);
    m_StatusMessage = GUIUnhandledExceptionStatus(description);
    AppendLog(m_StatusMessage + "\n");
    UpdateDetail();
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::FillDarkThemeMenu

    Carbon first, then the theme files in GUIThemesFolderPath() (see GUIThemeNames()), so a file added or removed
    since shows at once, then Save Theme Template and Open Themes Folder; the chosen one has the radio check. A
    chosen theme whose file is missing is still listed, so choosing it again reports why it isn't used.
*/
void TWin32GUIMainWindow::FillDarkThemeMenu()
{
    std::vector<std::string> names = GUIThemeNames(FileNamesInFolder(GUIThemesFolderPath()), ".ini");
    names.insert(names.begin(), GUIDefaultDarkThemeName());

    // Windows' file names ignore case, so a name saved in another case is still that file's.
    auto const sameName = [](std::string const& a, std::string const& b)
        {
            std::wstring const wideA = UTF8ToWide(a);
            std::wstring const wideB = UTF8ToWide(b);

            return CompareStringOrdinal(wideA.c_str(), static_cast<int>(wideA.size()), wideB.c_str(),
                static_cast<int>(wideB.size()), TRUE) == CSTR_EQUAL;
        };

    std::vector<std::string>::const_iterator chosen = std::find_if(names.begin(), names.end(),
        [this, &sameName](std::string const& name)
        {
            return sameName(name, m_DarkTheme);
        });

    if (chosen == names.end())
        chosen = names.insert(names.end(), m_DarkTheme);

    std::size_t const chosenIndex = static_cast<std::size_t>(chosen - names.begin());

    if (names.size() > MaxDarkThemes)
        names.erase(names.begin() + static_cast<std::ptrdiff_t>(MaxDarkThemes), names.end());

    while (GetMenuItemCount(m_MI_ViewDarkTheme) > 0)
        DeleteMenu(m_MI_ViewDarkTheme, 0, MF_BYPOSITION);

    UINT const enabled = m_CommandStates.Theme ? MF_ENABLED : MF_GRAYED;

    for (std::size_t i = 0; i < names.size(); ++i)
    {
        // Carbon's C is its accelerator; a theme file's name has none.
        std::wstring const caption = UTF8ToWide((i == 0 ? "&" : "") + GUIMenuItemText(names[i]));
        AppendMenuW(m_MI_ViewDarkTheme, MF_STRING | enabled, FirstDarkThemeCommand + i, caption.c_str());
    }

    if (chosenIndex < names.size())
    {
        CheckMenuRadioItem(m_MI_ViewDarkTheme, FirstDarkThemeCommand, static_cast<UINT>(FirstDarkThemeCommand +
            names.size() - 1), static_cast<UINT>(FirstDarkThemeCommand + chosenIndex), MF_BYCOMMAND);
    }

    AppendMenuW(m_MI_ViewDarkTheme, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(m_MI_ViewDarkTheme, MF_STRING | enabled, static_cast<UINT_PTR>(EWin32GUICommand::SaveThemeTemplate),
        L"&Save Theme Template...");
    AppendMenuW(m_MI_ViewDarkTheme, MF_STRING | enabled, static_cast<UINT_PTR>(EWin32GUICommand::OpenThemesFolder),
        L"&Open Themes Folder");

    m_DarkThemeNames = names;
    m_Theme.RefreshMenu(m_MI_ViewDarkTheme);
}
//---------------------------------------------------------------------------
HWND TWin32GUIMainWindow::Handle() const
{
    return m_Window;
}
//---------------------------------------------------------------------------
LRESULT TWin32GUIMainWindow::HandleMessage(UINT message, WPARAM wParam, LPARAM lParam)
{
    HWND const window = m_Window; // Still needed after WM_NCDESTROY clears m_Window.

    switch (message)
    {
        case WM_ACTIVATE:
            // The focus goes back to the control that had it, as in a dialog.
            if (LOWORD(wParam) == WA_INACTIVE)
            {
                HWND const focus = GetFocus();

                if (focus != nullptr && IsChild(m_Window, focus))
                    m_FocusedControl = focus;
            }
            else if (HIWORD(wParam) == 0) // Not minimized
            {
                // Unless it's been disabled since, e.g. by a run, when the next control takes it.
                HWND focus = m_FocusedControl;

                if (focus == nullptr || IsWindowEnabled(focus) == FALSE)
                    focus = NextTabStop(focus, true);

                SetFocus(focus);
            }

            return 0;

        case WM_CLOSE:
            OnClose();
            return 0;

        case WM_COMMAND:
        {
            WORD const id = LOWORD(wParam);

            if (id >= static_cast<WORD>(EWin32GUICommand::About) &&
                id <= static_cast<WORD>(EWin32GUICommand::ThemeWindows))
            {
                DoCommand(static_cast<EWin32GUICommand>(id));
                return 0;
            }

            std::size_t const darkTheme = static_cast<std::size_t>(id) - FirstDarkThemeCommand;

            if (id >= FirstDarkThemeCommand && darkTheme < m_DarkThemeNames.size())
            {
                // As DoCommand() does, nothing while the Theme commands are disabled.
                if (!m_CommandStates.Theme)
                    return 0;

                std::string const name = m_DarkThemeNames[darkTheme];

                RunGuarded("Theme > Dark Theme", [this, &name]()
                    {
                        SetDarkTheme(name);
                    });
                return 0;
            }

            if (reinterpret_cast<HWND>(lParam) == m_Edt_Filter && HIWORD(wParam) == EN_CHANGE)
            {
                if (m_SettingFilterText)
                    return 0; // Start() builds the tree once it has set the text.

                RunGuarded("the filter box", [this]()
                    {
                        ApplyFilterBox();
                    });
                return 0;
            }

            break;
        }

        case WM_DESTROY:
            if (m_Closing)
                PostQuitMessage(0);

            return 0;

        case WM_DPICHANGED:
            OnDPIChanged(HIWORD(wParam), *reinterpret_cast<RECT const*>(lParam));
            return 0;

        case WM_INITMENUPOPUP:
            if (reinterpret_cast<HMENU>(wParam) == m_MI_ViewDarkTheme)
            {
                RunGuarded("the Dark Theme menu", [this]()
                {
                    FillDarkThemeMenu();
                });
                return 0;
            }

            break;

        case WM_NCDESTROY:
            m_Window = nullptr;
            break;

        case WM_NOTIFY:
        {
            NMHDR const* const header = reinterpret_cast<NMHDR const*>(lParam);

            if (header->hwndFrom == m_PC_Results && header->code == TCN_SELCHANGE)
            {
                ShowResultsPage();
                return 0;
            }

            if (header->hwndFrom == m_LV_Failures)
            {
                // Not RunGuarded(): this runs while painting, where showing an error would only cause more
                // painting.
                if (header->code == NM_CUSTOMDRAW)
                    return OnFailuresCustomDraw(*reinterpret_cast<NMLVCUSTOMDRAW*>(lParam));

                if (header->code == NM_DBLCLK)
                {
                    RunGuarded("the Failures & Skips list", [this]()
                        {
                            OnFailuresDoubleClick();
                        });
                    return 0;
                }
            }

            if (header->hwndFrom == m_TV_Tests)
            {
                if (header->code == TVN_ITEMCHANGINGW)
                    return OnTreeItemChanging(*reinterpret_cast<NMTVITEMCHANGE const*>(lParam)) ? TRUE : FALSE;

                if (header->code == TVN_SELCHANGEDW)
                {
                    RunGuarded("the test tree", [this]()
                        {
                            UpdateDetail();
                        });
                    return 0;
                }
            }

            break;
        }

        case WM_SETTINGCHANGE:
            // With Use Windows Setting, Windows' app mode or high contrast changing changes the theme.
            if (IsThemeSettingChange(wParam, lParam))
            {
                RunGuarded("following Windows' theme", [this]()
                {
                    ApplyTheme();
                });
            }

            break;

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
                LayOut();

            return 0;

        case WM_SYSCOLORCHANGE:
            // The common controls need it passed on, and the light theme's colors are the system's.
            for (HWND child = GetWindow(m_Window, GW_CHILD); child != nullptr; child = GetWindow(child, GW_HWNDNEXT))
                SendMessageW(child, WM_SYSCOLORCHANGE, wParam, lParam);

            RunGuarded("the system colors' change", [this]()
            {
                m_Theme.Refresh();
                RecolorLog();
            });
            return 0;

        default:
            if (message == RunQueuedMessage)
            {
                RunQueued();
                return 0;
            }

            break;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
//---------------------------------------------------------------------------
bool TWin32GUIMainWindow::IsToolbarButton(HWND control) const
{
    for (TToolbarButton const& button : ToolbarButtons)
    {
        if (control != nullptr && control == this->*button.Control)
            return true;
    }

    return false;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::LayOut

    Places every control for the window's client size: the toolbar's at the .dfm's positions, the rest by
    LayOutMainWindow(), all at once.
*/
void TWin32GUIMainWindow::LayOut()
{
    if (m_SB_Status == nullptr)
        return; // WM_SIZE during CreateWindowExW(), before the controls exist.

    RECT client;
    GetClientRect(m_Window, &client);

    TWin32GUIMainLayout const layout = LayOutMainWindow(client.right, client.bottom, m_TestTreeWidth, m_DetailHeight,
        m_DPI);

    // The results pages fill the tab control's display area.
    RECT pages = { layout.Results.Left, layout.Results.Top, layout.Results.Right, layout.Results.Bottom };
    SendMessageW(m_PC_Results, TCM_ADJUSTRECT, FALSE, reinterpret_cast<LPARAM>(&pages));

    auto const scaled = [this](int value)
        {
            return ScaleForDPI(value, m_DPI);
        };
    HDWP positions = BeginDeferWindowPos(20);
    auto const place = [&positions](HWND control, int left, int top, int width, int height)
        {
            if (positions != nullptr)
            {
                positions = DeferWindowPos(positions, control, nullptr, left, top, width < 0 ? 0 : width,
                    height < 0 ? 0 : height, SWP_NOZORDER | SWP_NOACTIVATE);
            }
        };
    auto const placeRect = [&place](HWND control, TGUIRect const& rect)
        {
            place(control, rect.Left, rect.Top, rect.Width(), rect.Height());
        };

    for (TToolbarButton const& button : ToolbarButtons)
        place(this->*button.Control, scaled(button.Left), scaled(8), scaled(button.Width), scaled(26));

    place(m_Lbl_Filter, scaled(762), scaled(13), scaled(29), scaled(15));
    place(m_Edt_Filter, scaled(800), scaled(9), scaled(192), scaled(23));
    placeRect(m_TV_Tests, layout.TestTree);
    placeRect(m_Spl_Tests.Handle(), layout.TestsSplitter);
    placeRect(m_Mem_Detail, layout.Detail);
    placeRect(m_Spl_Detail.Handle(), layout.DetailSplitter);
    placeRect(m_PC_Results, layout.Results);
    place(m_LV_Failures, pages.left, pages.top, pages.right - pages.left, pages.bottom - pages.top);
    place(m_RE_Log, pages.left, pages.top, pages.right - pages.left, pages.bottom - pages.top);
    placeRect(m_PB_Progress, layout.ProgressBar);
    placeRect(m_SB_Status, layout.StatusBar);

    if (positions != nullptr)
        EndDeferWindowPos(positions);

    // The Detail column fills the rest of the list, as its AutoSize does in the .dfm.
    SendMessageW(m_LV_Failures, LVM_SETCOLUMNWIDTH, 2, LVSCW_AUTOSIZE_USEHEADER);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::LoadLayout

    Called from Start(), before the window is first shown. The panel sizes are applied once it's showing,
    since a window about to open maximized doesn't have the size they have to fit into until then.
*/
void TWin32GUIMainWindow::LoadLayout()
{
    TGUILayout layout;

    try
    {
        std::string const path = GUILayoutFilePath();

        if (!FileExists(path))
            return;

        layout = LoadGUILayout(TGUIINIDocument(ReadFileBytes(path)));
    }
    catch (...)
    {
        return; // A layout that can't be read only means the window opens with its defaults.
    }

    if (layout.Window.has_value())
    {
        TGUIWindowBounds const& window = *layout.Window;
        int const pixelsPerInch = static_cast<int>(m_DPI);
        TGUIRect const bounds(window.Left, window.Top, window.Left + ScaleFrom96(window.Width, pixelsPerInch),
            window.Top + ScaleFrom96(window.Height, pixelsPerInch));

        // Not where it can't be reached, e.g. on a monitor that's since been disconnected. Set as the window
        // placement's normal position, which is also where a window that opens maximized restores to.
        WINDOWPLACEMENT placement = {};
        placement.length = sizeof(placement);

        if (IsWindowReachable(bounds, MonitorWorkAreas()) && GetWindowPlacement(m_Window, &placement) != 0)
        {
            MONITORINFO const primary = PrimaryMonitorInfo();
            TGUIRect const normal = ScreenToWorkspace(bounds, ToGUIRect(primary.rcMonitor), ToGUIRect(primary.rcWork));
            placement.rcNormalPosition.left = normal.Left;
            placement.rcNormalPosition.top = normal.Top;
            placement.rcNormalPosition.right = normal.Right;
            placement.rcNormalPosition.bottom = normal.Bottom;
            placement.showCmd = SW_HIDE; // Show() shows it.
            SetWindowPlacement(m_Window, &placement);
        }
    }

    m_OpenMaximized = layout.Maximized;

    PostToUI([this, layout]()
        {
            RunGuarded("restoring the layout", [this, &layout]()
            {
                ApplyPanelSizes(layout);
            });
        });
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::LoadSelection()
{
    std::vector<TGUISavedTest> saved;

    try
    {
        std::string const path = GUISelectionFilePath();

        if (!FileExists(path))
            return;

        saved = ParseGUISelection(ReadFileBytes(path));
    }
    catch (...)
    {
        return; // A selection that can't be read only means every test starts out checked.
    }

    if (!saved.empty())
        AppendLog(DescribeGUISelectionRestore(RestoreGUISelection(m_TestList, saved)) + "\n");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::MoveFocusTo

    Also selects the filter box's text, as the VCL's TEdit does when it's tabbed to (AutoSelect), and shows the
    focus rectangle, which Windows hides until the keyboard is used.
*/
void TWin32GUIMainWindow::MoveFocusTo(HWND control)
{
    SetFocus(control);

    if (control == m_Edt_Filter)
        SendMessageW(m_Edt_Filter, EM_SETSEL, 0, -1);

    SendMessageW(m_Window, WM_CHANGEUISTATE, MAKEWPARAM(UIS_CLEAR, UISF_HIDEFOCUS), 0);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::NextTabStop

    The VCL GUI runner's tab order: the toolbar's buttons, the filter box, the test tree, the detail pane, the
    results' tabs, and the results' pages, skipping a control that's disabled or hidden. Windows' own order
    would put the results' pages before their tabs, since they're above them in z-order to get the mouse's
    clicks. From a control that isn't in the order (e.g. nullptr), the first or, going back, the last. nullptr
    if no control can take the focus.
*/
HWND TWin32GUIMainWindow::NextTabStop(HWND from, bool forward) const
{
    std::vector<HWND> order;

    for (TToolbarButton const& button : ToolbarButtons)
        order.push_back(this->*button.Control);

    order.insert(order.end(), { m_Edt_Filter, m_TV_Tests, m_Mem_Detail, m_PC_Results, m_LV_Failures, m_RE_Log });

    std::size_t const count = order.size();
    std::vector<HWND>::const_iterator const found = std::find(order.begin(), order.end(), from);
    // Stepping once from here reaches the first control, or going back, the last.
    std::size_t const start = found != order.end() ? static_cast<std::size_t>(found - order.begin()) :
            (forward ? count - 1 : 0);

    for (std::size_t step = 1; step <= count; ++step)
    {
        HWND const control = order[forward ? (start + step) % count : (start + count - step) % count];

        if (IsWindowVisible(control) != FALSE && IsWindowEnabled(control) != FALSE)
            return control;
    }

    return nullptr;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnClose

    A run can't be interrupted partway through a test, so closing during one asks it to stop after the
    current test, and closes the window once it has (see RunTests()).
*/
void TWin32GUIMainWindow::OnClose()
{
    if (m_Running)
    {
        RunGuarded("closing the window", [this]()
            {
                m_CloseAfterRun = true;
                m_Observer.RequestStop();
                m_StatusMessage = "Closing after the current test...";
                UpdateStatusBar();
            });
        return;
    }

    RunGuarded("closing the window", [this]()
        {
            // An --exit run is automated, so nobody arranged its window.
            if (!m_Options.ExitAfterRun && !m_Options.LayoutIgnore)
                SaveLayout();

            if (m_KeepSelection)
                SaveSelection();
        });

    m_Closing = true;
    DestroyWindow(m_Window);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::OnDetailSplitterDrag(int position)
{
    RunGuarded("the detail pane's splitter", [this, position]()
        {
            RECT client;
            GetClientRect(m_Window, &client);
            m_DetailHeight = ClampDetailHeight(position - ScaleForDPI(TWin32GUIMainSizes::ToolbarHeight, m_DPI),
                client.bottom, m_DPI);
            LayOut();
        });
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnDPIChanged

    The window has moved to a monitor with another DPI. Windows rescales dialogs, but not a window's own
    controls, so everything sized by the DPI is redone here, and the panel sizes keep their proportion.
*/
void TWin32GUIMainWindow::OnDPIChanged(UINT dpi, RECT const& suggested)
{
    RunGuarded("moving to another monitor", [this, dpi, &suggested]()
        {
            m_TestTreeWidth = MulDiv(m_TestTreeWidth, static_cast<int>(dpi), static_cast<int>(m_DPI));
            m_DetailHeight = MulDiv(m_DetailHeight, static_cast<int>(dpi), static_cast<int>(m_DPI));
            m_DPI = dpi;

            SetWindowPos(m_Window, nullptr, suggested.left, suggested.top, suggested.right - suggested.left,
                suggested.bottom - suggested.top, SWP_NOZORDER | SWP_NOACTIVATE);
            ApplyDPI();
        });
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnFailuresCustomDraw

    A row it can't match to a test is simply drawn in the text color. The theme draws the row; see
    TWin32GUITheme::OnListCustomDraw().
*/
LRESULT TWin32GUIMainWindow::OnFailuresCustomDraw(NMLVCUSTOMDRAW& draw) const
{
    COLORREF color = m_Theme.WindowTextColor();

    if (draw.nmcd.dwDrawStage == CDDS_ITEMPREPAINT)
    {
        std::size_t const index = static_cast<std::size_t>(draw.nmcd.lItemlParam);

        if (index < m_TestList.Count())
        {
            color = m_TestList.Status(index) == TGUITestStatus::Failed ? m_Theme.FailureTextColor() :
                    m_Theme.SkipTextColor();
        }
    }

    return m_Theme.OnListCustomDraw(draw, color);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnFailuresDoubleClick

    Selects the double-clicked failure's test in the tree, which shows its details.
*/
void TWin32GUIMainWindow::OnFailuresDoubleClick()
{
    int const row = static_cast<int>(SendMessageW(m_LV_Failures, LVM_GETNEXTITEM, static_cast<WPARAM>(-1),
        LVNI_SELECTED));

    if (row < 0)
        return;

    LVITEMW item = {};
    item.mask = LVIF_PARAM;
    item.iItem = row;
    SendMessageW(m_LV_Failures, LVM_GETITEMW, 0, reinterpret_cast<LPARAM>(&item));

    std::size_t const testIndex = static_cast<std::size_t>(item.lParam);

    if (testIndex >= m_TestList.Count())
        return;

    // A failure the filter box is hiding can't be selected, so the filter is cleared to show it.
    if (TestNode(testIndex) == nullptr)
        SetWindowTextW(m_Edt_Filter, L""); // Its EN_CHANGE rebuilds the tree.

    HTREEITEM const node = TestNode(testIndex);

    if (node == nullptr)
        return;

    SendMessageW(m_TV_Tests, TVM_SELECTITEM, TVGN_CARET, reinterpret_cast<LPARAM>(node));
    SendMessageW(m_TV_Tests, TVM_ENSUREVISIBLE, 0, reinterpret_cast<LPARAM>(node));
    SetFocus(m_TV_Tests);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnTabKey

    Tab and Shift+Tab move through NextTabStop()'s order, rather than IsDialogMessageW()'s, which would also
    let the detail pane and the log keep Tab, as multiline edit controls do. Ctrl+Tab and Ctrl+Shift+Tab switch
    the results' pages while the focus is on them, as the VCL's TPageControl does, and do nothing elsewhere.
    True if 'message' was one of these keys, which it then handled.
*/
bool TWin32GUIMainWindow::OnTabKey(MSG const& message)
{
    if (message.message != WM_KEYDOWN || message.wParam != VK_TAB || (GetKeyState(VK_MENU) & 0x8000) != 0 ||
        IsChild(m_Window, message.hwnd) == FALSE)
    {
        return false;
    }

    bool const back = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
    HWND const focus = GetFocus();

    if ((GetKeyState(VK_CONTROL) & 0x8000) != 0)
    {
        if (focus == m_PC_Results || focus == m_LV_Failures || focus == m_RE_Log)
        {
            int const count = static_cast<int>(SendMessageW(m_PC_Results, TCM_GETITEMCOUNT, 0, 0));
            int const current = static_cast<int>(SendMessageW(m_PC_Results, TCM_GETCURSEL, 0, 0));
            SelectResultsPage((current + (back ? count - 1 : 1)) % count);
        }

        return true;
    }

    HWND const next = NextTabStop(focus, !back);

    if (next != nullptr)
        MoveFocusTo(next);

    return true;
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::OnTestFinished(TTestCaseRecord const& record, std::string const& testLog)
{
    std::optional<std::size_t> const index = m_TestList.IndexOf(record.GroupName, record.TestName);

    if (index.has_value())
    {
        m_TestList.SetResult(*index, record, testLog);
        UpdateTestNodeImages(*index);

        if (record.Outcome != TTestOutcome::Pass)
            AddFailureItem(*index);
    }

    m_CurrentTestIndex.reset();
    ++m_RunFinishedCount;
    SetProgressPosition(static_cast<int>(m_RunFinishedCount));

    if (record.Outcome == TTestOutcome::Fail)
        SendMessageW(m_PB_Progress, PBM_SETSTATE, PBST_ERROR, 0);

    UpdateDetail();
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::OnTestStarted(std::string const& groupName, std::string const& testName)
{
    m_CurrentTestIndex = m_TestList.IndexOf(groupName, testName);

    if (m_CurrentTestIndex.has_value())
    {
        m_TestList.SetStatus(*m_CurrentTestIndex, TGUITestStatus::Running);
        UpdateTestNodeImages(*m_CurrentTestIndex);
    }

    m_StatusMessage = "Running " + groupName + "." + testName;
    UpdateDetail();
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::OnTestsSplitterDrag(int position)
{
    RunGuarded("the test tree's splitter", [this, position]()
        {
            RECT client;
            GetClientRect(m_Window, &client);
            m_TestTreeWidth = ClampTestTreeWidth(position, client.right, m_DPI);
            LayOut();
        });
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::OnTreeItemChanging

    With partial check boxes, a click (or Space) can cycle a check box into its partial state, which only a
    group's or suite's own tests should decide. So every check box change the user makes is vetoed here, and
    instead toggles the node's test, or all of its group's or suite's tests, in m_TestList; the tree is then
    updated from m_TestList, which also works out each group's and suite's partial state. Other changes to a
    node (selecting or expanding it), SyncTreeChecks()' own, and the tree giving an item its first check box
    go through. The tree does that on its own, even for an item of its own with no text or data.
*/
bool TWin32GUIMainWindow::OnTreeItemChanging(NMTVITEMCHANGE const& change)
{
    if (m_SyncingChecks || (change.uChanged & TVIF_STATE) == 0 ||
        ((change.uStateNew ^ change.uStateOld) & TVIS_STATEIMAGEMASK) == 0 ||
        (change.uStateOld & TVIS_STATEIMAGEMASK) == 0)
    {
        return false;
    }

    RunGuarded("a test tree check box", [this, &change]()
        {
            // The run in progress already decided which tests it runs, so a change now would only mislead.
            if (m_Running)
                return;

            // From the notification: the item's own state already reads as the new one here.
            bool const checked = (change.uStateOld & TVIS_STATEIMAGEMASK) ==
                INDEXTOSTATEIMAGEMASK(StateImageIndex(TGUICheckState::Checked));

            SetGUINodeChecked(m_TestList, TreeItemRef(m_TV_Tests, change.hItem), !checked);

            // Changing check boxes from inside the tree's own notification isn't safe, so the update waits until
            // this change has been fully handled.
            PostToUI([this]()
            {
                RunGuarded("a test tree check box", [this]()
                {
                    SyncTreeChecks();
                });
            });
        });

    return true;
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::PostToUI(std::function<void()> action)
{
    {
        std::lock_guard<std::mutex> const lock(m_QueueMutex);
        m_Queue.push_back(std::move(action));
    }

    PostMessageW(m_Window, RunQueuedMessage, 0, 0);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ProcessMessage(MSG& message)
{
    if (m_Window != nullptr)
    {
        // Hot keys first, then Tab, then a dialog's keyboard handling: the controls' & hot keys and arrow keys.
        if (TranslateAcceleratorW(m_Window, m_Accelerators, &message) != 0 || OnTabKey(message) ||
            IsDialogMessageW(m_Window, &message) != 0)
        {
            return;
        }
    }

    TranslateMessage(&message);
    DispatchMessageW(&message);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ProcessMessages

    The observer's YieldCallback while tests run on this thread, between tests: the counterpart of the VCL's
    Application->ProcessMessages(). A WM_QUIT found here (e.g. one a test posted) would otherwise be lost, so
    it's kept and posted again when the run ends, which then ends the GUI as the VCL GUI runner's does.
*/
void TWin32GUIMainWindow::ProcessMessages()
{
    MSG message;

    while (PeekMessageW(&message, nullptr, 0, 0, PM_REMOVE) != 0)
    {
        if (message.message == WM_QUIT)
            m_PendingQuitCode = static_cast<int>(message.wParam);
        else
            ProcessMessage(message);
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::RecolorLog

    AppendLog() colored each line for the theme then. Every line is given the text color first, in one step,
    then each failure and skip line its own color, as the VCL GUI runner's does. The selection and the scroll
    position are kept.
*/
void TWin32GUIMainWindow::RecolorLog()
{
    CHARRANGE selection = {};
    SendMessageW(m_RE_Log, EM_EXGETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    int const firstVisible = static_cast<int>(SendMessageW(m_RE_Log, EM_GETFIRSTVISIBLELINE, 0, 0));

    // Only while it's showing: WM_SETREDRAW's TRUE shows a hidden window, e.g. the log behind the Failures &
    // Skips page. (bcc32c's std::optional can't hold a TRedrawScope, which has no default constructor.)
    std::unique_ptr<TRedrawScope> const updating = IsWindowVisible(m_RE_Log) != FALSE ?
            std::make_unique<TRedrawScope>(m_RE_Log) : nullptr;

    CHARFORMAT2W format = {};
    format.cbSize = sizeof(format);
    format.dwMask = CFM_COLOR;
    format.crTextColor = m_Theme.WindowTextColor();
    SendMessageW(m_RE_Log, EM_SETCHARFORMAT, SCF_ALL, reinterpret_cast<LPARAM>(&format));

    int const lineCount = static_cast<int>(SendMessageW(m_RE_Log, EM_GETLINECOUNT, 0, 0));

    for (int i = 0; i < lineCount; ++i)
    {
        int const start = static_cast<int>(SendMessageW(m_RE_Log, EM_LINEINDEX, static_cast<WPARAM>(i), 0));
        int const length = static_cast<int>(SendMessageW(m_RE_Log, EM_LINELENGTH, static_cast<WPARAM>(start), 0));

        if (start < 0 || length <= 0)
            continue;

        // EM_GETLINE's buffer starts with its size, in characters.
        std::wstring text(static_cast<std::size_t>(length) + 1, L' ');
        text[0] = static_cast<wchar_t>((std::min)(length + 1, 0xFFFF));
        text.resize(static_cast<std::size_t>(SendMessageW(m_RE_Log, EM_GETLINE, static_cast<WPARAM>(i),
            reinterpret_cast<LPARAM>(&text[0]))));

        std::string const line = WideToUTF8(text);

        if (ClassifyGUILogLine(line) == TGUILogLineKind::Normal)
            continue;

        CHARRANGE range = { start, start + length };
        SendMessageW(m_RE_Log, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&range));
        format.crTextColor = LogLineColor(line, m_Theme);
        SendMessageW(m_RE_Log, EM_SETCHARFORMAT, SCF_SELECTION, reinterpret_cast<LPARAM>(&format));
    }

    SendMessageW(m_RE_Log, EM_EXSETSEL, 0, reinterpret_cast<LPARAM>(&selection));
    int const nowVisible = static_cast<int>(SendMessageW(m_RE_Log, EM_GETFIRSTVISIBLELINE, 0, 0));
    SendMessageW(m_RE_Log, EM_LINESCROLL, 0, firstVisible - nowVisible);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::RefreshAllNodeImages()
{
    TRedrawScope const updating(m_TV_Tests);

    for (HTREEITEM item = RelatedTreeItem(m_TV_Tests, nullptr, TVGN_ROOT); item != nullptr;
         item = NextTreeItem(m_TV_Tests, item))
    {
        SetNodeImage(item);
    }
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ReportUnexpectedError(std::string const& where, std::string const& description)
{
    std::string const message = "An unexpected error occurred in " + where + ": " + description;
    ShowMessageDialog(m_Window, m_Theme, Title, message, EWin32GUIMessageIcon::Error);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ResetLayout()
{
    if (IsZoomed(m_Window) != FALSE)
        ShowWindow(m_Window, SW_RESTORE);

    // The default size, centered on the monitor the window is on now.
    CenterOnMonitor();

    TGUILayout defaults;
    defaults.DetailHeight = TWin32GUIMainSizes::DefaultDetailHeight;
    defaults.TestTreeWidth = TWin32GUIMainSizes::DefaultTestTreeWidth;
    ApplyPanelSizes(defaults);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ResetSavedLayout

    For --layout-reset. The file is deleted now, rather than only not loaded, so a layout that caused a
    problem is gone even if the window doesn't get to close normally this time.
*/
void TWin32GUIMainWindow::ResetSavedLayout()
{
    std::string const path = GUILayoutFilePath();
    std::string outcome;

    if (!FileExists(path))
    {
        outcome = "there was no saved window layout to delete";
    }
    else
    {
        RemoveFile(path); // Reported below, from whether the file is still there.
        outcome = FileExists(path) ?
                "the saved window layout couldn't be deleted (" + path + "), so it's ignored this time" :
                "the saved window layout was deleted, so the window opens with the defaults";
    }

    AppendLog("Note: --layout-reset: " + outcome + ".\n");
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::RunCheckedTests()
{
    RunTests(m_TestList.CheckedFilter(), "the checked tests");
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::RunGuarded

    Runs an event handler's body. No exception may leave a window procedure, since unwinding through
    Windows' own frames is undefined, so one is caught here and reported with its message and 'where' it
    happened, as the VCL GUI runner's RunGuarded() reports a C++ exception.
*/
void TWin32GUIMainWindow::RunGuarded(std::string const& where, std::function<void()> const& body)
{
    try
    {
        body();
    }
    catch (std::exception const& ex)
    {
        ReportUnexpectedError(where, ex.what());
    }
    catch (...)
    {
        ReportUnexpectedError(where, "an object that isn't a std::exception");
    }
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::RunMessageLoop()
{
    MSG message;

    while (GetMessageW(&message, nullptr, 0, 0) > 0)
        ProcessMessage(message);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::RunQueued()
{
    std::deque<std::function<void()> > actions;

    {
        std::lock_guard<std::mutex> const lock(m_QueueMutex);
        actions.swap(m_Queue);
    }

    for (std::function<void()> const& action : actions)
        RunGuarded("a queued action", action);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::RunTests

    Runs on this (the main) thread, so tests that create windows work; the observer's YieldCallback lets the
    window repaint and respond to Stop between tests. Every test's result, the Failures & Skips list, and the
    log are cleared first, so they only ever show one run.

    A shuffled run's seed comes from m_Shuffle, rather than being left to TTestHandler::Run() to choose, so
    the GUI knows it and can show it.
*/
void TWin32GUIMainWindow::RunTests(TestFilter const& filter, std::string const& filterDescription)
{
    if (m_Running || m_RunAborted)
        return;

    m_TestList.ResetResults();
    RefreshAllNodeImages();
    SendMessageW(m_LV_Failures, LVM_DELETEALLITEMS, 0, 0);
    SetWindowTextW(m_RE_Log, L"");
    UpdateDetail();

    m_RunTotalCount = m_TestList.CountMatching(filter);
    m_RunFinishedCount = 0;
    m_CurrentTestIndex.reset();
    m_RunStart = std::chrono::steady_clock::now();
    m_RunEnd.reset();
    m_LastRunJUnitTestCases.reset();

    // Back to the normal state first, since the error state wouldn't draw the new position; see
    // SetProgressPosition().
    SendMessageW(m_PB_Progress, PBM_SETSTATE, PBST_NORMAL, 0);
    SendMessageW(m_PB_Progress, PBM_SETRANGE32, 0, static_cast<LPARAM>((std::max<std::size_t>)(m_RunTotalCount, 1)));
    SetProgressPosition(0);

    m_Observer.ResetStop();
    m_StatusMessage = "Running...";
    std::optional<unsigned int> const shuffleSeed = m_Shuffle.NextRunSeed();
    SetRunning(true); // Also shows the seed just chosen.

    try
    {
        TTestResults const results = m_Handler->Run(filter, filterDescription, shuffleSeed.has_value(), shuffleSeed,
            m_Options.CLI.TestTimeoutSeconds, m_Options.CLI.CatchCrashes);
        m_Observer.Flush();

        // After an aborted run, nothing runs again until the GUI is restarted (see WasRunAborted()).
        TGUIRunSummary const summary = SummarizeGUIRun(results);
        m_RunAborted = summary.Aborted;
        m_StatusMessage = summary.StatusMessage;
        m_LastRunExitCode = summary.ExitCode;
        m_LastRunJUnitTestCases = ToJUnitTestCases(results.CaseRecords);

        // A failure is only noted in the log, so it can't hold up --exit.
        if (!m_Options.CLI.JunitReportPath.empty())
        {
            std::string error;
            WriteJUnitReport(m_Options.CLI.JunitReportPath, error);
        }
    }
    // As in the console runner, an unhandled exception ends the run without a report, with its own exit code.
    catch (std::exception const& ex)
    {
        m_LastRunExitCode = ExitCode_UnhandledException;
        FailRunOnUnhandledException(ex.what());
    }
    catch (...)
    {
        m_LastRunExitCode = ExitCode_UnhandledExceptionUnknown;
        FailRunOnUnhandledException("an object that isn't a std::exception");
    }

    m_RunEnd = std::chrono::steady_clock::now();
    SetRunning(false);

    if (m_PendingQuitCode.has_value())
    {
        PostQuitMessage(*m_PendingQuitCode);
        m_PendingQuitCode.reset();
    }

    // Posted, not closed directly: this may still be inside a button's click handling.
    if (m_CloseAfterRun)
        PostMessageW(m_Window, WM_CLOSE, 0, 0);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::SaveLayout()
{
    try
    {
        std::string const path = GUILayoutFilePath();
        CreateFolderPath(GUISettingsFolderPath());

        // Starting from the file's current text keeps any other values in it.
        TGUIINIDocument ini(FileExists(path) ? ReadFileBytes(path) : std::string());
        SaveGUILayout(ini, CurrentLayout());
        WriteFileBytes(path, ini.Text());
    }
    catch (...)
    {
        // Not saving it (e.g. a read-only profile) only means the next start uses the defaults.
    }
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::SaveSelection()
{
    try
    {
        std::string const path = GUISelectionFilePath();
        CreateFolderPath(GUISettingsFolderPath());
        WriteFileBytes(path, FormatGUISelection(CaptureGUISelection(m_TestList)));
    }
    catch (...)
    {
        // Not saving it (e.g. a read-only profile) only means the next start checks every test.
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SaveThemeTemplate

    Writes CarbonThemeFileText() where the user chooses, starting in GUIThemesFolderPath(), created if needed,
    where View > Theme > Dark Theme lists it once it's saved there.
*/
void TWin32GUIMainWindow::SaveThemeTemplate()
{
    std::string const folder = GUIThemesFolderPath();
    CreateFolderPath(folder);

    std::optional<std::string> const path = AskSaveFilePath(m_Window, L"Save Theme Template",
        L"Theme files (*.ini)\0*.ini\0All files (*.*)\0*.*\0", L"ini", folder + "\\My Theme.ini");

    if (!path.has_value())
        return; // Canceled.

    try
    {
        WriteFileBytes(*path, CarbonThemeFileText());
    }
    catch (std::exception const& exception)
    {
        std::string const message = "The theme template couldn't be written to:\n" + *path + "\n\n" +
            exception.what();
        ShowMessageDialog(m_Window, m_Theme, Title, message, EWin32GUIMessageIcon::Error);
        return;
    }

    m_StatusMessage = "Theme template saved.";
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::SelectResultsPage(int page)
{
    // Unlike a click on a tab, this sends no TCN_SELCHANGE.
    SendMessageW(m_PC_Results, TCM_SETCURSEL, static_cast<WPARAM>(page), 0);
    ShowResultsPage();
}
//---------------------------------------------------------------------------
HTREEITEM TWin32GUIMainWindow::SelectedTreeItem() const
{
    return RelatedTreeItem(m_TV_Tests, nullptr, TVGN_CARET);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SetDarkTheme

    Read from its file again even if it's the current one, so a change to the file shows. Applied at once
    while dark; while light, it's still read, so its problems show in the log, and the status bar says it shows
    once the theme is dark.
*/
void TWin32GUIMainWindow::SetDarkTheme(std::string const& name)
{
    m_DarkTheme = name;

    if (!m_Options.LayoutIgnore)
        SaveGUIDarkThemeToLayoutFile(GUILayoutFilePath(), name);

    std::vector<std::string> const problems = m_Theme.SetDarkTheme(name);

    if (m_Theme.IsDark())
    {
        CreateStatusImages();
        RecolorLog();
        m_StatusMessage = "Dark theme: " + name + ".";
    }
    else
    {
        m_StatusMessage = "Dark theme: " + name + ", shown while the theme is dark.";
    }

    for (std::string const& problem : problems)
        AppendLog(problem + "\n");

    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::SetNodeImage(HTREEITEM item)
{
    int const image = static_cast<int>(GUINodeStatus(m_TestList, TreeItemRef(m_TV_Tests, item)));

    TVITEMW data = {};
    data.mask = TVIF_HANDLE | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    data.hItem = item;
    data.iImage = image;
    data.iSelectedImage = image;
    SendMessageW(m_TV_Tests, TVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&data));
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SetProgressPosition

    In the error state (red), a themed progress bar takes a new position without drawing it, so it stays where
    it was when it turned red, and back in the normal state it then shows the position it last took. Setting the
    position a second time draws it, as the VCL GUI runner found.
*/
void TWin32GUIMainWindow::SetProgressPosition(int position)
{
    SendMessageW(m_PB_Progress, PBM_SETPOS, static_cast<WPARAM>(position), 0);

    if (SendMessageW(m_PB_Progress, PBM_GETSTATE, 0, 0) != PBST_NORMAL)
        SendMessageW(m_PB_Progress, PBM_SETPOS, static_cast<WPARAM>(position), 0);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SetRunning

    Disabling the control with the focus moves the focus on (see UpdateCommands()), e.g. from Run Selected to
    Stop as a run starts. So when the run ends, the control that had the focus gets it back, as in the VCL GUI
    runner, unless the user has put it somewhere other than a toolbar button since.
*/
void TWin32GUIMainWindow::SetRunning(bool running)
{
    if (running)
        m_FocusBeforeRun = GetFocus();

    m_Running = running;
    UpdateCommands();
    UpdateStatusBar();

    if (!running && m_FocusBeforeRun != nullptr && IsWindowEnabled(m_FocusBeforeRun) != FALSE &&
        IsToolbarButton(GetFocus()))
    {
        SetFocus(m_FocusBeforeRun);
    }

    // Windows' setting changed during the run; see ApplyTheme().
    if (!running && m_ThemeChangePending)
        ApplyTheme();
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SetTheme

    Saved at once, rather than with the layout as the window closes, so the choice is kept even if the window
    doesn't get to close normally; but not with --layout-ignore, which saves nothing. Reset Layout leaves it
    alone.
*/
void TWin32GUIMainWindow::SetTheme(EGUITheme theme)
{
    m_ThemeChoice = theme;
    ShowThemeState();

    if (!m_Options.LayoutIgnore)
        SaveGUIThemeToLayoutFile(GUILayoutFilePath(), theme);

    ApplyTheme();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::SetTreeExpanded(bool expanded)
{
    TRedrawScope const updating(m_TV_Tests);

    for (HTREEITEM item = RelatedTreeItem(m_TV_Tests, nullptr, TVGN_ROOT); item != nullptr;
         item = NextTreeItem(m_TV_Tests, item))
    {
        if (RelatedTreeItem(m_TV_Tests, item, TVGN_CHILD) != nullptr)
            SendMessageW(m_TV_Tests, TVM_EXPAND, expanded ? TVE_EXPAND : TVE_COLLAPSE, reinterpret_cast<LPARAM>(item));
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::Show

    WinMain()'s 'showCommand' is usually SW_SHOWNORMAL, unless e.g. a shortcut asks for the window minimized or
    maximized.
*/
void TWin32GUIMainWindow::Show(int showCommand)
{
    if (m_OpenMaximized && (showCommand == SW_SHOWNORMAL || showCommand == SW_SHOWDEFAULT || showCommand == SW_SHOW))
        showCommand = SW_SHOWMAXIMIZED;

    ShowWindow(m_Window, showCommand);
    UpdateWindow(m_Window);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::ShowResultsPage

    The focus, if it's on the page being hidden, goes to the one shown, as in the VCL.
*/
void TWin32GUIMainWindow::ShowResultsPage()
{
    bool const logSelected = SendMessageW(m_PC_Results, TCM_GETCURSEL, 0, 0) == 1;
    HWND const shown = logSelected ? m_RE_Log : m_LV_Failures;
    HWND const hidden = logSelected ? m_LV_Failures : m_RE_Log;

    ShowWindow(shown, SW_SHOW);

    if (GetFocus() == hidden)
        SetFocus(shown);

    ShowWindow(hidden, SW_HIDE);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ShowShuffleState()
{
    CheckMenuItem(m_MM_Main, static_cast<UINT>(EWin32GUICommand::ShuffleOrder),
        MF_BYCOMMAND | (m_Shuffle.Enabled() ? MF_CHECKED : MF_UNCHECKED));
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ShowThemeState()
{
    EWin32GUICommand checked = EWin32GUICommand::ThemeWindows;

    if (m_ThemeChoice == EGUITheme::Dark)
        checked = EWin32GUICommand::ThemeDark;
    else if (m_ThemeChoice == EGUITheme::Light)
        checked = EWin32GUICommand::ThemeLight;

    CheckMenuRadioItem(m_MM_Main, static_cast<UINT>(EWin32GUICommand::ThemeDark),
        static_cast<UINT>(EWin32GUICommand::ThemeWindows), static_cast<UINT>(checked), MF_BYCOMMAND);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::Start

    Loads the registered tests and applies the command line's options: --project-name to the caption, and
    --filter and the partition options to which tests start out checked, and --filter to the filter box,
    and --shuffle and --shuffle-seed to the Options menu. Also restores the window layout and the test
    selection saved when it last closed. WinMain() set the theme up for 'theme' and 'darkTheme' before any
    window existed, so the --help dialog had it too, and its problems are noted in the log, where they stay
    until the first run clears it.
*/
void TWin32GUIMainWindow::Start(TGUIOptions const& options, EGUITheme theme, std::string const& darkTheme,
    std::vector<std::string> const& themeProblems)
{
    m_Options = options;
    m_ThemeChoice = theme;
    m_DarkTheme = darkTheme;
    ShowThemeState();
    // The dark theme's status dots, if WinMain() set up a theme file.
    CreateStatusImages();

    m_Observer.LogTextCallback = [this](std::string const& text)
        {
            AppendLog(text);
        };
    m_Observer.TestStartedCallback = [this](std::string const& groupName, std::string const& testName)
        {
            OnTestStarted(groupName, testName);
        };
    m_Observer.TestFinishedCallback = [this](TTestCaseRecord const& record, std::string const& testLog)
        {
            OnTestFinished(record, testLog);
        };
    m_Observer.YieldCallback = [this]()
        {
            ProcessMessages();
        };

    m_Handler = std::make_unique<TTestHandler>();
    m_Handler->SetRunObserver(&m_Observer); // Before Initialize(), so its output goes to the observer too.
    m_Handler->Initialize(m_Options.CLI.ProjectName);
    m_Observer.Flush();

    m_TestList.Load(m_Handler->GetTests());

    std::string filterDescription;
    TestFilter const initialFilter = TCLIParser::BuildTestFilter(*m_Handler, m_Options.CLI, filterDescription);
    m_TestList.CheckMatching(initialFilter);

    // Notes about the command line, in the log, where they stay until the first run clears it.
    if (initialFilter != nullptr)
        AppendLog("Initially checked: " + filterDescription + "\n");

    // Only when the command line chooses no tests (see m_KeepSelection). Before the tree is built below.
    m_KeepSelection = !m_Options.RunOnStart && initialFilter == nullptr;

    if (m_KeepSelection)
        LoadSelection();

    for (std::string const& note : GUICommandLineNotes(m_Options, "windows or controls"))
        AppendLog(note + "\n");

    for (std::string const& problem : themeProblems)
        AppendLog(problem + "\n");

    if (m_Options.LayoutReset)
        ResetSavedLayout();

    UpdateCaption();

    m_Shuffle.ApplyCommandLine(m_Options.CLI.Shuffle, m_Options.CLI.ShuffleSeed);
    ShowShuffleState();

    // The filter box's case-insensitive contains-match shows every test --filter's pattern matches (and
    // maybe a few more, left unchecked), so seeding it with the pattern never hides a checked test.
    if (m_Options.CLI.HasFilter)
    {
        // Without EN_CHANGE's rebuild, which only happens if the text changes, so the tree is built once either
        // way, just below.
        TScopedFlag const settingFilterText(m_SettingFilterText);
        SetWindowTextW(m_Edt_Filter, UTF8ToWide(m_Options.CLI.FilterPattern).c_str());
    }

    // Builds the tree.
    ApplyFilterBox();

    // Before --run is queued below, so the panel sizes LoadLayout() queues are applied before the run starts.
    if (!m_Options.LayoutIgnore && !m_Options.LayoutReset)
        LoadLayout();

    if (m_Options.RunOnStart)
    {
        // Queued, so it starts once the message loop is running and the window is showing.
        PostToUI([this]()
            {
                RunGuarded("--run", [this]()
                {
                    // Unlike Run Selected, runs even if nothing is checked, so --exit still reports and exits
                    // the way the console runner does when its filter matches nothing.
                    RunCheckedTests();

                    if (m_Options.ExitAfterRun)
                        PostMessageW(m_Window, WM_CLOSE, 0, 0);
                });
            });
    }
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::SyncTreeChecks

    Sets every node's check box from m_TestList: a test's to whether it's checked, and a group's or suite's
    to whether all, none, or only some of its tests are.
*/
void TWin32GUIMainWindow::SyncTreeChecks()
{
    {
        TRedrawScope const updating(m_TV_Tests);
        TScopedFlag const syncing(m_SyncingChecks);

        for (HTREEITEM item = RelatedTreeItem(m_TV_Tests, nullptr, TVGN_ROOT); item != nullptr;
             item = NextTreeItem(m_TV_Tests, item))
        {
            TVITEMW data = {};
            data.mask = TVIF_HANDLE | TVIF_STATE;
            data.hItem = item;
            data.stateMask = TVIS_STATEIMAGEMASK;
            data.state = INDEXTOSTATEIMAGEMASK(StateImageIndex(GUINodeCheckState(m_TestList,
                TreeItemRef(m_TV_Tests, item))));
            SendMessageW(m_TV_Tests, TVM_SETITEMW, 0, reinterpret_cast<LPARAM>(&data));
        }
    }

    UpdateStatusBar(); // For its checked count.
}
//---------------------------------------------------------------------------
HTREEITEM TWin32GUIMainWindow::TestNode(std::size_t testIndex) const
{
    return testIndex < m_TestNodes.size() ? m_TestNodes[testIndex] : nullptr;
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateCaption()
{
    SetWindowTextW(m_Window, UTF8ToWide(GUICaption(m_Options.CLI.ProjectName)).c_str());
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateCommands()
{
    TGUICommandContext context;
    context.FailedCount = m_TestList.StatusCount(TGUITestStatus::Failed);
    context.HasReport = m_LastRunJUnitTestCases.has_value();
    context.HasSelection = SelectedTreeItem() != nullptr;
    context.RunAborted = m_RunAborted;
    context.Running = m_Running;
    context.TreeHasNodes = SendMessageW(m_TV_Tests, TVM_GETCOUNT, 0, 0) > 0;

    m_CommandStates = GUICommandStates(context);
    HWND const focus = GetFocus();

    for (EWin32GUICommand const command : AllCommands)
    {
        EnableMenuItem(m_MM_Main, static_cast<UINT>(command),
            MF_BYCOMMAND | (IsCommandEnabled(m_CommandStates, command) ? MF_ENABLED : MF_GRAYED));
    }

    for (TToolbarButton const& button : ToolbarButtons)
        EnableWindow(this->*button.Control, IsCommandEnabled(m_CommandStates, button.Command) ? TRUE : FALSE);

    EnableWindow(m_Edt_Filter, m_CommandStates.FilterBox ? TRUE : FALSE);

    // A disabled control can't keep the focus, e.g. Run Selected as a run starts, and with no control holding it,
    // IsDialogMessageW() keeps Alt+letter from opening the menu. So it moves on to the next control, as a dialog's
    // does.
    if (focus != nullptr && IsChild(m_Window, focus) && IsWindowEnabled(focus) == FALSE)
    {
        HWND const next = NextTabStop(focus, true);

        if (next != nullptr)
            SetFocus(next);
    }
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateDetail()
{
    HTREEITEM const item = SelectedTreeItem();
    UpdateCommands(); // Copy Details, with or without a selection.

    std::string const text = item != nullptr ? AdjustLineBreaks(GUINodeDetailText(m_TestList,
        TreeItemRef(m_TV_Tests, item))) : std::string();
    SetWindowTextW(m_Mem_Detail, UTF8ToWide(text).c_str());
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateStatusBar()
{
    std::optional<double> elapsedSeconds;

    if (m_RunStart.has_value())
    {
        std::chrono::steady_clock::time_point const end = m_RunEnd.value_or(std::chrono::steady_clock::now());
        elapsedSeconds = std::chrono::duration<double>(end - *m_RunStart).count();
    }

    std::vector<std::string> const panels = GUIStatusPanels(m_TestList, m_RunFinishedCount, m_RunTotalCount,
        elapsedSeconds, m_Shuffle.StatusText(), m_StatusMessage);

    for (std::size_t i = 0; i < panels.size(); ++i)
        SendMessageW(m_SB_Status, SB_SETTEXTW, i, reinterpret_cast<LPARAM>(UTF8ToWide(panels[i]).c_str()));
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateTestNodeImages(std::size_t testIndex)
{
    HTREEITEM const testNode = TestNode(testIndex);

    if (testNode == nullptr)
        return; // Hidden by the filter box, e.g. a hidden failure rerun by Run Failed.

    // The test's own image, then those of its group and the suites holding it, whose status depends on it.
    for (HTREEITEM item = testNode; item != nullptr; item = RelatedTreeItem(m_TV_Tests, item, TVGN_PARENT))
        SetNodeImage(item);
}
//---------------------------------------------------------------------------
bool TWin32GUIMainWindow::WasRunAborted() const
{
    return m_RunAborted;
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::WindowProc

    Passes each message to the window that created it, whose address CreateWindowExW() received. No exception
    may leave a window procedure: one that escapes the message's handler is reported, and the message is left
    to Windows.
*/
LRESULT CALLBACK TWin32GUIMainWindow::WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (message == WM_NCCREATE)
    {
        TWin32GUIMainWindow* const mainWindow = static_cast<TWin32GUIMainWindow*>(
            reinterpret_cast<CREATESTRUCTW*>(lParam)->lpCreateParams);
        mainWindow->m_Window = window;
        SetWindowLongPtrW(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(mainWindow));
    }

    TWin32GUIMainWindow* const mainWindow = reinterpret_cast<TWin32GUIMainWindow*>(GetWindowLongPtrW(window,
        GWLP_USERDATA));

    if (mainWindow != nullptr)
    {
        try
        {
            return mainWindow->HandleMessage(message, wParam, lParam);
        }
        catch (std::exception const& ex)
        {
            try
            {
                mainWindow->ReportUnexpectedError("the main window", ex.what());
            }
            catch (...)
            {
            }
        }
        catch (...)
        {
            try
            {
                mainWindow->ReportUnexpectedError("the main window", "an object that isn't a std::exception");
            }
            catch (...)
            {
            }
        }
    }

    return DefWindowProcW(window, message, wParam, lParam);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::WriteJUnitReport

    Writes the file itself, rather than with TJUnitReportWriter::Write(), so a failure can say why: Write() only
    returns false.
*/
bool TWin32GUIMainWindow::WriteJUnitReport(std::string const& path, std::string& error)
{
    error.clear();

    try
    {
        if (!m_LastRunJUnitTestCases.has_value())
            throw std::runtime_error("no run has finished");

        WriteFileBytes(path, TJUnitReportWriter::BuildXML(m_Handler->GetProjectName(), *m_LastRunJUnitTestCases));
    }
    catch (std::exception const& ex)
    {
        error = ex.what();
    }

    if (error.empty())
        AppendLog("JUnit report written to: " + path + "\n");
    else
        AppendLog("Error: could not write JUnit report to: " + path + " (" + error + ")\n");

    return error.empty();
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
