/* **************************************************************************
ASWUnitTests_TestBase.h
Author: Anthony S. West - ASW Software

A simple unit testing framework.

To register a test module, create a class that inherits 'TTestGroupBase'
and self-register it with the ASW_REGISTER_TEST_GROUP macro
(see ASWUnitTests_Registry.h). No framework source file needs to change.

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

#ifndef ASWUnitTests_TestBaseH
#define ASWUnitTests_TestBaseH
//---------------------------------------------------------------------------
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

// std::source_location (C++20) is used where the compiler and library support it, which RAD Studio's 32-bit
// compilers (C++17) don't; code elsewhere tests the derived ASWUNITTESTS_SOURCE_LOCATION_ENABLED. The feature-test
// macro is checked rather than __has_include(<source_location>), since bcc32c has that header but can't compile it.
#if defined(__has_include)
#  if __has_include(<version>)
#    include <version>
#  endif
#endif
#if defined(__cpp_lib_source_location)
#  define ASWUNITTESTS_SOURCE_LOCATION_ENABLED 1
#  include <source_location>
#endif
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTestOutcome
/////////////////////////////////////////////////////////////////////////////
enum class TTestOutcome
{
    Pass,
    Fail,
    Skip
};


/////////////////////////////////////////////////////////////////////////////
// TTestCaseRecord
//
// One test's outcome, name, and timing, collected in TTestResults::CaseRecords.
// Deliberately format-agnostic (no ANSI color codes, no XML/JSON, etc.), so a
// caller can build a structured report from it without TTestResults itself
// depending on any particular report format.
/////////////////////////////////////////////////////////////////////////////
struct TTestCaseRecord
{
    std::string GroupName;
    std::string TestName;
    double DurationSeconds;
    TTestOutcome Outcome;
    std::string Message; // Failure/skip detail; empty for Pass.
};


/////////////////////////////////////////////////////////////////////////////
// TTestResults
//
// Stores test results.
// Provides common comparison methods that bump the counts on success/fail.
/////////////////////////////////////////////////////////////////////////////
class TTestResults
{
public:
    typedef std::vector<std::string> MsgList;

public:
    bool Crashed; // Set when a test crashed severely enough (see TCrashGuard::Run()) to abort the run.
    unsigned int FailedCount;
    unsigned int SkippedCount;
    bool Stopped; // Set when ITestRunObserver::StopRequested() ended the run early.
    unsigned int SuccessCount;
    bool TimedOut;
    MsgList Messages;
    std::vector<TTestCaseRecord> CaseRecords;

public:
    TTestResults();

    void AddMessages(MsgList const& list);
};


/////////////////////////////////////////////////////////////////////////////
// TestFilter
//
// A predicate matched against each test's "GroupName.TestName" full name.
// An empty (default-constructed) TestFilter means "run everything." Used
// by TTestHandler::Run() and ITestGroup::Run() to support CLI filtering
// (see main.cpp's --filter option) without either needing to know how the
// pattern itself is matched.
/////////////////////////////////////////////////////////////////////////////
typedef std::function<bool (std::string const& fullTestName)> TestFilter;


/////////////////////////////////////////////////////////////////////////////
// ITestRunObserver
//
// Optional hooks for following a test run as it happens, e.g. a GUI runner
// updating its display after each test. Set with
// TTestHandler::SetRunObserver(). With none set, nothing changes: output
// goes to std::cout as usual.
//
// Threading: OnTestStarted(), OnTestFinished(), and StopRequested() are
// always called on the thread that called TTestHandler::Run(). OnLog() is
// too, except under --test-timeout-seconds, where a test runs on a worker
// thread and its own log output arrives from there; an implementation that
// isn't thread-safe must handle that (e.g. by buffering under a mutex).
/////////////////////////////////////////////////////////////////////////////
class ITestRunObserver
{
public:
    virtual ~ITestRunObserver() = default;

    // Receives the text that would otherwise be written to std::cout, including any line break.
    virtual void OnLog(std::string const& text) = 0;
    // Called once for every test that records an outcome, including the failure recorded for a test that
    // timed out or crashed. Not called for a test whose unexpected exception escapes TTestHandler::Run().
    virtual void OnTestFinished(TTestCaseRecord const& record) = 0;
    virtual void OnTestStarted(std::string const& groupName, std::string const& testName) = 0;
    // Checked before each test and each group. Returning true ends the run there, with the returned
    // TTestResults::Stopped set; a test that's already running is never interrupted.
    virtual bool StopRequested() = 0;
};


/////////////////////////////////////////////////////////////////////////////
// ITestCase
//
// Interface for a test case.
/////////////////////////////////////////////////////////////////////////////
class ITestCase
{
public:
    typedef std::function<void ()> TestCallback;

public:
    explicit ITestCase() = default;
    ITestCase(ITestCase const&) = default;
    ITestCase(ITestCase&&) = default;  // move constructor
    virtual ~ITestCase() = default;

    ITestCase& operator=(const ITestCase&) = delete;
    ITestCase& operator=(ITestCase&&) = default;  // move assignment

    virtual void DoTest() = 0;
    virtual std::string const& GetName() const = 0;
    virtual TestCallback GetTestCallback() const = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TTestCase
//
// Holds info for a test case, including the test callback and the test name.
/////////////////////////////////////////////////////////////////////////////
class TTestCase : public ITestCase
{
private:
    typedef ITestCase inherited;

protected:
    TestCallback m_Callback;
    std::string m_Name;

public:
    TTestCase(TestCallback callback, std::string const& name);

    void DoTest() override;
    std::string const& GetName() const override;
    TestCallback GetTestCallback() const override;
};


/////////////////////////////////////////////////////////////////////////////
// ITestGroup
//
// Interface for a test group.
/////////////////////////////////////////////////////////////////////////////
class ITestGroup
{
public:
    typedef std::vector<std::unique_ptr<ITestCase> > TestCallbackList;

public:
    virtual ~ITestGroup()
    {
    }

    virtual TestCallbackList& GetTestCallbackList() = 0;
    virtual std::string const& GetTestGroupName() const = 0;
    virtual TTestResults const& Results() const = 0;
    // Throws a TExceptAbortRun (TExceptTestTimedOut or TExceptTestCrashed) if a test doesn't finish
    // within 'testTimeoutSeconds' (when given) or crashes severely enough for 'catchCrashes' to
    // decide the run shouldn't continue, after recording a synthetic failure for the offending test
    // in this group's own Results() either way.
    virtual void Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed,
        std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes) = 0;
    // Called by TTestHandler::SetRunObserver() (nullptr to clear); see ITestRunObserver. The default ignores it,
    // so an ITestGroup implementation that doesn't support an observer still works, just without its events.
    virtual void SetRunObserver(ITestRunObserver* /*observer*/)
    {
    }
    virtual void SetUp_Group() = 0;
    virtual void TearDown_Group() = 0;
};


