/* **************************************************************************
ASWUnitTests_Exception.h
Author: Anthony S. West - ASW Software

Copyright 2025 Anthony S. West

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
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

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
        std::string const& msg);
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

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_ExceptionH
