/* **************************************************************************
Test_ASWUnitTests_GUI_TestList.h
Author: Anthony S. West - ASW Software

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

//---------------------------------------------------------------------------
#ifndef Test_ASWUnitTests_GUI_TestListH
#define Test_ASWUnitTests_GUI_TestListH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_TestList
//
// Exercises the VCL GUI runner's test list and check state model
// (vcl/gui/src/ASWUnitTests_GUI_TestList.h). Listed in both vcl/ projects,
// so it also runs from the VCL console runner.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_TestList : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_CheckedFilter_MatchesExactlyTheCheckedTests();
    void Test_CheckMatching_ChecksExactlyTheMatches();
    void Test_CountMatching_CountsFilterMatches();
    void Test_DetailText_DescribesAFinishedTest();
    void Test_DetailText_DescribesATestThatHasNotRun();
    void Test_FailedFilter_MatchesEveryFailedTestEvenHiddenOnes();
    void Test_GroupDetailText_CountsEachStatus();
    void Test_GroupStatus_ShowsRunningThenWorstOutcome();
    void Test_IndexOf_FindsTestsByGroupAndTestName();
    void Test_Load_ChecksEveryTestAndListsGroupsInOrder();
    void Test_ResetResults_SetsEveryTestBackToNotRun();
    void Test_SetAllChecked_ChecksOrUnchecksEveryTest();
    void Test_SetChecked_UpdatesItsGroupsCheckState();
    void Test_SetGroupChecked_ChangesOnlyThatGroup();
    void Test_SetResult_StoresTheResultAndSetsTheStatus();
    void Test_SetVisibleFilter_HidesNonMatchingTests();
    void Test_SetVisibleFilter_LimitsCheckingToVisibleTests();
    void Test_StatusCount_CountsTestsWithEachStatus();
    void Test_StatusFromOutcome_MapsEachOutcome();
    void Test_StatusName_NamesEachStatus();

public:
    TTest_ASWUnitTests_GUI_TestList();
    ~TTest_ASWUnitTests_GUI_TestList() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_TestListH
