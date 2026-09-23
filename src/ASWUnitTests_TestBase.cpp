/* **************************************************************************
ASWUnitTests_TestBase.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTestCase
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
//TTestCase::TTestCase()
//    : inherited(),
//      m_Callback(nullptr)
//{
//}
//---------------------------------------------------------------------------
TTestCase::TTestCase(TestCallback callback, std::string const& name)
    : inherited(),
      m_Callback(callback),
      m_Name(name)
{
}
//---------------------------------------------------------------------------
void TTestCase::DoTest()
{
    if (nullptr != m_Callback)
        m_Callback();
}
//---------------------------------------------------------------------------
std::string const& TTestCase::GetName() const
{
    return m_Name;
}
//---------------------------------------------------------------------------
ITestCase::TestCallback TTestCase::GetTestCallback() const
{
    return m_Callback;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTestGroupBase
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTestGroupBase::TTestGroupBase(std::string const& name)
    : m_ExceptionExpected(false),
      m_TestFailedCheck(false),
      m_Name(name)
{
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    bool expected, bool actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
    {
        std::string expectedStr = (expected ? "true" : "false");
        std::string actualStr = (actual ? "true" : "false");
        throw TExceptEquals(method, line, expectedStr, actualStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    std::string const& expected, std::string const& actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, expected, actual, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        throw TExceptEquals(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertFalse(bool testVal, std::string const& method, int line, std::string const& msg)
{
    if (testVal)
        throw TExceptFalse(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNear(
    float expected, float actual, float tolerance, std::string const& method, int line, std::string const& msg)
{
    float const diff = std::fabs(expected - actual);

    if (diff > tolerance)
    {
        std::string expectedStr = std::to_string(expected) + " (tolerance " + std::to_string(tolerance) + ")";
        std::string actualStr = std::to_string(actual) + " (diff " + std::to_string(diff) + ")";
        throw TExceptEquals(method, line, expectedStr, actualStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNear(
    double expected, double actual, double tolerance, std::string const& method, int line, std::string const& msg)
{
    double const diff = std::fabs(expected - actual);

    if (diff > tolerance)
    {
        std::string expectedStr = std::to_string(expected) + " (tolerance " + std::to_string(tolerance) + ")";
        std::string actualStr = std::to_string(actual) + " (diff " + std::to_string(diff) + ")";
        throw TExceptEquals(method, line, expectedStr, actualStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    bool expected, bool actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
    {
        std::string valueStr = (expected ? "true" : "false");
        throw TExceptNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    std::string const& expected, std::string const& actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        throw TExceptNotEquals(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotNear(
    float expected, float actual, float tolerance, std::string const& method, int line, std::string const& msg)
{
    float const diff = std::fabs(expected - actual);

    if (diff <= tolerance)
    {
        std::string valueStr = std::to_string(actual) + " (diff " + std::to_string(diff) + " <= tolerance " +
            std::to_string(tolerance) + ")";
        throw TExceptNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotNear(
    double expected, double actual, double tolerance, std::string const& method, int line, std::string const& msg)
{
    double const diff = std::fabs(expected - actual);

    if (diff <= tolerance)
    {
        std::string valueStr = std::to_string(actual) + " (diff " + std::to_string(diff) + " <= tolerance " +
            std::to_string(tolerance) + ")";
        throw TExceptNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertTrue(bool testVal, std::string const& method, int line, std::string const& msg)
{
    if (!testVal)
        throw TExceptFalse(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    bool expected, bool actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, std::to_string(expected), std::to_string(actual), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    std::string const& expected, std::string const& actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
        SetTestFailedCheck(method, line, expected, actual, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
    {
        std::string expectedMsg = "Expected same values: \"" + msg + "\"";
        SetTestFailedCheck(method, line, expectedMsg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckFalse(bool testVal, std::string const& method, int line, std::string const& msg)
{
    if (testVal)
    {
        std::string expectedMsg = "Expected true but was false: \"" + msg + "\"";
        SetTestFailedCheck(method, line, expectedMsg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNear(
    float expected, float actual, float tolerance, std::string const& method, int line, std::string const& msg)
{
    float const diff = std::fabs(expected - actual);

    if (diff > tolerance)
    {
        std::string expectedStr = std::to_string(expected) + " (tolerance " + std::to_string(tolerance) + ")";
        std::string actualStr = std::to_string(actual) + " (diff " + std::to_string(diff) + ")";
        SetTestFailedCheck(method, line, expectedStr, actualStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNear(
    double expected, double actual, double tolerance, std::string const& method, int line, std::string const& msg)
{
    double const diff = std::fabs(expected - actual);

    if (diff > tolerance)
    {
        std::string expectedStr = std::to_string(expected) + " (tolerance " + std::to_string(tolerance) + ")";
        std::string actualStr = std::to_string(actual) + " (diff " + std::to_string(diff) + ")";
        SetTestFailedCheck(method, line, expectedStr, actualStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    bool expected, bool actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
    {
        std::string valueStr = (expected ? "true" : "false");
        SetTestFailedCheckNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    int64_t expected, int64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    uint64_t expected, uint64_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    int32_t expected, int32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    uint32_t expected, uint32_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    int16_t expected, int16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    uint16_t expected, uint16_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    int8_t expected, int8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    uint8_t expected, uint8_t actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, std::to_string(expected), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    std::string const& expected, std::string const& actual, std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (expected == actual)
        SetTestFailedCheckNotEquals(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotNear(
    float expected, float actual, float tolerance, std::string const& method, int line, std::string const& msg)
{
    float const diff = std::fabs(expected - actual);

    if (diff <= tolerance)
    {
        std::string valueStr = std::to_string(actual) + " (diff " + std::to_string(diff) + " <= tolerance " +
            std::to_string(tolerance) + ")";
        SetTestFailedCheckNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotNear(
    double expected, double actual, double tolerance, std::string const& method, int line, std::string const& msg)
{
    double const diff = std::fabs(expected - actual);

    if (diff <= tolerance)
    {
        std::string valueStr = std::to_string(actual) + " (diff " + std::to_string(diff) + " <= tolerance " +
            std::to_string(tolerance) + ")";
        SetTestFailedCheckNotEquals(method, line, valueStr, msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckTrue(bool testVal, std::string const& method, int line, std::string const& msg)
{
    if (!testVal)
    {
        std::string expectedMsg = "Expected false but was true: \"" + msg + "\"";
        SetTestFailedCheck(method, line, expectedMsg);
    }
}
//---------------------------------------------------------------------------
TTestGroupBase::TestCallbackList& TTestGroupBase::GetTestCallbackList()
{
    return m_TestCallbacks;
}
//---------------------------------------------------------------------------
std::string const& TTestGroupBase::GetTestGroupName() const
{
    return m_Name;
}
//---------------------------------------------------------------------------
void TTestGroupBase::Log(std::string const& msg)
{
    std::cout << msg << std::endl;
}
//---------------------------------------------------------------------------
void TTestGroupBase::LogAppend(std::string const& msg)
{
    std::cout << msg;
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::RegisterTest

    Developer: Call this method from the child of 'TTestGroupBase', in the constructor.
*/
void TTestGroupBase::RegisterTest(TTestCase const& testCase)
{
    m_TestCallbacks.push_back(std::make_unique<TTestCase>(testCase));
}
//---------------------------------------------------------------------------
void TTestGroupBase::RegisterTest(ITestCase::TestCallback callback, std::string const& testName)
{
    TTestCase testCase(callback, testName);
    RegisterTest(testCase);
}
//---------------------------------------------------------------------------
void TTestGroupBase::ResetTestFailedOneOrMoreChecks()
{
    m_TestFailedCheck = false;
}
//---------------------------------------------------------------------------
TTestResults const& TTestGroupBase::Results() const
{
    return m_Results;
}
//---------------------------------------------------------------------------
void TTestGroupBase::Run(TestFilter const& filter)
{
    //Test(std::bind(&TTestGroup_ASWTools_Version_Tests::Test_SetVersion, this, std::placeholders::_1));

    for (TestCallbackList::iterator it = m_TestCallbacks.begin(); it != m_TestCallbacks.end(); it++)
    {
        ITestCase& testCase = *it->get();

        if (filter != nullptr && !filter(m_Name + "." + testCase.GetName()))
            continue;

        Test(testCase);
    }
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::SetExceptionExpected

    Call with a value of true in a test where an exception is expected.
*/
void TTestGroupBase::SetExceptionExpected(bool expected, std::string const& method, int line, std::string const& msg)
{
    m_ExceptionExpected = expected;
    m_ExceptionExpectedText = method + " (" + std::to_string(line) + "): " + msg;
    m_ExpectedExceptionMessage.clear();
    m_ExpectedExceptionTypeChecker = nullptr;
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetTestFailedCheck(std::string const& method, int line, std::string const& msg)
{
    m_TestFailedCheck = true;

    std::string finalMsg = "  **Check failed for: \"" + method + "\" (" + std::to_string(line) + "): " + msg;
    m_Results.Messages.push_back(finalMsg);
    Log(finalMsg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetTestFailedCheck(std::string const& method, int line, std::string const& expected,
    std::string const& actual, std::string const& msg)
{
    std::string expectedMsg = "Expected \"" + expected + "\" but was \"" + actual + "\". " + msg;
    SetTestFailedCheck(method, line, expectedMsg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetTestFailedCheckNotEquals(std::string const& method, int line, std::string const& msg)
{
    std::string expectedMsg = "Both values are equal. " + msg;
    SetTestFailedCheck(method, line, expectedMsg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetTestFailedCheckNotEquals(
    std::string const& method, int line, std::string const& value, std::string const& msg)
{
    std::string expectedMsg = "Both values equal: \"" + value + "\". " + msg;
    SetTestFailedCheck(method, line, expectedMsg);
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::SetUp_Test

    Called just before calling the test callback
*/
void TTestGroupBase::SetUp_Test(ITestCase& /*testCase*/)
{
    // The child group can optionally override this method
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::Skip

    Call from within a test to abort it and have it reported as skipped.

    Can be called unconditionally to permanently skip a test without removing its RegisterTest() call,
    or after a runtime check to skip conditionally (e.g. a platform or environment-specific test).
    No explicit 'return' is needed afterward.
*/
void TTestGroupBase::Skip(std::string const& method, int line, std::string const& reason)
{
    throw TExceptSkipped(method, line, reason);
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::TearDown_Test

    Called just after calling the test callback
*/
void TTestGroupBase::TearDown_Test(ITestCase& /*testCase*/)
{
    // The child group can optionally override this method
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::Test

    Runs the test call back and sets success/error counts and messages.
*/
void TTestGroupBase::Test(ITestCase& testCase)
{
    try
    {
        // Reset for test
        SetExceptionExpected(false, "", 0, "");
        ResetTestFailedOneOrMoreChecks();

        // Run test
        if (nullptr != testCase.GetTestCallback())
        {
            std::string testFullName = m_Name + "." + testCase.GetName();

            std::string msg = "Running test: " + testFullName;
            Log(msg);

            try
            {
                SetUp_Test(testCase);
            }
            catch (...)
            {
                m_ExceptionExpected = false;
                Log("!!FATAL ERROR!!: SetUp_Test() exception for \"" + testFullName + "\"");
                throw;
            }

            try
            {
                testCase.DoTest();
            }
            catch (...)
            {
                TearDown_Test(testCase);
                throw;
            }

            try
            {
                TearDown_Test(testCase);
            }
            catch (...)
            {
                m_ExceptionExpected = false;
                Log("!!FATAL ERROR!!: TearDown_Test() exception for \"" + testFullName + "\"");
                throw;
            }
        }

        // Check for failures (for 'Exception expected' and 'Check' cases)
        if (m_ExceptionExpected)
        {
            m_ExceptionExpected = false;
            throw TExceptExpected(m_ExceptionExpectedText);
        }

        if (TestFailedOneOrMoreChecks())
        {
            m_Results.FailedCount++;
            std::string msg = "***Test failed: \"" + m_Name + "\"";
            m_Results.Messages.push_back(msg);
            Log(msg);
            return;
        }

        // Test passed
        m_Results.SuccessCount++;
    }
    catch (TExceptSkipped const& ex)
    {
        m_Results.SkippedCount++;
        std::string msg = "***Test skipped: \"" + m_Name + "\": " + ex.what();
        m_Results.Messages.push_back(msg);
        Log(msg);
    }
    catch (TTestException const& ex)
    {
        if (m_ExceptionExpected)
        {
            m_Results.SuccessCount++;
        }
        else
        {
            m_Results.FailedCount++;
            std::string msg = "***Test failed: \"" + m_Name + "\": " + ex.what();
            m_Results.Messages.push_back(msg);
            Log(msg);
        }
    }
    catch (std::exception const& ex)
    {
        if (m_ExceptionExpected)
        {
            // A type/message check is only requested via the templated SetExceptionExpected<TException>()
            // overload; the plain bool overload leaves both null/empty, matching any exception (legacy behavior).
            bool const typeMatches = (m_ExpectedExceptionTypeChecker == nullptr) || m_ExpectedExceptionTypeChecker(ex);
            bool const messageMatches = m_ExpectedExceptionMessage.empty() ||
                (std::string(ex.what()).find(m_ExpectedExceptionMessage) != std::string::npos);

            if (typeMatches && messageMatches)
            {
                m_Results.SuccessCount++;
            }
            else
            {
                m_Results.FailedCount++;

                std::string msg = "***Test failed: \"" + m_Name + "\": ";
                if (!typeMatches)
                    msg += "expected exception type was not thrown (caught a different exception): " +
                        std::string(ex.what());
                else
                    msg += "expected exception message to contain \"" + m_ExpectedExceptionMessage +
                        "\" but caught: " + std::string(ex.what());

                m_Results.Messages.push_back(msg);
                Log(msg);
            }
        }
        else
        {
            m_Results.FailedCount++;
            throw; // Unexpected failure
        }
    }
    catch (...)
    {
        if (m_ExceptionExpected)
        {
            if (m_ExpectedExceptionTypeChecker != nullptr)
            {
                // A specific type was requested, but what was thrown isn't a std::exception, so it can't be
                // inspected to confirm the type (or message) matched. Treat that as a failure, not a pass.
                m_Results.FailedCount++;

                std::string msg = "***Test failed: \"" + m_Name +
                    "\": expected a specific exception type, but a non-std::exception object was thrown instead.";
                m_Results.Messages.push_back(msg);
                Log(msg);
            }
            else
            {
                m_Results.SuccessCount++;
            }
        }
        else
        {
            m_Results.FailedCount++;
            throw; // Unexpected failure
        }
    }
}
//---------------------------------------------------------------------------
bool TTestGroupBase::TestFailedOneOrMoreChecks()
{
    return m_TestFailedCheck;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTestResults
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTestResults::TTestResults()
    : FailedCount(0),
      SkippedCount(0),
      SuccessCount(0)
{
}
//---------------------------------------------------------------------------
void TTestResults::AddMessages(MsgList const& list)
{
    if (&list == &Messages)
        return;

    for (MsgList::const_iterator it = list.begin(); it != list.end(); it++)
    {
        Messages.push_back(*it);
    }
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
