/* **************************************************************************
Test_ASWUnitTests_CrashGuard.cpp
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
#include "Test_ASWUnitTests_CrashGuard.h"
//---------------------------------------------------------------------------
#include <stdexcept>
#include <string>
#include <thread>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_CrashGuard.h"
//---------------------------------------------------------------------------

namespace
{

// Written through by CrashInsideTryBlock() and RecurseDeeplyThenCrash() below. volatile, so no
// compiler can prove the write is through a null pointer, which is undefined behavior it may assume
// never happens: Clang in a Release build (RAD Studio's bcc64x, llvm-mingw) treated the branch that
// wrote through a plain local null pointer in RecurseDeeplyThenCrash() as unreachable and dropped it
// with its depth check, so the recursion never stopped and overflowed the stack instead. At namespace
// scope, not a local: GCC 13 and 14 at -O3 drop the write through a local 'int* volatile' initialized
// to nullptr in the same function, so nothing crashes (seen in the Linux CI build; GCC 15 and Clang
// keep it).
int* volatile g_NullPointer = nullptr;

//---------------------------------------------------------------------------

void CrashInsideTryBlock();
int RecurseDeeplyThenCrash(int depth);
int RecurseUntilStackOverflows(int depth);

//---------------------------------------------------------------------------
/*
    CrashInsideTryBlock

    A try block's handler is linked into Win32's chain of exception handlers (NT_TIB::ExceptionList)
    while it's entered, so this crashes with the chain's head pointing into this function's stack
    frame, which the jump out of the crash abandons. A test run by the framework crashes like this,
    since the framework calls each test from inside its own try blocks. The crash is called through a
    volatile pointer so no compiler can see it can't throw and drop the try block. Used by
    Test_Run_AccessViolation_LaterExceptionStillCaught() below.
*/
void CrashInsideTryBlock()
{
    using TCrash = void (*)();
    TCrash volatile crash = []()
        {
            *g_NullPointer = 1;
        };

    try
    {
        crash();
    }
    catch (std::exception const&)
    {
    }
}
//---------------------------------------------------------------------------
/*
    RecurseDeeplyThenCrash

    A small per-frame footprint (unlike RecurseUntilStackOverflows() below), so 20 frames of this
    use only a few tens of KB and never overflows the stack on its own; the crash at the bottom is
    an ordinary access violation, deliberately reached from deep in the call stack. It writes through
    g_NullPointer, not a local null pointer, so the optimizer can't drop it (see g_NullPointer). Used
    by Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort() below.
*/
int RecurseDeeplyThenCrash(int depth)
{
    volatile char buffer[16];
    buffer[0] = static_cast<char>(depth);

    if (depth >= 20)
    {
        *g_NullPointer = 1;
        return buffer[0];
    }

    int const deeper = RecurseDeeplyThenCrash(depth + 1);
    return deeper + buffer[0];
}
//---------------------------------------------------------------------------
/*
    RecurseUntilStackOverflows

    Not a tail call: 'buffer[0]' is read again after the recursive call returns, so no compiler can
    turn this into a loop with constant stack usage the way it could a simple tail-recursive
    version. Verified to genuinely overflow the stack (rather than hang) under -O2 and -O3 on GCC/
    MinGW and under RAD Studio's bcc32c/bcc64, before this was trusted enough to commit as a
    permanent test - see the commit that added this file. The infinite recursion GCC/Clang/MSVC warn
    about here is the deliberate point, not a mistake, hence silencing just that one warning.
    bcc32c is Clang-based but, unlike bcc64x, doesn't define __GNUC__, so it needs its own branch.
*/
#if defined(__GNUC__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Winfinite-recursion"
#elif defined(__BORLANDC__) && defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Winfinite-recursion"
#elif defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4717)
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
#elif defined(__BORLANDC__) && defined(__clang__)
#  pragma clang diagnostic pop
#elif defined(_MSC_VER)
#  pragma warning(pop)
#endif
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

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
    RegisterTest(&TTest_ASWUnitTests_CrashGuard::Test_Run_AccessViolation_LaterExceptionStillCaught,
        "Run_AccessViolation_LaterExceptionStillCaught");
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
    CheckEquals(std::string("access violation"), result.Description, __func__, __LINE__,
        "the null-pointer write itself crashed, not something the optimizer turned it into");
    CheckFalse(result.ShouldAbortRun, __func__, __LINE__,
        "an ordinary access violation isn't severe enough to force an abort");
