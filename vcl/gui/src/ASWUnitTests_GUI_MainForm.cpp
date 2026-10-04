/* **************************************************************************
ASWUnitTests_GUI_MainForm.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

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

#include <vcl.h>
#pragma hdrstop

//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_MainForm.h"
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstdint>
#include <exception>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.IniFiles.hpp>
#include <System.IOUtils.hpp>
#include <Vcl.Clipbrd.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_JUnitReport.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
#include "ASWUnitTests_GUI_TextDialog.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

TColor const FailureColor = static_cast<TColor>(RGB(200, 0, 0));
TColor const SkipColor = static_cast<TColor>(RGB(190, 120, 0));

size_t DataToIndex(void* data);
std::string FirstLine(std::string const& text);
std::string FormatSeconds(double seconds);
void* IndexToData(size_t index);
TColor LogLineColor(std::string const& line);
std::vector<TRect> MonitorWorkAreas();
TNodeCheckState ToNodeCheckState(TGUICheckState state);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// A tree node's Data holds an index: a test node's, into TGUITestList; a group node's, into its GroupNames().
// The node's Level tells which (0 for a group).
size_t DataToIndex(void* data)
{
    return static_cast<size_t>(reinterpret_cast<std::uintptr_t>(data));
}
//---------------------------------------------------------------------------
std::string FirstLine(std::string const& text)
{
    return text.substr(0, text.find('\n'));
}
//---------------------------------------------------------------------------
// e.g. "0.84 s"
std::string FormatSeconds(double seconds)
{
    std::ostringstream text;
    text << std::fixed << std::setprecision(2) << seconds << " s";
    return text.str();
}
//---------------------------------------------------------------------------
void* IndexToData(size_t index)
{
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(index));
}
//---------------------------------------------------------------------------
// The Log tab's color for one line of the framework's log output: its failure lines (a failed test or
// check, an unhandled exception, or a fatal SetUp_Test()/TearDown_Test() error) and its skip lines stand out.
TColor LogLineColor(std::string const& line)
{
    if (line.find("***Test failed") != std::string::npos || line.find("**Check failed") != std::string::npos ||
        line.find("\" - failed (") != std::string::npos || line.find("Unhandled exception") != std::string::npos ||
        line.find("!!FATAL ERROR!!") != std::string::npos)
        return FailureColor;

    if (line.find("***Test skipped") != std::string::npos || line.find("\" - skipped (") != std::string::npos)
        return SkipColor;

    return clWindowText;
}
//---------------------------------------------------------------------------
// Every monitor's work area (the part the taskbar and other docked bars leave free), in screen pixels.
std::vector<TRect> MonitorWorkAreas()
{
    std::vector<TRect> workAreas;

    for (int i = 0; i < Screen->MonitorCount; ++i)
        workAreas.push_back(Screen->Monitors[i]->WorkareaRect);

    return workAreas;
}
//---------------------------------------------------------------------------
TNodeCheckState ToNodeCheckState(TGUICheckState state)
{
    switch (state)
    {
        case TGUICheckState::Checked: return ncsChecked;
        case TGUICheckState::Partial: return ncsPartial;
        case TGUICheckState::Unchecked: return ncsUnchecked;
    }

    return ncsUnchecked;
}
//---------------------------------------------------------------------------


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
    explicit TScopedFlag(bool& flag)
        : m_Flag(flag)
    {
        m_Flag = true;
    }

    ~TScopedFlag()
    {
        m_Flag = false;
    }

    TScopedFlag(TScopedFlag const&) = delete;
    TScopedFlag& operator=(TScopedFlag const&) = delete;
};


/////////////////////////////////////////////////////////////////////////////
// TTreeUpdateScope
//
// Brackets a batch of tree node changes with BeginUpdate()/EndUpdate(), so
// the tree repaints once afterward, and EndUpdate() still runs if an
// exception leaves the scope.
/////////////////////////////////////////////////////////////////////////////
class TTreeUpdateScope
{
private:
    TTreeNodes* const m_Nodes;

public:
    explicit TTreeUpdateScope(TTreeNodes* nodes)
        : m_Nodes(nodes)
    {
        m_Nodes->BeginUpdate();
    }

    ~TTreeUpdateScope()
    {
        m_Nodes->EndUpdate();
    }

    TTreeUpdateScope(TTreeUpdateScope const&) = delete;
    TTreeUpdateScope& operator=(TTreeUpdateScope const&) = delete;
};

//---------------------------------------------------------------------------

} // namespace


/////////////////////////////////////////////////////////////////////////////
// TFormASWUnitTestsGUIMain
/////////////////////////////////////////////////////////////////////////////

TFormASWUnitTestsGUIMain* FormASWUnitTestsGUIMain;

//---------------------------------------------------------------------------
__fastcall TFormASWUnitTestsGUIMain::TFormASWUnitTestsGUIMain(TComponent* Owner)
    : inherited(Owner),
      m_CloseAfterRun(false),
      m_LastRunExitCode(ExitCode_Success),
      m_RunFinishedCount(0),
      m_Running(false),
      m_RunTotalCount(0),
      m_StatusMessage("Ready."),
      m_SyncingChecks(false),
      m_TreeNodesStale(false)
{
    m_TreeViewWindowProc = TV_Tests->WindowProc;
    TV_Tests->WindowProc = TV_TestsWindowProc;
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::~TFormASWUnitTestsGUIMain

    This form's own members (m_TestList, m_TestNodes, and so on) are destroyed before the VCL base class
    destroys its controls, and destroying a control can fire its events. So every event that reads those
    members is detached first, along with anything still queued for this form, so none can run against
    members that no longer exist. The tree's own WindowProc is restored for the same reason.
*/
__fastcall TFormASWUnitTestsGUIMain::~TFormASWUnitTestsGUIMain()
{
    TV_Tests->WindowProc = m_TreeViewWindowProc;

    Edt_Filter->OnChange = nullptr;
    LV_Failures->OnCustomDrawItem = nullptr;
    LV_Failures->OnDblClick = nullptr;
    TV_Tests->OnChange = nullptr;
    TV_Tests->OnCheckStateChanging = nullptr;

    TThread::RemoveQueuedEvents(static_cast<TThread*>(nullptr)); // Everything queued with ForceQueue(nullptr, ...).
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_AboutExecute(TObject* /*Sender*/)
{
    RunGuarded("About", []()
    {
        ShowTextDialog(System::UnicodeString("About ") + Application->Title,
            TTestHandler::GetVersionFullStr() + "\n"
            "VCL GUI runner\n"
            "\n"
            "App bits: "
#if defined (_WIN64)
            "64"
#else
            "32"
#endif
            "\n\n"
            "https://github.com/ASWSoftware/asw-unit-tests\n"
            "\n"
            "Copyright 2026 Anthony S. West\n"
            "Licensed under the Apache License, Version 2.0.\n");
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_CommandLineOptionsExecute(TObject* /*Sender*/)
{
    RunGuarded("Command Line Options", []()
    {
        ShowTextDialog(Application->Title + System::UnicodeString(" - Command Line Options"),
            TGUICommandLine::HelpText());
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_CopyDetailsExecute(TObject* /*Sender*/)
{
    RunGuarded("Copy Details", [this]()
    {
        Clipboard()->AsText = Mem_Detail->Text;
        m_StatusMessage = "Details copied to the clipboard.";
        UpdateStatusBar();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_ExitExecute(TObject* /*Sender*/)
{
    // During a run, FormCloseQuery() stops it after the current test, then closes.
    RunGuarded("Exit", [this]()
    {
        Close();
    });
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::Act_ExportJUnitReportExecute

    Saves the latest run's JUnit report, the same one --report-junit writes, e.g. when --report-junit wasn't
    given. The dialog starts at --report-junit's path if there is one, and then at the last one chosen.
*/
void __fastcall TFormASWUnitTestsGUIMain::Act_ExportJUnitReportExecute(TObject* /*Sender*/)
{
    RunGuarded("Export JUnit Report", [this]()
    {
        if (m_Running || !m_LastRunJUnitTestCases.has_value())
            return;

        if (SD_JUnitReport->FileName.IsEmpty() && !m_Options.CLI.JunitReportPath.empty())
        {
            SD_JUnitReport->FileName = System::Sysutils::ExpandFileName(FromUTF8(m_Options.CLI.JunitReportPath));
        }
        else if (SD_JUnitReport->FileName.IsEmpty())
        {
            // e.g. "ASWUnitTests_VCL_GUI_JUnit.xml", in the dialog's default folder.
            SD_JUnitReport->FileName = System::Sysutils::ChangeFileExt(
                System::Sysutils::ExtractFileName(System::ParamStr(0)), "_JUnit.xml");
        }

        if (!SD_JUnitReport->Execute(Handle))
            return;

        std::string error;
        if (!WriteJUnitReport(SD_JUnitReport->FileName, error))
        {
            Application->MessageBox(FromUTF8("The JUnit report couldn't be written to:\n" +
                ToUTF8(SD_JUnitReport->FileName) + "\n\n" + error).c_str(), Application->Title.c_str(),
                MB_OK | MB_ICONERROR);
            return;
        }

        m_StatusMessage = "JUnit report exported.";
        UpdateStatusBar();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_FocusFilterExecute(TObject* /*Sender*/)
{
    RunGuarded("Filter", [this]()
    {
        if (!Edt_Filter->CanFocus())
            return;

        Edt_Filter->SetFocus();
        Edt_Filter->SelectAll();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_ResetLayoutExecute(TObject* /*Sender*/)
{
    RunGuarded("Reset Layout", [this]()
    {
        WindowState = wsNormal;

        // The default size, centered on the monitor the window is on now.
        if (m_DefaultLayout.Window.has_value())
        {
            CenterOnMonitor(ScaleFrom96(m_DefaultLayout.Window->Width, CurrentPPI),
                ScaleFrom96(m_DefaultLayout.Window->Height, CurrentPPI));
        }

        ApplyPanelSizes(m_DefaultLayout);
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_RunFailedExecute(TObject* /*Sender*/)
{
    RunGuarded("Run Failed", [this]()
    {
        if (m_TestList.StatusCount(TGUITestStatus::Failed) == 0)
            return;

        // Built before RunTests() resets every result, while the failures are still known.
        RunTests(m_TestList.FailedFilter(), "the previous run's failures");
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_RunSelectedExecute(TObject* /*Sender*/)
{
    RunGuarded("Run Selected", [this]()
    {
        if (m_TestList.CheckedCount() == 0)
        {
            m_StatusMessage = "No tests are checked.";
            UpdateStatusBar();
            return;
        }

        RunCheckedTests();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_SelectAllExecute(TObject* /*Sender*/)
{
    RunGuarded("Select All", [this]()
    {
        m_TestList.SetAllChecked(true);
        SyncTreeChecks();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_SelectFailedExecute(TObject* /*Sender*/)
{
    RunGuarded("Select Failed", [this]()
    {
        m_TestList.CheckOnlyFailed();
        SyncTreeChecks();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_SelectNoneExecute(TObject* /*Sender*/)
{
    RunGuarded("Select None", [this]()
    {
        m_TestList.SetAllChecked(false);
        SyncTreeChecks();
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::Act_StopExecute(TObject* /*Sender*/)
{
    RunGuarded("Stop", [this]()
    {
        m_Observer.RequestStop();
        m_StatusMessage = "Stopping after the current test...";
        UpdateStatusBar();
    });
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::AddFailureItem(size_t testIndex)
{
    TTestId const& test = m_TestList.Test(testIndex);
    TGUITestResult const* const result = m_TestList.Result(testIndex);

    TListItem* const item = LV_Failures->Items->Add();
    item->Caption = FromUTF8(StatusName(m_TestList.Status(testIndex)));
    item->SubItems->Add(FromUTF8(test.GroupName + "." + test.TestName));
    item->SubItems->Add(FromUTF8(result != nullptr ? FirstLine(result->Record.Message) : std::string()));
    item->Data = IndexToData(testIndex);
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::AppendLog(std::string const& text)
{
    // One line at a time, so each can have its own color. A trailing piece with no line break (from
    // LogAppend()) is appended as-is, and the next piece continues its line.
    size_t start = 0;
    while (start < text.size())
    {
        size_t const end = text.find('\n', start);
        bool const endsLine = (end != std::string::npos);
        std::string const line = text.substr(start, endsLine ? end - start : std::string::npos);

        // Past the end is clamped to the end, which sidesteps the rich edit counting a line break as one
        // character here but two in GetTextLen().
        RE_Log->SelStart = RE_Log->GetTextLen();
        RE_Log->SelLength = 0;
        RE_Log->SelAttributes->Color = LogLineColor(line);
        RE_Log->SelText = FromUTF8(line) + (endsLine ? System::UnicodeString("\r\n") : System::UnicodeString());

        start = endsLine ? end + 1 : text.size();
    }

    ::SendMessage(RE_Log->Handle, WM_VSCROLL, SB_BOTTOM, 0);
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::ApplyFilterBox

    The text matches anywhere in a test's "Group.Test" full name, ignoring case, and may use the same '*'
    and '?' wildcards as --filter. Unlike --filter, it's never anchored to the whole name, so "Parse?rg"
    finds "...ParseArguments...", just as plain text would.
*/
void TFormASWUnitTestsGUIMain::ApplyFilterBox()
{
    std::string const text = ToUTF8(Edt_Filter->Text.Trim());

    if (text.empty())
    {
        m_TestList.SetVisibleFilter(TestFilter());
    }
    else
    {
        std::string const pattern = "*" + text + "*";
        m_TestList.SetVisibleFilter([pattern](std::string const& fullTestName)
        {
            return TTestHandler::WildcardMatch(pattern, fullTestName, true);
        });
    }

    BuildTestTree();
    UpdateDetail();
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::ApplyPanelSizes(TGUILayout const& layout)
{
    // Kept within what the splitters allow, so each can still be dragged and nothing is squeezed out of view.
    if (layout.TestTreeWidth.has_value())
    {
        int const maximum = ClientWidth - Spl_Tests->Width - Spl_Tests->MinSize;
        TV_Tests->Width = std::max(Spl_Tests->MinSize,
            std::min(ScaleFrom96(*layout.TestTreeWidth, CurrentPPI), maximum));
    }

    if (layout.DetailHeight.has_value())
    {
        int const maximum = Pnl_Results->ClientHeight - Spl_Detail->Height - Spl_Detail->MinSize;
        Mem_Detail->Height = std::max(Spl_Detail->MinSize,
            std::min(ScaleFrom96(*layout.DetailHeight, CurrentPPI), maximum));
    }
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::BuildTestTree

    Rebuilds the tree from m_TestList: one node per group, in order, each with one child node per test.
*/
void TFormASWUnitTestsGUIMain::BuildTestTree()
{
    {
        TTreeUpdateScope const updating(TV_Tests->Items);

        TV_Tests->Items->Clear();
        m_GroupNodes.assign(m_TestList.GroupNames().size(), nullptr);
        m_TestNodes.assign(m_TestList.Count(), nullptr);

        // Only visible tests get nodes, and a group only gets one if any of its tests are visible.
        std::vector<std::string> const& groupNames = m_TestList.GroupNames();
        for (size_t groupIndex = 0; groupIndex < groupNames.size(); ++groupIndex)
        {
            std::vector<size_t> const testIndexes = m_TestList.TestsInGroup(groupNames[groupIndex]);
            if (testIndexes.empty())
                continue;

            TTreeNode* const groupNode = TV_Tests->Items->AddObject(nullptr, FromUTF8(groupNames[groupIndex]),
                IndexToData(groupIndex));
            m_GroupNodes[groupIndex] = groupNode;

            for (size_t testIndex : testIndexes)
            {
                m_TestNodes[testIndex] = TV_Tests->Items->AddChildObject(groupNode,
                    FromUTF8(m_TestList.Test(testIndex).TestName), IndexToData(testIndex));
            }
        }

        // The nodes just added are the tree's current ones, even if its window was recreated before this.
        m_TreeNodesStale = false;

        TV_Tests->FullExpand();
    }

    RefreshAllNodeImages();
    SyncTreeChecks();

    if (TV_Tests->Items->Count > 0)
        TV_Tests->Items->GetFirstNode()->MakeVisible();
}
//---------------------------------------------------------------------------
// Sizes the window to 'width' x 'height' pixels and centers it on its monitor's work area.
void TFormASWUnitTestsGUIMain::CenterOnMonitor(int width, int height)
{
    TRect const workArea = Monitor->WorkareaRect;
    SetBounds(std::max(workArea.Left, workArea.Left + (workArea.Width() - width) / 2),
        std::max(workArea.Top, workArea.Top + (workArea.Height() - height) / 2), width, height);
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::CreateStatusImages

    Draws one colored dot per TGUITestStatus into IL_Status, in the enum's order, so a status converts
    directly to its image index. Drawn at runtime, scaled to the form's DPI, rather than stored as images.
*/
void TFormASWUnitTestsGUIMain::CreateStatusImages()
{
    // In TGUITestStatus order: NotRun, Running, Passed, Failed, Skipped.
    TColor const colors[] = {
        static_cast<TColor>(RGB(190, 190, 190)),
        static_cast<TColor>(RGB(0, 120, 215)),
        static_cast<TColor>(RGB(16, 160, 16)),
        static_cast<TColor>(RGB(215, 40, 40)),
        static_cast<TColor>(RGB(230, 165, 0))
    };

    int const size = ScaleValue(16);
    int const inset = ScaleValue(3);

    IL_Status->Clear();
    IL_Status->SetSize(size, size);

    for (TColor const color : colors)
    {
        std::unique_ptr<Vcl::Graphics::TBitmap> const bitmap(new Vcl::Graphics::TBitmap());
        bitmap->SetSize(size, size);
        bitmap->Canvas->Brush->Color = clFuchsia; // Transparent, via AddMasked() below.
        bitmap->Canvas->FillRect(TRect(0, 0, size, size));
        bitmap->Canvas->Brush->Color = color;
        bitmap->Canvas->Pen->Color = color;
        bitmap->Canvas->Ellipse(inset, inset, size - inset, size - inset);
        IL_Status->AddMasked(bitmap.get(), clFuchsia);
    }
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::CurrentLayout

    The window's restored bounds come from GetWindowPlacement(), so a maximized (or minimized) window still
    saves the size it restores to.
*/
TGUILayout TFormASWUnitTestsGUIMain::CurrentLayout()
{
    TGUILayout layout;
    int const pixelsPerInch = CurrentPPI;

    layout.DetailHeight = ScaleTo96(Mem_Detail->Height, pixelsPerInch);
    layout.TestTreeWidth = ScaleTo96(TV_Tests->Width, pixelsPerInch);

    WINDOWPLACEMENT placement{};
    placement.length = sizeof(placement);

    if (::GetWindowPlacement(Handle, &placement))
    {
        TRect const bounds = WorkspaceToScreen(TRect(placement.rcNormalPosition), Screen->PrimaryMonitor->BoundsRect,
            Screen->PrimaryMonitor->WorkareaRect);

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
void __fastcall TFormASWUnitTestsGUIMain::Edt_FilterChange(TObject* /*Sender*/)
{
    RunGuarded("the filter box", [this]()
    {
        ApplyFilterBox();
    });
}
//---------------------------------------------------------------------------
int TFormASWUnitTestsGUIMain::ExitCode() const
{
    return m_Options.ExitAfterRun ? m_LastRunExitCode : ExitCode_Success;
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::FailRunOnUnhandledException(std::string const& description)
{
    m_Observer.Flush();

    // The test that was running when the exception escaped never finished, so it's the one that failed.
    if (m_CurrentTestIndex.has_value())
    {
        TTestId const& test = m_TestList.Test(*m_CurrentTestIndex);
        m_TestList.SetResult(*m_CurrentTestIndex, TTestCaseRecord{ test.GroupName, test.TestName, 0.0,
                                                                   TTestOutcome::Fail, "Unhandled exception: " + description }, std::string());
        UpdateTestNodeImages(*m_CurrentTestIndex);
        AddFailureItem(*m_CurrentTestIndex);
        m_CurrentTestIndex.reset();
    }

    PB_Progress->State = pbsError;
    m_StatusMessage = "The run was ended by an unhandled exception: " + description;
    AppendLog(m_StatusMessage + "\n");
    UpdateDetail();
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::FormClose(TObject* /*Sender*/, TCloseAction& /*Action*/)
{
    RunGuarded("closing the window", [this]()
    {
        // An --exit run is automated, so nobody arranged its window.
        if (!m_Options.ExitAfterRun && !m_Options.LayoutIgnore)
            SaveLayout();
    });
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::FormCloseQuery

    A run can't be interrupted partway through a test, so closing during one asks it to stop after the
    current test, and closes the window once it has (see RunTests()).
*/
void __fastcall TFormASWUnitTestsGUIMain::FormCloseQuery(TObject* /*Sender*/, bool& CanClose)
{
    RunGuarded("closing the window", [this, &CanClose]()
    {
        if (!m_Running)
            return;

        CanClose = false;
        m_CloseAfterRun = true;
        m_Observer.RequestStop();
        m_StatusMessage = "Closing after the current test...";
        UpdateStatusBar();
    });
}
//---------------------------------------------------------------------------
TTreeNode* TFormASWUnitTestsGUIMain::GroupNode(size_t groupIndex)
{
    if (m_TreeNodesStale)
        RebuildNodeCache();

    return groupIndex < m_GroupNodes.size() ? m_GroupNodes[groupIndex] : nullptr;
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::LoadLayout

    Called from Start(), before the window is first shown. The panel sizes are applied once it's showing,
    since a window about to open maximized doesn't have the size they have to fit into until then.
*/
void TFormASWUnitTestsGUIMain::LoadLayout()
{
    TGUILayout layout;

    try
    {
        System::UnicodeString const path = GUILayoutFilePath();
        if (!System::Sysutils::FileExists(path))
            return;

        std::unique_ptr<TMemIniFile> const ini(new TMemIniFile(path));
        layout = LoadGUILayout(*ini);
    }
    catch (...)
    {
        return; // A layout that can't be read only means the window opens with its defaults.
    }

    if (layout.Window.has_value())
    {
        TGUIWindowBounds const& window = *layout.Window;
        TRect const bounds(window.Left, window.Top, window.Left + ScaleFrom96(window.Width, CurrentPPI),
            window.Top + ScaleFrom96(window.Height, CurrentPPI));

        // Not where it can't be reached, e.g. on a monitor that's since been disconnected. Set as the window
        // placement's normal position, as VCL itself does when centering a form that opens maximized: that's
        // where such a window restores to, and setting BoundsRect before it's first shown doesn't change it.
        WINDOWPLACEMENT placement{};
        placement.length = sizeof(placement);

        if (IsWindowReachable(bounds, MonitorWorkAreas()) && ::GetWindowPlacement(Handle, &placement))
        {
            TRect const normal = ScreenToWorkspace(bounds, Screen->PrimaryMonitor->BoundsRect,
                Screen->PrimaryMonitor->WorkareaRect);
            placement.rcNormalPosition.left = normal.Left;
            placement.rcNormalPosition.top = normal.Top;
            placement.rcNormalPosition.right = normal.Right;
            placement.rcNormalPosition.bottom = normal.Bottom;
            placement.showCmd = SW_HIDE; // Application->Run() shows it.
            ::SetWindowPlacement(Handle, &placement);
        }
    }

    if (layout.Maximized)
        WindowState = wsMaximized;

    TThread::ForceQueue(nullptr, [this, layout]()
    {
        RunGuarded("restoring the layout", [this, &layout]()
        {
            ApplyPanelSizes(layout);
        });
    });
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::LV_FailuresCustomDrawItem(TCustomListView* Sender, TListItem* Item,
    TCustomDrawState /*State*/, bool& /*DefaultDraw*/)
{
    // Not RunGuarded(): this runs while painting, where showing an error would only cause more painting.
    // An item it can't match to a test is simply drawn in the default color.
    size_t const index = DataToIndex(Item->Data);
    if (index >= m_TestList.Count())
        return;

    bool const failed = m_TestList.Status(index) == TGUITestStatus::Failed;
    Sender->Canvas->Font->Color = failed ? FailureColor : SkipColor;
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::LV_FailuresDblClick

    Selects the double-clicked failure's test in the tree, which shows its details.
*/
void __fastcall TFormASWUnitTestsGUIMain::LV_FailuresDblClick(TObject* /*Sender*/)
{
    RunGuarded("the Failures & Skips list", [this]()
    {
        TListItem* const item = LV_Failures->Selected;
        if (item == nullptr)
            return;

        size_t const testIndex = DataToIndex(item->Data);
        if (testIndex >= m_TestList.Count())
            return;

        // A failure the filter box is hiding can't be selected, so the filter is cleared to show it.
        if (TestNode(testIndex) == nullptr)
            Edt_Filter->Text = System::UnicodeString();     // Its OnChange rebuilds the tree.

        TTreeNode* const node = TestNode(testIndex);
        if (node == nullptr)
            return;

        TV_Tests->Selected = node;
        node->MakeVisible();
        TV_Tests->SetFocus();
    });
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::OnTestFinished(TTestCaseRecord const& record, std::string const& testLog)
{
    std::optional<size_t> const index = m_TestList.IndexOf(record.GroupName, record.TestName);
    if (index.has_value())
    {
        m_TestList.SetResult(*index, record, testLog);
        UpdateTestNodeImages(*index);

        if (record.Outcome != TTestOutcome::Pass)
            AddFailureItem(*index);
    }

    m_CurrentTestIndex.reset();
    ++m_RunFinishedCount;
    PB_Progress->Position = static_cast<int>(m_RunFinishedCount);

    if (record.Outcome == TTestOutcome::Fail)
        PB_Progress->State = pbsError;

    UpdateDetail();
    UpdateStatusBar();
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::OnTestStarted(std::string const& groupName, std::string const& testName)
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
/*
    TFormASWUnitTestsGUIMain::RebuildNodeCache

    A recreated window loads its nodes back from what the old one saved, each with the Data BuildTestTree()
    gave it: a group node's index into m_TestList.GroupNames(), or a test node's into m_TestList. Reading
    the nodes also creates the window, if it hasn't been yet, so it always sees the loaded ones.
*/
void TFormASWUnitTestsGUIMain::RebuildNodeCache()
{
    m_GroupNodes.assign(m_TestList.GroupNames().size(), nullptr);
    m_TestNodes.assign(m_TestList.Count(), nullptr);

    for (TTreeNode* node = TV_Tests->Items->GetFirstNode(); node != nullptr; node = node->GetNext())
    {
        std::vector<TTreeNode*>& nodes = (node->Level == 0) ? m_GroupNodes : m_TestNodes;
        size_t const index = DataToIndex(node->Data);

        if (index < nodes.size())
            nodes[index] = node;
    }

    m_TreeNodesStale = false;
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::RefreshAllNodeImages()
{
    TTreeUpdateScope const updating(TV_Tests->Items);

    std::vector<std::string> const& groupNames = m_TestList.GroupNames();
    for (size_t groupIndex = 0; groupIndex < groupNames.size(); ++groupIndex)
    {
        TTreeNode* const node = GroupNode(groupIndex);
        if (node == nullptr)
            continue;

        int const image = static_cast<int>(m_TestList.GroupStatus(groupNames[groupIndex]));
        node->ImageIndex = image;
        node->SelectedIndex = image;
    }

    for (size_t testIndex = 0; testIndex < m_TestList.Count(); ++testIndex)
    {
        TTreeNode* const node = TestNode(testIndex);
        if (node == nullptr)
            continue;

        int const image = static_cast<int>(m_TestList.Status(testIndex));
        node->ImageIndex = image;
        node->SelectedIndex = image;
    }
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::ResetSavedLayout

    For --layout-reset. The file is deleted now, rather than only not loaded, so a layout that caused a
    problem is gone even if the window doesn't get to close normally this time.
*/
void TFormASWUnitTestsGUIMain::ResetSavedLayout()
{
    System::UnicodeString const path = GUILayoutFilePath();
    std::string outcome;

    if (!System::Sysutils::FileExists(path))
    {
        outcome = "there was no saved window layout to delete";
    }
    else
    {
        try
        {
            System::Ioutils::TFile::Delete(path);
        }
        catch (...)
        {
            // Reported below, from whether the file is still there.
        }

        outcome = System::Sysutils::FileExists(path) ?
                "the saved window layout couldn't be deleted (" + ToUTF8(path) + "), so it's ignored this time" :
                "the saved window layout was deleted, so the window opens with the defaults";
    }

    AppendLog("Note: --layout-reset: " + outcome + ".\n");
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::RunCheckedTests()
{
    RunTests(m_TestList.CheckedFilter(), "the checked tests");
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::RunGuarded

    Runs an event handler's body. A C++ exception can't be reported by the VCL, which can only show one
    that escapes an event handler as an unexplained "External exception EEFFACE", so one is caught here and
    reported with its message and 'where' it happened, both in a message box and in the log. A VCL (RTL)
    exception is rethrown, to be shown by the VCL as usual.
*/
void TFormASWUnitTestsGUIMain::RunGuarded(char const* where, std::function<void()> const& body)
{
    auto report = [this, where](std::string const& description)
        {
            std::string const message = std::string("An unexpected error occurred in ") + where + ": " + description;
            AppendLog(message + "\n");
            Application->MessageBox(FromUTF8(message).c_str(), Application->Title.c_str(), MB_OK | MB_ICONERROR);
        };

    try
    {
        body();
    }
    catch (System::Sysutils::Exception&)
    {
        throw;
    }
    catch (std::exception const& ex)
    {
        report(ex.what());
    }
    catch (...)
    {
        report("an object that isn't a std::exception or an RTL Exception");
    }
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::RunTests

    Runs on this (the main) thread, so tests that create VCL forms or controls work; the observer's
    YieldCallback lets the window repaint and respond to Stop between tests. Every test's result, the
    Failures & Skips list, and the log are cleared first, so they only ever show one run.
*/
void TFormASWUnitTestsGUIMain::RunTests(TestFilter const& filter, std::string const& filterDescription)
{
    if (m_Running)
        return;

    m_TestList.ResetResults();
    RefreshAllNodeImages();
    LV_Failures->Items->Clear();
    RE_Log->Clear();
    UpdateDetail();

    m_RunTotalCount = m_TestList.CountMatching(filter);
    m_RunFinishedCount = 0;
    m_CurrentTestIndex.reset();
    m_RunStart = std::chrono::steady_clock::now();
    m_RunEnd.reset();
    m_LastRunJUnitTestCases.reset();

    PB_Progress->Max = static_cast<int>(std::max<size_t>(m_RunTotalCount, 1));
    PB_Progress->Position = 0;
    PB_Progress->State = pbsNormal;

    m_Observer.ResetStop();
    m_StatusMessage = "Running...";
    SetRunning(true);

    try
    {
        TTestResults const results = m_Handler->Run(filter, filterDescription, m_Options.CLI.Shuffle,
            m_Options.CLI.ShuffleSeed, m_Options.CLI.TestTimeoutSeconds, m_Options.CLI.CatchCrashes);
        m_Observer.Flush();

        if (results.Stopped)
            m_StatusMessage = "Stopped before every test ran.";
        else if (results.TimedOut)
            m_StatusMessage = "A test timed out, so the rest of the run was abandoned.";
        else if (results.Crashed)
            m_StatusMessage = "A test crashed, so the rest of the run was abandoned.";
        else
            m_StatusMessage = "Done.";

        m_LastRunExitCode = ExitCodeForResults(results);
        m_LastRunJUnitTestCases = ToJUnitTestCases(results.CaseRecords);

        // A failure is only noted in the log, so it can't hold up --exit.
        if (!m_Options.CLI.JunitReportPath.empty())
        {
            std::string error;
            WriteJUnitReport(FromUTF8(m_Options.CLI.JunitReportPath), error);
        }
    }
    // As in the console runner, an unhandled exception ends the run without a report, with its own exit code.
    catch (std::exception const& ex)
    {
        m_LastRunExitCode = ExitCode_UnhandledException;
        FailRunOnUnhandledException(ex.what());
    }
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    catch (System::Sysutils::Exception& ex)
    {
        m_LastRunExitCode = ExitCode_UnhandledException;
        FailRunOnUnhandledException(DescribeRTLException(ex));
    }
#endif
    catch (...)
    {
        m_LastRunExitCode = ExitCode_UnhandledExceptionUnknown;
        FailRunOnUnhandledException("an object that isn't a std::exception or an RTL Exception");
    }

    m_RunEnd = std::chrono::steady_clock::now();
    SetRunning(false);

    if (m_CloseAfterRun)
    {
        // Not Close() directly: this may still be inside a button's click handling.
        TThread::ForceQueue(nullptr, [this]()
        {
            Close();
        });
    }
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::SaveLayout()
{
    try
    {
        System::UnicodeString const path = GUILayoutFilePath();
        System::Sysutils::ForceDirectories(System::Sysutils::ExtractFileDir(path));

        std::unique_ptr<TMemIniFile> const ini(new TMemIniFile(path));
        SaveGUILayout(*ini, CurrentLayout());
        ini->UpdateFile();
    }
    catch (...)
    {
        // Not saving it (e.g. a read-only profile) only means the next start uses the defaults.
    }
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::SetRunning(bool running)
{
    m_Running = running;

    // A modal dialog opened mid-run would hold up the rest of the run until it was closed, since the tests
    // run on this thread.
    Act_About->Enabled = !running;
    Act_CommandLineOptions->Enabled = !running;
    Act_ExportJUnitReport->Enabled = !running && m_LastRunJUnitTestCases.has_value();

    Act_RunFailed->Enabled = !running && m_TestList.StatusCount(TGUITestStatus::Failed) > 0;
    Act_RunSelected->Enabled = !running;
    Act_SelectAll->Enabled = !running;
    Act_SelectFailed->Enabled = !running && m_TestList.StatusCount(TGUITestStatus::Failed) > 0;
    Act_SelectNone->Enabled = !running;
    Act_Stop->Enabled = running;

    // Rebuilding the tree mid-run would discard the nodes the run is updating.
    Act_FocusFilter->Enabled = !running;
    Edt_Filter->Enabled = !running;

    UpdateStatusBar();
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::Start

    Loads the registered tests and applies the command line's options: --project-name to the caption, and
    --filter and the partition options to which tests start out checked, and --filter to the filter box.
    Also restores the window layout saved when it last closed.
*/
void TFormASWUnitTestsGUIMain::Start(TGUIOptions const& options)
{
    m_Options = options;

    // The GUI shows the framework's log output as plain text, so it must never contain ANSI color codes.
    TConsole::SetColorMode(TColorMode::Never);

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
    m_Observer.YieldCallback = []()
        {
            Application->ProcessMessages();
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

    for (std::string const& ignored : m_Options.IgnoredOptions)
        AppendLog("Note: " + ignored + " has no effect in the GUI, so it was ignored.\n");

    if (m_Options.CLI.TestTimeoutSeconds.has_value())
    {
        AppendLog("Note: --test-timeout-seconds runs each test on a worker thread, so a test that creates VCL "
            "forms or controls won't work with it.\n");
    }

    if (m_Options.LayoutIgnore)
        AppendLog("Note: --layout-ignore: the window layout isn't loaded or saved, so it opens with the defaults.\n");

    if (m_Options.LayoutReset)
        ResetSavedLayout();

    UpdateCaption();
    CreateStatusImages();

    // The filter box's case-insensitive contains-match shows every test --filter's pattern matches (and
    // maybe a few more, left unchecked), so seeding it with the pattern never hides a checked test.
    if (m_Options.CLI.HasFilter)
        Edt_Filter->Text = FromUTF8(m_Options.CLI.FilterPattern);

    // Called directly, since setting the text above doesn't necessarily raise OnChange before the window
    // is showing. Builds the tree.
    ApplyFilterBox();

    // Centered here rather than with the DFM's Position, which is poDesigned because LoadLayout() would
    // otherwise have to change Position at runtime, which needlessly recreates the whole window.
    CenterOnMonitor(Width, Height);

    // The DFM's layout, captured before LoadLayout() changes it, for Reset Layout. Loaded before --run is
    // queued below, so the panel sizes LoadLayout() queues are applied before the run starts.
    m_DefaultLayout = CurrentLayout();

    if (!m_Options.LayoutIgnore && !m_Options.LayoutReset)
        LoadLayout();

    if (m_Options.RunOnStart)
    {
        // Queued, so it starts once Application->Run() is processing messages and the window is showing.
        TThread::ForceQueue(nullptr, [this]()
        {
            RunGuarded("--run", [this]()
            {
                // Unlike Run Selected, runs even if nothing is checked, so --exit still reports and exits
                // the way the console runner does when its filter matches nothing.
                RunCheckedTests();

                if (m_Options.ExitAfterRun)
                    Close();
            });
        });
    }
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::SyncTreeChecks

    Sets every node's check box from m_TestList: a test's to whether it's checked, and a group's to whether
    all, none, or only some of its tests are.
*/
void TFormASWUnitTestsGUIMain::SyncTreeChecks()
{
    TScopedFlag const syncing(m_SyncingChecks);

    for (TTreeNode* node = TV_Tests->Items->GetFirstNode(); node != nullptr; node = node->GetNext())
    {
        size_t const index = DataToIndex(node->Data);

        if (node->Level == 0)
            node->CheckState = ToNodeCheckState(m_TestList.GroupCheckState(m_TestList.GroupNames()[index]));
        else
            node->CheckState = m_TestList.IsChecked(index) ? ncsChecked : ncsUnchecked;
    }

    UpdateStatusBar(); // For its checked count.
}
//---------------------------------------------------------------------------
TTreeNode* TFormASWUnitTestsGUIMain::TestNode(size_t testIndex)
{
    if (m_TreeNodesStale)
        RebuildNodeCache();

    return testIndex < m_TestNodes.size() ? m_TestNodes[testIndex] : nullptr;
}
//---------------------------------------------------------------------------
void __fastcall TFormASWUnitTestsGUIMain::TV_TestsChange(TObject* /*Sender*/, TTreeNode* /*Node*/)
{
    RunGuarded("the test tree", [this]()
    {
        UpdateDetail();
    });
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::TV_TestsCheckStateChanging

    With partial check boxes enabled, a click can cycle a check box into its partial state, which only a
    group's own tests should decide. So every click is taken over here: it toggles the clicked test, or all
    of the clicked group's tests, in m_TestList, and the tree is then updated from m_TestList, which also
    works out each group's partial state.
*/
void __fastcall TFormASWUnitTestsGUIMain::TV_TestsCheckStateChanging(TCustomTreeView* /*Sender*/, TTreeNode* Node,
    TNodeCheckState /*NewCheckState*/, TNodeCheckState OldCheckState, bool& AllowChange)
{
    if (m_SyncingChecks)
        return;

    AllowChange = false;

    RunGuarded("a test tree check box", [this, Node, OldCheckState]()
    {
        // The run in progress already decided which tests it runs, so a change now would only mislead.
        if (m_Running)
            return;

        bool const check = (OldCheckState != ncsChecked);
        size_t const index = DataToIndex(Node->Data);

        if (Node->Level == 0)
            m_TestList.SetGroupChecked(m_TestList.GroupNames()[index], check);
        else
            m_TestList.SetChecked(index, check);

        // Changing check boxes from inside the tree's own change notification isn't safe, so the update
        // waits until this click has been fully handled.
        TThread::ForceQueue(nullptr, [this]()
        {
            RunGuarded("a test tree check box", [this]()
            {
                SyncTreeChecks();
            });
        });
    });
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::TV_TestsWindowProc

    VCL recreates the tree's window when Windows' system colors change (e.g. switching high contrast on or
    off), and when some of its properties change. Recreating it frees every TTreeNode, and loads new ones in
    their place, without the OnDeletion event that would otherwise say so.
*/
void __fastcall TFormASWUnitTestsGUIMain::TV_TestsWindowProc(TMessage& message)
{
    if (message.Msg == WM_DESTROY)
        m_TreeNodesStale = true;

    m_TreeViewWindowProc(message);
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::UpdateCaption

    "ASWUnitTests - Framework version: <version>", plus " - <project name>" when --project-name gave one
    other than the default.
*/
void TFormASWUnitTestsGUIMain::UpdateCaption()
{
    std::string caption = "ASWUnitTests - Framework version: " + TTestHandler::GetVersionStr();

    if (m_Options.CLI.ProjectName != TCLIOptions().ProjectName)
        caption += " - " + m_Options.CLI.ProjectName;

    Caption = FromUTF8(caption);
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::UpdateDetail()
{
    TTreeNode* const node = TV_Tests->Selected;
    Act_CopyDetails->Enabled = (node != nullptr);

    if (node == nullptr)
    {
        Mem_Detail->Clear();
        return;
    }

    size_t const index = DataToIndex(node->Data);
    std::string const text = (node->Level == 0) ? m_TestList.GroupDetailText(m_TestList.GroupNames()[index]) :
            m_TestList.DetailText(index);

    Mem_Detail->Text = System::Sysutils::AdjustLineBreaks(FromUTF8(text));
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::UpdateStatusBar()
{
    auto setPanel = [this](int panel, std::string const& text)
        {
            SB_Status->Panels->Items[panel]->Text = FromUTF8(text);
        };

    // Counts only the tests the filter box shows, since those are the ones Run Selected runs.
    std::string checked = "Checked: " + std::to_string(m_TestList.CheckedCount()) + " / " +
        std::to_string(m_TestList.VisibleCount());
    if (m_TestList.VisibleCount() != m_TestList.Count())
        checked += " shown";
    setPanel(0, checked);
    setPanel(1, "Run: " + std::to_string(m_RunFinishedCount) + " / " + std::to_string(m_RunTotalCount));
    setPanel(2, "Passed: " + std::to_string(m_TestList.StatusCount(TGUITestStatus::Passed)));
    setPanel(3, "Failed: " + std::to_string(m_TestList.StatusCount(TGUITestStatus::Failed)));
    setPanel(4, "Skipped: " + std::to_string(m_TestList.StatusCount(TGUITestStatus::Skipped)));

    std::string elapsed;
    if (m_RunStart.has_value())
    {
        std::chrono::steady_clock::time_point const end = m_RunEnd.value_or(std::chrono::steady_clock::now());
        elapsed = FormatSeconds(std::chrono::duration<double>(end - *m_RunStart).count());
    }
    setPanel(5, elapsed);
    setPanel(6, m_StatusMessage);
}
//---------------------------------------------------------------------------
void TFormASWUnitTestsGUIMain::UpdateTestNodeImages(size_t testIndex)
{
    TTreeNode* const testNode = TestNode(testIndex);
    if (testNode == nullptr)
        return; // Hidden by the filter box, e.g. a hidden failure rerun by Run Failed.

    int const testImage = static_cast<int>(m_TestList.Status(testIndex));
    testNode->ImageIndex = testImage;
    testNode->SelectedIndex = testImage;

    TTreeNode* const groupNode = testNode->Parent;
    int const groupImage = static_cast<int>(m_TestList.GroupStatus(m_TestList.GroupNames()[DataToIndex(
        groupNode->Data)]));
    groupNode->ImageIndex = groupImage;
    groupNode->SelectedIndex = groupImage;
}
//---------------------------------------------------------------------------
/*
    TFormASWUnitTestsGUIMain::WriteJUnitReport

    Writes the file itself, rather than with TJUnitReportWriter::Write(), whose std::ofstream can't open a
    path with characters outside the system code page (e.g. a user folder's name) from a std::string.
*/
bool TFormASWUnitTestsGUIMain::WriteJUnitReport(System::UnicodeString const& path, std::string& error)
{
    error.clear();

    try
    {
        if (!m_LastRunJUnitTestCases.has_value())
            throw std::runtime_error("no run has finished");

        std::string const xml = TJUnitReportWriter::BuildXML(m_Handler->GetProjectName(), *m_LastRunJUnitTestCases);

        std::unique_ptr<TFileStream> const file(new TFileStream(path, fmCreate));
        file->WriteBuffer(xml.data(), static_cast<NativeInt>(xml.size()));
    }
    catch (System::Sysutils::Exception& ex)
    {
        error = ToUTF8(ex.Message);
    }
    catch (std::exception const& ex)
    {
        error = ex.what();
    }

    if (error.empty())
        AppendLog("JUnit report written to: " + ToUTF8(path) + "\n");
    else
        AppendLog("Error: could not write JUnit report to: " + ToUTF8(path) + " (" + error + ")\n");

    return error.empty();
}
//---------------------------------------------------------------------------
