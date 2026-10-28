/* **************************************************************************
Test_ASWUnitTests_GUI_View.h
Author: Anthony S. West - ASW Software

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
#ifndef Test_ASWUnitTests_GUI_ViewH
#define Test_ASWUnitTests_GUI_ViewH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_View
//
// Exercises the text the GUI runners show (gui/src/ASWUnitTests_GUI_View.h).
// Listed in every project, so it also runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_View : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_ClassifyGUILogLine_FailuresAndSkipsStandOut();
    void Test_FormatGUISeconds_TwoDecimals();
    void Test_GUIAboutText_NamesTheRunnerAndVersion();
    void Test_GUICaption_AddsAProjectNameOtherThanTheDefault();
    void Test_GUICommandLineNotes_OnePerOptionHandledDifferently();
    void Test_GUIDefaultJUnitReportFileName_ReplacesTheExtension();
    void Test_GUIFilterBoxFilter_MatchesAnywhereIgnoringCase();
    void Test_GUIFirstLine_StopsAtTheFirstLineBreak();
    void Test_GUIStatusPanels_ShowCountsAndMessage();
    void Test_GUIStatusPanelWidths_OnePerPanel();
    void Test_GUIUnhandledExceptionRecord_FailsTheRunningTest();
    void Test_SummarizeGUIRun_DescribesHowTheRunEnded();

public:
    TTest_ASWUnitTests_GUI_View();
    ~TTest_ASWUnitTests_GUI_View() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_ViewH
