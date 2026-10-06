/* **************************************************************************
Test_ASWUnitTests_RTLStrings.cpp
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
#include "Test_ASWUnitTests_RTLStrings.h"
//---------------------------------------------------------------------------

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

//---------------------------------------------------------------------------
#include <cstddef>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

TTestCaseRecord const* FindRecord(TTestResults const& results, std::string const& testName);
bool NameEndsWith(std::string const& name, std::string const& suffix);
System::String Text();

//---------------------------------------------------------------------------

// The record for 'testName' in 'results', or nullptr if there isn't one.
TTestCaseRecord const* FindRecord(TTestResults const& results, std::string const& testName)
{
    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (record.TestName == testName)
            return &record;
    }

    return nullptr;
}

//---------------------------------------------------------------------------
// True if 'name' ends with 'suffix'. Used below so a fixture test's own name ("..._Passes"/
// "..._Fails") documents its expected outcome, and the outer verifying test can check every
// registered test generically instead of hand-maintaining a separate expected-outcome table.
bool NameEndsWith(std::string const& name, std::string const& suffix)
{
    return name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//---------------------------------------------------------------------------
// The text most tests below compare with: "Hello" at the start, "hello" at the end and "World" in the middle, so a
// prefix, suffix or substring check matches at one place only, and only with the right case.
System::String Text()
{
    return System::String(L"Hello World hello");
}

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_RTLStringComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with a "_Passes" and a "_Fails" test for each
// System::String overload, comparing Text() with a narrow literal (Check) or a wide one (Assert). Each outcome would
// change if the overload forwarded to the wrong method: case-sensitive or IC, Not or not, start, end or anywhere.
// "NonASCII" checks that a failure shows a System::String as UTF-8, and the "SourceLocation" tests, where supported,
// use the std::source_location forms. Test names self-document expected outcome via NameEndsWith().
/////////////////////////////////////////////////////////////////////////////
class TFixture_RTLStringComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertContainsIC_Fails();
    void Test_AssertContainsIC_Passes();
    void Test_AssertContains_Fails();
    void Test_AssertContains_Passes();
    void Test_AssertEndsWithIC_Fails();
    void Test_AssertEndsWithIC_Passes();
    void Test_AssertEndsWith_Fails();
    void Test_AssertEndsWith_Passes();
    void Test_AssertEqualsIC_Fails();
    void Test_AssertEqualsIC_Passes();
    void Test_AssertEquals_Fails();
    void Test_AssertEquals_Passes();
    void Test_AssertNotContainsIC_Fails();
    void Test_AssertNotContainsIC_Passes();
    void Test_AssertNotContains_Fails();
    void Test_AssertNotContains_Passes();
    void Test_AssertNotEndsWithIC_Fails();
    void Test_AssertNotEndsWithIC_Passes();
    void Test_AssertNotEndsWith_Fails();
    void Test_AssertNotEndsWith_Passes();
    void Test_AssertNotEqualsIC_Fails();
    void Test_AssertNotEqualsIC_Passes();
    void Test_AssertNotEquals_Fails();
    void Test_AssertNotEquals_Passes();
    void Test_AssertNotStartsWithIC_Fails();
    void Test_AssertNotStartsWithIC_Passes();
    void Test_AssertNotStartsWith_Fails();
    void Test_AssertNotStartsWith_Passes();
    void Test_AssertStartsWithIC_Fails();
    void Test_AssertStartsWithIC_Passes();
    void Test_AssertStartsWith_Fails();
    void Test_AssertStartsWith_Passes();
    void Test_CheckContainsIC_Fails();
    void Test_CheckContainsIC_Passes();
    void Test_CheckContains_Fails();
    void Test_CheckContains_NonASCII_Fails();
    void Test_CheckContains_Passes();
    void Test_CheckEndsWithIC_Fails();
    void Test_CheckEndsWithIC_Passes();
    void Test_CheckEndsWith_Fails();
    void Test_CheckEndsWith_Passes();
    void Test_CheckEqualsIC_Fails();
    void Test_CheckEqualsIC_Passes();
    void Test_CheckEquals_Fails();
    void Test_CheckEquals_Passes();
    void Test_CheckNotContainsIC_Fails();
    void Test_CheckNotContainsIC_Passes();
    void Test_CheckNotContains_Fails();
    void Test_CheckNotContains_Passes();
    void Test_CheckNotEndsWithIC_Fails();
    void Test_CheckNotEndsWithIC_Passes();
    void Test_CheckNotEndsWith_Fails();
    void Test_CheckNotEndsWith_Passes();
    void Test_CheckNotEqualsIC_Fails();
    void Test_CheckNotEqualsIC_Passes();
    void Test_CheckNotEquals_Fails();
    void Test_CheckNotEquals_Passes();
    void Test_CheckNotStartsWithIC_Fails();
    void Test_CheckNotStartsWithIC_Passes();
    void Test_CheckNotStartsWith_Fails();
    void Test_CheckNotStartsWith_Passes();
    void Test_CheckStartsWithIC_Fails();
    void Test_CheckStartsWithIC_Passes();
    void Test_CheckStartsWith_Fails();
    void Test_CheckStartsWith_Passes();
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    void Test_SourceLocation_AssertEqualsIC_Passes();
    void Test_SourceLocation_CheckEndsWith_Fails();
    void Test_SourceLocation_CheckStartsWithIC_Passes();
#endif

public:
    int SourceLocationLine = 0; // The line of Test_SourceLocation_CheckEndsWith_Fails()'s failing call.

    TFixture_RTLStringComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_RTLStringComparisons::TFixture_RTLStringComparisons()
    : inherited("Fixture_RTLStringComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertContainsIC_Fails, "AssertContainsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertContainsIC_Passes, "AssertContainsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertContains_Fails, "AssertContains_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertContains_Passes, "AssertContains_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEndsWithIC_Fails, "AssertEndsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEndsWithIC_Passes, "AssertEndsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEndsWith_Fails, "AssertEndsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEndsWith_Passes, "AssertEndsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEqualsIC_Fails, "AssertEqualsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEqualsIC_Passes, "AssertEqualsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEquals_Fails, "AssertEquals_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertEquals_Passes, "AssertEquals_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotContainsIC_Fails, "AssertNotContainsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotContainsIC_Passes, "AssertNotContainsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotContains_Fails, "AssertNotContains_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotContains_Passes, "AssertNotContains_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEndsWithIC_Fails, "AssertNotEndsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEndsWithIC_Passes, "AssertNotEndsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEndsWith_Fails, "AssertNotEndsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEndsWith_Passes, "AssertNotEndsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEqualsIC_Fails, "AssertNotEqualsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEqualsIC_Passes, "AssertNotEqualsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEquals_Fails, "AssertNotEquals_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotEquals_Passes, "AssertNotEquals_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotStartsWithIC_Fails, "AssertNotStartsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotStartsWithIC_Passes, "AssertNotStartsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotStartsWith_Fails, "AssertNotStartsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertNotStartsWith_Passes, "AssertNotStartsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertStartsWithIC_Fails, "AssertStartsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertStartsWithIC_Passes, "AssertStartsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertStartsWith_Fails, "AssertStartsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_AssertStartsWith_Passes, "AssertStartsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckContainsIC_Fails, "CheckContainsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckContainsIC_Passes, "CheckContainsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckContains_Fails, "CheckContains_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckContains_NonASCII_Fails, "CheckContains_NonASCII_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckContains_Passes, "CheckContains_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEndsWithIC_Fails, "CheckEndsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEndsWithIC_Passes, "CheckEndsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEndsWith_Fails, "CheckEndsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEndsWith_Passes, "CheckEndsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEqualsIC_Fails, "CheckEqualsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEqualsIC_Passes, "CheckEqualsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEquals_Fails, "CheckEquals_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckEquals_Passes, "CheckEquals_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotContainsIC_Fails, "CheckNotContainsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotContainsIC_Passes, "CheckNotContainsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotContains_Fails, "CheckNotContains_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotContains_Passes, "CheckNotContains_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEndsWithIC_Fails, "CheckNotEndsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEndsWithIC_Passes, "CheckNotEndsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEndsWith_Fails, "CheckNotEndsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEndsWith_Passes, "CheckNotEndsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEqualsIC_Fails, "CheckNotEqualsIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEqualsIC_Passes, "CheckNotEqualsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEquals_Fails, "CheckNotEquals_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotEquals_Passes, "CheckNotEquals_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotStartsWithIC_Fails, "CheckNotStartsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotStartsWithIC_Passes, "CheckNotStartsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotStartsWith_Fails, "CheckNotStartsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckNotStartsWith_Passes, "CheckNotStartsWith_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckStartsWithIC_Fails, "CheckStartsWithIC_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckStartsWithIC_Passes, "CheckStartsWithIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckStartsWith_Fails, "CheckStartsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_CheckStartsWith_Passes, "CheckStartsWith_Passes");
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    RegisterTest(&TFixture_RTLStringComparisons::Test_SourceLocation_AssertEqualsIC_Passes,
        "SourceLocation_AssertEqualsIC_Passes");
    RegisterTest(&TFixture_RTLStringComparisons::Test_SourceLocation_CheckEndsWith_Fails,
        "SourceLocation_CheckEndsWith_Fails");
    RegisterTest(&TFixture_RTLStringComparisons::Test_SourceLocation_CheckStartsWithIC_Passes,
        "SourceLocation_CheckStartsWithIC_Passes");
#endif
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertContainsIC_Fails()
{
    AssertContainsIC(Text(), L"xyz", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertContainsIC_Passes()
{
    AssertContainsIC(Text(), L"LO WO", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertContains_Fails()
{
    AssertContains(Text(), L"WORLD", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertContains_Passes()
{
    AssertContains(Text(), L"lo Wo", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEndsWithIC_Fails()
{
    AssertEndsWithIC(Text(), L"World", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEndsWithIC_Passes()
{
    AssertEndsWithIC(Text(), L"D HELLO", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEndsWith_Fails()
{
    AssertEndsWith(Text(), L"Hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEndsWith_Passes()
{
    AssertEndsWith(Text(), L"hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEqualsIC_Fails()
{
    AssertEqualsIC(L"Hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEqualsIC_Passes()
{
    AssertEqualsIC(L"HELLO WORLD HELLO", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEquals_Fails()
{
    AssertEquals(L"hello world hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertEquals_Passes()
{
    AssertEquals(L"Hello World hello", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotContainsIC_Fails()
{
    AssertNotContainsIC(Text(), L"LO WO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotContainsIC_Passes()
{
    AssertNotContainsIC(Text(), L"xyz", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotContains_Fails()
{
    AssertNotContains(Text(), L"lo Wo", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotContains_Passes()
{
    AssertNotContains(Text(), L"WORLD", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEndsWithIC_Fails()
{
    AssertNotEndsWithIC(Text(), L"HELLO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEndsWithIC_Passes()
{
    AssertNotEndsWithIC(Text(), L"World", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEndsWith_Fails()
{
    AssertNotEndsWith(Text(), L"hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEndsWith_Passes()
{
    AssertNotEndsWith(Text(), L"Hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEqualsIC_Fails()
{
    AssertNotEqualsIC(L"HELLO WORLD HELLO", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEqualsIC_Passes()
{
    AssertNotEqualsIC(L"Hello", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEquals_Fails()
{
    AssertNotEquals(L"Hello World hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotEquals_Passes()
{
    AssertNotEquals(L"HELLO WORLD HELLO", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotStartsWithIC_Fails()
{
    AssertNotStartsWithIC(Text(), L"HELLO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotStartsWithIC_Passes()
{
    AssertNotStartsWithIC(Text(), L"World", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotStartsWith_Fails()
{
    AssertNotStartsWith(Text(), L"Hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertNotStartsWith_Passes()
{
    AssertNotStartsWith(Text(), L"hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertStartsWithIC_Fails()
{
    AssertStartsWithIC(Text(), L"World", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertStartsWithIC_Passes()
{
    AssertStartsWithIC(Text(), L"HELLO W", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertStartsWith_Fails()
{
    AssertStartsWith(Text(), L"hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_AssertStartsWith_Passes()
{
    AssertStartsWith(Text(), L"Hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckContainsIC_Fails()
{
    CheckContainsIC(Text(), "xyz", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckContainsIC_Passes()
{
    CheckContainsIC(Text(), "LO WO", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckContains_Fails()
{
    CheckContains(Text(), "WORLD", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckContains_NonASCII_Fails()
{
    // "cafe" with an e-acute, a space, and U+1F600 (a surrogate pair), then u-umlaut.
    CheckContains(System::String(L"caf" L"\x00E9" L" \U0001F600"), "\xC3\xBC", __func__, __LINE__, "not present");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckContains_Passes()
{
    CheckContains(Text(), "lo Wo", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEndsWithIC_Fails()
{
    CheckEndsWithIC(Text(), "World", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEndsWithIC_Passes()
{
    CheckEndsWithIC(Text(), "D HELLO", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEndsWith_Fails()
{
    CheckEndsWith(Text(), "Hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEndsWith_Passes()
{
    CheckEndsWith(Text(), "hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEqualsIC_Fails()
{
    CheckEqualsIC("Hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEqualsIC_Passes()
{
    CheckEqualsIC("HELLO WORLD HELLO", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEquals_Fails()
{
    CheckEquals("hello world hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckEquals_Passes()
{
    CheckEquals("Hello World hello", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotContainsIC_Fails()
{
    CheckNotContainsIC(Text(), "LO WO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotContainsIC_Passes()
{
    CheckNotContainsIC(Text(), "xyz", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotContains_Fails()
{
    CheckNotContains(Text(), "lo Wo", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotContains_Passes()
{
    CheckNotContains(Text(), "WORLD", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEndsWithIC_Fails()
{
    CheckNotEndsWithIC(Text(), "HELLO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEndsWithIC_Passes()
{
    CheckNotEndsWithIC(Text(), "World", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEndsWith_Fails()
{
    CheckNotEndsWith(Text(), "hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEndsWith_Passes()
{
    CheckNotEndsWith(Text(), "Hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEqualsIC_Fails()
{
    CheckNotEqualsIC("HELLO WORLD HELLO", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEqualsIC_Passes()
{
    CheckNotEqualsIC("Hello", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEquals_Fails()
{
    CheckNotEquals("Hello World hello", Text(), __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotEquals_Passes()
{
    CheckNotEquals("HELLO WORLD HELLO", Text(), __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotStartsWithIC_Fails()
{
    CheckNotStartsWithIC(Text(), "HELLO", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotStartsWithIC_Passes()
{
    CheckNotStartsWithIC(Text(), "World", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotStartsWith_Fails()
{
    CheckNotStartsWith(Text(), "Hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckNotStartsWith_Passes()
{
    CheckNotStartsWith(Text(), "hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckStartsWithIC_Fails()
{
    CheckStartsWithIC(Text(), "World", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckStartsWithIC_Passes()
{
    CheckStartsWithIC(Text(), "HELLO W", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckStartsWith_Fails()
{
    CheckStartsWith(Text(), "hello", __func__, __LINE__, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_CheckStartsWith_Passes()
{
    CheckStartsWith(Text(), "Hello", __func__, __LINE__, "passes");
}
//---------------------------------------------------------------------------
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
void TFixture_RTLStringComparisons::Test_SourceLocation_AssertEqualsIC_Passes()
{
    AssertEqualsIC(L"HELLO WORLD HELLO", Text(), "passes");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_SourceLocation_CheckEndsWith_Fails()
{
    SourceLocationLine = __LINE__ + 1;
    CheckEndsWith(Text(), std::string("Hello"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_RTLStringComparisons::Test_SourceLocation_CheckStartsWithIC_Passes()
{
    CheckStartsWithIC(Text(), std::wstring(L"HELLO W"), "passes");
}
//---------------------------------------------------------------------------
#endif

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_RTLStrings
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_RTLStrings::TTest_ASWUnitTests_RTLStrings()
    : inherited("ASWUnitTests_RTLStrings_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_RTLStrings::Test_Overloads_AcceptEveryTextKind, "Overloads_AcceptEveryTextKind");
    RegisterTest(&TTest_ASWUnitTests_RTLStrings::Test_Overloads_ForwardAndShowTheUsualMessages,
        "Overloads_ForwardAndShowTheUsualMessages");
    RegisterTest(&TTest_ASWUnitTests_RTLStrings::Test_Overloads_RejectNonText, "Overloads_RejectNonText");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_RTLStrings::~TTest_ASWUnitTests_RTLStrings()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::Test_Overloads_AcceptEveryTextKind()
{
    // Arrange
    System::String const text = Text();
    std::string const narrow = "Hello World hello";
    std::wstring const wide = L"Hello World hello";
    char narrowBuffer[] = "Hello";
    wchar_t wideBuffer[] = L"hello";
    // "cafe" with an e-acute, a space, and U+1F600 (a surrogate pair).
    System::String const nonASCII(L"caf" L"\x00E9" L" \U0001F600");

    // Act and Assert: each passes only when both texts convert to the same UTF-8.
    CheckEquals(text, System::String(L"Hello World hello"), __func__, __LINE__, "System::String and System::String");
    CheckEquals(text, "Hello World hello", __func__, __LINE__, "System::String and a narrow literal");
    CheckEquals("Hello World hello", text, __func__, __LINE__, "a narrow literal and System::String");
    CheckEquals(text, L"Hello World hello", __func__, __LINE__, "System::String and a wide literal");
    CheckEquals(L"Hello World hello", text, __func__, __LINE__, "a wide literal and System::String");
    CheckEquals(text, narrow, __func__, __LINE__, "System::String and std::string");
    CheckEquals(narrow, text, __func__, __LINE__, "std::string and System::String");
    CheckEquals(text, wide, __func__, __LINE__, "System::String and std::wstring");
    CheckEquals(wide, text, __func__, __LINE__, "std::wstring and System::String");
    CheckStartsWith(text, narrowBuffer, __func__, __LINE__, "System::String and a char buffer");
    CheckEndsWith(text, wideBuffer, __func__, __LINE__, "System::String and a wchar_t buffer");

    CheckEquals(System::String(), static_cast<char const*>(nullptr), __func__, __LINE__,
        "a null C string is empty text");
    CheckEquals(static_cast<wchar_t const*>(nullptr), System::String(), __func__, __LINE__,
        "a null wide C string is empty text");

    CheckEquals(nonASCII, "caf\xC3\xA9 \xF0\x9F\x98\x80", __func__, __LINE__, "non-ASCII text and its UTF-8 bytes");
    CheckEquals(nonASCII, L"caf" L"\x00E9" L" \U0001F600", __func__, __LINE__, "non-ASCII text and its wide form");
    CheckEquals(std::wstring(L"caf" L"\x00E9" L" \U0001F600"), nonASCII, __func__, __LINE__,
        "non-ASCII std::wstring and System::String");
    CheckNotEquals(nonASCII, "caf\xC3\x89 \xF0\x9F\x98\x80", __func__, __LINE__, "e-acute is not E-acute");
    CheckEqualsIC(System::String(L"CAF" L"\x00E9"), "caf\xC3\xA9", __func__, __LINE__,
        "ASCII letters ignore case next to a non-ASCII one");
    CheckEquals(System::String(L"\xD800"), L"\xD800", __func__, __LINE__,
        "an unpaired surrogate converts the same way in System::String and wide text");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::Test_Overloads_ForwardAndShowTheUsualMessages()
{
    // Arrange
    TFixture_RTLStringComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    std::size_t const expectedCount = 68;
#else
    std::size_t const expectedCount = 65;
#endif
    CheckEquals(expectedCount, results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
        {
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__,
                record.TestName + " should pass: " + record.Message);
        }
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. The
    // prefix is "Check failed for: \"Test_<name>\"" for a Check, and "<category> Test_<name>" for an Assert.
    struct TExpectedFailure
    {
        std::string TestName;
        std::string Category; // Empty for a Check.
        std::string Detail;
    };
    std::vector<TExpectedFailure> const expectedFailures = {
        { "AssertContainsIC_Fails", "Substring not found:",
          "): Expected \"Hello World hello\" to contain \"xyz\" (ignoring case). deliberate failure" },
        { "AssertContains_Fails", "Substring not found:",
          "): Expected \"Hello World hello\" to contain \"WORLD\". deliberate failure" },
        { "AssertEndsWithIC_Fails", "Suffix not found:",
          "): Expected \"Hello World hello\" to end with \"World\" (ignoring case). deliberate failure" },
        { "AssertEndsWith_Fails", "Suffix not found:",
          "): Expected \"Hello World hello\" to end with \"Hello\". deliberate failure" },
        { "AssertEqualsIC_Fails", "Values not equal:",
          "): Expected: \"Hello\" but was \"Hello World hello\" (ignoring case). deliberate failure" },
        { "AssertEquals_Fails", "Values not equal:",
          "): Expected: \"hello world hello\" but was \"Hello World hello\". deliberate failure" },
        { "AssertNotContainsIC_Fails", "Substring found:",
          "): Expected \"Hello World hello\" not to contain \"LO WO\" (ignoring case). deliberate failure" },
        { "AssertNotContains_Fails", "Substring found:",
          "): Expected \"Hello World hello\" not to contain \"lo Wo\". deliberate failure" },
        { "AssertNotEndsWithIC_Fails", "Suffix found:",
          "): Expected \"Hello World hello\" not to end with \"HELLO\" (ignoring case). deliberate failure" },
        { "AssertNotEndsWith_Fails", "Suffix found:",
          "): Expected \"Hello World hello\" not to end with \"hello\". deliberate failure" },
        { "AssertNotEqualsIC_Fails", "Values are equal:",
          "): Values: \"HELLO WORLD HELLO\" and \"Hello World hello\" (ignoring case). deliberate failure" },
        { "AssertNotEquals_Fails", "Values are equal:", "): Value: \"Hello World hello\". deliberate failure" },
        { "AssertNotStartsWithIC_Fails", "Prefix found:",
          "): Expected \"Hello World hello\" not to start with \"HELLO\" (ignoring case). deliberate failure" },
        { "AssertNotStartsWith_Fails", "Prefix found:",
          "): Expected \"Hello World hello\" not to start with \"Hello\". deliberate failure" },
        { "AssertStartsWithIC_Fails", "Prefix not found:",
          "): Expected \"Hello World hello\" to start with \"World\" (ignoring case). deliberate failure" },
        { "AssertStartsWith_Fails", "Prefix not found:",
          "): Expected \"Hello World hello\" to start with \"hello\". deliberate failure" },
        { "CheckContainsIC_Fails", "",
          "): Expected \"Hello World hello\" to contain \"xyz\" (ignoring case). deliberate failure" },
        { "CheckContains_Fails", "", "): Expected \"Hello World hello\" to contain \"WORLD\". deliberate failure" },
        { "CheckContains_NonASCII_Fails", "",
          "): Expected \"caf\xC3\xA9 \xF0\x9F\x98\x80\" to contain \"\xC3\xBC\". not present" },
        { "CheckEndsWithIC_Fails", "",
          "): Expected \"Hello World hello\" to end with \"World\" (ignoring case). deliberate failure" },
        { "CheckEndsWith_Fails", "", "): Expected \"Hello World hello\" to end with \"Hello\". deliberate failure" },
        { "CheckEqualsIC_Fails", "",
          "): Expected \"Hello\" but was \"Hello World hello\" (ignoring case). deliberate failure" },
        { "CheckEquals_Fails", "",
          "): Expected \"hello world hello\" but was \"Hello World hello\". deliberate failure" },
        { "CheckNotContainsIC_Fails", "",
          "): Expected \"Hello World hello\" not to contain \"LO WO\" (ignoring case). deliberate failure" },
        { "CheckNotContains_Fails", "",
          "): Expected \"Hello World hello\" not to contain \"lo Wo\". deliberate failure" },
        { "CheckNotEndsWithIC_Fails", "",
          "): Expected \"Hello World hello\" not to end with \"HELLO\" (ignoring case). deliberate failure" },
        { "CheckNotEndsWith_Fails", "",
          "): Expected \"Hello World hello\" not to end with \"hello\". deliberate failure" },
        { "CheckNotEqualsIC_Fails", "",
          "): Both values equal: \"HELLO WORLD HELLO\" and \"Hello World hello\" (ignoring case). deliberate failure" },
        { "CheckNotEquals_Fails", "", "): Both values equal: \"Hello World hello\". deliberate failure" },
        { "CheckNotStartsWithIC_Fails", "",
          "): Expected \"Hello World hello\" not to start with \"HELLO\" (ignoring case). deliberate failure" },
        { "CheckNotStartsWith_Fails", "",
          "): Expected \"Hello World hello\" not to start with \"Hello\". deliberate failure" },
        { "CheckStartsWithIC_Fails", "",
          "): Expected \"Hello World hello\" to start with \"World\" (ignoring case). deliberate failure" },
        { "CheckStartsWith_Fails", "",
          "): Expected \"Hello World hello\" to start with \"hello\". deliberate failure" },
    };

    for (TExpectedFailure const& expected : expectedFailures)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, __func__, __LINE__, expected.TestName + " has a record");

        std::string const prefix = expected.Category.empty() ?
                "Check failed for: \"Test_" + expected.TestName + "\" (" :
                expected.Category + " Test_" + expected.TestName + " (";
        CheckStartsWith(record->Message, prefix, __func__, __LINE__, expected.TestName + " names the test");
        CheckEndsWith(record->Message, expected.Detail, __func__, __LINE__, expected.TestName + " shows the texts");
    }

#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    // function_name() is compiler-specific, but always contains the function's own name.
    TTestCaseRecord const* const sourceLocation = FindRecord(results, "SourceLocation_CheckEndsWith_Fails");
    AssertNotNull(sourceLocation, __func__, __LINE__, "the source location test has a record");
    CheckContains(sourceLocation->Message, "Test_SourceLocation_CheckEndsWith_Fails", __func__, __LINE__,
        "the source location form reports its caller's function");
    CheckContains(sourceLocation->Message, "(" + std::to_string(fixture.SourceLocationLine) + ")", __func__, __LINE__,
        "and the line of its call");
    CheckEndsWith(sourceLocation->Message,
        "): Expected \"Hello World hello\" to end with \"Hello\". deliberate failure", __func__, __LINE__,
        "and the texts");
#endif
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLStrings::Test_Overloads_RejectNonText()
{
    // Checked at compile time. System::String converts implicitly from numbers and characters, so without this, a
    // number would be compared with its text, and CheckEquals(1.0, 2.0, ...) would compile.
    static_assert(TCanCheckEquals<System::String, System::String>::value, "two System::Strings");
    static_assert(TCanCheckEquals<System::String, char const*>::value, "System::String and a C string");
    static_assert(TCanCheckEquals<wchar_t const*, System::String>::value, "a wide C string and System::String");
    static_assert(TCanCheckEquals<std::string, System::String>::value, "std::string and System::String");
    static_assert(TCanCheckContains<System::String, std::wstring>::value, "System::String and std::wstring");

    static_assert(!TCanCheckEquals<System::String, int>::value, "System::String and a number");
    static_assert(!TCanCheckEquals<double, System::String>::value, "a number and System::String");
    static_assert(!TCanCheckEquals<System::String, wchar_t>::value, "System::String and a character");
    static_assert(!TCanCheckEquals<double, double>::value, "two floating-point values, as without System::String");
    static_assert(!TCanCheckContains<System::String, int>::value, "System::String and a number");
    static_assert(!TCanCheckContains<std::string, std::wstring>::value, "narrow and wide text without System::String");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_RTLStrings)

#endif // #if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