/////////////////////////////////////////////////////////////////////////////
// TTestGroupBase
//
// Abstract class for a test group that contains limited members for
// assertion testing.
//
// - 'Assert' methods cause the test to immediately fail.
// - 'Check' methods allow multiple "asserts" per test.
/////////////////////////////////////////////////////////////////////////////
class TTestGroupBase : public ITestGroup
{
private:
    typedef ITestGroup inherited;

    // The relation an ordering method (e.g. CheckGreaterThan()) checks 'value' has to 'bound'.
    enum class TOrdering
    {
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual
    };

    // Used by the Assert ordering methods: throws TExceptOrdering unless 'value' has relation 'ordering' to 'bound'.
    template <typename TValue, typename TBound>
    void AssertOrdering(TValue value, TBound bound, TOrdering ordering, std::string const& method, int line,
        std::string const& msg)
    {
        if (!OrderingHolds(CompareForOrdering(value, bound), ordering))
        {
            std::pair<std::string, std::string> const texts = FormatOrderingValues(value, bound);
            throw TExceptOrdering(method, line, texts.first, OrderingSymbol(ordering), texts.second, msg);
        }
    }

    // Used by the Check ordering methods: records a failure unless 'value' has relation 'ordering' to 'bound'.
    template <typename TValue, typename TBound>
    void CheckOrdering(TValue value, TBound bound, TOrdering ordering, std::string const& method, int line,
        std::string const& msg)
    {
        if (!OrderingHolds(CompareForOrdering(value, bound), ordering))
        {
            std::pair<std::string, std::string> const texts = FormatOrderingValues(value, bound);
            SetTestFailedCheck(method, line, "Expected " + texts.first + " to be " + OrderingSymbol(ordering) + " " +
                texts.second + ". " + msg);
        }
    }

