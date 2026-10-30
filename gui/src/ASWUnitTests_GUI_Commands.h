/* **************************************************************************
ASWUnitTests_GUI_Commands.h
Author: Anthony S. West - ASW Software

Which of a GUI runner's commands are enabled, independent of the menus and
buttons that offer them.

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
#ifndef ASWUnitTests_GUI_CommandsH
#define ASWUnitTests_GUI_CommandsH
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUICommandContext
//
// What decides which of a GUI runner's commands are enabled; see
// GUICommandStates().
/////////////////////////////////////////////////////////////////////////////
struct TGUICommandContext
{
    // Number of tests that failed in the latest run, visible or not.
    size_t FailedCount = 0;
    // Whether a finished run's JUnit report can be exported.
    bool HasReport = false;
    // Whether a test tree node is selected, so the detail pane shows something.
    bool HasSelection = false;
    // Whether a run has ended with a test that timed out or crashed, after which no further run can start.
    bool RunAborted = false;
    bool Running = false;
    // Whether the test tree has any nodes, i.e. the filter box doesn't hide every test.
    bool TreeHasNodes = false;
};


/////////////////////////////////////////////////////////////////////////////
// TGUICommandStates
//
// Whether each of a GUI runner's commands is enabled, one member per
// command, named as in the VCL GUI runner's actions (e.g. RunSelected for
// Act_RunSelected), plus the filter box. Theme is the GUI runners' View >
// Theme menu, its commands together, its Dark Theme submenu's included.
/////////////////////////////////////////////////////////////////////////////
struct TGUICommandStates
{
    bool About = false;
    bool CollapseAll = false;
    bool CommandLineOptions = false;
    bool CopyDetails = false;
    bool Exit = false;
    bool ExpandAll = false;
    bool ExportJUnitReport = false;
    bool FilterBox = false;
    bool FocusFilter = false;
    bool ResetLayout = false;
    bool RunFailed = false;
    bool RunSelected = false;
    bool SelectAll = false;
    bool SelectFailed = false;
    bool SelectNone = false;
    bool ShuffleOrder = false;
    bool ShuffleSeed = false;
    bool Stop = false;
    bool Theme = false;
};

//---------------------------------------------------------------------------

// Which of a GUI runner's commands are enabled in 'context'. Contains no UI code, so it's unit tested from the
// console runners (see gui/tests/Test_ASWUnitTests_GUI_Commands.cpp). The rules:
// - During a run, only Stop and the commands that can't affect it (Exit, which stops the run first, Copy
//   Details, Expand All, Collapse All, and Reset Layout) are enabled. Tests run on the GUI's main thread, so a
//   modal dialog (About, Command Line Options, Shuffle Seed, Export JUnit Report) would hold up the run until
//   it closed; the run's tests and order are already decided, so checking tests and changing the shuffle
//   would only mislead; changing the filter would rebuild the tree the run is updating; and changing the theme
//   would restyle (in the VCL GUI runner, recreate) the windows the run is updating.
// - Run Selected and Run Failed stay disabled once a run has been aborted.
// - Run Failed and Select Failed need a failed test, Export JUnit Report a finished run's report, Copy Details a
//   selected node, and Expand All and Collapse All a tree with nodes.
TGUICommandStates GUICommandStates(TGUICommandContext const& context);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_CommandsH
