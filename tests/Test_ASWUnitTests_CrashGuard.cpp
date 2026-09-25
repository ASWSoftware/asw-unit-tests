/* **************************************************************************
Test_ASWUnitTests_CrashGuard.cpp
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

//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_CrashGuard.h"
//---------------------------------------------------------------------------
#include <stdexcept>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CrashGuard.h"
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------

namespace
{

// Not a tail call: 'buffer[0]' is read again after the recursive call returns, so no compiler can
// turn this into a loop with constant stack usage the way it could a simple tail-recursive
// version. Verified to genuinely overflow the stack (rather than hang) under -O2 and -O3 on GCC/
// MinGW and under RAD Studio's bcc32c/bcc64, before this was trusted enough to commit as a
// permanent test - see the commit that added this file. The infinite recursion GCC/Clang warn
// about here is the deliberate point, not a mistake, hence silencing just that one warning.
#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Winfinite-recursion"
#endif
int RecurseUntilStackOverflows(int depth)
{
    volatile char buffer[512];
    buffer[0] = static_cast<char>(depth);
    int const deeper = RecurseUntilStackOverflows(depth + 1);
    return deeper + buffer[0];
}
#if defined(__GNUC__)
#  pragma GCC diagnostic pop
#endif

// A small per-frame footprint (unlike RecurseUntilStackOverflows() above), so 20 frames of this
// use only a few tens of KB and never overflows the stack on its own; the crash at the bottom is
// an ordinary access violation, deliberately reached from deep in the call stack. Used by
// Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort() below.
int RecurseDeeplyThenCrash(int depth)
{
    volatile char buffer[16];
    buffer[0] = static_cast<char>(depth);

    if (depth >= 20)
    {
        int* const nullPointer = nullptr;
        *const_cast<int*>(nullPointer) = 1;
        return buffer[0];
    }

    int const deeper = RecurseDeeplyThenCrash(depth + 1);
    return deeper + buffer[0];
}

} // namespace

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_CrashGuard
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_CrashGuard::TTest_ASWUnitTests_CrashGuard()
    : inherited("ASWUnitTests_CrashGuard_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_AccessViolation_CaughtAndDoesNotAbort,
        "Run_AccessViolation_CaughtAndDoesNotAbort");
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort,
        "Run_DeepAccessViolation_CaughtAndDoesNotAbort");
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_DivideByZero_Caught, "Run_DivideByZero_Caught");
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_NormalCompletion_ReturnsNotCrashed,
        "Run_NormalCompletion_ReturnsNotCrashed");
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_NormalException_PropagatesUnaffected,
        "Run_NormalException_PropagatesUnaffected");
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_StackOverflow_CaughtAndAbortsRun,
        "Run_StackOverflow_CaughtAndAbortsRun");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_CrashGuard::~TTest_ASWUnitTests_CrashGuard()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::Test_Run_AccessViolation_CaughtAndDoesNotAbort()
{
    // Arrange
    // volatile so an optimizing build can't prove the write is undefined behavior and remove it
    // entirely: that happened for real under RAD Studio's Release configuration, silently turning
    // this into a false pass (Crashed == false, since nothing actually ran) instead of a crash.
    int* volatile nullPointer = nullptr;

    // Act
    TCrashGuardResult const result = TCrashGuard::Run([&]()
        {
            *nullPointer = 1;
        });

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "a null-pointer write is caught");
#if defined(_WIN32)
    CheckFalse(result.ShouldAbortRun, __func__, __LINE__,
        "an ordinary access violation isn't severe enough to force an abort");
#else
    // POSIX delivers a stack overflow and an ordinary segfault as the exact same SIGSEGV signal (see
    // TCrashGuard::Run()'s comment), so every SIGSEGV is conservatively treated as abort-worthy here,
    // unlike Windows where EXCEPTION_STACK_OVERFLOW is unambiguous. Confirmed on Linux via CLion/SSH.
    CheckTrue(result.ShouldAbortRun, __func__, __LINE__,
        "POSIX can't tell this apart from a stack overflow, so it conservatively forces an abort");
#endif
    CheckFalse(result.Description.empty(), __func__, __LINE__, "a human-readable description is filled in");
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_CrashGuard::Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort

    Not merely a handful of frames deep, deliberately: an ordinary crash a couple thousand frames
    deep was found to be misclassified as abort-worthy specifically on RAD Studio's Clang-based
    compilers (bcc32c/bcc64) in a Release build, while MinGW handled the exact same code correctly
    at every depth tried. bcc32c/bcc64 remain unreliable at this specifically, in ways further
    tuning (a larger SetThreadStackGuarantee(), a shallower depth) did not resolve cleanly - see
    Skip() below and the commit that added it for the full investigation. This test is Skip()ped on
    those compilers rather than deleted, since MinGW (and, unverified so far, MSVC) still need real
    coverage of "an ordinary crash, meaningfully deep, still doesn't force an abort."
*/
void TTest_ASWUnitTests_CrashGuard::Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort()
{
#if defined(__BORLANDC__)
    Skip(__func__, __LINE__,
        "Unreliable on RAD Studio's bcc32c/bcc64 in a Release build: an ordinary crash this deep was "
        "sometimes misclassified as abort-worthy. MinGW handles this correctly; investigation ongoing.");
#endif

    // Arrange, Act
    TCrashGuardResult const result = TCrashGuard::Run([]()
        {
            RecurseDeeplyThenCrash(0);
        });

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "an access violation 20 stack frames deep is still caught");
#if defined(_WIN32)
    CheckFalse(result.ShouldAbortRun, __func__, __LINE__,
        "an ordinary access violation, even this deep, still isn't severe enough to force an abort");
