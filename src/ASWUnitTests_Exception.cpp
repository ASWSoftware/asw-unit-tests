/* **************************************************************************
ASWUnitTests_Exception.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "ASWUnitTests_Exception.h"
//---------------------------------------------------------------------------
#include <cstddef>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
namespace
{

std::string ToUTF8(System::UnicodeString const& text);

//---------------------------------------------------------------------------
std::string ToUTF8(System::UnicodeString const& text)
{
    System::UTF8String const utf8(text);
    return std::string(utf8.c_str(), static_cast<std::size_t>(utf8.Length()));
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
std::string DescribeRTLException(System::Sysutils::Exception& ex)
{
    return ToUTF8(ex.ClassName()) + ": " + RTLExceptionMessage(ex);
}
//---------------------------------------------------------------------------
std::string RTLExceptionMessage(System::Sysutils::Exception& ex)
{
    return ToUTF8(ex.Message);
}
//---------------------------------------------------------------------------
#endif


/////////////////////////////////////////////////////////////////////////////
// TTestException
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTestException::TTestException()
{
}
//---------------------------------------------------------------------------
TTestException::TTestException(std::string const& msg)
    : m_Message(msg)
{
}
//---------------------------------------------------------------------------
const char* TTestException::what() const noexcept
{
    return m_Message.c_str();
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptAbortRun
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptAbortRun::TExceptAbortRun(std::string const& msg)
{
    m_Message = msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptContains
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptContains::TExceptContains(std::string const& method, int line, std::string const& text,
    std::string const& substring, std::string const& msg, bool ignoreCase)
{
    m_Message = "Substring not found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" to contain \"" + substring + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptExpected
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptExpected::TExceptExpected(std::string const& msg)
{
    m_Message = "Exception expected: " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptEmpty
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptEmpty::TExceptEmpty(std::string const& method, int line, std::string const& detail, std::string const& msg)
{
    m_Message = "Not empty: " + method + " (" + std::to_string(line) + "): " + detail + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptEndsWith
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptEndsWith::TExceptEndsWith(std::string const& method, int line, std::string const& text,
    std::string const& suffix, std::string const& msg, bool ignoreCase)
{
    m_Message = "Suffix not found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" to end with \"" + suffix + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptEquals
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptEquals::TExceptEquals(std::string const& msg)
{
    m_Message = "Values not equal: " + msg;
}
//---------------------------------------------------------------------------
TExceptEquals::TExceptEquals(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Values not equal: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------
TExceptEquals::TExceptEquals(std::string const& method, int line, std::string const& expected,
    std::string const& actual, std::string const& msg, bool ignoreCase)
{
    m_Message = "Values not equal: " + method + " (" + std::to_string(line) + "): Expected: \"" + expected +
        "\" but was \"" + actual + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptFail
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptFail::TExceptFail(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Failed: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptFalse
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptFalse::TExceptFalse(std::string const& msg)
{
    m_Message = "Expected false but was true: " + msg;
}
//---------------------------------------------------------------------------
TExceptFalse::TExceptFalse(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Expected false but was true: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptMatches
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptMatches::TExceptMatches(std::string const& method, int line, std::string const& detail, std::string const& msg)
{
    m_Message = "Pattern not matched: " + method + " (" + std::to_string(line) + "): " + detail + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNoThrow
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNoThrow::TExceptNoThrow(std::string const& method, int line, std::string const& detail, std::string const& msg)
{
    m_Message = "Unexpected exception: " + method + " (" + std::to_string(line) + "): " + detail + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotContains
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotContains::TExceptNotContains(std::string const& method, int line, std::string const& text,
    std::string const& substring, std::string const& msg, bool ignoreCase)
{
    m_Message = "Substring found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" not to contain \"" + substring + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotEmpty
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotEmpty::TExceptNotEmpty(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Empty: " + method + " (" + std::to_string(line) + "): Expected not empty but was empty. " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotEndsWith
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotEndsWith::TExceptNotEndsWith(std::string const& method, int line, std::string const& text,
    std::string const& suffix, std::string const& msg, bool ignoreCase)
{
    m_Message = "Suffix found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" not to end with \"" + suffix + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotEquals
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotEquals::TExceptNotEquals(std::string const& msg)
{
    m_Message = "Values are equal: " + msg;
}
//---------------------------------------------------------------------------
TExceptNotEquals::TExceptNotEquals(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Values are equal: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------
TExceptNotEquals::TExceptNotEquals(
    std::string const& method, int line, std::string const& value, std::string const& msg)
{
    m_Message = "Values are equal: " + method + " (" + std::to_string(line) + "): Value: \"" + value + "\". " + msg;
}
//---------------------------------------------------------------------------
TExceptNotEquals::TExceptNotEquals(std::string const& method, int line, std::string const& value,
    std::string const& otherValue, std::string const& msg, bool ignoreCase)
{
    m_Message = "Values are equal: " + method + " (" + std::to_string(line) + "): Values: \"" + value + "\" and \"" +
        otherValue + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotMatches
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotMatches::TExceptNotMatches(std::string const& method, int line, std::string const& detail,
    std::string const& msg)
{
    m_Message = "Pattern matched: " + method + " (" + std::to_string(line) + "): " + detail + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotNull
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotNull::TExceptNotNull(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Expected not null but was null: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotSame
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotSame::TExceptNotSame(std::string const& method, int line, std::string const& value, std::string const& msg)
{
    m_Message = "Same object: " + method + " (" + std::to_string(line) +
        "): Expected a different object but both are \"" + value + "\". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNotStartsWith
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNotStartsWith::TExceptNotStartsWith(std::string const& method, int line, std::string const& text,
    std::string const& prefix, std::string const& msg, bool ignoreCase)
{
    m_Message = "Prefix found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" not to start with \"" + prefix + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptNull
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptNull::TExceptNull(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Expected null but was not null: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptOrdering
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptOrdering::TExceptOrdering(std::string const& method, int line, std::string const& value,
    std::string const& relation, std::string const& bound, std::string const& msg)
{
    m_Message = "Values out of order: " + method + " (" + std::to_string(line) + "): Expected " + value + " to be " +
        relation + " " + bound + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptSame
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptSame::TExceptSame(std::string const& method, int line, std::string const& expected,
    std::string const& actual, std::string const& msg)
{
    m_Message = "Not the same object: " + method + " (" + std::to_string(line) + "): Expected the same object as \"" +
        expected + "\" but was \"" + actual + "\". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptStartsWith
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptStartsWith::TExceptStartsWith(std::string const& method, int line, std::string const& text,
    std::string const& prefix, std::string const& msg, bool ignoreCase)
{
    m_Message = "Prefix not found: " + method + " (" + std::to_string(line) + "): Expected \"" + text +
        "\" to start with \"" + prefix + "\"" + (ignoreCase ? " (ignoring case)" : "") + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptSkipped
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptSkipped::TExceptSkipped(std::string const& msg)
{
    m_Message = "Test skipped: " + msg;
}
//---------------------------------------------------------------------------
TExceptSkipped::TExceptSkipped(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Test skipped: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptTestTimedOut
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptTestTimedOut::TExceptTestTimedOut(std::string const& msg)
    : TExceptAbortRun(msg)
{
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptTestCrashed
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptTestCrashed::TExceptTestCrashed(std::string const& msg)
    : TExceptAbortRun(msg)
{
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptThrows
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptThrows::TExceptThrows(std::string const& method, int line, std::string const& detail, std::string const& msg)
{
    m_Message = "Expected exception not caught: " + method + " (" + std::to_string(line) + "): " + detail + ". " + msg;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TExceptTrue
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptTrue::TExceptTrue(std::string const& msg)
{
    m_Message = "Expected true but was false: \"" + msg + "\"";
}
//---------------------------------------------------------------------------
TExceptTrue::TExceptTrue(std::string const& method, int line, std::string const& msg)
{
    m_Message = "Expected true but was false: " + method + " (" + std::to_string(line) + "): " + msg;
}
//---------------------------------------------------------------------------

// /////// Compiler specific exceptions after this line /////////////////////

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
/////////////////////////////////////////////////////////////////////////////
// TExceptRTLException
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptRTLException::TExceptRTLException(System::Sysutils::Exception& ex)
    : std::runtime_error(DescribeRTLException(ex)),
      m_RTLClass(ex.ClassType())
{
}
//---------------------------------------------------------------------------
System::TClass TExceptRTLException::RTLClass() const noexcept
{
    return m_RTLClass;
}
//---------------------------------------------------------------------------
#endif

} // namespace ASWUnitTests