    // Returns a negative number, zero or a positive number when 'value' is less than, equal to or greater than
    // 'bound', or nullopt when either is a floating-point NaN. Two integers are compared by value, like
    // ForwardIntegerComparison(); when one is negative and the other is above INT64_MAX, the negative one is less.
    // Otherwise both are converted to their common floating-point type, as the built-in operators do.
    template <typename TValue, typename TBound>
    static std::optional<int> CompareForOrdering(TValue value, TBound bound)
    {
        if constexpr (std::is_floating_point<TValue>::value || std::is_floating_point<TBound>::value)
        {
            typedef typename std::common_type<TValue, TBound>::type TCommon;
            TCommon const commonValue = static_cast<TCommon>(value);
            TCommon const commonBound = static_cast<TCommon>(bound);

            if (std::isnan(commonValue) || std::isnan(commonBound))
                return std::nullopt;

            return ThreeWayCompare(commonValue, commonBound);
        }
        else
        {
            if (FitsInInt64(value) && FitsInInt64(bound))
                return ThreeWayCompare(static_cast<int64_t>(value), static_cast<int64_t>(bound));

            if (IsNonNegative(value) && IsNonNegative(bound))
                return ThreeWayCompare(static_cast<uint64_t>(value), static_cast<uint64_t>(bound));

            return IsNonNegative(value) ? 1 : -1;
        }
    }

    template <typename TInteger>
    static bool FitsInInt64(TInteger value)
    {
        if constexpr (std::is_signed<TInteger>::value)
            return true;
        else
            return static_cast<uint64_t>(value) <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    }

    // Formats a pair of floating-point values for an ordering failure; see FormatOrderingValues().
    static std::pair<std::string, std::string> FormatFloatingPointValues(float value, float bound);
    static std::pair<std::string, std::string> FormatFloatingPointValues(double value, double bound);
    static std::pair<std::string, std::string> FormatFloatingPointValues(long double value, long double bound);

    // Formats 'value' and 'bound' for an ordering failure. Floating-point values are shown in their common type, with
    // as many digits as it reliably holds, or all of them when that would make two different values look equal.
    template <typename TValue, typename TBound>
    static std::pair<std::string, std::string> FormatOrderingValues(TValue value, TBound bound)
    {
        if constexpr (std::is_floating_point<TValue>::value || std::is_floating_point<TBound>::value)
        {
            typedef typename std::common_type<TValue, TBound>::type TCommon;
            return FormatFloatingPointValues(static_cast<TCommon>(value), static_cast<TCommon>(bound));
        }
        else
        {
            return std::pair<std::string, std::string>(std::to_string(value), std::to_string(bound));
        }
    }

    // Calls 'compare' with 'expected' and 'actual' as two int64_t when both fit, otherwise as two uint64_t when
    // neither is negative, otherwise (one negative, the other above INT64_MAX) as two decimal strings, so neither
    // value wraps around. Used by the integer template overloads of the Equals/NotEquals methods.
    template <typename TExpected, typename TActual, typename TCompare>
    static void ForwardIntegerComparison(TExpected expected, TActual actual, TCompare compare)
    {
        if (FitsInInt64(expected) && FitsInInt64(actual))
            compare(static_cast<int64_t>(expected), static_cast<int64_t>(actual));
        else if (IsNonNegative(expected) && IsNonNegative(actual))
            compare(static_cast<uint64_t>(expected), static_cast<uint64_t>(actual));
        else
            compare(std::to_string(expected), std::to_string(actual));
    }

    template <typename TInteger>
    static bool IsNonNegative(TInteger value)
    {
        if constexpr (std::is_signed<TInteger>::value)
            return value >= 0;
        else
            return true;
    }

