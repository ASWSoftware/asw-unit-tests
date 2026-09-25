/* **************************************************************************
ASWUnitTests_CrashGuard.cpp
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
#include "ASWUnitTests_CrashGuard.h"
//---------------------------------------------------------------------------
#include <csetjmp>
#include <cstdio>
#include <mutex>
//---------------------------------------------------------------------------
#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#else
#  include <csignal>
#  include <cstring>
#endif
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

#if defined(_WIN32)

//---------------------------------------------------------------------------
// Not a Microsoft structured exception: Windows implements C++ throw/catch using the same
// exception-dispatch machinery, with a compiler-specific code (e.g. GCC's 0x20474343, or
// Embarcadero's Clang-based 0x0EEFFACE/0xE36C6701) that IsRecognizedFault() below never matches,
// so CrashVectoredHandler() always lets it through untouched via EXCEPTION_CONTINUE_SEARCH.
//
// __try/__except was tried first, since it is the textbook approach and unwinds C++ objects
// properly when it works. It does not work here: it compiles under RAD Studio's Clang-based
// compilers (bcc32c/bcc64) but silently fails to catch anything at runtime on either one, and GCC/
// MinGW does not implement the keywords at all. AddVectoredExceptionHandler() + setjmp()/longjmp()
// below was verified instead, directly, against MSVC-compatible semantics on MinGW, bcc32c, and
// bcc64 (including recovering from a genuine stack overflow well enough to build a std::string and
// continue), so this framework's guarantees are only ever stated for what was actually observed to
// work, not for what the platform's documentation suggests should work.
thread_local std::jmp_buf* g_JumpBuf = nullptr;
thread_local unsigned long g_FaultCode = 0;

LONG WINAPI CrashVectoredHandler(PEXCEPTION_POINTERS info);
std::string DescribeFault(unsigned long code);
bool IsRecognizedFault(unsigned long code);

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
LONG WINAPI CrashVectoredHandler(PEXCEPTION_POINTERS info)
{
    unsigned long const code = info->ExceptionRecord->ExceptionCode;

    if (g_JumpBuf == nullptr || !IsRecognizedFault(code))
        return EXCEPTION_CONTINUE_SEARCH;

    g_FaultCode = code;
    std::longjmp(*g_JumpBuf, 1); // Never returns.
    return EXCEPTION_CONTINUE_SEARCH; // Unreachable; silences a "no return" warning on some compilers.
}

//---------------------------------------------------------------------------
std::string DescribeFault(unsigned long code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION: return "access violation";
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "array bounds exceeded";
        case EXCEPTION_DATATYPE_MISALIGNMENT: return "data misalignment";
        case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "floating point divide by zero";
        case EXCEPTION_FLT_INVALID_OPERATION: return "floating point invalid operation";
        case EXCEPTION_ILLEGAL_INSTRUCTION: return "illegal instruction";
        case EXCEPTION_INT_DIVIDE_BY_ZERO: return "integer divide by zero";
        case EXCEPTION_INT_OVERFLOW: return "integer overflow";
        case EXCEPTION_PRIV_INSTRUCTION: return "privileged instruction";
        case EXCEPTION_STACK_OVERFLOW: return "stack overflow";
        default:
        {
            char buffer[40];
            std::snprintf(buffer, sizeof(buffer), "structured exception 0x%08lX", code);
            return buffer;
        }
    }
}

//---------------------------------------------------------------------------
// Only these codes are ever treated as "the test crashed"; anything else, including a C++
// exception's own internal dispatch codes, is left completely alone (see the comment above
// g_JumpBuf), so ordinary C++ exception handling is unaffected by this handler being installed.
bool IsRecognizedFault(unsigned long code)
{
    switch (code)
    {
        case EXCEPTION_ACCESS_VIOLATION:
        case EXCEPTION_ARRAY_BOUNDS_EXCEEDED:
        case EXCEPTION_DATATYPE_MISALIGNMENT:
        case EXCEPTION_FLT_DENORMAL_OPERAND:
        case EXCEPTION_FLT_DIVIDE_BY_ZERO:
        case EXCEPTION_FLT_INEXACT_RESULT:
        case EXCEPTION_FLT_INVALID_OPERATION:
        case EXCEPTION_FLT_OVERFLOW:
        case EXCEPTION_FLT_STACK_CHECK:
        case EXCEPTION_FLT_UNDERFLOW:
        case EXCEPTION_ILLEGAL_INSTRUCTION:
        case EXCEPTION_IN_PAGE_ERROR:
        case EXCEPTION_INT_DIVIDE_BY_ZERO:
        case EXCEPTION_INT_OVERFLOW:
        case EXCEPTION_PRIV_INSTRUCTION:
        case EXCEPTION_STACK_OVERFLOW:
            return true;
        default:
            return false;
    }
}

#else // POSIX

//---------------------------------------------------------------------------
// SIGABRT is deliberately never installed for: it usually means the C runtime itself (e.g. glibc's
// malloc corruption checks, or a failed assert()) already detected the process's state is corrupt
// and is deliberately terminating rather than letting it continue. Recovering from that is widely
// considered unsafe even by libc implementations themselves.
thread_local sigjmp_buf* g_JumpBuf = nullptr;
thread_local int g_FaultSignal = 0;

// Backs the alternate signal stack installed by EnsureAlternateSignalStack() below. A real stack
// overflow leaves the thread's normal stack with no room left at all, and by default the kernel
// delivers SIGSEGV on that same, already-exhausted stack: the handler itself then has nowhere to
// run, so the process dies for real instead of the handler ever getting to call siglongjmp(). This
// was reproduced directly (exit code 139, no "***Test failed" line at all - the process never got
// far enough to report one) before being trusted as the cause. 64KB comfortably exceeds glibc's
// advertised minimum (MINSIGSTKSZ) on the mainstream x86-64 Linux target this was verified against,
// while the handler's own footprint (a comparison, an assignment, and a siglongjmp) is trivial, so
// there is no need to size this any closer to the edge. Thread-local, and (re)installed on every
// TCrashGuard::Run() call rather than once, because sigaltstack() is a per-thread OS attribute -
// mirroring why SetThreadStackGuarantee() on the Windows side below is also called every time.
thread_local char g_AltStackBuffer[64 * 1024];

void CrashSignalHandler(int signalNumber);
std::string DescribeFault(int signalNumber);
void EnsureAlternateSignalStack();
void InstallSignalHandlersOnce();

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void CrashSignalHandler(int signalNumber)
{
    if (g_JumpBuf == nullptr)
    {
        // No guard active on this thread; shouldn't happen, since the handler is only installed
        // once TCrashGuard::Run() is first called. Restore default handling and re-raise so the
        // process still dies the ordinary way instead of looping on the same fault.
        std::signal(signalNumber, SIG_DFL);
        std::raise(signalNumber);
        return;
    }

    g_FaultSignal = signalNumber;
    siglongjmp(*g_JumpBuf, 1);
}

//---------------------------------------------------------------------------
std::string DescribeFault(int signalNumber)
{
    switch (signalNumber)
    {
        case SIGSEGV: return "segmentation fault";
        case SIGFPE: return "floating point exception";
        case SIGILL: return "illegal instruction";
        case SIGBUS: return "bus error";
        default: return "signal " + std::to_string(signalNumber);
    }
}

//---------------------------------------------------------------------------
void EnsureAlternateSignalStack()
{
    stack_t stackDescriptor;
    stackDescriptor.ss_sp = g_AltStackBuffer;
    stackDescriptor.ss_size = sizeof(g_AltStackBuffer);
    stackDescriptor.ss_flags = 0;
    sigaltstack(&stackDescriptor, nullptr);
}

//---------------------------------------------------------------------------
void InstallSignalHandlersOnce()
{
    static std::once_flag installed;
    std::call_once(installed, []()
        {
            int const signalsToCatch[] = { SIGSEGV, SIGFPE, SIGILL, SIGBUS };

            for (int signalNumber : signalsToCatch)
            {
                struct sigaction action;
                std::memset(&action, 0, sizeof(action));
                action.sa_handler = CrashSignalHandler;
                sigemptyset(&action.sa_mask);
                action.sa_flags = SA_ONSTACK; // Run on the alternate stack; see g_AltStackBuffer above.
                sigaction(signalNumber, &action, nullptr);
            }
        });
}

#endif

} // namespace

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TCrashGuard
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
/*
    TCrashGuard::Run

    Runs 'body' with native-crash protection. If it completes normally - including by letting an
    ordinary C++ exception propagate out, which this never intercepts - returns a default
    (Crashed == false) result. If a native fault occurs, catches it and returns a result describing
    it instead of letting it take down the process.

    'body' is abandoned mid-execution via a raw jump (longjmp/siglongjmp), the same fundamental
    technique --test-timeout-seconds' abandoned worker thread relies on for a different reason: it
    does NOT run destructors for any objects that were under construction on 'body's stack at the
    moment of the fault. This is a real, accepted limitation of recovering from a hardware-level
    fault at all, on any platform, not an oversight.

    'ShouldAbortRun' is set for a Windows stack overflow specifically (EXCEPTION_STACK_OVERFLOW is
    unambiguous), or for ANY SIGSEGV on POSIX: unlike Windows, POSIX delivers a stack overflow and an
    ordinary segfault as the exact same signal, and reliably telling them apart needs sigaltstack()
    plus inspecting the faulting address against the thread's stack bounds - real extra complexity
    for what both platforms already treat as an edge case. Treating every POSIX SIGSEGV as abort-
    worthy is the conservative choice, at the cost of a deliberate platform difference: the same
    ordinary null-pointer dereference continues the run on Windows but aborts it on Linux/Mac.

    SetThreadStackGuarantee() handles an issue with a RAD Studio 13.1 Win64x build: an ordinary
    crash was caught correctly even 2000 stack frames deep, but a genuine stack overflow (necessarily
    many frames deep, from any real unbounded recursion) reliably took the whole process down instead
    of being caught, despite the exact same vectored handler and longjmp working for every other fault
    type. The difference is that Windows delivers STATUS_STACK_OVERFLOW with the thread already at the
    edge of its stack, inside the guard page, and the OS's own exception dispatch - searching registered
    handlers across however many stack frames exist before it ever reaches this code - has to do that
    search within whatever budget the guard page provides. An ordinary fault, even deep in a real call
    stack, doesn't start out of room the same way, which is why only the stack-overflow case needed this.
    Called every time (not once, unlike installing the handler below) because it's inherently per-thread,
    and 'body' may run on a different thread than last time via a timeout worker thread.

    EnsureAlternateSignalStack() handles the POSIX equivalent problem: a real stack overflow leaves no
    room on the thread's own stack, and the kernel's default is to deliver SIGSEGV on that same,
    already-exhausted stack, so the handler itself has nowhere to run and the process dies for real
    instead of reaching siglongjmp(). Reproduced directly (an unadorned exit code 139, with no test
    failure ever reported - the process never got that far) before sigaltstack() plus SA_ONSTACK was
    added to fix it. This is a different problem than the "which fault is this" ambiguity described
    above; sigaltstack() only ensures the handler for a SIGSEGV gets to run at all, it does not by
    itself distinguish a stack overflow from an ordinary one once it does.
*/
TCrashGuardResult TCrashGuard::Run(std::function<void()> const& body)
{
    TCrashGuardResult result;

#if defined(_WIN32)
    static PVOID const handlerHandle = AddVectoredExceptionHandler(1, CrashVectoredHandler);
    (void)handlerHandle; // Installed once for the process's lifetime; never removed.

    ULONG stackGuarantee = 64 * 1024;
    SetThreadStackGuarantee(&stackGuarantee);

    std::jmp_buf jumpBuf;
    std::jmp_buf* const previousJumpBuf = g_JumpBuf;
    g_JumpBuf = &jumpBuf;

    // setjmp() returns 0 on this, its first, normal call, so body() runs from here. If it faults,
    // CrashVectoredHandler() above calls longjmp(jumpBuf, 1) from wherever the fault happened; that
    // does not resume execution there, but instead restores the CPU/stack state saved by this same
    // setjmp() call and makes it return again, this time with the value longjmp() passed (1, so
    // nonzero), landing in the else branch below instead of falling through to body() a second time.
    if (setjmp(jumpBuf) == 0)
    {
        body();
    }
    else
    {
        result.Crashed = true;
        result.ShouldAbortRun = (g_FaultCode == EXCEPTION_STACK_OVERFLOW);
        result.Description = DescribeFault(g_FaultCode);
    }

    g_JumpBuf = previousJumpBuf;
#else
    InstallSignalHandlersOnce();
    EnsureAlternateSignalStack();

    sigjmp_buf jumpBuf;
    sigjmp_buf* const previousJumpBuf = g_JumpBuf;
    g_JumpBuf = &jumpBuf;

    // Same two-return trick as setjmp() above: this returns 0 on this, its first, normal call, so
    // body() runs from here. If it faults, CrashSignalHandler() above calls siglongjmp(jumpBuf, 1)
    // from wherever the fault happened, which makes this same sigsetjmp() call return again instead
    // of resuming execution at the fault, this time with the nonzero value siglongjmp() passed,
    // landing in the else branch below rather than falling through to body() a second time.
    if (sigsetjmp(jumpBuf, 1) == 0)
    {
        body();
    }
    else
    {
        result.Crashed = true;
        result.ShouldAbortRun = (g_FaultSignal == SIGSEGV);
        result.Description = DescribeFault(g_FaultSignal);
    }

    g_JumpBuf = previousJumpBuf;
#endif

    return result;
}

} // namespace ASWUnitTests