#else
    // POSIX delivers a stack overflow and an ordinary segfault as the exact same SIGSEGV signal (see
    // TCrashGuard::Run()'s comment), so every SIGSEGV is conservatively treated as abort-worthy here,
    // unlike Windows where EXCEPTION_STACK_OVERFLOW is unambiguous. Confirmed on Linux via CLion/SSH.
    CheckTrue(result.ShouldAbortRun, __func__, __LINE__,
        "POSIX can't tell this apart from a stack overflow, so it conservatively forces an abort");
#endif
    CheckNotEmpty(result.Description, __func__, __LINE__, "a human-readable description is filled in");
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_CrashGuard::Test_Run_AccessViolation_LaterExceptionStillCaught

    Recovering from a crash must leave the thread able to throw and catch exceptions again. With RAD
    Studio's 32-bit compiler (bcc32c), the jump out of the crash once left Win32's chain of exception
    handlers pointing into the abandoned stack, and the next exception on the thread then crashed the
    process for real. The crash has to happen inside a try block (see CrashInsideTryBlock()) for that
    chain to have moved on before the jump.
*/
void TTest_ASWUnitTests_CrashGuard::Test_Run_AccessViolation_LaterExceptionStillCaught()
{
    // Arrange
    bool caught = false;

    // Act
    TCrashGuardResult const result = TCrashGuard::Run([]()
        {
            CrashInsideTryBlock();
        });

    try
    {
        throw std::runtime_error("thrown after the crash");
    }
    catch (std::runtime_error const&)
    {
        caught = true;
    }

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "the access violation is caught");
    CheckTrue(caught, __func__, __LINE__, "an exception thrown on the same thread afterwards is caught normally");
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_CrashGuard::Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort

    Not merely a handful of frames deep, deliberately: an ordinary crash deep in the call stack is
    still only an access violation, not a stack overflow, and mustn't force an abort. This test was
    once skipped on RAD Studio's bcc64x, whose Release build reported a stack overflow: Clang had
    dropped the helper's write through a plain null pointer as undefined behavior, and the depth
    check with it, so the recursion never stopped (see g_NullPointer). The description is checked
    so a crash that turns into a different fault fails with a clear message.
*/
void TTest_ASWUnitTests_CrashGuard::Test_Run_DeepAccessViolation_CaughtAndDoesNotAbort()
{
    // Arrange, Act
    TCrashGuardResult const result = TCrashGuard::Run([]()
        {
            RecurseDeeplyThenCrash(0);
        });

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "an access violation 20 stack frames deep is still caught");
#if defined(_WIN32)
    CheckEquals(std::string("access violation"), result.Description, __func__, __LINE__,
        "the null-pointer write at the bottom crashed, not a stack overflow from unbounded recursion");
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

    The stack overflows on a thread of its own, never on the thread running the tests. A thread's
    stack takes one overflow at most: the first uses up its guard page, and a second one ends the
    process. So on the thread running the tests, a second run of this test in one process, as a GUI
    runner's Run Selected does, ended the GUI (0xC0000005 in the Win32 GUI runner). TCrashGuard::Run()
    sets up its stack guarantee (Windows) and alternate signal stack (POSIX) on each thread it runs
    on, so the new thread is guarded the same way.
*/
void TTest_ASWUnitTests_CrashGuard::Test_Run_StackOverflow_CaughtAndAbortsRun()
{
    // Arrange
    auto const overflow = []()
        {
            RecurseUntilStackOverflows(0);
        };
    TCrashGuardResult result;
    auto const runGuarded = [&]()
        {
            result = TCrashGuard::Run(overflow);
        };

    // Act
    std::thread(runGuarded).join();

    // Assert
    CheckTrue(result.Crashed, __func__, __LINE__, "a genuine stack overflow is caught");
    CheckTrue(result.ShouldAbortRun, __func__, __LINE__,
        "a stack overflow is severe enough to force an abort afterward");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_CrashGuard)
