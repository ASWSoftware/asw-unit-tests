/* **************************************************************************
ASWUnitTests_TestBase.h
Author: Anthony S. West - ASW Software

A simple unit testing framework.

To register a test module, create a class that inherits 'TTestGroupBase'
and self-register it with the ASW_REGISTER_TEST_GROUP macro
(see ASWUnitTests_Registry.h). No framework source file needs to change.

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

#ifndef ASWUnitTests_TestBaseH
#define ASWUnitTests_TestBaseH
//---------------------------------------------------------------------------
#include <cstdint>
#include <exception>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
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
    unsigned int FailedCount;
    unsigned int SkippedCount;
    unsigned int SuccessCount;
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
    virtual void Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed) = 0;
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

protected:
    bool m_ExceptionExpected;
    bool m_TestFailedCheck;
    std::string m_ExceptionExpectedText;
    std::string m_ExpectedExceptionMessage;
    std::function<bool (std::exception const&)> m_ExpectedExceptionTypeChecker;
    std::string m_Name;
    TTestResults m_Results;
    TestCallbackList m_TestCallbacks;

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
    virtual void ResetTestFailedOneOrMoreChecks();
    virtual void SetExceptionExpected(bool expected, std::string const& method, int line, std::string const& msg);
    // Expects a specific exception type (matched polymorphically, so a base class also matches its subclasses).
    // When 'expectedMessage' is non-empty, the caught exception's what() must also contain it as a substring.
    template <typename TException>
    void SetExceptionExpected(std::string const& method, int line, std::string const& msg,
        std::string const& expectedMessage = std::string())
    {
        SetExceptionExpected(true, method, line, msg);
        m_ExpectedExceptionMessage = expectedMessage;
        m_ExpectedExceptionTypeChecker = [](std::exception const& ex)
            {
                return dynamic_cast<TException const*>(&ex) != nullptr;
            };
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

public:
    TTestGroupBase(std::string const& name);

    TestCallbackList& GetTestCallbackList() override;
    std::string const& GetTestGroupName() const override;
    TTestResults const& Results() const override;
    void Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed) override;
};

} // namespace ASWUnitTests

#endif // ASWUnitTests_TestBaseH
