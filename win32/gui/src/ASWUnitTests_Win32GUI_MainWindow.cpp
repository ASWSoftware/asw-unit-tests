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
#include <cstddef>
#include <cstdint>
#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Tree.h"
#include "ASWUnitTests_GUI_View.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_MainLayout.h"
#include "ASWUnitTests_Win32GUI_System.h"
#include "ASWUnitTests_Win32GUI_TextDialog.h"
//---------------------------------------------------------------------------

// RAD Studio's Win32 (bcc32c) links its own Windows import libraries, which don't include SetWindowTheme()'s.
// CMake links uxtheme for the other compilers, and RAD Studio's Win64x links it already.
#if defined(__BORLANDC__) && !defined(_WIN64)
#  pragma comment(lib, "uxtheme.lib")
#endif

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// The log's color for failure lines, as the VCL GUI runner's.
COLORREF const FailureColor = RGB(200, 0, 0);
wchar_t const MainWindowClassName[] = L"ASWUnitTests_Win32GUI_Main";
// Posted by PostToUI(), so the message loop runs the queued actions.
UINT const RunQueuedMessage = WM_APP + 1;
// The log's color for skip lines, as the VCL GUI runner's.
COLORREF const SkipColor = RGB(190, 120, 0);
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
    EWin32GUICommand::ResetLayout,
    EWin32GUICommand::RunFailed,
    EWin32GUICommand::RunSelected,
    EWin32GUICommand::SelectAll,
    EWin32GUICommand::SelectFailed,
    EWin32GUICommand::SelectNone,
    EWin32GUICommand::ShuffleOrder,
    EWin32GUICommand::ShuffleSeed,
    EWin32GUICommand::Stop,
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

