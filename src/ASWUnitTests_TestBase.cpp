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
#include <algorithm>
#include <chrono>
#include <cmath>
#include <future>
#include <iomanip>
#include <iostream>
#include <memory>
#include <random>
#include <sstream>
#include <thread>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_CrashGuard.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace
{

// Returns 'str' for display in a failure message, or "(null)" when it's nullptr.
std::string CStringDisplayText(char const* str)
{
    return (str != nullptr) ? std::string(str) : std::string("(null)");
}

// Returns 'str' as a string, or an empty one when it's nullptr.
std::string CStringOrEmpty(char const* str)
{
    return (str != nullptr) ? std::string(str) : std::string();
}

std::wstring CStringOrEmpty(wchar_t const* str)
{
    return (str != nullptr) ? std::wstring(str) : std::wstring();
}

// True if 'a' and 'b' are the same character, ignoring the case of the ASCII letters A-Z only, so the result doesn't
// depend on the platform or the current locale (and a byte of a multi-byte UTF-8 sequence is never changed).
template <typename TChar>
bool CharEqualsIgnoringASCIICase(TChar a, TChar b)
{
    auto const toLower = [](TChar character)
        {
            if (character >= 'A' && character <= 'Z')
                return static_cast<TChar>(character - 'A' + 'a');

            return character;
        };

    return toLower(a) == toLower(b);
}

// True if 'text' contains 'substring', comparing characters with CharEqualsIgnoringASCIICase().
template <typename TString>
bool ContainsIgnoringASCIICase(TString const& text, TString const& substring)
{
    // std::search finds an empty substring at the start, which is the end of an empty text.
    return substring.empty() || (std::search(text.begin(), text.end(), substring.begin(), substring.end(),
        CharEqualsIgnoringASCIICase<typename TString::value_type>) != text.end());
}

// True if 'a' and 'b' are the same text, comparing characters with CharEqualsIgnoringASCIICase().
template <typename TString>
bool EqualsIgnoringASCIICase(TString const& a, TString const& b)
{
    return a.size() == b.size() &&
        std::equal(a.begin(), a.end(), b.begin(), CharEqualsIgnoringASCIICase<typename TString::value_type>);
}

std::string FormatDurationMs(std::chrono::high_resolution_clock::time_point start)
{
    double const elapsedMs = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - start).count();

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(3) << elapsedMs << " ms";
    return oss.str();
}

} // namespace

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
      m_LogSuppressed(false),
      m_TestFailedCheck(false),
      m_Name(name),
      m_RunObserver(nullptr)
{
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertContains(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) == std::string::npos)
        throw TExceptContains(method, line, text, substring, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertContains(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) == std::wstring::npos)
        throw TExceptContains(method, line, WideToUTF8(text), WideToUTF8(substring), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertContainsIC(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (!ContainsIgnoringASCIICase(text, substring))
        throw TExceptContains(method, line, text, substring, msg, true);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertContainsIC(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (!ContainsIgnoringASCIICase(text, substring))
        throw TExceptContains(method, line, WideToUTF8(text), WideToUTF8(substring), msg, true);
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
void TTestGroupBase::AssertEquals(
    char const* expected, char const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        throw TExceptEquals(method, line, CStringDisplayText(expected), CStringDisplayText(actual), msg);

    AssertEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEquals(
    wchar_t const* expected, wchar_t const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        throw TExceptEquals(method, line, msg);

    AssertEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEqualsIC(std::string const& expected, std::string const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (!EqualsIgnoringASCIICase(expected, actual))
        throw TExceptEquals(method, line, expected, actual, msg, true);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertEqualsIC(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (!EqualsIgnoringASCIICase(expected, actual))
        throw TExceptEquals(method, line, WideToUTF8(expected), WideToUTF8(actual), msg, true);
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
void TTestGroupBase::AssertNotContains(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) != std::string::npos)
        throw TExceptNotContains(method, line, text, substring, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotContains(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) != std::wstring::npos)
        throw TExceptNotContains(method, line, WideToUTF8(text), WideToUTF8(substring), msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotContainsIC(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (ContainsIgnoringASCIICase(text, substring))
        throw TExceptNotContains(method, line, text, substring, msg, true);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotContainsIC(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (ContainsIgnoringASCIICase(text, substring))
        throw TExceptNotContains(method, line, WideToUTF8(text), WideToUTF8(substring), msg, true);
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
void TTestGroupBase::AssertNotEquals(
    char const* expected, char const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        return;

    AssertNotEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEquals(
    wchar_t const* expected, wchar_t const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        return;

    AssertNotEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEqualsIC(std::string const& expected, std::string const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (EqualsIgnoringASCIICase(expected, actual))
        throw TExceptNotEquals(method, line, expected, actual, msg, true);
}
//---------------------------------------------------------------------------
void TTestGroupBase::AssertNotEqualsIC(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (EqualsIgnoringASCIICase(expected, actual))
        throw TExceptNotEquals(method, line, WideToUTF8(expected), WideToUTF8(actual), msg, true);
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
        throw TExceptTrue(method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckContains(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) == std::string::npos)
        SetTestFailedCheck(method, line, "Expected \"" + text + "\" to contain \"" + substring + "\". " + msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckContains(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) == std::wstring::npos)
    {
        SetTestFailedCheck(method, line,
            "Expected \"" + WideToUTF8(text) + "\" to contain \"" + WideToUTF8(substring) + "\". " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckContainsIC(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (!ContainsIgnoringASCIICase(text, substring))
    {
        SetTestFailedCheck(method, line,
            "Expected \"" + text + "\" to contain \"" + substring + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckContainsIC(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (!ContainsIgnoringASCIICase(text, substring))
    {
        SetTestFailedCheck(method, line, "Expected \"" + WideToUTF8(text) + "\" to contain \"" +
            WideToUTF8(substring) + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    bool expected, bool actual, std::string const& method, int line, std::string const& msg)
{
    if (expected != actual)
    {
        std::string expectedStr = (expected ? "true" : "false");
        std::string actualStr = (actual ? "true" : "false");
        SetTestFailedCheck(method, line, expectedStr, actualStr, msg);
    }
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
void TTestGroupBase::CheckEquals(
    char const* expected, char const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
    {
        SetTestFailedCheck(method, line, CStringDisplayText(expected), CStringDisplayText(actual), msg);
        return;
    }

    CheckEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEquals(
    wchar_t const* expected, wchar_t const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
    {
        std::string expectedMsg = "Expected same values: \"" + msg + "\"";
        SetTestFailedCheck(method, line, expectedMsg);
        return;
    }

    CheckEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEqualsIC(std::string const& expected, std::string const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (!EqualsIgnoringASCIICase(expected, actual))
    {
        SetTestFailedCheck(method, line,
            "Expected \"" + expected + "\" but was \"" + actual + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckEqualsIC(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (!EqualsIgnoringASCIICase(expected, actual))
    {
        SetTestFailedCheck(method, line, "Expected \"" + WideToUTF8(expected) + "\" but was \"" +
            WideToUTF8(actual) + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckFalse(bool testVal, std::string const& method, int line, std::string const& msg)
{
    if (testVal)
    {
        std::string expectedMsg = "Expected false but was true: \"" + msg + "\"";
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
void TTestGroupBase::CheckNotContains(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) != std::string::npos)
        SetTestFailedCheck(method, line, "Expected \"" + text + "\" not to contain \"" + substring + "\". " + msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotContains(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (text.find(substring) != std::wstring::npos)
    {
        SetTestFailedCheck(method, line,
            "Expected \"" + WideToUTF8(text) + "\" not to contain \"" + WideToUTF8(substring) + "\". " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotContainsIC(std::string const& text, std::string const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (ContainsIgnoringASCIICase(text, substring))
    {
        SetTestFailedCheck(method, line,
            "Expected \"" + text + "\" not to contain \"" + substring + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotContainsIC(std::wstring const& text, std::wstring const& substring,
    std::string const& method, int line, std::string const& msg)
{
    if (ContainsIgnoringASCIICase(text, substring))
    {
        SetTestFailedCheck(method, line, "Expected \"" + WideToUTF8(text) + "\" not to contain \"" +
            WideToUTF8(substring) + "\" (ignoring case). " + msg);
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
void TTestGroupBase::CheckNotEquals(
    char const* expected, char const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        return;

    CheckNotEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEquals(
    wchar_t const* expected, wchar_t const* actual, std::string const& method, int line, std::string const& msg)
{
    if ((expected == nullptr) != (actual == nullptr))
        return;

    CheckNotEquals(CStringOrEmpty(expected), CStringOrEmpty(actual), method, line, msg);
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEqualsIC(std::string const& expected, std::string const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (EqualsIgnoringASCIICase(expected, actual))
    {
        SetTestFailedCheck(method, line,
            "Both values equal: \"" + expected + "\" and \"" + actual + "\" (ignoring case). " + msg);
    }
}
//---------------------------------------------------------------------------
void TTestGroupBase::CheckNotEqualsIC(std::wstring const& expected, std::wstring const& actual,
    std::string const& method, int line, std::string const& msg)
{
    if (EqualsIgnoringASCIICase(expected, actual))
    {
        SetTestFailedCheck(method, line, "Both values equal: \"" + WideToUTF8(expected) + "\" and \"" +
            WideToUTF8(actual) + "\" (ignoring case). " + msg);
    }
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
        std::string expectedMsg = "Expected true but was false: \"" + msg + "\"";
        SetTestFailedCheck(method, line, expectedMsg);
    }
}
//---------------------------------------------------------------------------
bool TTestGroupBase::ExceptionTypeExpected() const
{
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    if (m_ExpectedRTLExceptionTypeChecker != nullptr)
        return true;
#endif

    return m_ExpectedExceptionTypeChecker != nullptr;
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
    if (m_LogSuppressed)
        return;

    if (m_RunObserver != nullptr)
    {
        m_RunObserver->OnLog(msg + "\n");
        return;
    }

    std::cout << msg << std::endl;
}
//---------------------------------------------------------------------------
void TTestGroupBase::LogAppend(std::string const& msg)
{
    if (m_LogSuppressed)
        return;

    if (m_RunObserver != nullptr)
    {
        m_RunObserver->OnLog(msg);
        return;
    }

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
/*
    TTestGroupBase::ReportCrashedTest

    Called by RunCatchingCrashes() when TCrashGuard::Run() catches a native crash while running
    'testCase'. The synthetic failure record is added directly (not via Test()'s own finish() lambda,
    for the same reason ReportTimedOutTest() below does the same) so this group's Results() ends up
    exactly like any other completed run: safe for TTestHandler::Run() to merge and for
    --report-junit to write out.

    Throws TExceptTestCrashed only when 'abortRun' is set (see TCrashGuard::Run() for when that is);
    otherwise returns normally; RunWithTimeout()'s loop in Run() simply continues to the next test.
*/
void TTestGroupBase::ReportCrashedTest(ITestCase& testCase, std::string const& description, bool abortRun)
{
    std::string const testFullName = m_Name + "." + testCase.GetName();
    std::string const detail = "Test crashed: " + description + ".";
    std::string const tag = "***Test failed";
    std::string const plainMsg = tag + ": \"" + testFullName + "\": " + detail;

    m_Results.FailedCount++;
    m_Results.Messages.push_back(plainMsg);
    m_Results.CaseRecords.push_back(
        TTestCaseRecord{ m_Name, testCase.GetName(), 0.0, TTestOutcome::Fail, detail });

    Log(TConsole::Colorize(tag, TLogKind::Fail) + plainMsg.substr(tag.size()));

    if (abortRun)
    {
        Log("!!CRASH!!: \"" + testFullName + "\" crashed in a way that isn't safe to continue past (" +
            description + "); abandoning the remaining run.");
        throw TExceptTestCrashed("Test \"" + testFullName + "\" crashed: " + description + ".");
    }

    Log("!!CRASH!!: \"" + testFullName + "\" crashed (" + description + "); continuing with the next test.");
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::ReportTimedOutTest

    Called by RunWithTimeout() when a test's worker thread does not finish within its allotted
    timeout. The synthetic failure record is added directly (not via Test()'s own finish() lambda,
    which only that thread should touch) so this group's Results() ends up exactly like any other
    completed run: safe for TTestHandler::Run() to merge and for --report-junit to write out.
*/
void TTestGroupBase::ReportTimedOutTest(ITestCase& testCase, unsigned int testTimeoutSeconds)
{
    std::string const testFullName = m_Name + "." + testCase.GetName();
    std::string const detail = "Test exceeded its " + std::to_string(testTimeoutSeconds) +
        " second timeout and was abandoned.";
    std::string const tag = "***Test failed";
    std::string const plainMsg = tag + ": \"" + testFullName + "\": " + detail;

    m_Results.FailedCount++;
    m_Results.Messages.push_back(plainMsg);
    m_Results.CaseRecords.push_back(TTestCaseRecord{
            m_Name, testCase.GetName(), static_cast<double>(testTimeoutSeconds), TTestOutcome::Fail, detail });

    Log(TConsole::Colorize(tag, TLogKind::Fail) + plainMsg.substr(tag.size()));
    Log("!!TIMEOUT!!: \"" + testFullName + "\" exceeded " + std::to_string(testTimeoutSeconds) +
        " second(s); abandoning the remaining run.");

    throw TExceptTestTimedOut("Test \"" + testFullName + "\" timed out after " +
        std::to_string(testTimeoutSeconds) + " second(s).");
}
//---------------------------------------------------------------------------
void TTestGroupBase::ResetTestFailedOneOrMoreChecks()
{
    m_TestFailedCheck = false;
    m_CheckFailureMessages.clear();
}
//---------------------------------------------------------------------------
TTestResults const& TTestGroupBase::Results() const
{
    return m_Results;
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::Run

    Starts from empty Results(), so running the same group again (e.g. repeated runs in the VCL GUI
    runner) reports only this run's outcomes rather than adding them to the previous run's.

    'shuffleSeed', when set, runs this group's tests in a shuffled order derived from it (see
    TTestHandler::Run() for how the seed is chosen/derived); otherwise tests run in registration order.

    'testTimeoutSeconds', when set, aborts the run (see RunWithTimeout()/ReportTimedOutTest()) if any
    single test does not finish within that many seconds. 'catchCrashes' additionally protects each
    test against a native crash (see RunCatchingCrashes()/ReportCrashedTest()). Either can throw a
    TExceptAbortRun out of this method, skipping any tests after the one that triggered it.

    With an ITestRunObserver set (see SetRunObserver()), each test is bracketed by OnTestStarted()/
    OnTestFinished() here, around RunWithTimeout() rather than inside Test(), so they're called on this
    thread even when the test itself runs on a worker thread. OnTestFinished() is still called for a
    test whose outcome was recorded before an exception left this method (e.g. a timed-out test's
    synthetic failure). StopRequested() is checked before each test; if it returns true, the remaining
    tests are skipped and Results().Stopped is set.
*/
void TTestGroupBase::Run(TestFilter const& filter, std::optional<unsigned int> shuffleSeed,
    std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes)
{
    //Test(std::bind(&TTestGroup_ASWTools_Version_Tests::Test_SetVersion, this, std::placeholders::_1));

    m_Results = TTestResults();

    std::vector<size_t> order(m_TestCallbacks.size());
    for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    if (shuffleSeed.has_value())
    {
        std::mt19937 rng(*shuffleSeed);
        std::shuffle(order.begin(), order.end(), rng);
    }

    for (size_t index : order)
    {
        ITestCase& testCase = *m_TestCallbacks[index].get();

        if (filter != nullptr && !filter(m_Name + "." + testCase.GetName()))
            continue;

        if (m_RunObserver == nullptr)
        {
            RunWithTimeout(testCase, testTimeoutSeconds, catchCrashes);
            continue;
        }

        if (m_RunObserver->StopRequested())
        {
            m_Results.Stopped = true;
            break;
        }

        // Reports this test's record, if it recorded one (a test whose unexpected exception propagates doesn't).
        size_t const recordsBefore = m_Results.CaseRecords.size();
        auto notifyFinished = [&]()
            {
                if (m_Results.CaseRecords.size() > recordsBefore)
                    m_RunObserver->OnTestFinished(m_Results.CaseRecords.back());
            };

        m_RunObserver->OnTestStarted(m_Name, testCase.GetName());

        try
        {
            RunWithTimeout(testCase, testTimeoutSeconds, catchCrashes);
        }
        catch (...)
        {
            notifyFinished();
            throw;
        }

        notifyFinished();
    }
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::RunCatchingCrashes

    With 'catchCrashes' false, calls Test(testCase) directly: no guard, no overhead.

    Otherwise, runs Test(testCase) through TCrashGuard::Run() (see ASWUnitTests_CrashGuard.h). A
    normal completion, including via an ordinary C++ exception, passes through unaffected. A caught
    crash is handed to ReportCrashedTest(), which either records it as a failure and returns (the
    common case: the next test still runs) or additionally throws to abort the rest of the run, for
    the specific crash types TCrashGuard::Run() considers too severe to continue past.
*/
void TTestGroupBase::RunCatchingCrashes(ITestCase& testCase, bool catchCrashes)
{
    if (!catchCrashes)
    {
        Test(testCase);
        return;
    }

    TCrashGuardResult const result = TCrashGuard::Run([this, &testCase]()
        {
            Test(testCase);
        });

    if (result.Crashed)
        ReportCrashedTest(testCase, result.Description, result.ShouldAbortRun);
}
//---------------------------------------------------------------------------
/*
    TTestGroupBase::RunWithTimeout

    With no 'testTimeoutSeconds', calls RunCatchingCrashes(testCase, catchCrashes) directly: no
    thread, no overhead beyond whatever that call itself adds.

    Otherwise, runs it on a worker thread and waits on it with a timeout. The worker thread is the
    only one that ever touches 'testCase' or this group's members while it's running, so there's no
    data race with the waiting thread here. If it finishes in time, whatever it threw (if anything)
    is rethrown by future.get(), preserving normal exception propagation exactly as if it had run
    directly. The one exception, with ASWUNITTESTS_RTL_EXCEPTIONS enabled: an RTL exception arrives
    as a TExceptRTLException carrying its description and class (see the worker's catch). If it
    times out, the worker thread is detached (never joined: it may be stuck forever, and there is no
    safe, portable way to force a thread to unwind) and ReportTimedOutTest() records the failure and
    throws to abort the rest of the run.
*/
void TTestGroupBase::RunWithTimeout(ITestCase& testCase, std::optional<unsigned int> testTimeoutSeconds,
    bool catchCrashes)
{
    if (!testTimeoutSeconds.has_value())
    {
        RunCatchingCrashes(testCase, catchCrashes);
        return;
    }

    // Held by shared_ptr, not a plain local, because a detached thread (below) may still be running well after this
    // function has returned or thrown, and would otherwise write to a promise whose stack storage no longer exists.
    std::shared_ptr<std::promise<void> > done = std::make_shared<std::promise<void> >();
    std::future<void> future = done->get_future();

    std::thread worker([this, &testCase, done, catchCrashes]()
            {
        try
        {
            RunCatchingCrashes(testCase, catchCrashes);
            done->set_value();
        }
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        catch (System::Sysutils::Exception& ex)
        {
            // An RTL exception can't be carried by std::exception_ptr past its own handler: rethrowing it
            // later, even on the same thread, terminates the process (verified on bcc64x). Hand over a
            // TExceptRTLException instead, which keeps its description and class.
            done->set_exception(std::make_exception_ptr(TExceptRTLException(ex)));
        }
#endif
        catch (...)
        {
            done->set_exception(std::current_exception());
        }
            });

    if (future.wait_for(std::chrono::seconds(*testTimeoutSeconds)) == std::future_status::timeout)
    {
        worker.detach();
        ReportTimedOutTest(testCase, *testTimeoutSeconds);
    }

    worker.join();
    future.get();
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
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    m_ExpectedRTLExceptionTypeChecker = nullptr;
#endif
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetLogSuppressed(bool suppressed)
{
    m_LogSuppressed = suppressed;
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetRunObserver(ITestRunObserver* observer)
{
    m_RunObserver = observer;
}
//---------------------------------------------------------------------------
void TTestGroupBase::SetTestFailedCheck(std::string const& method, int line, std::string const& msg)
{
    m_TestFailedCheck = true;

    std::string const checkFailure = "Check failed for: \"" + method + "\" (" + std::to_string(line) + "): " + msg;
    m_CheckFailureMessages.push_back(checkFailure);

    std::string finalMsg = "  **" + checkFailure;
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
    std::string const testFullName = m_Name + "." + testCase.GetName();
    std::chrono::high_resolution_clock::time_point const testStart = std::chrono::high_resolution_clock::now();

    // Records the outcome, logs the plain "***Test failed"/"***Test skipped" detail line (colorized only
    // for the console, never in the stored message/record), and logs the "Finished test" timing line.
    // 'detailMessage' is the failure/skip detail text, or empty for a pass. A failed test's record also
    // gets any Check* failures it collected, ahead of 'detailMessage', since those were only logged as
    // they happened; the logged line omits them to avoid printing each one twice.
    auto finish = [&](TTestOutcome outcome, char const* status, std::string const& detailMessage)
        {
            std::string recordDetail = detailMessage;

            if (outcome == TTestOutcome::Fail && !m_CheckFailureMessages.empty())
            {
                std::string checkFailures;
                for (std::string const& checkFailure : m_CheckFailureMessages)
                    checkFailures += (checkFailures.empty() ? std::string() : std::string("\n")) + checkFailure;

                recordDetail = detailMessage.empty() ? checkFailures : (checkFailures + "\n" + detailMessage);
            }

            TLogKind const kind = (outcome == TTestOutcome::Fail) ? TLogKind::Fail :
                    (outcome == TTestOutcome::Skip) ? TLogKind::Skip : TLogKind::Pass;

            if (outcome != TTestOutcome::Pass)
            {
                std::string const tag = (outcome == TTestOutcome::Fail) ? "***Test failed" : "***Test skipped";
                std::string const plainMsg = tag + ": \"" + testFullName + "\"" +
                    (detailMessage.empty() ? std::string() : (": " + detailMessage));

                m_Results.Messages.push_back(plainMsg);
                Log(TConsole::Colorize(tag, kind) + plainMsg.substr(tag.size()));
            }

            double const durationSeconds = std::chrono::duration<double>(
                std::chrono::high_resolution_clock::now() - testStart).count();
            m_Results.CaseRecords.push_back(
                TTestCaseRecord{ m_Name, testCase.GetName(), durationSeconds, outcome, recordDetail });

            Log("Finished test: \"" + testFullName + "\" - " + TConsole::Colorize(status, kind) + " (" +
                FormatDurationMs(testStart) + ")");
        };

    // Scores an exception caught while one was expected. 'typeMatches' is whether it satisfies the requested
    // type (true when no specific type was requested), 'message' is the text checked for the expected substring,
    // and 'description' is how the exception is shown in a failure detail.
    // Records a pass, unless a Check* failure earlier in the test means it failed after all, even if everything
    // after that (including an expected exception arriving) went as intended. Every path that would otherwise
    // pass the test goes through here, so none of them can overlook an earlier Check* failure.
    auto finishPassUnlessChecksFailed = [&]()
        {
            if (TestFailedOneOrMoreChecks())
            {
                // The record gets the Check* failures themselves; see finish().
                m_Results.FailedCount++;
                finish(TTestOutcome::Fail, "failed", std::string());
                return;
            }

            m_Results.SuccessCount++;
            finish(TTestOutcome::Pass, "passed", std::string());
        };

    auto finishExpectedException = [&](bool typeMatches, std::string const& message, std::string const& description)
        {
            bool const messageMatches = m_ExpectedExceptionMessage.empty() ||
                (message.find(m_ExpectedExceptionMessage) != std::string::npos);

            if (typeMatches && messageMatches)
            {
                finishPassUnlessChecksFailed();
                return;
            }

            m_Results.FailedCount++;

            std::string detail;
            if (!typeMatches)
                detail = "expected exception type was not thrown (caught a different exception): " + description;
            else
                detail = "expected exception message to contain \"" + m_ExpectedExceptionMessage + "\" but caught: " +
                    description;

            finish(TTestOutcome::Fail, "failed", detail);
        };

    try
    {
        // Reset for test
        SetExceptionExpected(false, "", 0, "");
        ResetTestFailedOneOrMoreChecks();

        // Run test
        if (nullptr != testCase.GetTestCallback())
        {
            Log("Running test: " + testFullName);

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

        finishPassUnlessChecksFailed();
    }
    catch (TExceptSkipped const& ex)
    {
        m_Results.SkippedCount++;
        finish(TTestOutcome::Skip, "skipped", ex.what());
    }
    catch (TTestException const& ex)
    {
        // This framework's own failure signal (an Assert* failure, or TExceptExpected when an expected exception
        // never came), never the exception a test is waiting for, so it fails the test even while an exception
        // is expected, without checking the type or message the test asked for.
        m_Results.FailedCount++;
        finish(TTestOutcome::Fail, "failed", ex.what());
    }
    catch (std::exception const& ex)
    {
        if (m_ExceptionExpected)
        {
            // A type/message check is only requested via the templated SetExceptionExpected<TException>()
            // overload; the plain bool overload leaves both null/empty, matching any exception (legacy behavior).
            // A requested RTL exception type (see ExceptionTypeExpected()) never matches a std::exception.
            bool const typeMatches = (m_ExpectedExceptionTypeChecker != nullptr) ?
                    m_ExpectedExceptionTypeChecker(ex) : !ExceptionTypeExpected();
            finishExpectedException(typeMatches, ex.what(), ex.what());
        }
        else
        {
            m_Results.FailedCount++;
            throw; // Unexpected failure
        }
    }
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    catch (System::Sysutils::Exception& ex)
    {
        if (m_ExceptionExpected)
        {
            // Mirrors the std::exception case above: a requested std::exception type never matches here.
            bool const typeMatches = (m_ExpectedRTLExceptionTypeChecker != nullptr) ?
                    m_ExpectedRTLExceptionTypeChecker(ex) : !ExceptionTypeExpected();
            finishExpectedException(typeMatches, RTLExceptionMessage(ex), DescribeRTLException(ex));
        }
        else
        {
            m_Results.FailedCount++;
            throw; // Unexpected failure
        }
    }
#endif
    catch (...)
    {
        if (m_ExceptionExpected)
        {
            if (ExceptionTypeExpected())
            {
                // A specific type was requested, but what was thrown isn't an exception class this framework
                // can inspect to confirm the type (or message) matched. Treat that as a failure, not a pass.
                m_Results.FailedCount++;
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
                finish(TTestOutcome::Fail, "failed", "expected a specific exception type, but an object that is "
                    "neither a std::exception nor an RTL Exception was thrown instead.");
#else
                finish(TTestOutcome::Fail, "failed",
                    "expected a specific exception type, but a non-std::exception object was thrown instead.");
#endif
            }
            else
            {
                finishPassUnlessChecksFailed();
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
    : Crashed(false),
      FailedCount(0),
      SkippedCount(0),
      Stopped(false),
      SuccessCount(0),
      TimedOut(false)
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
