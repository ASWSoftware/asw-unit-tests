/* **************************************************************************
UT_Helper_StringChecks.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 ASW Software

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
#include "UT_Helper_StringChecks.h"
//---------------------------------------------------------------------------
#include <iterator>
#include <stdexcept>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TStringCheckProbe
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TStringCheckProbe::TCheck const TStringCheckProbe::Checks[18] =
{
    { "Contains", ERule::Matches, false },
    { "ContainsIC", ERule::Matches, true },
    { "EndsWith", ERule::Matches, false },
    { "EndsWithIC", ERule::Matches, true },
    { "Equals", ERule::Equals, false },
    { "EqualsIC", ERule::Equals, true },
    { "Matches", ERule::Matches, false },
    { "NotContains", ERule::NotMatches, false },
    { "NotContainsIC", ERule::NotMatches, true },
    { "NotEndsWith", ERule::NotMatches, false },
    { "NotEndsWithIC", ERule::NotMatches, true },
    { "NotEquals", ERule::NotEquals, false },
    { "NotEqualsIC", ERule::NotEquals, true },
    { "NotMatches", ERule::NotMatches, false },
    { "NotStartsWith", ERule::NotMatches, false },
    { "NotStartsWithIC", ERule::NotMatches, true },
    { "StartsWith", ERule::Matches, false },
    { "StartsWithIC", ERule::Matches, true },
};

//---------------------------------------------------------------------------
TStringCheckProbe::TStringCheckProbe()
    : inherited("Helper_StringCheckProbe")
{
    SetLogSuppressed(true);
}
//---------------------------------------------------------------------------
/*
    TStringCheckProbe::AssertFailure

    Calls 'assertion' and returns its failure's message, or nullopt when it passes.
*/
template <typename TAssertion>
std::optional<std::string> TStringCheckProbe::AssertFailure(TAssertion assertion)
{
    try
    {
        assertion();
    }
    catch (TTestException const& ex)
    {
        return std::string(ex.what());
    }

    return std::nullopt;
}
//---------------------------------------------------------------------------
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
/*
    TStringCheckProbe::CheckEqualsThroughSourceLocation

    Calls CheckEquals() through its std::source_location overload, which forwards to the method/line one, and
    returns its failure message, or nullopt when it passes.
*/
std::optional<std::string> TStringCheckProbe::CheckEqualsThroughSourceLocation(std::string const& expected,
    wchar_t const* actual)
{
    CheckEquals(expected, actual, "msg");
    return TakeCheckFailure();
}
//---------------------------------------------------------------------------
#endif
/*
    TStringCheckProbe::IndexOf

    The position of the method family named 'name' in Checks, and in RunAsserts()'s and RunChecks()'s results.
*/
std::size_t TStringCheckProbe::IndexOf(std::string const& name)
{
    for (std::size_t i = 0; i < std::size(Checks); ++i)
    {
        if (name == Checks[i].Name)
            return i;
    }

    throw std::logic_error("no string check named " + name);
}
//---------------------------------------------------------------------------
template <typename TFirst, typename TSecond>
std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts(TFirst first, TSecond second)
{
    std::vector<std::optional<std::string> > results;

    results.push_back(AssertFailure([&]() {
            AssertContains(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertContainsIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertEndsWith(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertEndsWithIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertEquals(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertEqualsIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertMatches(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotContains(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotContainsIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotEndsWith(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotEndsWithIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotEquals(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotEqualsIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotMatches(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotStartsWith(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertNotStartsWithIC(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertStartsWith(first, second, __func__, __LINE__, "msg");
        }));
    results.push_back(AssertFailure([&]() {
            AssertStartsWithIC(first, second, __func__, __LINE__, "msg");
        }));

    return results;
}
//---------------------------------------------------------------------------
template <typename TFirst, typename TSecond>
std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks(TFirst first, TSecond second)
{
    std::vector<std::optional<std::string> > results;

    CheckContains(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckContainsIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckEndsWith(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckEndsWithIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckEquals(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckEqualsIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckMatches(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotContains(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotContainsIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotEndsWith(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotEndsWithIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotEquals(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotEqualsIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotMatches(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotStartsWith(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckNotStartsWithIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckStartsWith(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());
    CheckStartsWithIC(first, second, __func__, __LINE__, "msg");
    results.push_back(TakeCheckFailure());

    return results;
}
//---------------------------------------------------------------------------
/*
    TStringCheckProbe::TakeCheckFailure

    The message of the Check failure since the last call, or nullopt if there was none, then forgets it, for the
    next Check method.
*/
std::optional<std::string> TStringCheckProbe::TakeCheckFailure()
{
    std::optional<std::string> failure;

    if (TestFailedOneOrMoreChecks())
        failure = m_CheckFailureMessages.back();

    ResetTestFailedOneOrMoreChecks();
    return failure;
}
//---------------------------------------------------------------------------

// The pairs of types the tests use.
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::string, char const*>(
    std::string, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::string, std::wstring>(
    std::string, std::wstring);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::string, wchar_t const*>(
    std::string, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<char const*, std::string>(
    char const*, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<char const*, std::wstring>(
    char const*, std::wstring);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<char const*, wchar_t const*>(
    char const*, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::wstring, std::string>(
    std::wstring, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::wstring, char const*>(
    std::wstring, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<std::wstring, wchar_t const*>(
    std::wstring, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<wchar_t const*, std::string>(
    wchar_t const*, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<wchar_t const*, char const*>(
    wchar_t const*, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunAsserts<wchar_t const*, std::wstring>(
    wchar_t const*, std::wstring);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::string, char const*>(
    std::string, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::string, std::wstring>(
    std::string, std::wstring);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::string, wchar_t const*>(
    std::string, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<char const*, std::string>(
    char const*, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<char const*, std::wstring>(
    char const*, std::wstring);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<char const*, wchar_t const*>(
    char const*, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::wstring, std::string>(
    std::wstring, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::wstring, char const*>(
    std::wstring, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<std::wstring, wchar_t const*>(
    std::wstring, wchar_t const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<wchar_t const*, std::string>(
    wchar_t const*, std::string);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<wchar_t const*, char const*>(
    wchar_t const*, char const*);
template std::vector<std::optional<std::string> > TStringCheckProbe::RunChecks<wchar_t const*, std::wstring>(
    wchar_t const*, std::wstring);

//---------------------------------------------------------------------------

} // namespace ASWUnitTests
