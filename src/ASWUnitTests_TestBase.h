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
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <functional>
#include <limits>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <type_traits>
#include <typeinfo>
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
    Skip,
};


/////////////////////////////////////////////////////////////////////////////
// TTestCaseRecord
//
// One test's outcome, name, and timing, collected in TTestResults::CaseRecords.
// Deliberately format-agnostic (no ANSI color codes, no XML/JSON, etc.), so a
// caller can build a structured report from it without TTestResults itself
// depending on any particular report format.
//
// GroupName is the group's qualified name (see
// ITestGroup::GetQualifiedTestGroupName()), which is just the group's own
// name when it isn't in a suite. SuitePath is the suite part on its own, so
// a caller can split the two even when a name contains '.'.
/////////////////////////////////////////////////////////////////////////////
struct TTestCaseRecord
{
    std::string GroupName;
    std::string TestName;
    double DurationSeconds;
    TTestOutcome Outcome;
    std::string Message; // Failure/skip detail; empty for Pass.
    // Empty when the group isn't in a suite. Defaulted, so a brace initializer written before this member existed
    // still compiles without a -Wmissing-field-initializers warning.
    std::string SuitePath = std::string();
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
// A predicate matched against each test's "GroupName.TestName" full name,
// where GroupName is the group's qualified name (see
// ITestGroup::GetQualifiedTestGroupName()). An empty (default-constructed) TestFilter means "run everything." Used
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

    // Receives the text that would otherwise be written to std::cout, including any line break, as plain text: without
    // the ANSI color codes the console gets, whatever the color mode (see TConsole::StripColor()).
    virtual void OnLog(std::string const& text) = 0;
    // Called once for every test that records an outcome, including the failure recorded for a test that
    // timed out, crashed, or threw an exception it didn't expect, or whose group's SetUp_Group() threw (which
    // OnTestStarted() is called for too, though the test doesn't run). Not called for a test whose SetUp_Test() or
    // TearDown_Test() threw, since that exception escapes TTestHandler::Run().
    virtual void OnTestFinished(TTestCaseRecord const& record) = 0;
    // 'groupName' is the group's qualified name, as in TTestCaseRecord::GroupName.
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
// TSuitePath
//
// The suite a test group belongs to, given to the TTestGroupBase (or
// TAutoTestGroup) constructor, e.g. TSuitePath("Framework.Console") for
// suite "Console" inside suite "Framework". Suites only organize groups:
// they have no set-up or tear-down of their own, a suite's groups run
// together, and the suite path prefixes each of its tests' full names, so
// --filter "Framework.*" selects every test in suite "Framework". An empty
// path means the group isn't in a suite. Throws TExceptInvalidSuitePath if
// any suite name is empty, e.g. "Framework..Console" or ".Framework".
/////////////////////////////////////////////////////////////////////////////
class TSuitePath
{
private:
    std::string m_Path;
    std::vector<std::string> m_Names;

public:
    explicit TSuitePath(std::string const& path);

    // The suite names in order, outermost first; empty for an empty path.
    std::vector<std::string> const& GetNames() const;
    std::string const& GetPath() const;
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

