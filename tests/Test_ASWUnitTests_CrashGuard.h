/* **************************************************************************
Test_ASWUnitTests_CrashGuard.h
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
#ifndef Test_ASWUnitTests_CrashGuardH
#define Test_ASWUnitTests_CrashGuardH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_CrashGuard
//
// Exercises TCrashGuard::Run() directly: a deliberate access violation (both
// shallow and 20 stack frames deep - see
// Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort()'s comment for why both
// matter), an integer divide by zero, and a genuine stack overflow (via a
// recursive function shaped so the compiler can't turn it into a loop under
// optimization - see Test_Run_StackOverflow_CaughtAndAbortsRun()'s comment,
// and the commit that added it for the -O2/-O3 verification across all four
// Windows compilers this project targets before this was trusted enough to
// commit). If TCrashGuard::Run() ever regresses into not catching one of
// these, the corresponding test hangs rather than failing normally; this is
// exactly the scenario --test-timeout-seconds (see the CI workflow) exists
// to contain.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_CrashGuard : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_Run_AccessViolation_CaughtAndDoesNotAbort();
    void Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort();
    void Test_Run_DivideByZero_Caught();
    void Test_Run_NormalCompletion_ReturnsNotCrashed();
    void Test_Run_NormalException_PropagatesUnaffected();
    void Test_Run_StackOverflow_CaughtAndAbortsRun();

public:
    TTest_ASWUnitTests_CrashGuard();
    ~TTest_ASWUnitTests_CrashGuard() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_CrashGuardH