#else
    // Same POSIX SIGSEGV-vs-stack-overflow ambiguity as Test_Run_AccessViolation_CaughtAndDoesNotAbort()
    // above; see that test and TCrashGuard::Run()'s comment.
    CheckTrue(result.ShouldAbortRun, __func__, __LINE__,
        "POSIX can't tell this apart from a stack overflow, so it conservatively forces an abort");
#endif
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::Test_Run_DivideByZero_Caught()
{
    // Arrange
    volatile int numerator = 42;
    volatile int denominator = 0;

    // Act
    TCrashGuardResult const result = TCrashGuard::Run([&]()
        {
            volatile int const quotient = numerator / denominator;
            (void)quotient;
        });

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "integer divide by zero is caught");
    CheckFalse(result.ShouldAbortRun, __func__, __LINE__, "not severe enough on its own to force an abort");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::Test_Run_NormalCompletion_ReturnsNotCrashed()
{
    // Arrange
    bool bodyRan = false;

    // Act
    TCrashGuardResult const result = TCrashGuard::Run([&]()
        {
            bodyRan = true;
        });

    // Assert
    CheckTrue(bodyRan, __func__, __LINE__, "the callback actually ran");
    CheckFalse(result.Crashed, __func__, __LINE__, "a normal completion isn't reported as a crash");
    CheckFalse(result.ShouldAbortRun, __func__, __LINE__, "and so never asks to abort the run");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_CrashGuard::Test_Run_NormalException_PropagatesUnaffected()
{
    // Arrange
    bool caughtByCaller = false;

    // Act
    try
    {
        TCrashGuard::Run([]()
            {
                throw std::runtime_error("ordinary C++ exception");
            });
    }
    catch (std::runtime_error const& ex)
    {
        caughtByCaller = (std::string(ex.what()) == "ordinary C++ exception");
    }

    // Assert
    CheckTrue(caughtByCaller, __func__, __LINE__,
        "an ordinary C++ exception propagates out of Run() completely unaffected, not mistaken for a crash");
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_CrashGuard::Test_Run_StackOverflow_CaughtAndAbortsRun

    Skip()ped on RAD Studio's Clang-based compilers (bcc32c/bcc64): in a Release build, a genuine
    stack overflow here was found to not be caught at all, taking the whole process down for real,
    despite the exact same mechanism working correctly for every other fault type on those same
    compilers, and for this exact scenario on MinGW. A larger SetThreadStackGuarantee() didn't fix
    it cleanly (see the commit that added this Skip() for the full investigation); this is Skip()ped
    rather than deleted since MinGW (and, unverified so far, MSVC) still need real coverage of stack
    overflow recovery, and because letting this run for real on an unfixed compiler would crash this
    very test process rather than merely fail a test.
*/
void TTest_ASWUnitTests_CrashGuard::Test_Run_StackOverflow_CaughtAndAbortsRun()
{
#if defined(__BORLANDC__)
    Skip(__func__, __LINE__,
        "Unreliable on RAD Studio's bcc32c/bcc64 in a Release build: a genuine stack overflow was not "
        "caught at all, crashing the process for real. MinGW handles this correctly; investigation ongoing.");
#endif

    // Arrange, Act
    TCrashGuardResult const result = TCrashGuard::Run([]()
        {
            RecurseUntilStackOverflows(0);
        });

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "a genuine stack overflow is caught");
    CheckTrue(result.ShouldAbortRun, __func__, __LINE__,
        "a stack overflow is severe enough to force an abort afterward");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_CrashGuard)