    // GetTestGroupName(), prefixed by GetSuitePath() and a '.' when the group is in a suite, e.g.
    // "Framework.Console.TTest_TConsole". Identifies the group in its tests' full names, records and reports.
    std::string GetQualifiedTestGroupName() const;
    // The suite this group belongs to, e.g. "Framework.Console" (see TSuitePath); empty when it isn't in one.
    // Not pure, so an ITestGroup implementation that predates suites still works, outside any suite.
    virtual std::string const& GetSuitePath() const
    {
        static std::string const noSuitePath;
        return noSuitePath;
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

    friend class TTestHandler; // For SetSuitePath()

private:
    // The relation an ordering method (e.g. CheckGreaterThan()) checks 'value' has to 'bound'.
    enum class TOrdering
    {
        GreaterThan,
        GreaterThanOrEqual,
        LessThan,
        LessThanOrEqual
    };

    // The Near or NotNear method a failure is reported for (see ReportNearFailure()).
    enum class ENearMethod
    {
        AssertNear,
        AssertNotNear,
        CheckNear,
        CheckNotNear
    };

    // Why a Near or NotNear method failed (see NearFailure()): an invalid tolerance (negative or NaN), or the
    // values' texts, as std::to_string() shows them.
    struct TNearFailure
    {
        bool InvalidTolerance = false;
        std::string Expected;
        std::string Actual;
        std::string Difference;
        std::string Tolerance;
    };

    // An integer for the Near methods, as its sign and magnitude, so two integers of any types can be subtracted
    // exactly (see ToNearInteger()).
    struct TNearInteger
    {
        bool Negative = false;
        uint64_t Magnitude = 0;
    };

    // True when TValue has a get() member returning a pointer, like std::unique_ptr and std::shared_ptr.
    template <typename TValue, typename = void>
    struct THasPointerGet : std::false_type
    {
    };
    template <typename TValue>
    struct THasPointerGet<TValue, std::void_t<decltype(std::declval<TValue const&>().get())> > :
        std::is_pointer<decltype(std::declval<TValue const&>().get())>
    {
    };

    // True when TValue has a size() member, which DescribeContents() shows as an element count.
    template <typename TValue, typename = void>
    struct THasSize : std::false_type
    {
    };
    template <typename TValue>
    struct THasSize<TValue, std::void_t<decltype(std::declval<TValue const&>().size())> > : std::true_type
    {
    };

    // The kind of text a string check's argument is (see TextKindOf()), which decides how it's compared with another.
    enum class ETextKind
    {
        None,          // Not text.
        NarrowString,  // std::string, as UTF-8.
        NarrowCString, // char const*, char* or a char array, as UTF-8.
        WideString,    // std::wstring.
        WideCString,   // wchar_t const*, wchar_t* or a wchar_t array.
        RTL            // System::String, or an AnsiStringT such as AnsiString or UTF8String, in RTL builds.
    };

    // True for an AnsiStringT, such as AnsiString or UTF8String, in RTL builds.
    template <typename TText>
    struct TIsAnsiStringT : std::false_type
    {
    };
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    template <unsigned short CodePage>
    struct TIsAnsiStringT<System::AnsiStringT<CodePage> > : std::true_type
    {
    };
#endif

    // A string check's std::string, char const*, std::wstring and wchar_t const* overloads, which ForwardAsUTF8()
    // and ForwardAsWide() forward to.
    using TUTF8TextCheck = void (TTestGroupBase::*)(std::string const&, std::string const&, std::string const&, int,
        std::string const&);
    using TUTF8CStringCheck = void (TTestGroupBase::*)(char const*, char const*, std::string const&, int,
        std::string const&);
    using TWideTextCheck = void (TTestGroupBase::*)(std::wstring const&, std::wstring const&, std::string const&, int,
        std::string const&);
    using TWideCStringCheck = void (TTestGroupBase::*)(wchar_t const*, wchar_t const*, std::string const&, int,
        std::string const&);

    // A lock on a group's m_RecordingMutex, like a std::unique_lock, that also counts its holds in m_RecordingHolds,
    // so ReleaseRecordingHolds() can release the ones a crash left behind.
    class TRecordingLock
    {
    private:
        TTestGroupBase& m_Group;
        bool m_Locked;

    public:
        explicit TRecordingLock(TTestGroupBase& group);
        TRecordingLock(TRecordingLock const&) = delete;
        TRecordingLock& operator=(TRecordingLock const&) = delete;
        ~TRecordingLock();

        void Lock();
        bool OwnsLock() const;
        void Unlock();
    };

private:
    // How many times m_RecordingMutex's owner holds it through TRecordingLocks. Only read or changed while holding it.
    unsigned int m_RecordingHolds;

private:
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

    // Locks 'lock', for a change to this group's results or log, and returns true, unless the calling thread runs a
    // test of this group that timed out and was abandoned (see RunWithTimeout()), which must leave both alone; it
    // then returns false, with 'lock' still locked.
    bool BeginRecording(TRecordingLock& lock);
    // The same, for recording a test's outcome, after which a test running on a worker thread counts as finished in
    // time, however long its thread then takes to end.
    bool BeginRecordingOutcome(TRecordingLock& lock);

    // Runs 'callable' for the Throws/NoThrow methods and returns a description of what it threw (see
    // DescribeException()), or nullopt when it threw nothing. A failure signal from this framework (a TTestException,
    // e.g. an Assert* failure, Fail() or Skip()) passes straight through. Each exception is caught exactly once,
    // never rethrown and caught again, since an RTL exception object is freed when the first handler that catches
    // it ends.
    template <typename TCallable>
    static std::optional<std::string> CallAndDescribeException(TCallable&& callable)
    {
        try
        {
            static_cast<void>(std::forward<TCallable>(callable)());
        }
        catch (TTestException const&)
        {
            throw;
        }
        catch (std::exception const& ex)
        {
            return DescribeException(ex);
        }
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        catch (System::Sysutils::Exception& ex)
        {
            return DescribeException(ex);
        }
#endif
        catch (...)
        {
            return DescribeUnknownException();
        }

        return std::nullopt;
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

    // Describes a non-empty 'value' for an Empty failure: its text (e.g. "was \"abc\""), with wide text as UTF-8, its
    // element count (e.g. "had 3 elements") when it has a size(), or "was not empty".
    static std::string DescribeContents(std::string const& text);
    static std::string DescribeContents(std::wstring const& text);
    template <typename TValue>
    static std::string DescribeContents(TValue const& value)
    {
        // Text types other than std::string/std::wstring, such as std::string_view.
        if constexpr (std::is_constructible<std::string, TValue const&>::value)
        {
            return DescribeContents(std::string(value));
        }
        else if constexpr (std::is_constructible<std::wstring, TValue const&>::value)
        {
            return DescribeContents(std::wstring(value));
        }
        else if constexpr (THasSize<TValue>::value)
        {
            unsigned long long const size = static_cast<unsigned long long>(value.size());
            return "had " + std::to_string(size) + ((size == 1) ? " element" : " elements");
        }
        else
        {
            return "was not empty";
        }
    }

    // Describes 'ex' for a Throws/NoThrow failure: what() for a std::exception, or "ClassName: Message" for an RTL
    // exception (see DescribeRTLException()).
    static std::string DescribeException(std::exception const& ex);
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    static std::string DescribeException(System::Sysutils::Exception& ex);
#endif

    // Describes 'ex', caught as a TException that is neither a std::exception nor an RTL exception, and so has no
    // message, by its type (its dynamic type, for a polymorphic class), e.g. "an object of type \"int\"".
    template <typename TException>
    static std::string DescribeOtherException(TException const& ex)
    {
        return "an object of type \"" + TypeName(typeid(ex)) + "\"";
    }

    // Describes an exception that is neither a std::exception nor (in RTL builds) an RTL exception, whose type isn't
    // known, e.g. "a non-std::exception object".
    static std::string DescribeUnknownException();

    // The message a Throws method checks for its expected substring: what(), or an RTL exception's Message.
    static std::string ExceptionMessage(std::exception const& ex);
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    static std::string ExceptionMessage(System::Sysutils::Exception& ex);
#endif

    template <typename TInteger>
    static bool FitsInInt64(TInteger value)
    {
        if constexpr (std::is_signed<TInteger>::value)
            return true;
        else
            return static_cast<uint64_t>(value) <= static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    }

    // Formats 'address' in hexadecimal for a pointer comparison failure, e.g. "0x7ffd5a2c"; see FormatPointer().
    static std::string FormatAddress(std::uintptr_t address);

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

    // Formats 'pointer' for a pointer comparison failure: its address (see FormatAddress()), or "(null)".
    template <typename TPointer>
    static std::string FormatPointer(TPointer pointer)
    {
        if (pointer == nullptr)
            return "(null)";

        if constexpr (std::is_function<typename std::remove_pointer<TPointer>::type>::value)
        {
            // A function pointer can't portably be cast to an integer, so its bytes are copied instead.
            std::uintptr_t address = 0;
            std::memcpy(&address, &pointer, (sizeof(pointer) < sizeof(address)) ? sizeof(pointer) : sizeof(address));
            return FormatAddress(address);
        }
        else
        {
            return FormatAddress(reinterpret_cast<std::uintptr_t>(pointer));
        }
    }

    // Forwards a string check's two texts, of any text types (see TextKindOf()), to its std::string overload 'check'
    // as UTF-8, or, when either is a null C string, to its char const* overload 'cStringCheck' with that one null,
    // which applies the null C string rules. Used by the templates for mixed string types. The texts are taken by
    // value, so an array arrives as a pointer, and one instantiation serves every length of literal.
    template <typename TFirst, typename TSecond>
    void ForwardAsUTF8(TFirst first, TSecond second, TUTF8TextCheck check, TUTF8CStringCheck cStringCheck,
        std::string const& method, int line, std::string const& msg)
    {
        std::string const firstText = TextToUTF8(first);
        std::string const secondText = TextToUTF8(second);

        if (IsNullText(first) || IsNullText(second))
        {
            (this->*cStringCheck)(IsNullText(first) ? nullptr : firstText.c_str(),
                IsNullText(second) ? nullptr : secondText.c_str(), method, line, msg);
        }
        else
        {
            (this->*check)(firstText, secondText, method, line, msg);
        }
    }
    // The same, as wide text, for two wide texts, and for the Matches methods unless both texts are narrow.
    template <typename TFirst, typename TSecond>
    void ForwardAsWide(TFirst first, TSecond second, TWideTextCheck check, TWideCStringCheck cStringCheck,
        std::string const& method, int line, std::string const& msg)
    {
        std::wstring const firstText = TextToWide(first);
        std::wstring const secondText = TextToWide(second);

        if (IsNullText(first) || IsNullText(second))
        {
            (this->*cStringCheck)(IsNullText(first) ? nullptr : firstText.c_str(),
                IsNullText(second) ? nullptr : secondText.c_str(), method, line, msg);
        }
        else
        {
            (this->*check)(firstText, secondText, method, line, msg);
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

    // True when TException is a type the Throws methods and SetExceptionExpected<TException>() can expect: any type
    // that can be thrown and caught by reference (an object type, but not an array). In RTL builds, an RTL class
    // (a System::TObject descendant) must be an Exception or a subclass.
    template <typename TException>
    static constexpr bool IsExpectableException()
    {
        if constexpr (!std::is_object<TException>::value || std::is_array<TException>::value)
            return false;
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        else if constexpr (std::is_base_of<System::TObject, TException>::value)
            return std::is_base_of<System::Sysutils::Exception, TException>::value;
#endif
        else
            return true;
    }

    template <typename TInteger>
    static bool IsNonNegative(TInteger value)
    {
        if constexpr (std::is_signed<TInteger>::value)
            return value >= 0;
        else
            return true;
    }

    // True when 'text' is a null C string. A char or wchar_t array never is.
    template <typename TText>
    static bool IsNullText(TText const& text)
    {
        static_cast<void>(text);

        if constexpr (std::is_pointer<TText>::value)
            return text == nullptr;
        else
            return false;
    }

    // Why a Near method (or with 'expectNear' false, a NotNear method) fails for these values, or nullopt when it
    // passes. A negative or NaN tolerance fails both. Two values are near when they're equal (so equal infinities
    // are) or differ by at most 'tolerance'; a NaN is near nothing. The template is defined in the .cpp, for float,
    // double and long double.
    template <typename TFloat>
    static std::optional<TNearFailure> NearFailure(TFloat expected, TFloat actual, TFloat tolerance, bool expectNear);
    static std::optional<TNearFailure> NearFailure(TNearInteger expected, TNearInteger actual, TNearInteger tolerance,
        bool expectNear);

    // Runs 'callable' for the NoThrow methods (e.g. CheckNoThrow()) and returns why it failed, or an empty string when
    // it threw nothing. A failure signal from this framework (e.g. an Assert* failure inside 'callable') passes
    // straight through, failing the test as usual.
    template <typename TCallable>
    static std::string NoThrowFailure(TCallable&& callable)
    {
        std::optional<std::string> const thrown = CallAndDescribeException(std::forward<TCallable>(callable));
        if (thrown != std::nullopt)
            return "Expected no exception but caught: " + *thrown;

        return std::string();
    }

    // True when 'order' (from CompareForOrdering()) satisfies 'ordering'; never when it's nullopt (a NaN).
    static bool OrderingHolds(std::optional<int> order, TOrdering ordering);
    // The operator for 'ordering' in a failure message, e.g. ">=".
    static char const* OrderingSymbol(TOrdering ordering);

    // Releases every hold of m_RecordingMutex this thread has left, after a crash caught on it skipped the
    // TRecordingLocks' destructors.
    void ReleaseRecordingHolds();

    // Records 'failure' (from NearFailure()) as 'which' does, or throws it for an Assert method.
    void ReportNearFailure(TNearFailure const& failure, ENearMethod which, std::string const& method, int line,
        std::string const& msg);

    // The address the Same methods (e.g. CheckSame()) compare: a smart pointer's get(), or a raw pointer (or an
    // array, or a function) as it is.
    template <typename TValue, typename std::enable_if<
        std::is_pointer<decltype(std::declval<TValue const&>().get())>::value, int>::type = 0>
    static auto SameAddress(TValue const& value)
    {
        return value.get();
    }
    template <typename TValue, typename std::enable_if<
        std::is_pointer<typename std::decay<TValue const>::type>::value, int>::type = 0>
    static typename std::decay<TValue const>::type SameAddress(TValue const& value)
    {
        return value;
    }

    // The std::exception 'ex' is, for a Throws method's message check: 'ex' itself, or for a polymorphic class not
    // derived from std::exception, the std::exception it's part of (e.g. when TException is a mixin base of a
    // std::runtime_error subclass); nullptr when there's none.
    template <typename TException>
    static std::exception const* StdExceptionOf(TException const& ex)
    {
        static_cast<void>(ex);

        if constexpr (std::is_base_of<std::exception, TException>::value)
            return std::addressof(ex);
        else if constexpr (std::is_polymorphic<TException>::value)
            return dynamic_cast<std::exception const*>(std::addressof(ex));
        else
            return nullptr;
    }

    // Returns 'text' as UTF-8, for the templates for mixed string types (see ForwardAsUTF8()). A null C string is
    // empty text; see IsNullText().
    static std::string TextToUTF8(std::string const& text);
    static std::string TextToUTF8(std::wstring const& text);
    static std::string TextToUTF8(char const* text);
    static std::string TextToUTF8(wchar_t const* text);
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    static std::string TextToUTF8(System::String const& text);
    template <unsigned short CodePage>
    static std::string TextToUTF8(System::AnsiStringT<CodePage> const& text)
    {
        return TextToUTF8(System::String(text));
    }
#endif
    // The same, as wide text (see ForwardAsWide()).
    static std::wstring TextToWide(std::string const& text);
    static std::wstring TextToWide(std::wstring const& text);
    static std::wstring TextToWide(char const* text);
    static std::wstring TextToWide(wchar_t const* text);
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    static std::wstring TextToWide(System::String const& text);
    template <unsigned short CodePage>
    static std::wstring TextToWide(System::AnsiStringT<CodePage> const& text)
    {
        return TextToWide(System::String(text));
    }
#endif

    template <typename T>
    static int ThreeWayCompare(T a, T b)
    {
        return (a < b) ? -1 : ((b < a) ? 1 : 0);
    }

    // Runs 'callable' for the Throws methods (e.g. CheckThrows()) and returns why it failed, or an empty string when
    // it threw a TException (or a subclass) whose message contains 'expectedMessage', when that isn't empty (a
    // TException with no message then fails). A failure signal from this framework (e.g. an Assert* failure inside
    // 'callable') passes straight through, failing the test as usual, unless TException asks for one (e.g.
    // TExceptTrue).
    template <typename TException, typename TCallable>
    static std::string ThrowsFailure(TCallable&& callable, std::string const& expectedMessage)
    {
        static_assert(IsExpectableException<TException>(), "TException must be a type that can be thrown and caught "
            "by reference (not an array), and with ASWUNITTESTS_RTL_EXCEPTIONS an RTL class must be an Exception");

        // A type that is neither a std::exception nor an RTL exception is caught by const reference, as Test() does
        // (see m_ExpectedOtherExceptionTypeChecker): MSVC never matches a pointer to const (e.g. a thrown string
        // literal, a char const*) with a handler that's a non-const reference to it, even for its exact type.
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        constexpr bool isRTLException = std::is_base_of<System::Sysutils::Exception, TException>::value;
#else
        constexpr bool isRTLException = false;
#endif
        constexpr bool caughtAsIs = isRTLException || std::is_base_of<std::exception, TException>::value;
        typedef typename std::conditional<caughtAsIs, TException, TException const>::type TCaught;

        bool caught = false;
        std::string failure;

        // The TException handler is in its own try block, so anything else 'callable' throws propagates once to
        // CallAndDescribeException()'s handlers rather than being rethrown and caught again (see there).
        std::optional<std::string> const otherException = CallAndDescribeException([&]()
            {
                try
                {
                    static_cast<void>(std::forward<TCallable>(callable)());
                }
                catch (TCaught& ex)
                {
                    // A TException that is a base of TTestException (e.g. std::exception) also catches the
                    // framework's own signals, which must still pass through.
                    if constexpr (std::is_base_of<TException, TTestException>::value &&
                                  !std::is_same<TException, TTestException>::value)
                    {
                        if (dynamic_cast<TTestException*>(&ex) != nullptr)
                            throw;
                    }

                    caught = true;

                    if (!expectedMessage.empty())
                        failure = ThrowsMessageFailure(ex, expectedMessage);
                }
            });

        if (otherException != std::nullopt)
            return "Expected a different exception type but caught: " + *otherException;

        if (!caught)
            return "Expected an exception but none was thrown";

        return failure;
    }

    // Why 'ex', a TException a Throws method caught, fails its non-empty 'expectedMessage', or an empty string when
    // its message contains it: an RTL exception's Message, or a std::exception's what() (see StdExceptionOf()). Any
    // other type has no message, so it fails.
    template <typename TException>
    static std::string ThrowsMessageFailure(TException& ex, std::string const& expectedMessage)
    {
        std::string const expected = "Expected the exception message to contain \"" + expectedMessage + "\" but caught";

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        if constexpr (std::is_base_of<System::Sysutils::Exception, TException>::value)
        {
            if (ExceptionMessage(ex).find(expectedMessage) == std::string::npos)
                return expected + ": " + DescribeException(ex);
        }
        else
#endif
        {
            std::exception const* const stdEx = StdExceptionOf(ex);
            if (stdEx == nullptr)
                return expected + " an exception without a message: " + DescribeOtherException(ex);
            if (ExceptionMessage(*stdEx).find(expectedMessage) == std::string::npos)
                return expected + ": " + DescribeException(*stdEx);
        }

        return std::string();
    }

    template <typename TInteger>
    static TNearInteger ToNearInteger(TInteger value)
    {
        TNearInteger result;

        if constexpr (std::is_signed<TInteger>::value)
        {
            result.Negative = (value < 0);
            // A negative value's magnitude, computed so the most negative int64_t doesn't overflow.
            result.Magnitude = result.Negative ? static_cast<uint64_t>(-(static_cast<int64_t>(value) + 1)) + 1 :
                    static_cast<uint64_t>(value);
        }
        else
        {
            result.Magnitude = static_cast<uint64_t>(value);
        }

        return result;
    }

    // Puts the group in suite 'suitePath', for TTestHandler when the group's registration names its suite (see
    // ASW_REGISTER_TEST_GROUP_IN_SUITE).
    void SetSuitePath(std::string const& suitePath);

    // Why 'value' fails a type check against TType (with 'expectType' false for an IsNotType method), or an empty
    // string when it passes. The check uses dynamic_cast, so a subclass of TType counts as a TType, and a null pointer
    // never does. A failure shows the type names (see TypeName()), with the object's dynamic type.
    template <typename TType, typename TValue>
    static std::string TypeCheckFailure(TValue const& value, bool expectType)
    {
        static_assert(std::is_class<TType>::value, "TType must be a class type, e.g. CheckIsType<TCircle>(shape, ...)");

        auto const pointer = TypeCheckPointer(value);
        typedef typename std::remove_cv<typename std::remove_pointer<decltype(pointer)>::type>::type TPointee;
        static_assert(std::is_polymorphic<TPointee>::value,
            "the value must point to a polymorphic class (one with a virtual function), as dynamic_cast requires");

        bool const isType = (dynamic_cast<TType const*>(pointer) != nullptr);
        if (isType == expectType)
            return std::string();

        std::string const actual = (pointer == nullptr) ? std::string("null") :
                ("\"" + TypeCheckObjectName(pointer) + "\"");
        return std::string(expectType ? "Expected type \"" : "Expected not type \"") + TypeCheckName<TType>() +
            "\" but was " + actual;
    }

    // The name a type check failure shows for TType: in RTL builds, a TObject descendant's ClassName() (e.g.
    // "TStringList"), the same with every compiler; otherwise TypeName().
    template <typename TType>
    static std::string TypeCheckName()
    {
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        if constexpr (std::is_base_of<System::TObject, TType>::value)
            return TextToUTF8(__classid(TType)->ClassName());
        else
#endif
        return TypeName(typeid(TType));
    }

    // The same for the object 'pointer' points to, by its dynamic type: its ClassName() or TypeName().
    template <typename TPointee>
    static std::string TypeCheckObjectName(TPointee const* pointer)
    {
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        if constexpr (std::is_base_of<System::TObject, TPointee>::value)
            return TextToUTF8(const_cast<TPointee*>(pointer)->ClassName());
        else
#endif
        return TypeName(typeid(*pointer));
    }

    // The pointer a type check inspects: a raw pointer as it is, a smart pointer's get(), or an object's address.
    template <typename TValue>
    static auto TypeCheckPointer(TValue const& value)
    {
        if constexpr (std::is_pointer<TValue>::value)
            return value;
        else if constexpr (THasPointerGet<TValue>::value)
            return value.get();
        else
            return std::addressof(value);
    }

    // 'type''s name for a failure message, demangled where the compiler's name() is mangled (e.g. GCC, Clang and
    // bcc32c).
    static std::string TypeName(std::type_info const& type);

    // The work of the Near and NotNear templates for mixed types (see 'which'). A float or double common type goes to
    // the virtual overload, so an override still sees the call; a long double one, or integers (compared exactly, as
    // their sign and magnitude), to NearFailure().
    template <typename TExpected, typename TActual, typename TTolerance>
    void VerifyMixedNear(ENearMethod which, TExpected expected, TActual actual, TTolerance tolerance,
        std::string const& method, int line, std::string const& msg)
    {
        typedef typename std::common_type<TExpected, TActual, TTolerance>::type TCommon;

        if constexpr (std::is_same<TCommon, float>::value || std::is_same<TCommon, double>::value)
        {
            TCommon const commonExpected = static_cast<TCommon>(expected);
            TCommon const commonActual = static_cast<TCommon>(actual);
            TCommon const commonTolerance = static_cast<TCommon>(tolerance);

            switch (which)
            {
                case ENearMethod::AssertNear:
                    AssertNear(commonExpected, commonActual, commonTolerance, method, line, msg);
                    break;
                case ENearMethod::AssertNotNear:
                    AssertNotNear(commonExpected, commonActual, commonTolerance, method, line, msg);
                    break;
                case ENearMethod::CheckNear:
                    CheckNear(commonExpected, commonActual, commonTolerance, method, line, msg);
                    break;
                case ENearMethod::CheckNotNear:
                    CheckNotNear(commonExpected, commonActual, commonTolerance, method, line, msg);
                    break;
            }
        }
        else
        {
            bool const expectNear = (which == ENearMethod::AssertNear || which == ENearMethod::CheckNear);
            std::optional<TNearFailure> failure;

            if constexpr (std::is_integral<TCommon>::value)
            {
                failure = NearFailure(ToNearInteger(expected), ToNearInteger(actual), ToNearInteger(tolerance),
                    expectNear);
            }
            else
            {
                failure = NearFailure(static_cast<TCommon>(expected), static_cast<TCommon>(actual),
                    static_cast<TCommon>(tolerance), expectNear);
            }

            if (failure != std::nullopt)
                ReportNearFailure(*failure, which, method, line, msg);
        }
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
    // For an expected type not derived from std::exception (see SetExceptionExpected<TException>()): called only from
    // Test()'s catch (...) handler, which no std::exception or RTL exception reaches, it rethrows the exception being
    // handled to match it against that type, and returns its description (see DescribeOtherException()) when it
    // matches, or nullopt.
    std::function<std::optional<std::string> ()> m_ExpectedOtherExceptionTypeChecker;
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    std::function<bool (System::Sysutils::Exception&)> m_ExpectedRTLExceptionTypeChecker;
#endif
    std::string m_Name;
    std::string m_QualifiedName; // See GetQualifiedTestGroupName().
    // Held while a test records a result or logs, and while RunWithTimeout() decides to abandon a test that timed out,
    // so an abandoned test can't change the results or the log; see BeginRecording(). The framework locks it through
    // TRecordingLocks, so a crash can't leave it locked (see ReleaseRecordingHolds()).
    std::recursive_mutex m_RecordingMutex;
    TTestResults m_Results;
    ITestRunObserver* m_RunObserver; // Not owned; nullptr when none is set.
    std::string m_SuitePath;
    TestCallbackList m_TestCallbacks;

    // True when the current test's exception expectation (see SetExceptionExpected<TException>()) also
    // requires a specific exception type, rather than accepting any thrown exception.
    virtual bool ExceptionTypeExpected() const;

    // Aborts the current test and reports it as failed, like a failed Assert* method, even while an exception is
    // expected.
    virtual void Fail(std::string const& method, int line, std::string const& msg);

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
    // Expects a specific exception type, of any kind (e.g. a std::exception class, another class, or int), matched
    // polymorphically, so a base class also matches its subclasses. When 'expectedMessage' is non-empty, the caught
    // exception's what() must also contain it as a substring; a type with no message (neither a std::exception nor
    // an RTL exception) then fails. With ASWUNITTESTS_RTL_EXCEPTIONS enabled (see ASWUnitTests_Exception.h),
    // 'TException' may also be an RTL exception class (System::Sysutils::Exception or a subclass), whose Message is
    // checked instead of what().
    template <typename TException>
    void SetExceptionExpected(std::string const& method, int line, std::string const& msg,
        std::string const& expectedMessage = std::string())
    {
        static_assert(IsExpectableException<TException>(), "TException must be a type that can be thrown and caught "
            "by reference (not an array), and with ASWUNITTESTS_RTL_EXCEPTIONS an RTL class must be an Exception");

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
            // A class also matches a std::exception it's a base of (e.g. a mixin of a std::runtime_error subclass).
            if constexpr (std::is_class_v<TException>)
            {
                m_ExpectedExceptionTypeChecker = [](std::exception const& ex)
                    {
                        return dynamic_cast<TException const*>(&ex) != nullptr;
                    };
            }

            if constexpr (!std::is_base_of_v<std::exception, TException>)
            {
                m_ExpectedOtherExceptionTypeChecker = []()
                    {
                        std::optional<std::string> description;

                        try
                        {
                            throw;
                        }
                        catch (TException const& ex)
                        {
                            description = DescribeOtherException(ex);
                        }
                        catch (...)
                        {
                        }

                        return description;
                    };
            }
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
    // Any two pointers, compared by address, except two C strings, narrow or wide, which the overloads above and the
    // templates for mixed string types (see TEnableIfMixedText) compare by content. Without these, two pointers would
    // convert to bool and match the bool overload. Two pointers that can't be compared with each other (e.g. int*
    // and long*) are a compile error.
    template <typename TPointer, typename TChar>
    using TPointsTo = std::is_same<typename std::remove_cv<typename std::remove_pointer<TPointer>::type>::type, TChar>;
    template <typename TPointer>
    using TIsCStringPointer = std::integral_constant<bool, TPointsTo<TPointer, char>::value ||
        TPointsTo<TPointer, wchar_t>::value>;
    template <typename TExpected, typename TActual>
    using TEnableIfPointers = typename std::enable_if<std::is_pointer<TExpected>::value &&
        std::is_pointer<TActual>::value &&
        !(TIsCStringPointer<TExpected>::value && TIsCStringPointer<TActual>::value), int>::type;
    template <typename TExpected, typename TActual, TEnableIfPointers<TExpected, TActual> = 0>
    void AssertEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (expected != actual)
            throw TExceptEquals(method, line, FormatPointer(expected), FormatPointer(actual), msg);
    }
    // Two values of the same scoped enum (enum class) type, compared by their underlying values, which a failure
    // shows. An unscoped enum converts to an integer and matches an overload above instead.
    template <typename TEnum>
    using TEnableIfScopedEnum = typename std::enable_if<std::is_enum<TEnum>::value &&
        !std::is_convertible<TEnum, int>::value, int>::type;
    template <typename TEnum, TEnableIfScopedEnum<TEnum> = 0>
    void AssertEquals(TEnum expected, TEnum actual, std::string const& method, int line, std::string const& msg)
    {
        typedef typename std::underlying_type<TEnum>::type TUnderlying;
        AssertEquals(static_cast<TUnderlying>(expected), static_cast<TUnderlying>(actual), method, line, msg);
    }

    virtual void AssertEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    // A null C string equals only another null one, as in CheckEquals(), and a failure shows it as "(null)".
    void AssertEqualsIC(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void AssertEqualsIC(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);

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
    template <typename TExpected, typename TActual, TEnableIfPointers<TExpected, TActual> = 0>
    void CheckEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (expected != actual)
            SetTestFailedCheck(method, line, FormatPointer(expected), FormatPointer(actual), msg);
    }
    template <typename TEnum, TEnableIfScopedEnum<TEnum> = 0>
    void CheckEquals(TEnum expected, TEnum actual, std::string const& method, int line, std::string const& msg)
    {
        typedef typename std::underlying_type<TEnum>::type TUnderlying;
        CheckEquals(static_cast<TUnderlying>(expected), static_cast<TUnderlying>(actual), method, line, msg);
    }

    virtual void CheckEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void CheckEqualsIC(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void CheckEqualsIC(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);

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
    // Two pointers, compared by address; see TEnableIfPointers.
    template <typename TExpected, typename TActual, TEnableIfPointers<TExpected, TActual> = 0>
    void AssertNotEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (expected == actual)
            throw TExceptNotEquals(method, line, FormatPointer(expected), msg);
    }
    // Two values of the same scoped enum type, compared by their underlying values; see TEnableIfScopedEnum.
    template <typename TEnum, TEnableIfScopedEnum<TEnum> = 0>
    void AssertNotEquals(TEnum expected, TEnum actual, std::string const& method, int line, std::string const& msg)
    {
        typedef typename std::underlying_type<TEnum>::type TUnderlying;
        AssertNotEquals(static_cast<TUnderlying>(expected), static_cast<TUnderlying>(actual), method, line, msg);
    }

    virtual void AssertNotEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEqualsIC(std::wstring const& expected, std::wstring const& actual,
        std::string const& method, int line, std::string const& msg);
    // A null C string equals only another null one, as in CheckNotEquals().
    void AssertNotEqualsIC(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void AssertNotEqualsIC(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);

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
    template <typename TExpected, typename TActual, TEnableIfPointers<TExpected, TActual> = 0>
    void CheckNotEquals(TExpected expected, TActual actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (expected == actual)
            SetTestFailedCheckNotEquals(method, line, FormatPointer(expected), msg);
    }
    template <typename TEnum, TEnableIfScopedEnum<TEnum> = 0>
    void CheckNotEquals(TEnum expected, TEnum actual, std::string const& method, int line, std::string const& msg)
    {
        typedef typename std::underlying_type<TEnum>::type TUnderlying;
        CheckNotEquals(static_cast<TUnderlying>(expected), static_cast<TUnderlying>(actual), method, line, msg);
    }

    virtual void CheckNotEqualsIC(std::string const& expected, std::string const& actual, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEqualsIC(std::wstring const& expected, std::wstring const& actual, std::string const& method,
        int line, std::string const& msg);
    void CheckNotEqualsIC(char const* expected, char const* actual, std::string const& method, int line,
        std::string const& msg);
    void CheckNotEqualsIC(wchar_t const* expected, wchar_t const* actual, std::string const& method, int line,
        std::string const& msg);

protected: // Assertion/Check methods - Near (absolute tolerance)
    // 'actual' is near 'expected' when they're equal (so equal infinities are) or differ by at most 'tolerance'; a NaN
    // is near nothing. A negative or NaN tolerance fails both Near and NotNear. The templates take any three integer
    // or floating-point types except bool (all float or all double use the overloads): integers are compared
    // exactly, and otherwise all three are converted to their common type, as the built-in operators do.
    template <typename TValue>
    static constexpr bool IsNearArithmetic()
    {
        return std::is_arithmetic<TValue>::value && !std::is_same<TValue, bool>::value;
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    static constexpr bool IsMixedNear()
    {
        constexpr bool allSame = std::is_same<TExpected, TActual>::value && std::is_same<TActual, TTolerance>::value;
        constexpr bool hasOverload = allSame &&
            (std::is_same<TExpected, float>::value || std::is_same<TExpected, double>::value);

        return IsNearArithmetic<TExpected>() && IsNearArithmetic<TActual>() && IsNearArithmetic<TTolerance>() &&
            !hasOverload;
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    using TEnableIfMixedNear = typename std::enable_if<IsMixedNear<TExpected, TActual, TTolerance>(), int>::type;

    virtual void AssertNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void AssertNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, typename TTolerance,
        TEnableIfMixedNear<TExpected, TActual, TTolerance> = 0>
    void AssertNear(TExpected expected, TActual actual, TTolerance tolerance, std::string const& method, int line,
        std::string const& msg)
    {
        VerifyMixedNear(ENearMethod::AssertNear, expected, actual, tolerance, method, line, msg);
    }

    virtual void AssertNotNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void AssertNotNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, typename TTolerance,
        TEnableIfMixedNear<TExpected, TActual, TTolerance> = 0>
    void AssertNotNear(TExpected expected, TActual actual, TTolerance tolerance, std::string const& method, int line,
        std::string const& msg)
    {
        VerifyMixedNear(ENearMethod::AssertNotNear, expected, actual, tolerance, method, line, msg);
    }

    virtual void CheckNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void CheckNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, typename TTolerance,
        TEnableIfMixedNear<TExpected, TActual, TTolerance> = 0>
    void CheckNear(TExpected expected, TActual actual, TTolerance tolerance, std::string const& method, int line,
        std::string const& msg)
    {
        VerifyMixedNear(ENearMethod::CheckNear, expected, actual, tolerance, method, line, msg);
    }

    virtual void CheckNotNear(float expected, float actual, float tolerance, std::string const& method, int line,
        std::string const& msg);
    virtual void CheckNotNear(double expected, double actual, double tolerance, std::string const& method, int line,
        std::string const& msg);
    template <typename TExpected, typename TActual, typename TTolerance,
        TEnableIfMixedNear<TExpected, TActual, TTolerance> = 0>
    void CheckNotNear(TExpected expected, TActual actual, TTolerance tolerance, std::string const& method, int line,
        std::string const& msg)
    {
        VerifyMixedNear(ENearMethod::CheckNotNear, expected, actual, tolerance, method, line, msg);
    }

protected: // Assertion/Check methods - Boolean
    virtual void AssertFalse(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void AssertTrue(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void CheckFalse(bool testVal, std::string const& method, int line, std::string const& msg);
    virtual void CheckTrue(bool testVal, std::string const& method, int line, std::string const& msg);

protected: // Assertion/Check methods - Null (value compared with nullptr)
    // Any type that can be compared with nullptr, e.g. a raw pointer, std::unique_ptr, std::shared_ptr or
    // std::function, except an array, which is never null.
    template <typename TValue>
    using TEnableIfNullComparable = typename std::enable_if<!std::is_array<TValue>::value &&
        std::is_convertible<decltype(std::declval<TValue const&>() == nullptr), bool>::value, int>::type;

    template <typename TValue, TEnableIfNullComparable<TValue> = 0>
    void AssertNotNull(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        bool const isNull = (value == nullptr);
        if (isNull)
            throw TExceptNotNull(method, line, msg);
    }
    template <typename TValue, TEnableIfNullComparable<TValue> = 0>
    void AssertNull(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        bool const isNull = (value == nullptr);
        if (!isNull)
            throw TExceptNull(method, line, msg);
    }

    template <typename TValue, TEnableIfNullComparable<TValue> = 0>
    void CheckNotNull(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        bool const isNull = (value == nullptr);
        if (isNull)
            SetTestFailedCheck(method, line, "Expected not null but was null: \"" + msg + "\"");
    }
    template <typename TValue, TEnableIfNullComparable<TValue> = 0>
    void CheckNull(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        bool const isNull = (value == nullptr);
        if (!isNull)
            SetTestFailedCheck(method, line, "Expected null but was not null: \"" + msg + "\"");
    }

protected: // Assertion/Check methods - Same (identity, compared by address)
    // Any two raw pointers, arrays or smart pointers (std::unique_ptr, std::shared_ptr) that point to types that can
    // be compared with each other, mixed freely. Unlike CheckEquals(), two C strings are compared by address too.
    template <typename TExpected, typename TActual>
    using TEnableIfSameComparable = typename std::enable_if<std::is_convertible<
        decltype(SameAddress(std::declval<TExpected const&>()) == SameAddress(std::declval<TActual const&>())),
        bool>::value, int>::type;

    template <typename TExpected, typename TActual, TEnableIfSameComparable<TExpected, TActual> = 0>
    void AssertNotSame(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (SameAddress(expected) == SameAddress(actual))
            throw TExceptNotSame(method, line, FormatPointer(SameAddress(expected)), msg);
    }
    template <typename TExpected, typename TActual, TEnableIfSameComparable<TExpected, TActual> = 0>
    void AssertSame(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (SameAddress(expected) != SameAddress(actual))
            throw TExceptSame(method, line, FormatPointer(SameAddress(expected)), FormatPointer(SameAddress(actual)),
                msg);
    }

    template <typename TExpected, typename TActual, TEnableIfSameComparable<TExpected, TActual> = 0>
    void CheckNotSame(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (SameAddress(expected) == SameAddress(actual))
        {
            SetTestFailedCheck(method, line, "Expected a different object but both are \"" +
                FormatPointer(SameAddress(expected)) + "\". " + msg);
        }
    }
    template <typename TExpected, typename TActual, TEnableIfSameComparable<TExpected, TActual> = 0>
    void CheckSame(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if (SameAddress(expected) != SameAddress(actual))
        {
            SetTestFailedCheck(method, line, "Expected the same object as \"" + FormatPointer(SameAddress(expected)) +
                "\" but was \"" + FormatPointer(SameAddress(actual)) + "\". " + msg);
        }
    }

protected: // Assertion/Check methods - Type (dynamic type, via dynamic_cast)
    // Each takes a raw pointer, a smart pointer (std::unique_ptr, std::shared_ptr) or an object, of a polymorphic
    // class, and checks whether it is a TType, a subclass counting as one. A null pointer is never a TType. A failure
    // shows the expected type and the object's actual type; see TypeCheckFailure().
    template <typename TType, typename TValue>
    void AssertIsNotType(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = TypeCheckFailure<TType>(value, false);
        if (!failure.empty())
            throw TExceptIsNotType(method, line, failure, msg);
    }
    template <typename TType, typename TValue>
    void AssertIsType(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = TypeCheckFailure<TType>(value, true);
        if (!failure.empty())
            throw TExceptIsType(method, line, failure, msg);
    }

    template <typename TType, typename TValue>
    void CheckIsNotType(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = TypeCheckFailure<TType>(value, false);
        if (!failure.empty())
            SetTestFailedCheck(method, line, failure + ". " + msg);
    }
    template <typename TType, typename TValue>
    void CheckIsType(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = TypeCheckFailure<TType>(value, true);
        if (!failure.empty())
            SetTestFailedCheck(method, line, failure + ". " + msg);
    }

protected: // Assertion/Check methods - Throws (exception from a callable)
    // Each runs 'callable' (e.g. a lambda) and checks whether it throws. Unlike with SetExceptionExpected(), the test
    // carries on afterwards (unless an Assert method fails), so it can check several calls and the state after each.
    // The Throws methods pass when 'callable' throws a TException or a subclass (TException being of any kind, as
    // with SetExceptionExpected<TException>(), e.g. a std::exception class, with ASWUNITTESTS_RTL_EXCEPTIONS an RTL
    // Exception, another class, or int) whose message contains 'expectedMessage', when that isn't empty: what(), or an
    // RTL exception's Message. A TException with no message then fails. A failure signal from this framework inside
    // 'callable' (e.g. an Assert* failure, Fail() or Skip()) still ends the test as usual, unless TException asks for
    // one.
    template <typename TCallable>
    void AssertNoThrow(TCallable&& callable, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = NoThrowFailure(std::forward<TCallable>(callable));
        if (!failure.empty())
            throw TExceptNoThrow(method, line, failure, msg);
    }
    template <typename TException, typename TCallable>
    void AssertThrows(TCallable&& callable, std::string const& method, int line, std::string const& msg,
        std::string const& expectedMessage = std::string())
    {
        std::string const failure = ThrowsFailure<TException>(std::forward<TCallable>(callable), expectedMessage);
        if (!failure.empty())
            throw TExceptThrows(method, line, failure, msg);
    }

    template <typename TCallable>
    void CheckNoThrow(TCallable&& callable, std::string const& method, int line, std::string const& msg)
    {
        std::string const failure = NoThrowFailure(std::forward<TCallable>(callable));
        if (!failure.empty())
            SetTestFailedCheck(method, line, failure + ". " + msg);
    }
    template <typename TException, typename TCallable>
    void CheckThrows(TCallable&& callable, std::string const& method, int line, std::string const& msg,
        std::string const& expectedMessage = std::string())
    {
        std::string const failure = ThrowsFailure<TException>(std::forward<TCallable>(callable), expectedMessage);
        if (!failure.empty())
            SetTestFailedCheck(method, line, failure + ". " + msg);
    }

protected: // Assertion/Check methods - Empty (string or container)
    // Anything with an empty() member, e.g. std::string, std::wstring, std::vector or std::map. A failure shows a
    // non-empty value's text or element count; see DescribeContents().
    template <typename TValue>
    using TEnableIfHasEmpty = typename std::enable_if<
        std::is_convertible<decltype(std::declval<TValue const&>().empty()), bool>::value, int>::type;

    template <typename TValue, TEnableIfHasEmpty<TValue> = 0>
    void AssertEmpty(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        if (!value.empty())
            throw TExceptEmpty(method, line, "Expected empty but " + DescribeContents(value), msg);
    }
    // A C string, or a character array taken as one (so an array holding "" is empty, whatever its size). A null C
    // string is neither empty nor not empty: both methods fail, showing it as "(null)".
    void AssertEmpty(char const* text, std::string const& method, int line, std::string const& msg);
    void AssertEmpty(wchar_t const* text, std::string const& method, int line, std::string const& msg);
    template <typename TValue, TEnableIfHasEmpty<TValue> = 0>
    void AssertNotEmpty(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        if (value.empty())
            throw TExceptNotEmpty(method, line, msg);
    }
    void AssertNotEmpty(char const* text, std::string const& method, int line, std::string const& msg);
    void AssertNotEmpty(wchar_t const* text, std::string const& method, int line, std::string const& msg);

    template <typename TValue, TEnableIfHasEmpty<TValue> = 0>
    void CheckEmpty(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        if (!value.empty())
            SetTestFailedCheck(method, line, "Expected empty but " + DescribeContents(value) + ". " + msg);
    }
    void CheckEmpty(char const* text, std::string const& method, int line, std::string const& msg);
    void CheckEmpty(wchar_t const* text, std::string const& method, int line, std::string const& msg);
    template <typename TValue, TEnableIfHasEmpty<TValue> = 0>
    void CheckNotEmpty(TValue const& value, std::string const& method, int line, std::string const& msg)
    {
        if (value.empty())
            SetTestFailedCheck(method, line, "Expected not empty but was empty. " + msg);
    }
    void CheckNotEmpty(char const* text, std::string const& method, int line, std::string const& msg);
    void CheckNotEmpty(wchar_t const* text, std::string const& method, int line, std::string const& msg);

protected: // Assertion/Check methods - Contains (substring)
    // A null C string, as the text or the substring, matches nothing: the Contains methods fail, showing it as
    // "(null)", and the NotContains ones pass.
    virtual void AssertContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void AssertContains(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void AssertContains(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void AssertContainsIC(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void AssertContainsIC(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void AssertNotContains(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void AssertNotContains(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotContainsIC(std::wstring const& text, std::wstring const& substring,
        std::string const& method, int line, std::string const& msg);
    void AssertNotContainsIC(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void AssertNotContainsIC(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void CheckContains(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void CheckContains(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void CheckContainsIC(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void CheckContainsIC(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotContains(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotContains(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void CheckNotContains(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void CheckNotContains(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotContainsIC(std::string const& text, std::string const& substring, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotContainsIC(std::wstring const& text, std::wstring const& substring, std::string const& method,
        int line, std::string const& msg);
    void CheckNotContainsIC(char const* text, char const* substring, std::string const& method, int line,
        std::string const& msg);
    void CheckNotContainsIC(wchar_t const* text, wchar_t const* substring, std::string const& method, int line,
        std::string const& msg);

protected: // Assertion/Check methods - Starts/Ends With (prefix/suffix)
    // A null C string, as the text or the prefix or suffix, matches nothing: the StartsWith and EndsWith methods
    // fail, showing it as "(null)", and their Not forms pass.
    virtual void AssertEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void AssertEndsWith(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void AssertEndsWith(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void AssertEndsWithIC(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void AssertEndsWithIC(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void AssertNotEndsWith(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void AssertNotEndsWith(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void AssertNotEndsWithIC(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void AssertNotEndsWithIC(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void AssertNotStartsWith(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void AssertNotStartsWith(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotStartsWithIC(std::wstring const& text, std::wstring const& prefix,
        std::string const& method, int line, std::string const& msg);
    void AssertNotStartsWithIC(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void AssertNotStartsWithIC(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void AssertStartsWith(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void AssertStartsWith(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void AssertStartsWithIC(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void AssertStartsWithIC(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void CheckEndsWith(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void CheckEndsWith(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void CheckEndsWithIC(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void CheckEndsWithIC(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotEndsWith(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEndsWith(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void CheckNotEndsWith(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void CheckNotEndsWith(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotEndsWithIC(std::string const& text, std::string const& suffix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEndsWithIC(std::wstring const& text, std::wstring const& suffix, std::string const& method,
        int line, std::string const& msg);
    void CheckNotEndsWithIC(char const* text, char const* suffix, std::string const& method, int line,
        std::string const& msg);
    void CheckNotEndsWithIC(wchar_t const* text, wchar_t const* suffix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void CheckNotStartsWith(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void CheckNotStartsWith(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void CheckNotStartsWithIC(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void CheckNotStartsWithIC(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckStartsWith(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckStartsWith(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void CheckStartsWith(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void CheckStartsWith(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckStartsWithIC(std::string const& text, std::string const& prefix, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckStartsWithIC(std::wstring const& text, std::wstring const& prefix, std::string const& method,
        int line, std::string const& msg);
    void CheckStartsWithIC(char const* text, char const* prefix, std::string const& method, int line,
        std::string const& msg);
    void CheckStartsWithIC(wchar_t const* text, wchar_t const* prefix, std::string const& method, int line,
        std::string const& msg);

protected: // Assertion/Check methods - Matches (regular expression)
    // Each checks whether the whole of 'text' matches the ECMAScript regular expression 'pattern' (std::regex_match;
    // start or end a pattern with ".*" to match part of the text). Wide text is matched with std::wregex. An invalid
    // pattern fails the check, showing why, rather than throwing std::regex_error. A null C string, as the text or
    // the pattern, matches nothing: the Matches methods fail, showing it as "(null)", and the NotMatches ones pass.
    virtual void AssertMatches(std::string const& text, std::string const& pattern, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertMatches(std::wstring const& text, std::wstring const& pattern, std::string const& method,
        int line, std::string const& msg);
    void AssertMatches(char const* text, char const* pattern, std::string const& method, int line,
        std::string const& msg);
    void AssertMatches(wchar_t const* text, wchar_t const* pattern, std::string const& method, int line,
        std::string const& msg);

    virtual void AssertNotMatches(std::string const& text, std::string const& pattern, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotMatches(std::wstring const& text, std::wstring const& pattern, std::string const& method,
        int line, std::string const& msg);
    void AssertNotMatches(char const* text, char const* pattern, std::string const& method, int line,
        std::string const& msg);
    void AssertNotMatches(wchar_t const* text, wchar_t const* pattern, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckMatches(std::string const& text, std::string const& pattern, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckMatches(std::wstring const& text, std::wstring const& pattern, std::string const& method,
        int line, std::string const& msg);
    void CheckMatches(char const* text, char const* pattern, std::string const& method, int line,
        std::string const& msg);
    void CheckMatches(wchar_t const* text, wchar_t const* pattern, std::string const& method, int line,
        std::string const& msg);

    virtual void CheckNotMatches(std::string const& text, std::string const& pattern, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotMatches(std::wstring const& text, std::wstring const& pattern, std::string const& method,
        int line, std::string const& msg);
    void CheckNotMatches(char const* text, char const* pattern, std::string const& method, int line,
        std::string const& msg);
    void CheckNotMatches(wchar_t const* text, wchar_t const* pattern, std::string const& method, int line,
        std::string const& msg);

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

protected: // Assertion/Check methods - Memory (bytes)
    // Each compares the 'size' bytes at 'expected' with those at 'actual'. A failure shows the offset of the first
    // differing byte and up to 16 bytes from there, in hex. Zero bytes, or the same pointer twice (even null), are
    // equal; exactly one null pointer isn't equal to anything, and neither is read.
    virtual void AssertEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& method,
        int line, std::string const& msg);
    virtual void AssertNotEqualsMem(void const* expected, void const* actual, std::size_t size,
        std::string const& method, int line, std::string const& msg);
    virtual void CheckEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& method,
        int line, std::string const& msg);
    virtual void CheckNotEqualsMem(void const* expected, void const* actual, std::size_t size,
        std::string const& method, int line, std::string const& msg);

protected: // Assertion/Check methods - Mixed string types
    // The Equals, Contains, StartsWith/EndsWith and Matches methods, IC and Not forms included, for two texts that
    // the overloads above don't take as they are: a std::string with a C string, narrow text with wide, or an RTL
    // string (System::String, AnsiString, UTF8String or another AnsiStringT, in RTL builds) with any text. Both are
    // converted to UTF-8 and compared as two std::string values, except that two wide texts are compared as two
    // std::wstring values, and the Matches methods match any texts but two narrow ones as wide text (std::wregex).
    // A null C string follows the same rules as in the C string overloads, whatever it's compared with.
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    template <typename TText>
    using TIsRTLText = std::integral_constant<bool, std::is_same<TText, System::String>::value ||
        TIsAnsiStringT<TText>::value>;
#else
    template <typename TText>
    using TIsRTLText = std::false_type;
#endif
    // The kind of text TText is (see ETextKind). An array is a C string.
    template <typename TText>
    static constexpr ETextKind TextKindOf()
    {
        using TDecayed = typename std::decay<TText>::type;

        if constexpr (std::is_same<TDecayed, std::string>::value)
            return ETextKind::NarrowString;
        else if constexpr (std::is_same<TDecayed, char const*>::value || std::is_same<TDecayed, char*>::value)
            return ETextKind::NarrowCString;
        else if constexpr (std::is_same<TDecayed, std::wstring>::value)
            return ETextKind::WideString;
        else if constexpr (std::is_same<TDecayed, wchar_t const*>::value || std::is_same<TDecayed, wchar_t*>::value)
            return ETextKind::WideCString;
        else if constexpr (TIsRTLText<TDecayed>::value)
            return ETextKind::RTL;
        else
            return ETextKind::None;
    }
    template <typename TText>
    static constexpr bool IsNarrowText()
    {
        return TextKindOf<TText>() == ETextKind::NarrowString || TextKindOf<TText>() == ETextKind::NarrowCString;
    }
    template <typename TText>
    static constexpr bool IsWideText()
    {
        return TextKindOf<TText>() == ETextKind::WideString || TextKindOf<TText>() == ETextKind::WideCString;
    }
    // A character array, such as a literal, with a std::string or std::wstring of its width, which converts to it and
    // goes to its overload as before: an array is never null, so it needs nothing these templates add, and leaving it
    // out saves an instantiation for each length of literal.
    template <typename TArray, typename TString>
    static constexpr bool IsArrayWithItsString()
    {
        constexpr bool isArray = std::is_array<TArray>::value;
        constexpr ETextKind arrayKind = TextKindOf<TArray>();
        constexpr ETextKind stringKind = TextKindOf<TString>();

        return isArray && ((arrayKind == ETextKind::NarrowCString && stringKind == ETextKind::NarrowString) ||
            (arrayKind == ETextKind::WideCString && stringKind == ETextKind::WideString));
    }
    // Two texts of different kinds, or an RTL string with any text, but not an array with its string (see
    // IsArrayWithItsString()).
    template <typename TFirst, typename TSecond>
    static constexpr bool IsMixedText()
    {
        constexpr ETextKind firstKind = TextKindOf<TFirst>();
        constexpr ETextKind secondKind = TextKindOf<TSecond>();
        constexpr bool bothText = firstKind != ETextKind::None && secondKind != ETextKind::None;
        constexpr bool mixed = firstKind != secondKind || firstKind == ETextKind::RTL;

        return bothText && mixed && !IsArrayWithItsString<TFirst, TSecond>() &&
            !IsArrayWithItsString<TSecond, TFirst>();
    }
    template <typename TFirst, typename TSecond>
    using TEnableIfMixedText = typename std::enable_if<IsMixedText<TFirst, TSecond>(), int>::type;

    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void AssertContains(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::AssertContains, &TTestGroupBase::AssertContains, method,
                line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::AssertContains, &TTestGroupBase::AssertContains, method,
                line, msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void AssertContainsIC(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::AssertContainsIC, &TTestGroupBase::AssertContainsIC, method,
                line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::AssertContainsIC, &TTestGroupBase::AssertContainsIC, method,
                line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void AssertEndsWith(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::AssertEndsWith, &TTestGroupBase::AssertEndsWith, method, line,
                msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::AssertEndsWith, &TTestGroupBase::AssertEndsWith, method, line,
                msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void AssertEndsWithIC(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::AssertEndsWithIC, &TTestGroupBase::AssertEndsWithIC, method,
                line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::AssertEndsWithIC, &TTestGroupBase::AssertEndsWithIC, method,
                line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void AssertEquals(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::AssertEquals, &TTestGroupBase::AssertEquals, method, line,
                msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::AssertEquals, &TTestGroupBase::AssertEquals, method, line,
                msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void AssertEqualsIC(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::AssertEqualsIC, &TTestGroupBase::AssertEqualsIC, method,
                line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::AssertEqualsIC, &TTestGroupBase::AssertEqualsIC, method,
                line, msg);
    }
    template <typename TText, typename TPattern, TEnableIfMixedText<TText, TPattern> = 0>
    void AssertMatches(TText const& text, TPattern const& pattern, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsNarrowText<TText>() && IsNarrowText<TPattern>())
            ForwardAsUTF8(text, pattern, &TTestGroupBase::AssertMatches, &TTestGroupBase::AssertMatches, method, line,
                msg);
        else
            ForwardAsWide(text, pattern, &TTestGroupBase::AssertMatches, &TTestGroupBase::AssertMatches, method, line,
                msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void AssertNotContains(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::AssertNotContains, &TTestGroupBase::AssertNotContains,
                method, line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::AssertNotContains, &TTestGroupBase::AssertNotContains,
                method, line, msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void AssertNotContainsIC(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::AssertNotContainsIC, &TTestGroupBase::AssertNotContainsIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::AssertNotContainsIC, &TTestGroupBase::AssertNotContainsIC,
                method, line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void AssertNotEndsWith(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::AssertNotEndsWith, &TTestGroupBase::AssertNotEndsWith, method,
                line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::AssertNotEndsWith, &TTestGroupBase::AssertNotEndsWith, method,
                line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void AssertNotEndsWithIC(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::AssertNotEndsWithIC, &TTestGroupBase::AssertNotEndsWithIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::AssertNotEndsWithIC, &TTestGroupBase::AssertNotEndsWithIC,
                method, line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void AssertNotEquals(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::AssertNotEquals, &TTestGroupBase::AssertNotEquals, method,
                line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::AssertNotEquals, &TTestGroupBase::AssertNotEquals, method,
                line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void AssertNotEqualsIC(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::AssertNotEqualsIC, &TTestGroupBase::AssertNotEqualsIC,
                method, line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::AssertNotEqualsIC, &TTestGroupBase::AssertNotEqualsIC,
                method, line, msg);
    }
    template <typename TText, typename TPattern, TEnableIfMixedText<TText, TPattern> = 0>
    void AssertNotMatches(TText const& text, TPattern const& pattern, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsNarrowText<TText>() && IsNarrowText<TPattern>())
            ForwardAsUTF8(text, pattern, &TTestGroupBase::AssertNotMatches, &TTestGroupBase::AssertNotMatches, method,
                line, msg);
        else
            ForwardAsWide(text, pattern, &TTestGroupBase::AssertNotMatches, &TTestGroupBase::AssertNotMatches, method,
                line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void AssertNotStartsWith(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::AssertNotStartsWith, &TTestGroupBase::AssertNotStartsWith,
                method, line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::AssertNotStartsWith, &TTestGroupBase::AssertNotStartsWith,
                method, line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void AssertNotStartsWithIC(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::AssertNotStartsWithIC, &TTestGroupBase::AssertNotStartsWithIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::AssertNotStartsWithIC, &TTestGroupBase::AssertNotStartsWithIC,
                method, line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void AssertStartsWith(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::AssertStartsWith, &TTestGroupBase::AssertStartsWith, method,
                line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::AssertStartsWith, &TTestGroupBase::AssertStartsWith, method,
                line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void AssertStartsWithIC(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::AssertStartsWithIC, &TTestGroupBase::AssertStartsWithIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::AssertStartsWithIC, &TTestGroupBase::AssertStartsWithIC,
                method, line, msg);
    }

    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void CheckContains(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::CheckContains, &TTestGroupBase::CheckContains, method, line,
                msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::CheckContains, &TTestGroupBase::CheckContains, method, line,
                msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void CheckContainsIC(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::CheckContainsIC, &TTestGroupBase::CheckContainsIC, method,
                line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::CheckContainsIC, &TTestGroupBase::CheckContainsIC, method,
                line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void CheckEndsWith(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::CheckEndsWith, &TTestGroupBase::CheckEndsWith, method, line,
                msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::CheckEndsWith, &TTestGroupBase::CheckEndsWith, method, line,
                msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void CheckEndsWithIC(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::CheckEndsWithIC, &TTestGroupBase::CheckEndsWithIC, method,
                line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::CheckEndsWithIC, &TTestGroupBase::CheckEndsWithIC, method,
                line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void CheckEquals(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::CheckEquals, &TTestGroupBase::CheckEquals, method, line,
                msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::CheckEquals, &TTestGroupBase::CheckEquals, method, line,
                msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void CheckEqualsIC(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::CheckEqualsIC, &TTestGroupBase::CheckEqualsIC, method,
                line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::CheckEqualsIC, &TTestGroupBase::CheckEqualsIC, method,
                line, msg);
    }
    template <typename TText, typename TPattern, TEnableIfMixedText<TText, TPattern> = 0>
    void CheckMatches(TText const& text, TPattern const& pattern, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsNarrowText<TText>() && IsNarrowText<TPattern>())
            ForwardAsUTF8(text, pattern, &TTestGroupBase::CheckMatches, &TTestGroupBase::CheckMatches, method, line,
                msg);
        else
            ForwardAsWide(text, pattern, &TTestGroupBase::CheckMatches, &TTestGroupBase::CheckMatches, method, line,
                msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void CheckNotContains(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::CheckNotContains, &TTestGroupBase::CheckNotContains, method,
                line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::CheckNotContains, &TTestGroupBase::CheckNotContains, method,
                line, msg);
    }
    template <typename TText, typename TSubstring, TEnableIfMixedText<TText, TSubstring> = 0>
    void CheckNotContainsIC(TText const& text, TSubstring const& substring, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSubstring>())
            ForwardAsWide(text, substring, &TTestGroupBase::CheckNotContainsIC, &TTestGroupBase::CheckNotContainsIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, substring, &TTestGroupBase::CheckNotContainsIC, &TTestGroupBase::CheckNotContainsIC,
                method, line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void CheckNotEndsWith(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::CheckNotEndsWith, &TTestGroupBase::CheckNotEndsWith, method,
                line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::CheckNotEndsWith, &TTestGroupBase::CheckNotEndsWith, method,
                line, msg);
    }
    template <typename TText, typename TSuffix, TEnableIfMixedText<TText, TSuffix> = 0>
    void CheckNotEndsWithIC(TText const& text, TSuffix const& suffix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TSuffix>())
            ForwardAsWide(text, suffix, &TTestGroupBase::CheckNotEndsWithIC, &TTestGroupBase::CheckNotEndsWithIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, suffix, &TTestGroupBase::CheckNotEndsWithIC, &TTestGroupBase::CheckNotEndsWithIC,
                method, line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void CheckNotEquals(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::CheckNotEquals, &TTestGroupBase::CheckNotEquals, method,
                line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::CheckNotEquals, &TTestGroupBase::CheckNotEquals, method,
                line, msg);
    }
    template <typename TExpected, typename TActual, TEnableIfMixedText<TExpected, TActual> = 0>
    void CheckNotEqualsIC(TExpected const& expected, TActual const& actual, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TExpected>() && IsWideText<TActual>())
            ForwardAsWide(expected, actual, &TTestGroupBase::CheckNotEqualsIC, &TTestGroupBase::CheckNotEqualsIC,
                method, line, msg);
        else
            ForwardAsUTF8(expected, actual, &TTestGroupBase::CheckNotEqualsIC, &TTestGroupBase::CheckNotEqualsIC,
                method, line, msg);
    }
    template <typename TText, typename TPattern, TEnableIfMixedText<TText, TPattern> = 0>
    void CheckNotMatches(TText const& text, TPattern const& pattern, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsNarrowText<TText>() && IsNarrowText<TPattern>())
            ForwardAsUTF8(text, pattern, &TTestGroupBase::CheckNotMatches, &TTestGroupBase::CheckNotMatches, method,
                line, msg);
        else
            ForwardAsWide(text, pattern, &TTestGroupBase::CheckNotMatches, &TTestGroupBase::CheckNotMatches, method,
                line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void CheckNotStartsWith(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::CheckNotStartsWith, &TTestGroupBase::CheckNotStartsWith,
                method, line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::CheckNotStartsWith, &TTestGroupBase::CheckNotStartsWith,
                method, line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void CheckNotStartsWithIC(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::CheckNotStartsWithIC, &TTestGroupBase::CheckNotStartsWithIC,
                method, line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::CheckNotStartsWithIC, &TTestGroupBase::CheckNotStartsWithIC,
                method, line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void CheckStartsWith(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::CheckStartsWith, &TTestGroupBase::CheckStartsWith, method,
                line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::CheckStartsWith, &TTestGroupBase::CheckStartsWith, method,
                line, msg);
    }
    template <typename TText, typename TPrefix, TEnableIfMixedText<TText, TPrefix> = 0>
    void CheckStartsWithIC(TText const& text, TPrefix const& prefix, std::string const& method, int line,
        std::string const& msg)
    {
        if constexpr (IsWideText<TText>() && IsWideText<TPrefix>())
            ForwardAsWide(text, prefix, &TTestGroupBase::CheckStartsWithIC, &TTestGroupBase::CheckStartsWithIC, method,
                line, msg);
        else
            ForwardAsUTF8(text, prefix, &TTestGroupBase::CheckStartsWithIC, &TTestGroupBase::CheckStartsWithIC, method,
                line, msg);
    }

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
protected: // Assertion/Check methods - System::String (RTL)
    // The Empty methods for an RTL string: a System::String, or an AnsiStringT such as AnsiString or UTF8String.
    // Exactly those, so nothing else converts to one implicitly (e.g. a C string, only in RTL builds).
    template <typename TText>
    using TEnableIfRTLString = typename std::enable_if<TIsRTLText<typename std::decay<TText>::type>::value,
        int>::type;

    template <typename TText, TEnableIfRTLString<TText> = 0>
    void AssertEmpty(TText const& text, std::string const& method, int line, std::string const& msg)
    {
        AssertEmpty(TextToUTF8(text), method, line, msg);
    }
    template <typename TText, TEnableIfRTLString<TText> = 0>
    void AssertNotEmpty(TText const& text, std::string const& method, int line, std::string const& msg)
    {
        AssertNotEmpty(TextToUTF8(text), method, line, msg);
    }

    template <typename TText, TEnableIfRTLString<TText> = 0>
    void CheckEmpty(TText const& text, std::string const& method, int line, std::string const& msg)
    {
        CheckEmpty(TextToUTF8(text), method, line, msg);
    }
    template <typename TText, TEnableIfRTLString<TText> = 0>
    void CheckNotEmpty(TText const& text, std::string const& method, int line, std::string const& msg)
    {
        CheckNotEmpty(TextToUTF8(text), method, line, msg);
    }
#endif // #if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

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
    template <typename TValue>
    void AssertEmpty(TValue&& value, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        AssertEmpty(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    void AssertEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertEqualsMem(expected, actual, size, loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TType, typename TValue>
    void AssertIsNotType(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertIsNotType<TType>(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TType, typename TValue>
    void AssertIsType(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertIsType<TType>(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TText, typename TPattern>
    void AssertMatches(TText&& text, TPattern&& pattern, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertMatches(std::forward<TText>(text), std::forward<TPattern>(pattern), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void AssertNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TCallable>
    void AssertNoThrow(TCallable&& callable, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNoThrow(std::forward<TCallable>(callable), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TValue>
    void AssertNotEmpty(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEmpty(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    void AssertNotEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotEqualsMem(expected, actual, size, loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPattern>
    void AssertNotMatches(TText&& text, TPattern&& pattern, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotMatches(std::forward<TText>(text), std::forward<TPattern>(pattern), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void AssertNotNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TValue>
    void AssertNotNull(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotNull(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertNotSame(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertNotSame(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
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
    template <typename TValue>
    void AssertNull(TValue&& value, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        AssertNull(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void AssertSame(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        AssertSame(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
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
    template <typename TException, typename TCallable>
    void AssertThrows(TCallable&& callable, std::string const& msg, std::string const& expectedMessage = std::string(),
        std::source_location loc = std::source_location::current())
    {
        AssertThrows<TException>(std::forward<TCallable>(callable), loc.function_name(), static_cast<int>(loc.line()),
            msg, expectedMessage);
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
    template <typename TValue>
    void CheckEmpty(TValue&& value, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckEmpty(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    void CheckEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckEqualsMem(expected, actual, size, loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TType, typename TValue>
    void CheckIsNotType(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckIsNotType<TType>(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TType, typename TValue>
    void CheckIsType(TValue&& value, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckIsType<TType>(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TText, typename TPattern>
    void CheckMatches(TText&& text, TPattern&& pattern, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckMatches(std::forward<TText>(text), std::forward<TPattern>(pattern), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void CheckNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TCallable>
    void CheckNoThrow(TCallable&& callable, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNoThrow(std::forward<TCallable>(callable), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    template <typename TValue>
    void CheckNotEmpty(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEmpty(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
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
    void CheckNotEqualsMem(void const* expected, void const* actual, std::size_t size, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotEqualsMem(expected, actual, size, loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TText, typename TPattern>
    void CheckNotMatches(TText&& text, TPattern&& pattern, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotMatches(std::forward<TText>(text), std::forward<TPattern>(pattern), loc.function_name(),
            static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual, typename TTolerance>
    void CheckNotNear(TExpected&& expected, TActual&& actual, TTolerance&& tolerance, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotNear(std::forward<TExpected>(expected), std::forward<TActual>(actual),
            std::forward<TTolerance>(tolerance), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TValue>
    void CheckNotNull(TValue&& value, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotNull(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckNotSame(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckNotSame(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
            static_cast<int>(loc.line()), msg);
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
    template <typename TValue>
    void CheckNull(TValue&& value, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckNull(std::forward<TValue>(value), loc.function_name(), static_cast<int>(loc.line()), msg);
    }
    template <typename TExpected, typename TActual>
    void CheckSame(TExpected&& expected, TActual&& actual, std::string const& msg,
        std::source_location loc = std::source_location::current())
    {
        CheckSame(std::forward<TExpected>(expected), std::forward<TActual>(actual), loc.function_name(),
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
    template <typename TException, typename TCallable>
    void CheckThrows(TCallable&& callable, std::string const& msg, std::string const& expectedMessage = std::string(),
        std::source_location loc = std::source_location::current())
    {
        CheckThrows<TException>(std::forward<TCallable>(callable), loc.function_name(), static_cast<int>(loc.line()),
            msg, expectedMessage);
    }
    void CheckTrue(bool testVal, std::string const& msg, std::source_location loc = std::source_location::current())
    {
        CheckTrue(testVal, loc.function_name(), static_cast<int>(loc.line()), msg);
    }

    void Fail(std::string const& msg, std::source_location loc = std::source_location::current())
    {
        Fail(loc.function_name(), static_cast<int>(loc.line()), msg);
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
    // For a group in a suite, e.g. inherited(TSuitePath("Framework.Console"), "TTest_TConsole").
    TTestGroupBase(TSuitePath const& suitePath, std::string const& name);

    std::string const& GetSuitePath() const override;
    TestCallbackList& GetTestCallbackList() override;
    std::string const& GetTestGroupName() const override;
    TTestResults const& Results() const override;
    void Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed,
        std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes) override;
    virtual void SetLogSuppressed(bool suppressed);
    void SetRunObserver(ITestRunObserver* observer) override;
};


/////////////////////////////////////////////////////////////////////////////
// TAutoTestGroup
//
// A TTestGroupBase whose test methods register themselves where they're
// defined, with the ASW_TEST macro (see ASWUnitTests_Registry.h), so a test
// can't be written but left out of the constructor. To use ASW_TEST, derive
// the group from TAutoTestGroup<itself>, and declare its Test_ methods
// public, since ASW_TEST takes their addresses from outside the class:
//
//     class TTest_TMyClass : public TAutoTestGroup<TTest_TMyClass>
//
// Each ASW_TEST adds its method to this class's list during static
// initialization. This constructor registers that list's tests in
// alphabetical order by name, before the derived constructor runs, so any
// tests the derived constructor registers itself (with RegisterTest() or
// RegisterTestCases()) follow them, in the order it registers them. A name
// registered twice throws TExceptDuplicateTestName.
//
// The ASW_TEST tests are registered with TTestGroupBase::RegisterTest(),
// since a virtual call from this constructor can't reach a derived class's
// override of it.
/////////////////////////////////////////////////////////////////////////////
template <typename TDerived>
class TAutoTestGroup : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

public:
    typedef void (TDerived::*TestMethod)();

private:
    struct TAutoTest
    {
        std::string Name;
        TestMethod Method;
    };

    // Constructed on first use, so an ASW_TEST in any translation unit can add to it during static initialization,
    // whatever order those units are initialized in.
    static std::vector<TAutoTest>& AutoTests()
    {
        static std::vector<TAutoTest> tests;
        return tests;
    }

protected:
    explicit TAutoTestGroup(std::string const& name)
        : TAutoTestGroup(TSuitePath(std::string()), name)
    {
    }

    // For a group in a suite; see TTestGroupBase's matching constructor.
    TAutoTestGroup(TSuitePath const& suitePath, std::string const& name)
        : inherited(suitePath, name)
    {
        std::vector<TAutoTest> tests = AutoTests();

        std::stable_sort(tests.begin(), tests.end(), [](TAutoTest const& a, TAutoTest const& b)
            {
                return a.Name < b.Name;
            });

        for (TAutoTest const& test : tests)
            RegisterTest(test.Method, test.Name);
    }

public:
    // Called by ASW_TEST. Returns a value for the static variable that ASW_TEST initializes with it.
    static int AddAutoTest(TestMethod method, char const* name)
    {
        static_assert(std::is_base_of<TAutoTestGroup<TDerived>, TDerived>::value,
            "ASW_TEST requires its class to derive from TAutoTestGroup<that class>");

        AutoTests().push_back(TAutoTest{ name, method });
        return 0;
    }
};

} // namespace ASWUnitTests

#endif // ASWUnitTests_TestBaseH
