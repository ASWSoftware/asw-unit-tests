/* **************************************************************************
Test_ASWUnitTests_Win32GUI_MainLayout.cpp
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
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_Win32GUI_MainLayout.h"
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_MainLayout.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

std::string Bounds(TGUIRect const& rect);

//---------------------------------------------------------------------------
/*
    Bounds

    The rectangle as "left,top,right,bottom", to compare one with a single CheckEquals().
*/
std::string Bounds(TGUIRect const& rect)
{
    return std::to_string(rect.Left) + "," + std::to_string(rect.Top) + "," + std::to_string(rect.Right) + "," +
        std::to_string(rect.Bottom);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_MainLayout
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_MainLayout::TTest_ASWUnitTests_Win32GUI_MainLayout()
    : inherited("ASWUnitTests_Win32GUI_MainLayout_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampDetailHeight_LeavesRoomForTheResultsTabs,
        "ClampDetailHeight_LeavesRoomForTheResultsTabs");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampDetailHeight_MinimumWinsInASmallWindow,
        "ClampDetailHeight_MinimumWinsInASmallWindow");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampTestTreeWidth_LeavesRoomForTheResults,
        "ClampTestTreeWidth_LeavesRoomForTheResults");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampTestTreeWidth_MinimumWinsInASmallWindow,
        "ClampTestTreeWidth_MinimumWinsInASmallWindow");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_GivesNoNegativeHeightInAVeryShortWindow,
        "LayOutMainWindow_GivesNoNegativeHeightInAVeryShortWindow");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_KeepsThePanelSizesWhenTheWindowShrinks,
        "LayOutMainWindow_KeepsThePanelSizesWhenTheWindowShrinks");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_MatchesTheDFMAt96DPI,
        "LayOutMainWindow_MatchesTheDFMAt96DPI");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_ScalesWithTheDPI,
        "LayOutMainWindow_ScalesWithTheDPI");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_MainLayout::~TTest_ASWUnitTests_Win32GUI_MainLayout()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampDetailHeight_LeavesRoomForTheResultsTabs()
{
    // Arrange: the .dfm's window, with 580 pixels between the toolbar and the progress bar; 515 leaves the
    // splitter (5) and the tabs' minimum (60) below the detail pane.

    // Act, Assert
    CheckEquals(170, ClampDetailHeight(170, 661, 96), __func__, __LINE__, "a size that fits is kept");
    CheckEquals(60, ClampDetailHeight(10, 661, 96), __func__, __LINE__, "at least the minimum");
    CheckEquals(515, ClampDetailHeight(600, 661, 96), __func__, __LINE__, "leaving the tabs their minimum");
    CheckEquals(772, ClampDetailHeight(900, 992, 144), __func__, __LINE__,
        "at 144 DPI: 870 between the bars, less the splitter (8) and the tabs' minimum (90)");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampDetailHeight_MinimumWinsInASmallWindow()
{
    // Act, Assert: 100 pixels between the toolbar and the progress bar can't fit both minimums.
    CheckEquals(60, ClampDetailHeight(80, 181, 96), __func__, __LINE__, "the detail pane's minimum");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampTestTreeWidth_LeavesRoomForTheResults()
{
    // Act, Assert
    CheckEquals(380, ClampTestTreeWidth(380, 1008, 96), __func__, __LINE__, "a size that fits is kept");
    CheckEquals(150, ClampTestTreeWidth(20, 1008, 96), __func__, __LINE__, "at least the minimum");
    CheckEquals(853, ClampTestTreeWidth(1000, 1008, 96), __func__, __LINE__,
        "leaving the splitter (5) and the results' minimum (150)");
    CheckEquals(225, ClampTestTreeWidth(20, 1512, 144), __func__, __LINE__, "the minimum at 144 DPI");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_ClampTestTreeWidth_MinimumWinsInASmallWindow()
{
    // Act, Assert
    CheckEquals(150, ClampTestTreeWidth(200, 250, 96), __func__, __LINE__, "the tree's minimum");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_GivesNoNegativeHeightInAVeryShortWindow()
{
    // Act: a client area so short that the progress bar (21 to 41) is above the toolbar's bottom (42).
    TWin32GUIMainLayout const layout = LayOutMainWindow(100, 60, 380, 170, 96);

    // Assert
    CheckEquals(std::string("0,42,380,42"), Bounds(layout.TestTree), __func__, __LINE__, "the tree has no height");
    CheckEquals(std::string("380,42,385,42"), Bounds(layout.TestsSplitter), __func__, __LINE__,
        "nor has its splitter");
    CheckEquals(std::string("0,21,100,41"), Bounds(layout.ProgressBar), __func__, __LINE__,
        "the progress bar stays above the status bar");
    CheckEquals(std::string("0,41,100,60"), Bounds(layout.StatusBar), __func__, __LINE__,
        "the status bar stays at the bottom");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_KeepsThePanelSizesWhenTheWindowShrinks()
{
    // Act: a window narrower than the tree and shorter than the detail pane.
    TWin32GUIMainLayout const layout = LayOutMainWindow(300, 200, 380, 170, 96);

    // Assert
    CheckEquals(std::string("0,42,380,161"), Bounds(layout.TestTree), __func__, __LINE__, "the tree keeps its width");
    CheckEquals(std::string("385,42,385,212"), Bounds(layout.Detail), __func__, __LINE__,
        "the detail pane keeps its height, with no width left");
    CheckEquals(std::string("385,217,385,217"), Bounds(layout.Results), __func__, __LINE__,
        "the results tabs get nothing, never a negative size");
    CheckEquals(std::string("0,161,300,181"), Bounds(layout.ProgressBar), __func__, __LINE__,
        "the progress bar stays above the status bar");
    CheckEquals(std::string("0,181,300,200"), Bounds(layout.StatusBar), __func__, __LINE__,
        "the status bar stays at the bottom");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_MatchesTheDFMAt96DPI()
{
    // Act: the .dfm's client size, tree width and detail height.
    TWin32GUIMainLayout const layout = LayOutMainWindow(1008, 661, 380, 170, 96);

    // Assert: each control's Left, Top, Left + Width and Top + Height in the .dfm (the results' within
    // Pnl_Results, which starts at 385,42).
    CheckEquals(std::string("0,42,380,622"), Bounds(layout.TestTree), __func__, __LINE__, "TV_Tests");
    CheckEquals(std::string("380,42,385,622"), Bounds(layout.TestsSplitter), __func__, __LINE__, "Spl_Tests");
    CheckEquals(std::string("385,42,1008,212"), Bounds(layout.Detail), __func__, __LINE__, "Mem_Detail");
    CheckEquals(std::string("385,212,1008,217"), Bounds(layout.DetailSplitter), __func__, __LINE__, "Spl_Detail");
    CheckEquals(std::string("385,217,1008,622"), Bounds(layout.Results), __func__, __LINE__, "PC_Results");
    CheckEquals(std::string("0,622,1008,642"), Bounds(layout.ProgressBar), __func__, __LINE__, "PB_Progress");
    CheckEquals(std::string("0,642,1008,661"), Bounds(layout.StatusBar), __func__, __LINE__, "SB_Status");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_MainLayout::Test_LayOutMainWindow_ScalesWithTheDPI()
{
    // Act: at 144 DPI, the toolbar is 63 tall, the progress bar 30, the status bar 29 (28.5, rounded) and the
    // splitters 8 (7.5, rounded).
    TWin32GUIMainLayout const layout = LayOutMainWindow(1512, 992, 570, 255, 144);

    // Assert
    CheckEquals(std::string("0,63,570,933"), Bounds(layout.TestTree), __func__, __LINE__, "TV_Tests");
    CheckEquals(std::string("570,63,578,933"), Bounds(layout.TestsSplitter), __func__, __LINE__, "Spl_Tests");
    CheckEquals(std::string("578,63,1512,318"), Bounds(layout.Detail), __func__, __LINE__, "Mem_Detail");
    CheckEquals(std::string("578,318,1512,326"), Bounds(layout.DetailSplitter), __func__, __LINE__, "Spl_Detail");
    CheckEquals(std::string("578,326,1512,933"), Bounds(layout.Results), __func__, __LINE__, "PC_Results");
    CheckEquals(std::string("0,933,1512,963"), Bounds(layout.ProgressBar), __func__, __LINE__, "PB_Progress");
    CheckEquals(std::string("0,963,1512,992"), Bounds(layout.StatusBar), __func__, __LINE__, "SB_Status");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_MainLayout)
