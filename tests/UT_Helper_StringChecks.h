/* **************************************************************************
UT_Helper_StringChecks.h
Author: Anthony S. West - ASW Software

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
#ifndef UT_Helper_StringChecksH
#define UT_Helper_StringChecksH
//---------------------------------------------------------------------------
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TStringCheckProbe
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) group whose RunChecks() calls every Check method of the
// Equals, Contains, StartsWith, EndsWith and Matches families, IC and Not forms included, with the same two
// texts, and returns what each did, and whose RunAsserts() does the same with the Assert methods, so
// TTest_ASWUnitTests_TestBase can check them with texts of any two types. Their templates are defined, and
// instantiated for the pairs of types the tests use, in this module rather than in the test module: there,
// they make it too big for RAD Studio's Win32 linker (its line number cache, in a Debug build).
/////////////////////////////////////////////////////////////////////////////
class TStringCheckProbe : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

public:
    // How a string check treats its texts, which decides each case's expected outcome.
    enum class ERule
    {
        Equals,    // Equals and EqualsIC: a null C string equals only another one.
        NotEquals, // NotEquals and NotEqualsIC.
        Matches,   // Contains, StartsWith, EndsWith and Matches: a null C string matches nothing.
        NotMatches // Their Not forms.
    };

    struct TCheck
    {
        char const* Name;
        ERule Rule;
        bool IgnoresCase;
    };

public:
    // The method families RunAsserts() and RunChecks() call, in the order of their results, e.g. "ContainsIC"
    // for AssertContainsIC() and CheckContainsIC().
    static TCheck const Checks[18];

private:
    template <typename TAssertion>
    static std::optional<std::string> AssertFailure(TAssertion assertion);

private:
    std::optional<std::string> TakeCheckFailure();

public:
    static std::size_t IndexOf(std::string const& name);

public:
    TStringCheckProbe();

#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    std::optional<std::string> CheckEqualsThroughSourceLocation(std::string const& expected, wchar_t const* actual);
#endif
    // Calls the Assert method of each of Checks with 'first' and 'second' and returns, in the same order, each one's
    // failure message, or nullopt for each that passed. The texts are taken by value, so an array is a C string.
    // Instantiated in the .cpp for each two different types of std::string, char const*, std::wstring and
    // wchar_t const*.
    template <typename TFirst, typename TSecond>
    std::vector<std::optional<std::string> > RunAsserts(TFirst first, TSecond second);
    // The same, with the Check methods.
    template <typename TFirst, typename TSecond>
    std::vector<std::optional<std::string> > RunChecks(TFirst first, TSecond second);

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef UT_Helper_StringChecksH
