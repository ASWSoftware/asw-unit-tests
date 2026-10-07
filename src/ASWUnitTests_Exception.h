/* **************************************************************************
ASWUnitTests_Exception.h
Author: Anthony S. West - ASW Software

Copyright 2025-2026 ASW Software

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
#ifndef ASWUnitTests_ExceptionH
#define ASWUnitTests_ExceptionH
//---------------------------------------------------------------------------
#include <exception>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------
// Opt-in support for RAD Studio RTL exceptions (System::Sysutils::Exception and
// its subclasses, shared by VCL and FMX), which don't derive from std::exception.
// A C++Builder Clang project that links the RTL can opt-in by defining
// ASWUNITTESTS_RTL_EXCEPTIONS; code elsewhere tests the derived
// ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED instead, keeping the check in one place.
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS)
#  if !(defined(__BORLANDC__) && defined(__clang__))
#    error "ASWUNITTESTS_RTL_EXCEPTIONS requires RAD Studio's Clang-based compilers (bcc32c/bcc64x)"
#  endif
#  define ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED 1
#  include <System.SysUtils.hpp>
#endif
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
// Returns "ClassName: Message" for 'ex' (e.g. "EConvertError: 'abc' is not a
// valid integer value"), converted to UTF-8 so non-ASCII text survives.
std::string DescribeRTLException(System::Sysutils::Exception& ex);
// Returns just 'ex.Message', converted to UTF-8.
std::string RTLExceptionMessage(System::Sysutils::Exception& ex);
#endif

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TTestException
/////////////////////////////////////////////////////////////////////////////
class TTestException : public std::exception
{
protected:
    std::string m_Message;

protected:
    TTestException();

public:
    TTestException(std::string const& msg);

    const char* what() const noexcept override;
};


/////////////////////////////////////////////////////////////////////////////
// TExceptExpected
/////////////////////////////////////////////////////////////////////////////
class TExceptExpected : public TTestException
{
public:
    TExceptExpected(std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptEquals
/////////////////////////////////////////////////////////////////////////////
class TExceptEquals : public TTestException
{
public:
    TExceptEquals(std::string const& msg);
    TExceptEquals(std::string const& method, int line, std::string const& msg);
    TExceptEquals(std::string const& method, int line, std::string const& expected, std::string const& actual,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotEquals
/////////////////////////////////////////////////////////////////////////////
class TExceptNotEquals : public TTestException
{
public:
    TExceptNotEquals(std::string const& msg);
    TExceptNotEquals(std::string const& method, int line, std::string const& msg);
    TExceptNotEquals(std::string const& method, int line, std::string const& value, std::string const& msg);
    TExceptNotEquals(std::string const& method, int line, std::string const& value, std::string const& otherValue,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptContains
/////////////////////////////////////////////////////////////////////////////
class TExceptContains : public TTestException
{
public:
    TExceptContains(std::string const& method, int line, std::string const& text, std::string const& substring,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotContains
/////////////////////////////////////////////////////////////////////////////
class TExceptNotContains : public TTestException
{
public:
    TExceptNotContains(std::string const& method, int line, std::string const& text, std::string const& substring,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptStartsWith
/////////////////////////////////////////////////////////////////////////////
class TExceptStartsWith : public TTestException
{
public:
    TExceptStartsWith(std::string const& method, int line, std::string const& text, std::string const& prefix,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotStartsWith
/////////////////////////////////////////////////////////////////////////////
class TExceptNotStartsWith : public TTestException
{
public:
    TExceptNotStartsWith(std::string const& method, int line, std::string const& text, std::string const& prefix,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptEndsWith
/////////////////////////////////////////////////////////////////////////////
class TExceptEndsWith : public TTestException
{
public:
    TExceptEndsWith(std::string const& method, int line, std::string const& text, std::string const& suffix,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotEndsWith
/////////////////////////////////////////////////////////////////////////////
class TExceptNotEndsWith : public TTestException
{
public:
    TExceptNotEndsWith(std::string const& method, int line, std::string const& text, std::string const& suffix,
        std::string const& msg, bool ignoreCase = false);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptOrdering
/////////////////////////////////////////////////////////////////////////////
class TExceptOrdering : public TTestException
{
public:
    TExceptOrdering(std::string const& method, int line, std::string const& value, std::string const& relation,
        std::string const& bound, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptFalse
/////////////////////////////////////////////////////////////////////////////
class TExceptFalse : public TTestException
{
public:
    TExceptFalse(std::string const& msg);
    TExceptFalse(std::string const& method, int line, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptTrue
/////////////////////////////////////////////////////////////////////////////
class TExceptTrue : public TTestException
{
public:
    TExceptTrue(std::string const& msg);
    TExceptTrue(std::string const& method, int line, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNull
/////////////////////////////////////////////////////////////////////////////
class TExceptNull : public TTestException
{
public:
    TExceptNull(std::string const& method, int line, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotNull
/////////////////////////////////////////////////////////////////////////////
class TExceptNotNull : public TTestException
{
public:
    TExceptNotNull(std::string const& method, int line, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptSame
/////////////////////////////////////////////////////////////////////////////
class TExceptSame : public TTestException
{
public:
    TExceptSame(std::string const& method, int line, std::string const& expected, std::string const& actual,
        std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptNotSame
/////////////////////////////////////////////////////////////////////////////
class TExceptNotSame : public TTestException
{
public:
    TExceptNotSame(std::string const& method, int line, std::string const& value, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptSkipped
//
// Thrown by TTestGroupBase::Skip() to abort the current test and have it
// reported as skipped rather than passed or failed.
/////////////////////////////////////////////////////////////////////////////
class TExceptSkipped : public TTestException
{
public:
    TExceptSkipped(std::string const& msg);
    TExceptSkipped(std::string const& method, int line, std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptAbortRun
//
// Common base for a signal that a single test's failure is severe enough that
// running any further tests or groups in this process isn't considered safe.
// Caught by TTestHandler::Run() (matched by this base, not by either derived
// type individually) to stop running further groups after merging in the
// group's own results, which already include a synthetic failure record for
// the test that triggered it. Never caught inside TTestGroupBase::Test()
// itself.
/////////////////////////////////////////////////////////////////////////////
class TExceptAbortRun : public TTestException
{
public:
    TExceptAbortRun(std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptTestTimedOut
//
// Thrown by TTestGroupBase::Run() when a test's worker thread does not finish
// within its --test-timeout-seconds allotment, to unwind the run after the
// abandoned test's own synthetic failure record has already been added to
// its group's results.
/////////////////////////////////////////////////////////////////////////////
class TExceptTestTimedOut : public TExceptAbortRun
{
public:
    TExceptTestTimedOut(std::string const& msg);
};


/////////////////////////////////////////////////////////////////////////////
// TExceptTestCrashed
//
// Thrown by TTestGroupBase::ReportCrashedTest() when --catch-crashes caught a
// native fault (see ASWUnitTests_CrashGuard.h) severe enough to abort the run
// rather than continue (currently: a stack overflow on Windows, or any
// SIGSEGV on POSIX, since POSIX can't cheaply distinguish an ordinary
// segfault from a stack overflow - see TCrashGuard::Run()). A crash that
// isn't severe enough to abort is recorded as a failure without throwing at
// all, so the run continues normally.
/////////////////////////////////////////////////////////////////////////////
class TExceptTestCrashed : public TExceptAbortRun
{
public:
    TExceptTestCrashed(std::string const& msg);
};

// /////// Compiler specific exceptions after this line /////////////////////

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
/////////////////////////////////////////////////////////////////////////////
// TExceptRTLException
//
// Stands in for an RTL exception that must outlive its own catch handler,
// which the RTL exception object itself can't do: std::exception_ptr can't
// carry one past its handler (see TTestGroupBase::RunWithTimeout(), which
// throws this in its place). what() is the DescribeRTLException() text, and
// RTLClass() is the original exception's class, so a caller can still check
// its type, e.g. RTLClass()->InheritsFrom(__classid(EConvertError)). A class
// reference is static metadata, so it stays valid after the original
// exception object is freed.
/////////////////////////////////////////////////////////////////////////////
class TExceptRTLException : public std::runtime_error
{
private:
    System::TClass m_RTLClass;

public:
    explicit TExceptRTLException(System::Sysutils::Exception& ex);

    System::TClass RTLClass() const noexcept;
};

#endif

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_ExceptionH