void AppendCommand(HMENU menu, EWin32GUICommand command, wchar_t const* caption);
HMENU AppendSubmenu(HMENU menuBar, wchar_t const* caption);
char const* CommandName(EWin32GUICommand command);
bool IsCommandEnabled(TGUICommandStates const& states, EWin32GUICommand command);
COLORREF LogLineColor(std::string const& line);
HTREEITEM NextTreeItem(HWND tree, HTREEITEM item);
HTREEITEM RelatedTreeItem(HWND tree, HTREEITEM item, UINT relation);
UINT StateImageIndex(TGUICheckState state);
TGUINodeRef TreeItemRef(HWND tree, HTREEITEM item);
std::string WindowText(HWND window);

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

        case EWin32GUICommand::ResetLayout:
            return "Reset Layout";

        case EWin32GUICommand::RunFailed:
            return "Run Failed";

        case EWin32GUICommand::RunSelected:
            return "Run Selected";

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

        case EWin32GUICommand::ResetLayout:
            return states.ResetLayout;

        case EWin32GUICommand::RunFailed:
            return states.RunFailed;

        case EWin32GUICommand::RunSelected:
            return states.RunSelected;

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
    }

    return false;
}
//---------------------------------------------------------------------------
/*
    LogLineColor

    The log's color for one line of the framework's log output; see ClassifyGUILogLine().
*/
COLORREF LogLineColor(std::string const& line)
{
    switch (ClassifyGUILogLine(line))
    {
        case TGUILogLineKind::Failure:
            return FailureColor;

        case TGUILogLineKind::Skip:
            return SkipColor;

        case TGUILogLineKind::Normal:
            break;
    }

    return GetSysColor(COLOR_WINDOWTEXT);
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
std::string WindowText(HWND window)
{
    std::wstring text(static_cast<std::size_t>(GetWindowTextLengthW(window)) + 1, L'\0');
    text.resize(static_cast<std::size_t>(GetWindowTextW(window, &text[0], static_cast<int>(text.size()))));

    return WideToUTF8(text);
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
TWin32GUIMainWindow::TToolbarButton const TWin32GUIMainWindow::ToolbarButtons[7] = {
    { EWin32GUICommand::RunSelected, L"R&un Selected", 8, 110, &TWin32GUIMainWindow::m_Btn_RunSelected },
    { EWin32GUICommand::RunFailed, L"Run Faile&d", 124, 100, &TWin32GUIMainWindow::m_Btn_RunFailed },
    { EWin32GUICommand::Stop, L"&Stop", 230, 80, &TWin32GUIMainWindow::m_Btn_Stop },
    { EWin32GUICommand::SelectAll, L"Select &All", 328, 90, &TWin32GUIMainWindow::m_Btn_SelectAll },
    { EWin32GUICommand::SelectNone, L"Select &None", 424, 90, &TWin32GUIMainWindow::m_Btn_SelectNone },
    { EWin32GUICommand::SelectFailed, L"Select Fai&led", 520, 90, &TWin32GUIMainWindow::m_Btn_SelectFailed },
    { EWin32GUICommand::CopyDetails, L"&Copy Details", 628, 110, &TWin32GUIMainWindow::m_Btn_CopyDetails },
};

//---------------------------------------------------------------------------
TWin32GUIMainWindow::TWin32GUIMainWindow()
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
      m_MM_Main(nullptr),
      m_PB_Progress(nullptr),
      m_PC_Results(nullptr),
      m_RE_Log(nullptr),
      m_SB_Status(nullptr),
      m_TV_Tests(nullptr),
      m_Accelerators(nullptr),
      m_Closing(false),
      m_DetailHeight(TWin32GUIMainSizes::DefaultDetailHeight),
      m_DPI(USER_DEFAULT_SCREEN_DPI),
      m_FocusedControl(nullptr),
      m_RichEditLibrary(nullptr),
      m_SettingFilterText(false),
      m_StatusImages(nullptr),
      m_StatusMessage("Ready."),
      m_SyncingChecks(false),
      m_TestTreeWidth(TWin32GUIMainSizes::DefaultTestTreeWidth),
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
        format.crTextColor = LogLineColor(line);
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

    // The status bar's parts' widths, from the .dfm's panels.
    std::vector<int> rightEdges;
    int right = 0;

    for (int const width : GUIStatusPanelWidths())
    {
        right += ScaleForDPI(width, m_DPI);
        rightEdges.push_back(right);
    }

    SendMessageW(m_SB_Status, SB_SETPARTS, rightEdges.size(), reinterpret_cast<LPARAM>(rightEdges.data()));

    // The Detail column's width is set by LayOut(), to fill the list.
    SendMessageW(m_LV_Failures, LVM_SETCOLUMNWIDTH, 0, ScaleForDPI(70, m_DPI));
    SendMessageW(m_LV_Failures, LVM_SETCOLUMNWIDTH, 1, ScaleForDPI(240, m_DPI));

    LayOut();
    InvalidateRect(m_Window, nullptr, TRUE);
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

    m_DPI = WindowDPI(m_Window);
    m_TestTreeWidth = ScaleForDPI(TWin32GUIMainSizes::DefaultTestTreeWidth, m_DPI);
    m_DetailHeight = ScaleForDPI(TWin32GUIMainSizes::DefaultDetailHeight, m_DPI);

    CreateControls();
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

    m_TV_Tests = CreateChild(WS_EX_CLIENTEDGE, WC_TREEVIEWW, L"", WS_TABSTOP | TVS_HASBUTTONS | TVS_HASLINES |
        TVS_LINESATROOT | TVS_SHOWSELALWAYS);
    // The partial state, for a group or suite with only some of its tests checked, has to be asked for before
    // the check boxes, which create the check box images. The check boxes only once the tree exists, as
    // TVS_CHECKBOXES' documentation says, or they may not show their state.
    SendMessageW(m_TV_Tests, TVM_SETEXTENDEDSTYLE, TVS_EX_PARTIALCHECKBOXES, TVS_EX_PARTIALCHECKBOXES);
    SetWindowLongPtrW(m_TV_Tests, GWL_STYLE, GetWindowLongPtrW(m_TV_Tests, GWL_STYLE) | TVS_CHECKBOXES);
    // Drawn like File Explorer's tree (arrows to expand, a light selection), as the VCL draws its trees.
    SetWindowTheme(m_TV_Tests, L"Explorer", nullptr);
    m_Spl_Tests.Create(m_Window, TWin32GUISplitter::EOrientation::Vertical, [this](int position)
        {
            OnTestsSplitterDrag(position);
        });

    m_Mem_Detail = CreateChild(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_TABSTOP | WS_VSCROLL | ES_MULTILINE | ES_READONLY |
        ES_AUTOVSCROLL);
    SendMessageW(m_Mem_Detail, EM_SETLIMITTEXT, 0, 0); // Otherwise 32K characters at most.
    m_Spl_Detail.Create(m_Window, TWin32GUISplitter::EOrientation::Horizontal, [this](int position)
        {
            OnDetailSplitterDrag(position);
        });

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

    m_RE_Log = CreateChild(WS_EX_CLIENTEDGE, MSFTEDIT_CLASS, L"", WS_TABSTOP | WS_VSCROLL | WS_HSCROLL |
        ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL);
    SendMessageW(m_RE_Log, EM_EXLIMITTEXT, 0, 0x7FFFFFFE); // Otherwise 32K characters at most.
    SendMessageW(m_RE_Log, EM_SETTARGETDEVICE, 0, 1); // No word wrap.
    ShowWindow(m_RE_Log, SW_HIDE);

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

    m_PB_Progress = CreateChild(0, PROGRESS_CLASSW, L"", 0);
    // Placed and sized by LayOut(), at the .dfm's height, rather than by itself at its font's.
    m_SB_Status = CreateChild(0, STATUSCLASSNAMEW, L"", SBARS_SIZEGRIP | CCS_NORESIZE | CCS_NOPARENTALIGN);
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
    draws its own, rather than stored as images.
*/
void TWin32GUIMainWindow::CreateStatusImages()
{
    // In TGUITestStatus order: NotRun, Running, Passed, Failed, Skipped.
    COLORREF const colors[] = {
        RGB(190, 190, 190),
        RGB(0, 120, 215),
        RGB(16, 160, 16),
        RGB(215, 40, 40),
        RGB(230, 165, 0),
    };
    // Each dot's background, left out by ImageList_AddMasked().
    COLORREF const transparent = RGB(255, 0, 255);

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
    TWin32GUIMainWindow::DoCommand

    A disabled command does nothing, since a hot key still sends it when its menu item and button are
    disabled. The commands for running tests and their results aren't connected yet.
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
                    ShowTextDialog(m_Window, std::string("About ") + Title, GUIAboutText("Win32 GUI runner"));
                    break;

                case EWin32GUICommand::CollapseAll:
                    SetTreeExpanded(false);
                    // A selected node that's now hidden passes the selection to the top-level node above it.
                    UpdateDetail();
                    break;

                case EWin32GUICommand::CommandLineOptions:
                    ShowTextDialog(m_Window, std::string(Title) + " - Command Line Options", TGUICommandLine::HelpText());
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

                case EWin32GUICommand::FocusFilter:
                    SetFocus(m_Edt_Filter);
                    SendMessageW(m_Edt_Filter, EM_SETSEL, 0, -1);
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

                default:
                    break;
            }
        });
}
//---------------------------------------------------------------------------
int TWin32GUIMainWindow::ExitCode() const
{
    return ExitCode_Success;
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
                SetFocus(m_FocusedControl != nullptr ? m_FocusedControl : GetNextDlgTabItem(m_Window, nullptr, FALSE));
            }

            return 0;

        case WM_CLOSE:
            m_Closing = true;
            DestroyWindow(m_Window);
            return 0;

        case WM_COMMAND:
        {
            WORD const id = LOWORD(wParam);

            if (id >= static_cast<WORD>(EWin32GUICommand::About) && id <= static_cast<WORD>(EWin32GUICommand::Stop))
            {
                DoCommand(static_cast<EWin32GUICommand>(id));
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

        case WM_CTLCOLORSTATIC:
            // The read-only detail pane is drawn like an editable one, as the VCL GUI runner's is, rather than gray.
            if (reinterpret_cast<HWND>(lParam) == m_Mem_Detail)
            {
                HDC const context = reinterpret_cast<HDC>(wParam);
                SetTextColor(context, GetSysColor(COLOR_WINDOWTEXT));
                SetBkColor(context, GetSysColor(COLOR_WINDOW));
                return reinterpret_cast<LRESULT>(GetSysColorBrush(COLOR_WINDOW));
            }

            break;

        case WM_DESTROY:
            if (m_Closing)
                PostQuitMessage(0);

            return 0;

        case WM_DPICHANGED:
            OnDPIChanged(HIWORD(wParam), *reinterpret_cast<RECT const*>(lParam));
            return 0;

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

        case WM_SIZE:
            if (wParam != SIZE_MINIMIZED)
                LayOut();

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
    MessageBoxW(m_Window, UTF8ToWide(message).c_str(), UTF8ToWide(Title).c_str(), MB_OK | MB_ICONERROR);
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
    {
        if (m_Window != nullptr)
        {
            // Hot keys first, then a dialog's keyboard handling: Tab between controls, and their & hot keys.
            if (TranslateAcceleratorW(m_Window, m_Accelerators, &message) != 0 ||
                IsDialogMessageW(m_Window, &message) != 0)
            {
                continue;
            }
        }

        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
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
HTREEITEM TWin32GUIMainWindow::SelectedTreeItem() const
{
    return RelatedTreeItem(m_TV_Tests, nullptr, TVGN_CARET);
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
void TWin32GUIMainWindow::Show(int showCommand)
{
    ShowWindow(m_Window, showCommand);
    UpdateWindow(m_Window);
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::ShowResultsPage()
{
    bool const logSelected = SendMessageW(m_PC_Results, TCM_GETCURSEL, 0, 0) == 1;

    ShowWindow(m_LV_Failures, logSelected ? SW_HIDE : SW_SHOW);
    ShowWindow(m_RE_Log, logSelected ? SW_SHOW : SW_HIDE);
}
//---------------------------------------------------------------------------
/*
    TWin32GUIMainWindow::Start

    Loads the registered tests and applies the command line's options: --project-name to the caption, and
    --filter and the partition options to which tests start out checked, and --filter to the filter box,
    and --shuffle and --shuffle-seed to the shuffle setting.
*/
void TWin32GUIMainWindow::Start(TGUIOptions const& options)
{
    m_Options = options;

    m_Observer.LogTextCallback = [this](std::string const& text)
        {
            AppendLog(text);
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

    for (std::string const& note : GUICommandLineNotes(m_Options, "windows or controls"))
        AppendLog(note + "\n");

    UpdateCaption();

    m_Shuffle.ApplyCommandLine(m_Options.CLI.Shuffle, m_Options.CLI.ShuffleSeed);

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
void TWin32GUIMainWindow::UpdateCaption()
{
    SetWindowTextW(m_Window, UTF8ToWide(GUICaption(m_Options.CLI.ProjectName)).c_str());
}
//---------------------------------------------------------------------------
void TWin32GUIMainWindow::UpdateCommands()
{
    TGUICommandContext context;
    context.FailedCount = m_TestList.StatusCount(TGUITestStatus::Failed);
    context.HasSelection = SelectedTreeItem() != nullptr;
    context.TreeHasNodes = SendMessageW(m_TV_Tests, TVM_GETCOUNT, 0, 0) > 0;

    m_CommandStates = GUICommandStates(context);

    for (EWin32GUICommand const command : AllCommands)
    {
        EnableMenuItem(m_MM_Main, static_cast<UINT>(command),
            MF_BYCOMMAND | (IsCommandEnabled(m_CommandStates, command) ? MF_ENABLED : MF_GRAYED));
    }

    for (TToolbarButton const& button : ToolbarButtons)
        EnableWindow(this->*button.Control, IsCommandEnabled(m_CommandStates, button.Command) ? TRUE : FALSE);

    EnableWindow(m_Edt_Filter, m_CommandStates.FilterBox ? TRUE : FALSE);
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
    // Nothing has run yet.
    std::vector<std::string> const panels = GUIStatusPanels(m_TestList, 0, 0, std::nullopt, m_Shuffle.StatusText(),
        m_StatusMessage);

    for (std::size_t i = 0; i < panels.size(); ++i)
        SendMessageW(m_SB_Status, SB_SETTEXTW, i, reinterpret_cast<LPARAM>(UTF8ToWide(panels[i]).c_str()));
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

} // namespace ASWUnitTests