    // True when 'order' (from CompareForOrdering()) satisfies 'ordering'; never when it's nullopt (a NaN).
    static bool OrderingHolds(std::optional<int> order, TOrdering ordering);
    // The operator for 'ordering' in a failure message, e.g. ">=".
    static char const* OrderingSymbol(TOrdering ordering);

    template <typename T>
    static int ThreeWayCompare(T a, T b)
    {
        return (a < b) ? -1 : ((b < a) ? 1 : 0);
    }

protected:
    bool m_ExceptionExpected;
    bool m_LogSuppressed;
    bool m_TestFailedCheck;
    // The current test's Check* failures, without the log line's "  **" prefix; see Test() for how a failed
    // test's record uses them.
    std::vector<std::string> m_CheckFailureMessages;
    std::string m_ExceptionExpectedText;
    std::string m_ExpectedExceptionMessage;
    std::function<bool (std::exception const&)> m_ExpectedExceptionTypeChecker;
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    std::function<bool (System::Sysutils::Exception&)> m_ExpectedRTLExceptionTypeChecker;
#endif
    std::string m_Name;
    TTestResults m_Results;
    ITestRunObserver* m_RunObserver; // Not owned; nullptr when none is set.
    TestCallbackList m_TestCallbacks;

    // True when the current test's exception expectation (see SetExceptionExpected<TException>()) also
    // requires a specific exception type, rather than accepting any thrown exception.
    virtual bool ExceptionTypeExpected() const;

    virtual void Log(std::string const& msg);
    virtual void LogAppend(std::string const& msg);

    virtual void RegisterTest(TTestCase const& testCase);
    virtual void RegisterTest(ITestCase::TestCallback callback, std::string const& testName);
    template <typename T>
    void RegisterTest(void (T::*callback)(), std::string const& testName)
    {
        RegisterTest([this, callback]()
            {
                (static_cast<T*>(this)->*callback)();
            }, testName);
    }

    template <typename TParam>
    struct TNonDeduced
    {
        typedef TParam Type;
    };

    // Registers one test case per element of 'params', invoking 'callback' with that element in turn.
    // Generated test names are "testNameBase[i]", using the row's zero-based index, unless 'nameGenerator'
    // supplies a per-row label instead.
    template <typename T, typename TParam>
    void RegisterTestCases(void (T::*callback)(TParam const&), std::string const& testNameBase,
        std::vector<TParam> const& params,
        std::function<std::string (typename TNonDeduced<TParam>::Type const&)> const& nameGenerator = nullptr)
    {
        for (std::size_t i = 0; i < params.size(); ++i)
        {
            TParam const param = params[i];
            std::string const caseName = nameGenerator ?
                    (testNameBase + "[" + nameGenerator(param) + "]") :
                    (testNameBase + "[" + std::to_string(i) + "]");
            RegisterTest([this, callback, param]()
                {
                    (static_cast<T*>(this)->*callback)(param);
                }, caseName);
        }
    }

