/* **************************************************************************
Test_ASWUnitTests_TestBase.h
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
#ifndef Test_ASWUnitTests_TestBaseH
#define Test_ASWUnitTests_TestBaseH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_TestBase
//
// Exercises TTestGroupBase's own Check*/Assert*/Skip and Run() bookkeeping,
// by constructing a small unregistered fixture group (see the .cpp), running
// it directly, and inspecting its Results() - never through the real,
// self-registered suite, so a deliberately-triggered failure here never
// affects this run's actual pass/fail counts or exit code.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_TestBase : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_CheckNear_ToleranceBoundaryIsInclusive();
    void Test_Check_ContinuesButAssert_Aborts();
    void Test_Run_AppliesFilterToSkipNonMatchingTests();
    void Test_Run_RecordsOutcomeCountsAndCaseRecords();
    void Test_Run_ShuffleSeedProducesDeterministicOrder();
    void Test_SetExceptionExpected_MatchesTypeAndMessage();
    void Test_SetLogSuppressed_SilencesFixtureOutput();

public:
    TTest_ASWUnitTests_TestBase();
    ~TTest_ASWUnitTests_TestBase() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_TestBaseH
