/* **************************************************************************
ASWUnitTests_Exception.cpp
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
// TExceptExpected
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptExpected::TExceptExpected(std::string const& msg)
{
    m_Message = "Exception expected: " + msg;
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
// TExceptAbortRun
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TExceptAbortRun::TExceptAbortRun(std::string const& msg)
{
    m_Message = msg;
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
