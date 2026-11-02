/* **************************************************************************
ASWUnitTests_WebGUI_API.h
Author: Anthony S. West - ASW Software

The Web GUI runner's API: the runner's state, which its server keeps and
changes as runs and commands go, and the JSON its page is sent from it,
built with the same shared units (gui/src) the other GUI runners use, so
the page shows what they show. Contains no socket code, so it's unit
tested from the console runners (see
web/tests/Test_ASWUnitTests_WebGUI_API.cpp).

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
#ifndef ASWUnitTests_WebGUI_APIH
#define ASWUnitTests_WebGUI_APIH
//---------------------------------------------------------------------------
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Commands.h"
#include "ASWUnitTests_GUI_Shuffle.h"
#include "ASWUnitTests_GUI_TestList.h"
#include "ASWUnitTests_GUI_Theme.h"
#include "ASWUnitTests_GUI_Tree.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWebGUILog
//
// The Web GUI runner's log, as whole lines, so each line's kind
// (ClassifyGUILogLine()) is that of all its text: log text arrives in
// pieces that needn't end with a line break, and a piece that continues
// the last line changes that line, which is then sent again whole. It
// keeps where the lines not yet sent to the page start, so each flush sends
// one log event for all of them (see WebGUILogEvent()). Nothing is dropped,
// as in the other GUI runners.
/////////////////////////////////////////////////////////////////////////////
class TWebGUILog
{
private:
    // The first line changed since MarkSent(); empty if none was.
    std::optional<std::size_t> m_FirstUnsentLine;
    // Whether the last line has no line break yet, so the next text continues it.
    bool m_LastLineOpen;
    // The lines, without their line breaks.
    std::vector<std::string> m_Lines;

public:
    TWebGUILog();

    // Adds 'text', in which each '\n' ends a line; text after the last one starts a line, or continues the
    // last one if it had no line break yet, which the next text continues.
    void Append(std::string const& text);
    // Removes every line, e.g. at the start of a run; the page is then sent the log from line 0.
    void Clear();
    // The first line that changed (was added, continued or ended) or, after Clear(), 0, since MarkSent();
    // empty if nothing changed.
    std::optional<std::size_t> FirstUnsentLine() const;
    std::vector<std::string> const& Lines() const;
    // Records that the page was sent every line, after a flush.
    void MarkSent();
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIState
//
// What the Web GUI runner shows, beyond each tab's own selection, expanded
// nodes and scroll positions: its server keeps one, changes it as runs and
// the page's commands go, and builds the page's JSON from it, so every tab
// shows the same.
/////////////////////////////////////////////////////////////////////////////
struct TWebGUIState
{
    // The View > Theme > Dark Theme choice: GUIDefaultDarkThemeName() or a theme file's name.
    std::string DarkTheme = GUIDefaultDarkThemeName();
    // How long the current or latest run has taken, which the server sets before each flush; empty if no run
    // has started.
    std::optional<double> ElapsedSeconds;
    // The indexes of the tests that failed or were skipped in this run, in the order they finished, for the
    // Failures & Skips list.
    std::vector<std::size_t> FailureOrder;
    // The filter box's text.
    std::string FilterText;
    // Whether a finished run's JUnit report can be exported.
    bool HasReport = false;
    TWebGUILog Log;
    // --project-name's name, for the caption.
    std::string ProjectName = TCLIOptions().ProjectName;
    // Whether a run has ended with a test that timed out or crashed, after which no further run can start.
    bool RunAborted = false;
    // How many tests of the current or latest run have finished.
    std::size_t RunFinishedCount = 0;
    bool Running = false;
    // How many tests the current or latest run has.
    std::size_t RunTotalCount = 0;
    TGUIShuffle Shuffle;
    // The status bar's message, e.g. "Done.".
    std::string StatusMessage;
    TGUITestList TestList;
    // The View > Theme choice.
    EGUITheme Theme = EGUITheme::Windows;
};

//---------------------------------------------------------------------------

// The "checks" event, sent after any check box changes: {"type":"checks","checks":"..."}, one digit per node of
// BuildGUITree()'s tree, in its order, each its TGUICheckState's value.
std::string WebGUIChecksEvent(TWebGUIState const& state);
// The command states the page is sent: GUICommandStates() as if a node were selected, since the selection is
// each tab's own, so the page enables Copy Details only while its tab has one too.
TGUICommandStates WebGUICommandStates(TWebGUIState const& state);
// The answer to the page's request for a node's detail text: {"node":"<id>","text":"..."}, the text being
// GUINodeDetailText()'s. 'ref' must be a node of the state's test list.
std::string WebGUIDetailJSON(TWebGUIState const& state, TGUINodeRef const& ref);
// The "log" event, sent when the log changed: {"type":"log","from":<from>,"lines":[[text, kind], ...]}, the log's
// lines from index 'from' on, each with its TGUILogLineKind's value. The page drops its lines from 'from' on and
// adds these, so sending a line twice changes nothing, and after Clear() 'from' is 0. Throws std::logic_error if
// 'from' is past the log's end.
std::string WebGUILogEvent(TWebGUIState const& state, std::size_t from);
// A node's id in the page's JSON: its kind's letter ('s', 'g' or 't') and its index, e.g. "g3", so it stays
// the same while the test list does, whatever the filter box hides.
std::string WebGUINodeId(TGUINodeRef const& ref);
// The first event of every event stream, with all the page shows: {"type":"snapshot",...} with the caption,
// the tree event's members, the state event's members, "log", every line of the log as the log event writes
// them, and "failures", the Failures & Skips list's rows, in FailureOrder's order, each [id, status name,
// "Group.Test", the first line of its message]. Throws std::logic_error if FailureOrder has an index that
// isn't a test's, which would be a bug in the server.
std::string WebGUISnapshotEvent(TWebGUIState const& state);
// The "state" event, sent whenever anything changed: {"type":"state",...} with "commands" (each of
// WebGUICommandStates()' flags, named as TGUICommandStates' members), "panels" (GUIStatusPanels()' texts),
// "progress" ("finished" and "total" tests, and "failed", whether any test failed, which turns the bar red),
// "shuffle" ("enabled", "fixedSeed" or null, and "seedToShow", Shuffle Seed...'s offer) and "theme"
// ("choice", "Windows", "Light" or "Dark", and "dark", the dark theme's name).
std::string WebGUIStateEvent(TWebGUIState const& state);
// The "finished" event, sent when the test at 'index' finished: the started event's members, as they are now,
// and, if it failed or was skipped, "failure", its Failures & Skips row as the snapshot writes it. Throws
// std::logic_error if 'index' isn't a test's.
std::string WebGUITestFinishedEvent(TWebGUIState const& state, std::size_t index);
// The "started" event, sent when the test at 'index' started: {"type":"started","test":"<id>","nodes":{...}},
// "nodes" giving the status of each node the test's status changes, by id: its own, its group's and its suites',
// innermost first; none while the filter box hides it, since a group's or suite's status is that of its visible
// tests. Throws std::logic_error if 'index' isn't a test's.
std::string WebGUITestStartedEvent(TWebGUIState const& state, std::size_t index);
// The "tree" event, sent after the filter box changes: {"type":"tree",...} with "filter", the filter box's
// text, "nodes", BuildGUITree()'s nodes in order, each {"id","parent","text"} ("parent" being null for a
// top-level node), and "statuses" and "checks", one digit per node in the same order, each its
// TGUITestStatus' or TGUICheckState's value.
std::string WebGUITreeEvent(TWebGUIState const& state);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_WebGUI_APIH