    // Called when TCrashGuard::Run() (see ASWUnitTests_CrashGuard.h) catches a native crash while
    // running 'testCase' for --catch-crashes. Records a synthetic failed outcome for the crashed
    // test, naming 'description', exactly like any other failure. Throws TExceptTestCrashed only
    // when 'abortRun' is set (see TCrashGuard::Run() for when that is); otherwise returns normally,
    // so the run continues with the next test.
    virtual void ReportCrashedTest(ITestCase& testCase, std::string const& description, bool abortRun);
    // Called when a test's worker thread does not finish within 'testTimeoutSeconds'. Records a
    // synthetic failed outcome for the abandoned test (so it flows into the normal results/JUnit
    // report exactly like any other failure) and throws TExceptTestTimedOut. The worker thread
    // itself is never joined; there is no safe way to stop a thread that may be stuck in an
    // infinite loop, so it is abandoned.
    virtual void ReportTimedOutTest(ITestCase& testCase, unsigned int testTimeoutSeconds);
    virtual void ResetTestFailedOneOrMoreChecks();
    // Runs 'testCase' directly when 'catchCrashes' is false (no overhead/behavior change from before
    // this feature existed). Otherwise runs it through TCrashGuard::Run(); see ReportCrashedTest()
    // for what happens if that catches something.
    virtual void RunCatchingCrashes(ITestCase& testCase, bool catchCrashes);
    // Runs 'testCase' (via RunCatchingCrashes(), so 'catchCrashes' still applies either way) directly
    // when 'testTimeoutSeconds' is unset (no overhead/behavior change from before that feature
    // existed). Otherwise runs it on a worker thread and waits with a timeout; see
    // ReportTimedOutTest() for what happens if it doesn't finish in time.
    virtual void RunWithTimeout(ITestCase& testCase, std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes);
    virtual void SetExceptionExpected(bool expected, std::string const& method, int line, std::string const& msg);
    // Expects a specific exception type (matched polymorphically, so a base class also matches its subclasses).
    // When 'expectedMessage' is non-empty, the caught exception's what() must also contain it as a substring.
    // With ASWUNITTESTS_RTL_EXCEPTIONS enabled (see ASWUnitTests_Exception.h), 'TException' may also be an RTL
    // exception class (System::Sysutils::Exception or a subclass), whose Message is checked instead of what().
    template <typename TException>
    void SetExceptionExpected(std::string const& method, int line, std::string const& msg,
        std::string const& expectedMessage = std::string())
    {
        SetExceptionExpected(true, method, line, msg);
        m_ExpectedExceptionMessage = expectedMessage;
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        if constexpr (std::is_base_of_v<System::Sysutils::Exception, TException>)
        {
            m_ExpectedRTLExceptionTypeChecker = [](System::Sysutils::Exception& ex)
                {
                    return dynamic_cast<TException*>(&ex) != nullptr;
                };
        }
        else
#endif
        {
            m_ExpectedExceptionTypeChecker = [](std::exception const& ex)
                {
                    return dynamic_cast<TException const*>(&ex) != nullptr;
                };
        }
    }
    virtual void SetTestFailedCheck(std::string const& method, int line, std::string const& msg);
    virtual void SetTestFailedCheck(std::string const& method, int line, std::string const& expected,
        std::string const& actual, std::string const& msg);
    virtual void SetTestFailedCheckNotEquals(std::string const& method, int line, std::string const& msg);
    virtual void SetTestFailedCheckNotEquals(std::string const& method, int line, std::string const& value,
        std::string const& msg);
    virtual void SetUp_Test(ITestCase& testCase); // Called just before calling the test callback
    virtual void Skip(std::string const& method, int line, std::string const& reason); // Aborts current test
    virtual void TearDown_Test(ITestCase& testCase); // Called just after calling the test callback
    virtual void Test(ITestCase& testCase); // Called for each registered test
    virtual bool TestFailedOneOrMoreChecks();

protected: // Assertion/Check methods - Equals
    virtual void AssertEquals(
        bool expected, bool actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(
        uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertEquals(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEquals(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void AssertEquals(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void AssertEquals(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);
    // Any two integer types except bool that no overload above matches exactly (e.g. int and int64_t), compared
    // by value.
    template <typename TExpected, typename TActual>
    using TEnableIfIntegers = typename std::enable_if<std::is_integral<TExpected>::value &&
        std::is_integral<TActual>::value && !std::is_same<TExpected, bool>::value &&
        !std::is_same<TActual, bool>::value, int>::type;
    template <typename TExpected, typename TActual, TEnableIfIntegers<TExpected, TActual> = 0>
    void AssertEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        ForwardIntegerComparison(expected, actual, [&](auto expectedValue, auto actualValue)
            {
                AssertEquals(expectedValue, actualValue, method, line, msg);
            });
    }

    virtual void AssertEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckEquals(
        bool expected, bool actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(
        uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckEquals(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEquals(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void CheckEquals(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void CheckEquals(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, TEnableIfIntegers<TExpected, TActual> = 0>
    void CheckEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        ForwardIntegerComparison(expected, actual, [&](auto expectedValue, auto actualValue)
            {
                CheckEquals(expectedValue, actualValue, method, line, msg);
            });
    }

    virtual void CheckEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);

protected: // Assertion/Check methods - Not Equals
    virtual void AssertNotEquals(
        bool expected, bool actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(
        uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void AssertNotEquals(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEquals(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void AssertNotEquals(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void AssertNotEquals(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, TEnableIfIntegers<TExpected, TActual> = 0>
    void AssertNotEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        ForwardIntegerComparison(expected, actual, [&](auto expectedValue, auto actualValue)
            {
                AssertNotEquals(expectedValue, actualValue, method, line, msg);
            });
    }

    virtual void AssertNotEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEqualsIC(std::wstring const& expected, std::wstring const& actual,
        std::string const& method, int line, std::string const& msg);

    virtual void CheckNotEquals(
        bool expected, bool actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(
        uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg);
    virtual void CheckNotEquals(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEquals(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void CheckNotEquals(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void CheckNotEquals(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, TEnableIfIntegers<TExpected, TActual> = 0>
    void CheckNotEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        ForwardIntegerComparison(expected, actual, [&](auto expectedValue, auto actualValue)
            {
                CheckNotEquals(expectedValue, actualValue, method, line, msg);
            });
    }

    virtual void CheckNotEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);

protected: // Assertion/Check methods - Near (floating point, absolute tolerance)
    virtual void AssertNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void AssertNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void AssertNotNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void CheckNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void CheckNotNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);

protected: // Assertion/Check methods - Boolean
    virtual void AssertFalse(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void AssertTrue(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void CheckFalse(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void CheckTrue(bool testVal, std::string const& method, int line, std::string const& msg);

protected: // Assertion/Check methods - Contains (substring)
    virtual void AssertContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotContainsIC(std::wstring const& text, std::wstring const& substring,
        std::string const& method, int line, std::string const& msg);

    virtual void CheckContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);

protected: // Assertion/Check methods - Starts/Ends With (prefix/suffix)
    virtual void AssertEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertNotStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotStartsWithIC(std::wstring const& text, std::wstring const& prefix,
        std::string const& method, int line, std::string const& msg);

    virtual void AssertStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void AssertStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckNotStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

    virtual void CheckStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);

protected: // Assertion/Check methods - Ordering (value compared with a bound)
    // Any two integer or floating-point types except bool.
    template <typename TValue, typename TBound>
    using TEnableIfOrderable = typename std::enable_if<std::is_arithmetic<TValue>::value &&
        std::is_arithmetic<TBound>::value && !std::is_same<TValue, bool>::value &&
        !std::is_same<TBound, bool>::value, int>::type;

    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void AssertGreaterThan(TValue value, TBound bound, std::string const& method, int line, std::string const& msg)
    {
        AssertOrdering(value, bound, TOrdering::GreaterThan, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void AssertGreaterThanOrEqual(TValue value, TBound bound, std::string const& method, int line,
        std::string const& msg)
    {
        AssertOrdering(value, bound, TOrdering::GreaterThanOrEqual, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void AssertLessThan(TValue value, TBound bound, std::string const& method, int line, std::string const& msg)
    {
        AssertOrdering(value, bound, TOrdering::LessThan, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void AssertLessThanOrEqual(TValue value, TBound bound, std::string const& method, int line,
        std::string const& msg)
    {
        AssertOrdering(value, bound, TOrdering::LessThanOrEqual, method, line, msg);
    }

    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void CheckGreaterThan(TValue value, TBound bound, std::string const& method, int line, std::string const& msg)
    {
        CheckOrdering(value, bound, TOrdering::GreaterThan, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void CheckGreaterThanOrEqual(TValue value, TBound bound, std::string const& method, int line,
        std::string const& msg)
    {
        CheckOrdering(value, bound, TOrdering::GreaterThanOrEqual, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void CheckLessThan(TValue value, TBound bound, std::string const& method, int line, std::string const& msg)
    {
        CheckOrdering(value, bound, TOrdering::LessThan, method, line, msg);
    }
    template <typename TValue, typename TBound, TEnableIfOrderable<TValue, TBound> = 0>
    void CheckLessThanOrEqual(TValue value, TBound bound, std::string const& method, int line,
        std::string const& msg)
    {
        CheckOrdering(value, bound, TOrdering::LessThanOrEqual, method, line, msg);
    }

#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
protected: // Assertion/Check methods - std::source_location
    // Each takes a location (by default, the caller's) in place of a method and line, and forwards to the
    // method/line overload with loc.function_name() and loc.line().
    template <typename TText, typename TSubstring>
    void AssertContains(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertContains(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void AssertContainsIC(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertContainsIC(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void AssertEndsWith(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertEndsWith(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void AssertEndsWithIC(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertEndsWithIC(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertEquals(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertEquals(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertEqualsIC(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertEqualsIC(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    void AssertFalse(bool testVal, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        AssertFalse(testVal, loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void AssertGreaterThan(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertGreaterThan(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void AssertGreaterThanOrEqual(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertGreaterThanOrEqual(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void AssertLessThan(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertLessThan(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void AssertLessThanOrEqual(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertLessThanOrEqual(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void AssertNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void AssertNotContains(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotContains(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void AssertNotContainsIC(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotContainsIC(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void AssertNotEndsWith(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEndsWith(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void AssertNotEndsWithIC(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEndsWithIC(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertNotEquals(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEquals(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertNotEqualsIC(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEqualsIC(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void AssertNotNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void AssertNotStartsWith(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotStartsWith(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void AssertNotStartsWithIC(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotStartsWithIC(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void AssertStartsWith(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertStartsWith(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void AssertStartsWithIC(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertStartsWithIC(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    void AssertTrue(bool testVal, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        AssertTrue(testVal, loc.function_name(), static_cast<int>(loc.line()), msg);
    }

    template <typename TText, typename TSubstring>
    void CheckContains(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckContains(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void CheckContainsIC(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckContainsIC(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void CheckEndsWith(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckEndsWith(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void CheckEndsWithIC(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckEndsWithIC(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckEquals(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckEquals(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckEqualsIC(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckEqualsIC(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    void CheckFalse(bool testVal, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckFalse(testVal, loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void CheckGreaterThan(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckGreaterThan(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void CheckGreaterThanOrEqual(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckGreaterThanOrEqual(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void CheckLessThan(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckLessThan(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TValue, typename TBound>
    void CheckLessThanOrEqual(TValue&& value, TBound&& bound, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckLessThanOrEqual(std::forward<TValue>(value), std::forward<TBound>(bound), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void CheckNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void CheckNotContains(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotContains(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSubstring>
    void CheckNotContainsIC(TText&& text, TSubstring&& substring, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotContainsIC(std::forward<TText>(text), std::forward<TSubstring>(substring), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void CheckNotEndsWith(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEndsWith(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TSuffix>
    void CheckNotEndsWithIC(TText&& text, TSuffix&& suffix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEndsWithIC(std::forward<TText>(text), std::forward<TSuffix>(suffix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckNotEquals(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEquals(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckNotEqualsIC(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEqualsIC(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void CheckNotNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void CheckNotStartsWith(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotStartsWith(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void CheckNotStartsWithIC(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotStartsWithIC(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void CheckStartsWith(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckStartsWith(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPrefix>
    void CheckStartsWithIC(TText&& text, TPrefix&& prefix, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckStartsWithIC(std::forward<TText>(text), std::forward<TPrefix>(prefix), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    void CheckTrue(bool testVal, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckTrue(testVal, loc.function_name(), static_cast<int>(loc.line()), msg);
    }

    void SetExceptionExpected(bool expected, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        SetExceptionExpected(expected, loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TException>
    void SetExceptionExpected(std::string const& msg, std::string const& expectedMessage = std::string(),
        std::source_location loc = std::source_location::current())
    {
        SetExceptionExpected<TException>(loc.function_name(), static_cast<int>(loc.line()), msg, expectedMessage);
    }
    void Skip(std::string const& reason, std::source_location loc = std::source_location::current())
    {
        Skip(loc.function_name(), static_cast<int>(loc.line()), reason);
    }
#endif // #if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)

public:
    TTestGroupBase(std::string const& name);

    TestCallbackList& GetTestCallbackList() override;
    std::string const& GetTestGroupName() const override;
    TTestResults const& Results() const override;
    void Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed,
        std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes) override;
    virtual void SetLogSuppressed(bool suppressed);
    void SetRunObserver(ITestRunObserver* observer) override;
};

} // namespace ASWUnitTests

#endif // ASWUnitTests_TestBaseH
