/* **************************************************************************
Test_ASWUnitTests_GUI_Commands.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
#include "Test_ASWUnitTests_GUI_Commands.h"
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Commands.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

//---------------------------------------------------------------------------

std::string EnabledCommands(TGUICommandStates const& states);

//---------------------------------------------------------------------------
/*
    EnabledCommands

    The names of the enabled commands, in TGUICommandStates' order, separated by spaces. Compared whole, so a
    failure shows every command's state.
*/
std::string EnabledCommands(TGUICommandStates const& states)
{
    std::string text;

    auto const add = [&text](bool enabled, char const* name)
        {
            if (enabled)
                text += (text.empty() ? "" : " ") + std::string(name);
        };

    add(states.About, "About");
    add(states.CollapseAll, "CollapseAll");
    add(states.CommandLineOptions, "CommandLineOptions");
    add(states.CopyDetails, "CopyDetails");
    add(states.Exit, "Exit");
    add(states.ExpandAll, "ExpandAll");
    add(states.ExportJUnitReport, "ExportJUnitReport");
    add(states.FilterBox, "FilterBox");
    add(states.FocusFilter, "FocusFilter");
    add(states.ResetLayout, "ResetLayout");
    add(states.RunFailed, "RunFailed");
    add(states.RunSelected, "RunSelected");
    add(states.SelectAll, "SelectAll");
    add(states.SelectFailed, "SelectFailed");
    add(states.SelectNone, "SelectNone");
    add(states.ShuffleOrder, "ShuffleOrder");
    add(states.ShuffleSeed, "ShuffleSeed");
    add(states.Stop, "Stop");

    return text;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Commands
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Commands::TTest_ASWUnitTests_GUI_Commands()
    : inherited("ASWUnitTests_GUI_Commands_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_AbortedRunCannotRunAgain,
        "GUICommandStates_AbortedRunCannotRunAgain");
    RegisterTest(&TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_FailuresEnableRunAndSelectFailed,
        "GUICommandStates_FailuresEnableRunAndSelectFailed");
    RegisterTest(&TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_IdleWithNothingRunYet,
        "GUICommandStates_IdleWithNothingRunYet");
    RegisterTest(&TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_ReportSelectionAndTreeEnableTheirCommands,
        "GUICommandStates_ReportSelectionAndTreeEnableTheirCommands");
    RegisterTest(&TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_RunningLeavesOnlyHarmlessCommands,
        "GUICommandStates_RunningLeavesOnlyHarmlessCommands");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Commands::~TTest_ASWUnitTests_GUI_Commands()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_AbortedRunCannotRunAgain()
{
    // Arrange: a run that a test's timeout or crash ended, with failures and a report.
    TGUICommandContext context;
    context.FailedCount = 1;
    context.HasReport = true;
    context.RunAborted = true;

    // Act and Assert
    CheckEquals(std::string("About CommandLineOptions Exit ExportJUnitReport FilterBox FocusFilter ResetLayout "
        "SelectAll SelectFailed SelectNone ShuffleOrder ShuffleSeed"), EnabledCommands(GUICommandStates(context)),
        __func__, __LINE__, "neither Run Selected nor Run Failed, but everything else that doesn't run tests");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_FailuresEnableRunAndSelectFailed()
{
    // Arrange
    TGUICommandContext context;
    context.FailedCount = 2;

    // Act and Assert
    CheckEquals(std::string("About CommandLineOptions Exit FilterBox FocusFilter ResetLayout RunFailed RunSelected "
        "SelectAll SelectFailed SelectNone ShuffleOrder ShuffleSeed"), EnabledCommands(GUICommandStates(context)),
        __func__, __LINE__, "Run Failed and Select Failed, once a test has failed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_IdleWithNothingRunYet()
{
    // Act and Assert: as the window opens, before its tree is built.
    CheckEquals(std::string("About CommandLineOptions Exit FilterBox FocusFilter ResetLayout RunSelected SelectAll "
        "SelectNone ShuffleOrder ShuffleSeed"), EnabledCommands(GUICommandStates(TGUICommandContext())), __func__,
        __LINE__, "nothing that needs a failure, a report, a selection, or tree nodes, and not Stop");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_ReportSelectionAndTreeEnableTheirCommands()
{
    // Arrange
    TGUICommandContext report;
    report.HasReport = true;
    TGUICommandContext selection;
    selection.HasSelection = true;
    TGUICommandContext tree;
    tree.TreeHasNodes = true;

    // Act
    TGUICommandStates const reportStates = GUICommandStates(report);
    TGUICommandStates const selectionStates = GUICommandStates(selection);
    TGUICommandStates const treeStates = GUICommandStates(tree);

    // Assert
    CheckTrue(reportStates.ExportJUnitReport, __func__, __LINE__, "a report can be exported");
    CheckFalse(reportStates.CopyDetails || reportStates.ExpandAll, __func__, __LINE__, "but nothing else changes");
    CheckTrue(selectionStates.CopyDetails, __func__, __LINE__, "a selected node's details can be copied");
    CheckFalse(selectionStates.ExportJUnitReport || selectionStates.ExpandAll, __func__, __LINE__,
        "nothing else changes");
    CheckTrue(treeStates.CollapseAll && treeStates.ExpandAll, __func__, __LINE__,
        "a tree can be expanded and collapsed");
    CheckFalse(treeStates.ExportJUnitReport || treeStates.CopyDetails, __func__, __LINE__, "nothing else changes");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Commands::Test_GUICommandStates_RunningLeavesOnlyHarmlessCommands()
{
    // Arrange: every other condition met, so only the run itself disables commands.
    TGUICommandContext context;
    context.FailedCount = 3;
    context.HasReport = true;
    context.HasSelection = true;
    context.Running = true;
    context.TreeHasNodes = true;

    // Act and Assert
    CheckEquals(std::string("CollapseAll CopyDetails Exit ExpandAll ResetLayout Stop"),
        EnabledCommands(GUICommandStates(context)), __func__, __LINE__,
        "Stop, and only the commands that can't change the run or hold it up with a dialog");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Commands)
