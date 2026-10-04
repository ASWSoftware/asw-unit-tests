/* **************************************************************************
Test_ASWUnitTests_TestBase.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

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
#include "Test_ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <map>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

size_t CountOccurrences(std::string const& text, std::string const& needle);
TTestCaseRecord const* FindRecord(TTestResults const& results, std::string const& testName);
bool NameEndsWith(std::string const& name, std::string const& suffix);

//---------------------------------------------------------------------------
// Number of non-overlapping occurrences of 'needle' in 'text'.
size_t CountOccurrences(std::string const& text, std::string const& needle)
{
    size_t count = 0;

    for (size_t pos = text.find(needle); pos != std::string::npos; pos = text.find(needle, pos + needle.size()))
        ++count;

    return count;
}

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
// registered test generically instead of hand-maintaining a separate expected-outcome table. Also
// used to check the end of a failure message.
bool NameEndsWith(std::string const& name, std::string const& suffix)
{
    return name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
}

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TRecordingObserver
//
// An ITestRunObserver that records everything it's told, for the Run_*RunObserver* tests below to
// inspect. Events are recorded as "started:<test>" and "finished:<test>" strings, in order. Every
// event except OnLog() must arrive on the thread that constructed this observer (the one calling
// Run()), which EventsOnConstructingThread tracks; OnLog() may legitimately arrive from a test's
// worker thread under a timeout, so it's recorded under a lock instead.
/////////////////////////////////////////////////////////////////////////////
class TRecordingObserver : public ITestRunObserver
{
private:
    std::mutex m_LogMutex;
    std::string m_LogText;
    std::thread::id const m_ConstructingThread;

public:
    std::vector<std::string> Events;
    bool EventsOnConstructingThread = true;
    std::vector<TTestCaseRecord> FinishedRecords;
    size_t StopAfterFinishedCount = std::numeric_limits<size_t>::max();
    size_t StopRequestedCalls = 0;

public:
    TRecordingObserver()
        : m_ConstructingThread(std::this_thread::get_id())
    {
    }

    std::string LogText()
    {
        std::lock_guard<std::mutex> lock(m_LogMutex);
        return m_LogText;
    }

    void OnLog(std::string const& text) override
    {
        std::lock_guard<std::mutex> lock(m_LogMutex);
        m_LogText += text;
    }

    void OnTestFinished(TTestCaseRecord const& record) override
    {
        EventsOnConstructingThread = EventsOnConstructingThread && (std::this_thread::get_id() == m_ConstructingThread);
        Events.push_back("finished:" + record.TestName);
        FinishedRecords.push_back(record);
    }

    void OnTestStarted(std::string const& /*groupName*/, std::string const& testName) override
    {
        EventsOnConstructingThread = EventsOnConstructingThread && (std::this_thread::get_id() == m_ConstructingThread);
        Events.push_back("started:" + testName);
    }

    bool StopRequested() override
    {
        EventsOnConstructingThread = EventsOnConstructingThread && (std::this_thread::get_id() == m_ConstructingThread);
        ++StopRequestedCalls;
        return FinishedRecords.size() >= StopAfterFinishedCount;
    }
};

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFixture_BoolComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one deliberately failing test
// per bool overload of AssertEquals()/CheckEquals()/AssertNotEquals()/CheckNotEquals(), so
// Test_Equals_ShowsBoolValuesAsTrueOrFalse below can check that every failure message shows the
// values as "true"/"false" rather than "1"/"0".
/////////////////////////////////////////////////////////////////////////////
class TFixture_BoolComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEquals_Different_Fails();
    void Test_AssertNotEquals_Same_Fails();
    void Test_CheckEquals_Different_Fails();
    void Test_CheckNotEquals_Same_Fails();

public:
    TFixture_BoolComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_BoolComparisons::TFixture_BoolComparisons()
    : inherited("Fixture_BoolComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_BoolComparisons::Test_AssertEquals_Different_Fails, "AssertEquals_Different_Fails");
    RegisterTest(&TFixture_BoolComparisons::Test_AssertNotEquals_Same_Fails, "AssertNotEquals_Same_Fails");
    RegisterTest(&TFixture_BoolComparisons::Test_CheckEquals_Different_Fails, "CheckEquals_Different_Fails");
    RegisterTest(&TFixture_BoolComparisons::Test_CheckNotEquals_Same_Fails, "CheckNotEquals_Same_Fails");
}
//---------------------------------------------------------------------------
void TFixture_BoolComparisons::Test_AssertEquals_Different_Fails()
{
    AssertEquals(true, false, __func__, __LINE__, "different values");
}
//---------------------------------------------------------------------------
void TFixture_BoolComparisons::Test_AssertNotEquals_Same_Fails()
{
    AssertNotEquals(true, true, __func__, __LINE__, "same value");
}
//---------------------------------------------------------------------------
void TFixture_BoolComparisons::Test_CheckEquals_Different_Fails()
{
    CheckEquals(true, false, __func__, __LINE__, "different values");
}
//---------------------------------------------------------------------------
void TFixture_BoolComparisons::Test_CheckNotEquals_Same_Fails()
{
    CheckNotEquals(true, true, __func__, __LINE__, "same value");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_CStringComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing the C string overloads of
// AssertEquals()/CheckEquals()/AssertNotEquals()/CheckNotEquals(), narrow and wide. Without those
// overloads, two C strings convert to bool and match the bool overload, so the "DifferentLiterals"
// tests pass/fail the wrong way round. The "SameContentDifferentBuffers" tests compare a literal with
// a separate array holding the same text, so they fail if the pointers are compared instead of the
// content (two identical literals may share one address). Test names self-document expected outcome
// via NameEndsWith(), same as TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_CStringComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEquals_DifferentLiterals_Fails();
    void Test_AssertEquals_NullAndNonNull_Fails();
    void Test_AssertEquals_SameContentDifferentBuffers_Passes();
    void Test_AssertEquals_Wide_DifferentLiterals_Fails();
    void Test_AssertEquals_Wide_SameContentDifferentBuffers_Passes();
    void Test_AssertNotEquals_DifferentLiterals_Passes();
    void Test_AssertNotEquals_SameContentDifferentBuffers_Fails();
    void Test_AssertNotEquals_Wide_DifferentLiterals_Passes();
    void Test_AssertNotEquals_Wide_SameContentDifferentBuffers_Fails();
    void Test_CheckEquals_BothNull_Passes();
    void Test_CheckEquals_DifferentLiterals_Fails();
    void Test_CheckEquals_EmptyAndNull_Fails();
    void Test_CheckEquals_NullAndEmpty_Fails();
    void Test_CheckEquals_SameContentDifferentBuffers_Passes();
    void Test_CheckEquals_Wide_BothNull_Passes();
    void Test_CheckEquals_Wide_DifferentLiterals_Fails();
    void Test_CheckEquals_Wide_NullAndEmpty_Fails();
    void Test_CheckEquals_Wide_SameContentDifferentBuffers_Passes();
    void Test_CheckNotEquals_BothNull_Fails();
    void Test_CheckNotEquals_DifferentLiterals_Passes();
    void Test_CheckNotEquals_NullAndEmpty_Passes();
    void Test_CheckNotEquals_SameContentDifferentBuffers_Fails();
    void Test_CheckNotEquals_Wide_BothNull_Fails();
    void Test_CheckNotEquals_Wide_DifferentLiterals_Passes();
    void Test_CheckNotEquals_Wide_NullAndEmpty_Passes();
    void Test_CheckNotEquals_Wide_SameContentDifferentBuffers_Fails();

public:
    TFixture_CStringComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_CStringComparisons::TFixture_CStringComparisons()
    : inherited("Fixture_CStringComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_CStringComparisons::Test_AssertEquals_DifferentLiterals_Fails,
        "AssertEquals_DifferentLiterals_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertEquals_NullAndNonNull_Fails,
        "AssertEquals_NullAndNonNull_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertEquals_SameContentDifferentBuffers_Passes,
        "AssertEquals_SameContentDifferentBuffers_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertEquals_Wide_DifferentLiterals_Fails,
        "AssertEquals_Wide_DifferentLiterals_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertEquals_Wide_SameContentDifferentBuffers_Passes,
        "AssertEquals_Wide_SameContentDifferentBuffers_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertNotEquals_DifferentLiterals_Passes,
        "AssertNotEquals_DifferentLiterals_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertNotEquals_SameContentDifferentBuffers_Fails,
        "AssertNotEquals_SameContentDifferentBuffers_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertNotEquals_Wide_DifferentLiterals_Passes,
        "AssertNotEquals_Wide_DifferentLiterals_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_AssertNotEquals_Wide_SameContentDifferentBuffers_Fails,
        "AssertNotEquals_Wide_SameContentDifferentBuffers_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_BothNull_Passes, "CheckEquals_BothNull_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_DifferentLiterals_Fails,
        "CheckEquals_DifferentLiterals_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_EmptyAndNull_Fails, "CheckEquals_EmptyAndNull_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_NullAndEmpty_Fails, "CheckEquals_NullAndEmpty_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_SameContentDifferentBuffers_Passes,
        "CheckEquals_SameContentDifferentBuffers_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_Wide_BothNull_Passes,
        "CheckEquals_Wide_BothNull_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_Wide_DifferentLiterals_Fails,
        "CheckEquals_Wide_DifferentLiterals_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_Wide_NullAndEmpty_Fails,
        "CheckEquals_Wide_NullAndEmpty_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckEquals_Wide_SameContentDifferentBuffers_Passes,
        "CheckEquals_Wide_SameContentDifferentBuffers_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_BothNull_Fails, "CheckNotEquals_BothNull_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_DifferentLiterals_Passes,
        "CheckNotEquals_DifferentLiterals_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_NullAndEmpty_Passes,
        "CheckNotEquals_NullAndEmpty_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_SameContentDifferentBuffers_Fails,
        "CheckNotEquals_SameContentDifferentBuffers_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_Wide_BothNull_Fails,
        "CheckNotEquals_Wide_BothNull_Fails");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_Wide_DifferentLiterals_Passes,
        "CheckNotEquals_Wide_DifferentLiterals_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_Wide_NullAndEmpty_Passes,
        "CheckNotEquals_Wide_NullAndEmpty_Passes");
    RegisterTest(&TFixture_CStringComparisons::Test_CheckNotEquals_Wide_SameContentDifferentBuffers_Fails,
        "CheckNotEquals_Wide_SameContentDifferentBuffers_Fails");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertEquals_DifferentLiterals_Fails()
{
    AssertEquals("abc", "xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertEquals_NullAndNonNull_Fails()
{
    char const* const nullStr = nullptr;
    AssertEquals("abc", nullStr, __func__, __LINE__, "null never matches a non-null string");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertEquals_SameContentDifferentBuffers_Passes()
{
    char const buffer[] = "abc";
    AssertEquals("abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertEquals_Wide_DifferentLiterals_Fails()
{
    AssertEquals(L"abc", L"xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertEquals_Wide_SameContentDifferentBuffers_Passes()
{
    wchar_t const buffer[] = L"abc";
    AssertEquals(L"abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertNotEquals_DifferentLiterals_Passes()
{
    AssertNotEquals("abc", "xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertNotEquals_SameContentDifferentBuffers_Fails()
{
    char const buffer[] = "abc";
    AssertNotEquals("abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertNotEquals_Wide_DifferentLiterals_Passes()
{
    AssertNotEquals(L"abc", L"xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_AssertNotEquals_Wide_SameContentDifferentBuffers_Fails()
{
    wchar_t const buffer[] = L"abc";
    AssertNotEquals(L"abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_BothNull_Passes()
{
    char const* const nullStr = nullptr;
    CheckEquals(nullStr, nullStr, __func__, __LINE__, "two nulls match");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_DifferentLiterals_Fails()
{
    CheckEquals("abc", "xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_EmptyAndNull_Fails()
{
    char const* const nullStr = nullptr;
    CheckEquals("", nullStr, __func__, __LINE__, "null never matches a non-null string, even an empty one");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_NullAndEmpty_Fails()
{
    char const* const nullStr = nullptr;
    CheckEquals(nullStr, "", __func__, __LINE__, "null never matches a non-null string, even an empty one");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_SameContentDifferentBuffers_Passes()
{
    char const buffer[] = "abc";
    CheckEquals("abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_Wide_BothNull_Passes()
{
    wchar_t const* const nullStr = nullptr;
    CheckEquals(nullStr, nullStr, __func__, __LINE__, "two nulls match");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_Wide_DifferentLiterals_Fails()
{
    CheckEquals(L"abc", L"xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_Wide_NullAndEmpty_Fails()
{
    wchar_t const* const nullStr = nullptr;
    CheckEquals(nullStr, L"", __func__, __LINE__, "null never matches a non-null string, even an empty one");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckEquals_Wide_SameContentDifferentBuffers_Passes()
{
    wchar_t const buffer[] = L"abc";
    CheckEquals(L"abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_BothNull_Fails()
{
    char const* const nullStr = nullptr;
    CheckNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_DifferentLiterals_Passes()
{
    CheckNotEquals("abc", "xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_NullAndEmpty_Passes()
{
    char const* const nullStr = nullptr;
    CheckNotEquals(nullStr, "", __func__, __LINE__, "null differs from a non-null string, even an empty one");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_SameContentDifferentBuffers_Fails()
{
    char const buffer[] = "abc";
    CheckNotEquals("abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_Wide_BothNull_Fails()
{
    wchar_t const* const nullStr = nullptr;
    CheckNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_Wide_DifferentLiterals_Passes()
{
    CheckNotEquals(L"abc", L"xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_Wide_NullAndEmpty_Passes()
{
    wchar_t const* const nullStr = nullptr;
    CheckNotEquals(nullStr, L"", __func__, __LINE__, "null differs from a non-null string, even an empty one");
}
//---------------------------------------------------------------------------
void TFixture_CStringComparisons::Test_CheckNotEquals_Wide_SameContentDifferentBuffers_Fails()
{
    wchar_t const buffer[] = L"abc";
    CheckNotEquals(L"abc", buffer, __func__, __LINE__, "same text in a separate buffer");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ContainsComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertContains()/CheckContains()/
// AssertNotContains()/CheckNotContains(), narrow and wide. "Wide_NonASCII" checks that a wide failure shows
// its text as UTF-8. Test names self-document expected outcome via NameEndsWith(), same as
// TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ContainsComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertContains_Absent_Fails();
    void Test_AssertContains_Present_Passes();
    void Test_AssertContains_Wide_Absent_Fails();
    void Test_AssertContains_Wide_Present_Passes();
    void Test_AssertNotContains_Absent_Passes();
    void Test_AssertNotContains_Present_Fails();
    void Test_AssertNotContains_Wide_Absent_Passes();
    void Test_AssertNotContains_Wide_Present_Fails();
    void Test_CheckContains_Absent_Fails();
    void Test_CheckContains_DifferentCase_Fails();
    void Test_CheckContains_EmptySubstring_Passes();
    void Test_CheckContains_Present_Passes();
    void Test_CheckContains_SubstringLongerThanText_Fails();
    void Test_CheckContains_Wide_Absent_Fails();
    void Test_CheckContains_Wide_NonASCII_Fails();
    void Test_CheckContains_Wide_Present_Passes();
    void Test_CheckNotContains_Absent_Passes();
    void Test_CheckNotContains_EmptySubstring_Fails();
    void Test_CheckNotContains_Present_Fails();
    void Test_CheckNotContains_Wide_Absent_Passes();
    void Test_CheckNotContains_Wide_Present_Fails();

public:
    TFixture_ContainsComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_ContainsComparisons::TFixture_ContainsComparisons()
    : inherited("Fixture_ContainsComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_ContainsComparisons::Test_AssertContains_Absent_Fails, "AssertContains_Absent_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertContains_Present_Passes, "AssertContains_Present_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertContains_Wide_Absent_Fails,
        "AssertContains_Wide_Absent_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertContains_Wide_Present_Passes,
        "AssertContains_Wide_Present_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertNotContains_Absent_Passes,
        "AssertNotContains_Absent_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertNotContains_Present_Fails,
        "AssertNotContains_Present_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertNotContains_Wide_Absent_Passes,
        "AssertNotContains_Wide_Absent_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_AssertNotContains_Wide_Present_Fails,
        "AssertNotContains_Wide_Present_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_Absent_Fails, "CheckContains_Absent_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_DifferentCase_Fails,
        "CheckContains_DifferentCase_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_EmptySubstring_Passes,
        "CheckContains_EmptySubstring_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_Present_Passes, "CheckContains_Present_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_SubstringLongerThanText_Fails,
        "CheckContains_SubstringLongerThanText_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_Wide_Absent_Fails,
        "CheckContains_Wide_Absent_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_Wide_NonASCII_Fails,
        "CheckContains_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckContains_Wide_Present_Passes,
        "CheckContains_Wide_Present_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckNotContains_Absent_Passes,
        "CheckNotContains_Absent_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckNotContains_EmptySubstring_Fails,
        "CheckNotContains_EmptySubstring_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckNotContains_Present_Fails,
        "CheckNotContains_Present_Fails");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckNotContains_Wide_Absent_Passes,
        "CheckNotContains_Wide_Absent_Passes");
    RegisterTest(&TFixture_ContainsComparisons::Test_CheckNotContains_Wide_Present_Fails,
        "CheckNotContains_Wide_Present_Fails");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertContains_Absent_Fails()
{
    AssertContains(std::string("hello world"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertContains_Present_Passes()
{
    AssertContains(std::string("hello world"), "lo wo", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertContains_Wide_Absent_Fails()
{
    AssertContains(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertContains_Wide_Present_Passes()
{
    AssertContains(std::wstring(L"hello world"), L"lo wo", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertNotContains_Absent_Passes()
{
    AssertNotContains(std::string("hello world"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertNotContains_Present_Fails()
{
    AssertNotContains(std::string("hello world"), "world", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertNotContains_Wide_Absent_Passes()
{
    AssertNotContains(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_AssertNotContains_Wide_Present_Fails()
{
    AssertNotContains(std::wstring(L"hello world"), L"world", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_Absent_Fails()
{
    CheckContains(std::string("hello world"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_DifferentCase_Fails()
{
    CheckContains(std::string("hello world"), "World", __func__, __LINE__, "the comparison is case-sensitive");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_EmptySubstring_Passes()
{
    CheckContains(std::string("hello world"), "", __func__, __LINE__, "every string contains the empty string");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_Present_Passes()
{
    CheckContains(std::string("hello world"), "hello world", __func__, __LINE__, "the whole text is a substring");
    CheckContains(std::string("hello world"), "lo wo", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_SubstringLongerThanText_Fails()
{
    CheckContains(std::string("hello"), "hello world", __func__, __LINE__, "the text contains the start only");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_Wide_Absent_Fails()
{
    CheckContains(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_Wide_NonASCII_Fails()
{
    // "cafe" with an e-acute, a space, and U+1F600 (a surrogate pair where wchar_t is 16 bits), then u-umlaut.
    CheckContains(std::wstring(L"caf" L"\x00E9" L" \U0001F600"), L"\x00FC", __func__, __LINE__, "not present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckContains_Wide_Present_Passes()
{
    CheckContains(std::wstring(L"hello world"), L"lo wo", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckNotContains_Absent_Passes()
{
    CheckNotContains(std::string("hello world"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckNotContains_EmptySubstring_Fails()
{
    CheckNotContains(std::string("hello world"), "", __func__, __LINE__, "every string contains the empty string");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckNotContains_Present_Fails()
{
    CheckNotContains(std::string("hello world"), "world", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckNotContains_Wide_Absent_Passes()
{
    CheckNotContains(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsComparisons::Test_CheckNotContains_Wide_Present_Fails()
{
    CheckNotContains(std::wstring(L"hello world"), L"world", __func__, __LINE__, "substring present");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ContainsICComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertContainsIC()/CheckContainsIC()/
// AssertNotContainsIC()/CheckNotContainsIC(), narrow and wide. Only the ASCII letters A-Z are case-folded: the
// "NonLetters" tests use characters 0x20 apart, which a fold that just sets bit 0x20 would wrongly match, and the
// "NonASCII" tests show that accented letters keep their case. Test names self-document expected outcome via
// NameEndsWith(), same as TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ContainsICComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertContainsIC_Absent_Fails();
    void Test_AssertContainsIC_DifferentCase_Passes();
    void Test_AssertContainsIC_Wide_Absent_Fails();
    void Test_AssertContainsIC_Wide_DifferentCase_Passes();
    void Test_AssertNotContainsIC_Absent_Passes();
    void Test_AssertNotContainsIC_DifferentCase_Fails();
    void Test_AssertNotContainsIC_Wide_Absent_Passes();
    void Test_AssertNotContainsIC_Wide_DifferentCase_Fails();
    void Test_CheckContainsIC_Absent_Fails();
    void Test_CheckContainsIC_BothEmpty_Passes();
    void Test_CheckContainsIC_DifferentCase_Passes();
    void Test_CheckContainsIC_EmptySubstring_Passes();
    void Test_CheckContainsIC_EmptyText_Fails();
    void Test_CheckContainsIC_NonASCII_Fails();
    void Test_CheckContainsIC_NonLetters_Fails();
    void Test_CheckContainsIC_SubstringAtEnd_Passes();
    void Test_CheckContainsIC_SubstringLongerThanText_Fails();
    void Test_CheckContainsIC_Wide_Absent_Fails();
    void Test_CheckContainsIC_Wide_DifferentCase_Passes();
    void Test_CheckContainsIC_Wide_NonASCII_Fails();
    void Test_CheckNotContainsIC_Absent_Passes();
    void Test_CheckNotContainsIC_DifferentCase_Fails();
    void Test_CheckNotContainsIC_EmptySubstring_Fails();
    void Test_CheckNotContainsIC_Wide_Absent_Passes();
    void Test_CheckNotContainsIC_Wide_DifferentCase_Fails();

public:
    TFixture_ContainsICComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_ContainsICComparisons::TFixture_ContainsICComparisons()
    : inherited("Fixture_ContainsICComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertContainsIC_Absent_Fails, "AssertContainsIC_Absent_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertContainsIC_DifferentCase_Passes,
        "AssertContainsIC_DifferentCase_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertContainsIC_Wide_Absent_Fails,
        "AssertContainsIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertContainsIC_Wide_DifferentCase_Passes,
        "AssertContainsIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Absent_Passes,
        "AssertNotContainsIC_Absent_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertNotContainsIC_DifferentCase_Fails,
        "AssertNotContainsIC_DifferentCase_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Wide_Absent_Passes,
        "AssertNotContainsIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Wide_DifferentCase_Fails,
        "AssertNotContainsIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_Absent_Fails, "CheckContainsIC_Absent_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_BothEmpty_Passes,
        "CheckContainsIC_BothEmpty_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_DifferentCase_Passes,
        "CheckContainsIC_DifferentCase_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_EmptySubstring_Passes,
        "CheckContainsIC_EmptySubstring_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_EmptyText_Fails,
        "CheckContainsIC_EmptyText_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_NonASCII_Fails,
        "CheckContainsIC_NonASCII_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_NonLetters_Fails,
        "CheckContainsIC_NonLetters_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_SubstringAtEnd_Passes,
        "CheckContainsIC_SubstringAtEnd_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_SubstringLongerThanText_Fails,
        "CheckContainsIC_SubstringLongerThanText_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_Absent_Fails,
        "CheckContainsIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_DifferentCase_Passes,
        "CheckContainsIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_NonASCII_Fails,
        "CheckContainsIC_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Absent_Passes,
        "CheckNotContainsIC_Absent_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckNotContainsIC_DifferentCase_Fails,
        "CheckNotContainsIC_DifferentCase_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckNotContainsIC_EmptySubstring_Fails,
        "CheckNotContainsIC_EmptySubstring_Fails");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Wide_Absent_Passes,
        "CheckNotContainsIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Wide_DifferentCase_Fails,
        "CheckNotContainsIC_Wide_DifferentCase_Fails");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertContainsIC_Absent_Fails()
{
    AssertContainsIC(std::string("Hello World"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertContainsIC_DifferentCase_Passes()
{
    AssertContainsIC(std::string("Hello World"), "lO wO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertContainsIC_Wide_Absent_Fails()
{
    AssertContainsIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertContainsIC_Wide_DifferentCase_Passes()
{
    AssertContainsIC(std::wstring(L"Hello World"), L"lO wO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Absent_Passes()
{
    AssertNotContainsIC(std::string("Hello World"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertNotContainsIC_DifferentCase_Fails()
{
    AssertNotContainsIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Wide_Absent_Passes()
{
    AssertNotContainsIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_AssertNotContainsIC_Wide_DifferentCase_Fails()
{
    AssertNotContainsIC(std::wstring(L"Hello World"), L"WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_Absent_Fails()
{
    CheckContainsIC(std::string("Hello World"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_BothEmpty_Passes()
{
    CheckContainsIC(std::string(), "", __func__, __LINE__, "even an empty string contains the empty string");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_DifferentCase_Passes()
{
    CheckContainsIC(std::string("Hello World"), "lO wO", __func__, __LINE__, "case differs");
    CheckContainsIC(std::string("Hello World"), "hello world", __func__, __LINE__, "the whole text, case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_EmptySubstring_Passes()
{
    CheckContainsIC(std::string("Hello World"), "", __func__, __LINE__, "every string contains the empty string");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_EmptyText_Fails()
{
    CheckContainsIC(std::string(), "a", __func__, __LINE__, "an empty string contains no letters");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_NonASCII_Fails()
{
    // UTF-8 E-acute and e-acute.
    CheckContainsIC(std::string("caf\xC3\x89"), "caf\xC3\xA9", __func__, __LINE__, "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_NonLetters_Fails()
{
    CheckContainsIC(std::string("a@b[c"), "a`b{c", __func__, __LINE__, "@ and `, and [ and {, are not letters");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_SubstringAtEnd_Passes()
{
    CheckContainsIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "case differs, at the end");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_SubstringLongerThanText_Fails()
{
    CheckContainsIC(std::string("Hello"), "HELLO WORLD", __func__, __LINE__, "the text contains the start only");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_Absent_Fails()
{
    CheckContainsIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_DifferentCase_Passes()
{
    CheckContainsIC(std::wstring(L"Hello World"), L"lO wO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckContainsIC_Wide_NonASCII_Fails()
{
    // E-acute and e-acute.
    CheckContainsIC(std::wstring(L"caf" L"\x00C9"), L"caf" L"\x00E9", __func__, __LINE__, "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Absent_Passes()
{
    CheckNotContainsIC(std::string("Hello World"), "xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckNotContainsIC_DifferentCase_Fails()
{
    CheckNotContainsIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckNotContainsIC_EmptySubstring_Fails()
{
    CheckNotContainsIC(std::string("Hello World"), "", __func__, __LINE__, "every string contains the empty string");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Wide_Absent_Passes()
{
    CheckNotContainsIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "substring absent");
}
//---------------------------------------------------------------------------
void TFixture_ContainsICComparisons::Test_CheckNotContainsIC_Wide_DifferentCase_Fails()
{
    CheckNotContainsIC(std::wstring(L"Hello World"), L"WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_EqualsICComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEqualsIC()/CheckEqualsIC()/
// AssertNotEqualsIC()/CheckNotEqualsIC(), narrow and wide. Only the ASCII letters A-Z are case-folded, as in
// TFixture_ContainsICComparisons above. "DifferentLength" catches a comparison that stops at the end of the shorter
// value. Test names self-document expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_EqualsICComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEqualsIC_DifferentCase_Passes();
    void Test_AssertEqualsIC_DifferentText_Fails();
    void Test_AssertEqualsIC_Wide_DifferentCase_Passes();
    void Test_AssertEqualsIC_Wide_DifferentText_Fails();
    void Test_AssertNotEqualsIC_DifferentCase_Fails();
    void Test_AssertNotEqualsIC_DifferentText_Passes();
    void Test_AssertNotEqualsIC_Wide_DifferentCase_Fails();
    void Test_AssertNotEqualsIC_Wide_DifferentText_Passes();
    void Test_CheckEqualsIC_BothEmpty_Passes();
    void Test_CheckEqualsIC_DifferentCase_Passes();
    void Test_CheckEqualsIC_DifferentLength_Fails();
    void Test_CheckEqualsIC_DifferentText_Fails();
    void Test_CheckEqualsIC_EmptyAndNonEmpty_Fails();
    void Test_CheckEqualsIC_NonASCII_Fails();
    void Test_CheckEqualsIC_NonLetters_Fails();
    void Test_CheckEqualsIC_Wide_DifferentCase_Passes();
    void Test_CheckEqualsIC_Wide_DifferentText_Fails();
    void Test_CheckEqualsIC_Wide_NonASCII_Fails();
    void Test_CheckNotEqualsIC_DifferentCase_Fails();
    void Test_CheckNotEqualsIC_DifferentText_Passes();
    void Test_CheckNotEqualsIC_SameText_Fails();
    void Test_CheckNotEqualsIC_Wide_DifferentCase_Fails();
    void Test_CheckNotEqualsIC_Wide_DifferentText_Passes();

public:
    TFixture_EqualsICComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_EqualsICComparisons::TFixture_EqualsICComparisons()
    : inherited("Fixture_EqualsICComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertEqualsIC_DifferentCase_Passes,
        "AssertEqualsIC_DifferentCase_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertEqualsIC_DifferentText_Fails,
        "AssertEqualsIC_DifferentText_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertEqualsIC_Wide_DifferentCase_Passes,
        "AssertEqualsIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertEqualsIC_Wide_DifferentText_Fails,
        "AssertEqualsIC_Wide_DifferentText_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_DifferentCase_Fails,
        "AssertNotEqualsIC_DifferentCase_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_DifferentText_Passes,
        "AssertNotEqualsIC_DifferentText_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_Wide_DifferentCase_Fails,
        "AssertNotEqualsIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_Wide_DifferentText_Passes,
        "AssertNotEqualsIC_Wide_DifferentText_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_BothEmpty_Passes, "CheckEqualsIC_BothEmpty_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentCase_Passes,
        "CheckEqualsIC_DifferentCase_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentLength_Fails,
        "CheckEqualsIC_DifferentLength_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentText_Fails,
        "CheckEqualsIC_DifferentText_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_EmptyAndNonEmpty_Fails,
        "CheckEqualsIC_EmptyAndNonEmpty_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_NonASCII_Fails, "CheckEqualsIC_NonASCII_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_NonLetters_Fails, "CheckEqualsIC_NonLetters_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_DifferentCase_Passes,
        "CheckEqualsIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_DifferentText_Fails,
        "CheckEqualsIC_Wide_DifferentText_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_NonASCII_Fails,
        "CheckEqualsIC_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_DifferentCase_Fails,
        "CheckNotEqualsIC_DifferentCase_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_DifferentText_Passes,
        "CheckNotEqualsIC_DifferentText_Passes");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_SameText_Fails,
        "CheckNotEqualsIC_SameText_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_Wide_DifferentCase_Fails,
        "CheckNotEqualsIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_Wide_DifferentText_Passes,
        "CheckNotEqualsIC_Wide_DifferentText_Passes");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertEqualsIC_DifferentCase_Passes()
{
    AssertEqualsIC(std::string("Hello World"), std::string("hELLO wORLD"), __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertEqualsIC_DifferentText_Fails()
{
    AssertEqualsIC(std::string("Hello"), std::string("Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertEqualsIC_Wide_DifferentCase_Passes()
{
    AssertEqualsIC(std::wstring(L"Hello World"), std::wstring(L"hELLO wORLD"), __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertEqualsIC_Wide_DifferentText_Fails()
{
    AssertEqualsIC(std::wstring(L"Hello"), std::wstring(L"Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_DifferentCase_Fails()
{
    AssertNotEqualsIC(std::string("Hello World"), std::string("hELLO wORLD"), __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_DifferentText_Passes()
{
    AssertNotEqualsIC(std::string("Hello"), std::string("Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_Wide_DifferentCase_Fails()
{
    AssertNotEqualsIC(std::wstring(L"Hello World"), std::wstring(L"hELLO wORLD"), __func__, __LINE__,
        "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_AssertNotEqualsIC_Wide_DifferentText_Passes()
{
    AssertNotEqualsIC(std::wstring(L"Hello"), std::wstring(L"Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_BothEmpty_Passes()
{
    CheckEqualsIC(std::string(), std::string(), __func__, __LINE__, "two empty strings are equal");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentCase_Passes()
{
    CheckEqualsIC(std::string("Hello World"), std::string("hELLO wORLD"), __func__, __LINE__, "case differs");
    CheckEqualsIC(std::string("Hello World"), std::string("Hello World"), __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentLength_Fails()
{
    CheckEqualsIC(std::string("Hello"), std::string("HELLO WORLD"), __func__, __LINE__, "only the start matches");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_DifferentText_Fails()
{
    CheckEqualsIC(std::string("Hello"), std::string("Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_EmptyAndNonEmpty_Fails()
{
    CheckEqualsIC(std::string(), std::string("a"), __func__, __LINE__, "an empty string differs from a letter");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_NonASCII_Fails()
{
    // UTF-8 E-acute and e-acute.
    CheckEqualsIC(std::string("caf\xC3\x89"), std::string("caf\xC3\xA9"), __func__, __LINE__,
        "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_NonLetters_Fails()
{
    CheckEqualsIC(std::string("a@b[c"), std::string("a`b{c"), __func__, __LINE__,
        "@ and `, and [ and {, are not letters");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_DifferentCase_Passes()
{
    CheckEqualsIC(std::wstring(L"Hello World"), std::wstring(L"hELLO wORLD"), __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_DifferentText_Fails()
{
    CheckEqualsIC(std::wstring(L"Hello"), std::wstring(L"Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckEqualsIC_Wide_NonASCII_Fails()
{
    // E-acute and e-acute.
    CheckEqualsIC(std::wstring(L"caf" L"\x00C9"), std::wstring(L"caf" L"\x00E9"), __func__, __LINE__,
        "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_DifferentCase_Fails()
{
    CheckNotEqualsIC(std::string("Hello World"), std::string("hELLO wORLD"), __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_DifferentText_Passes()
{
    CheckNotEqualsIC(std::string("Hello"), std::string("Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_SameText_Fails()
{
    CheckNotEqualsIC(std::string("Hello"), std::string("Hello"), __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_Wide_DifferentCase_Fails()
{
    CheckNotEqualsIC(std::wstring(L"Hello World"), std::wstring(L"hELLO wORLD"), __func__, __LINE__,
        "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EqualsICComparisons::Test_CheckNotEqualsIC_Wide_DifferentText_Passes()
{
    CheckNotEqualsIC(std::wstring(L"Hello"), std::wstring(L"Help"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixtureSpecificError
//
// Unrelated (sibling) exception type for TFixture_ExceptionExpectations
// below, so a "wrong type" scenario can throw a real type that isn't just a
// differently-worded same class.
/////////////////////////////////////////////////////////////////////////////
class TFixtureSpecificError : public std::runtime_error
{
public:
    explicit TFixtureSpecificError(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};


/////////////////////////////////////////////////////////////////////////////
// TFixtureOtherError
//
// Unrelated (sibling) exception type for TFixture_ExceptionExpectations
// below, so a "wrong type" scenario can throw a real type that isn't just a
// differently-worded same class.
/////////////////////////////////////////////////////////////////////////////
class TFixtureOtherError : public std::runtime_error
{
public:
    explicit TFixtureOtherError(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_ExceptionExpectations
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test method per
// SetExceptionExpected()/Test() branch: no throw, a generic expectation satisfied by either a
// std::exception or a non-std::exception throw, and a specific-type expectation crossed with an
// exact type match, a polymorphic base-type match, a wrong sibling type, a non-std::exception
// throw, and a message substring that's present or absent; plus an Assert* failure while a generic
// or specific exception is expected, and an earlier Check* failure followed by the expected
// exception, on each path that would otherwise pass. Every test name ends with "_Passes" or
// "_Fails", read generically by Test_SetExceptionExpected_MatchesTypeAndMessage below via
// NameEndsWith() rather than a hand-maintained table.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ExceptionExpectations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_CheckFailedThenAssertFailedWhileExceptionExpected_Fails();
    void Test_CheckFailedThenGenericExpectedExceptionThrown_Fails();
    void Test_CheckFailedThenNonStdExpectedExceptionThrown_Fails();
    void Test_CheckFailedThenSpecificExpectedExceptionThrown_Fails();
    void Test_ExceptionExpectedButNoneThrown_Fails();
    void Test_GenericExceptionExpected_AssertFails_Fails();
    void Test_GenericExceptionExpected_NonStdExceptionThrown_Passes();
    void Test_GenericExceptionExpected_StdExceptionThrown_Passes();
    void Test_SpecificTypeExpected_AssertFails_Fails();
    void Test_SpecificTypeExpected_ExactTypeThrown_Passes();
    void Test_SpecificTypeExpected_MessageSubstringAbsent_Fails();
    void Test_SpecificTypeExpected_MessageSubstringPresent_Passes();
    void Test_SpecificTypeExpected_NonStdExceptionThrown_Fails();
    void Test_SpecificTypeExpected_PolymorphicBaseTypeThrown_Passes();
    void Test_SpecificTypeExpected_WrongSiblingTypeThrown_Fails();

public:
    TFixture_ExceptionExpectations();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_ExceptionExpectations::TFixture_ExceptionExpectations()
    : inherited("Fixture_ExceptionExpectations")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_ExceptionExpectations::Test_CheckFailedThenAssertFailedWhileExceptionExpected_Fails,
        "CheckFailedThenAssertFailedWhileExceptionExpected_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_CheckFailedThenGenericExpectedExceptionThrown_Fails,
        "CheckFailedThenGenericExpectedExceptionThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_CheckFailedThenNonStdExpectedExceptionThrown_Fails,
        "CheckFailedThenNonStdExpectedExceptionThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_CheckFailedThenSpecificExpectedExceptionThrown_Fails,
        "CheckFailedThenSpecificExpectedExceptionThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails,
        "ExceptionExpectedButNoneThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_GenericExceptionExpected_AssertFails_Fails,
        "GenericExceptionExpected_AssertFails_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_GenericExceptionExpected_NonStdExceptionThrown_Passes,
        "GenericExceptionExpected_NonStdExceptionThrown_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_GenericExceptionExpected_StdExceptionThrown_Passes,
        "GenericExceptionExpected_StdExceptionThrown_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_AssertFails_Fails,
        "SpecificTypeExpected_AssertFails_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_ExactTypeThrown_Passes,
        "SpecificTypeExpected_ExactTypeThrown_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_MessageSubstringAbsent_Fails,
        "SpecificTypeExpected_MessageSubstringAbsent_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_MessageSubstringPresent_Passes,
        "SpecificTypeExpected_MessageSubstringPresent_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_NonStdExceptionThrown_Fails,
        "SpecificTypeExpected_NonStdExceptionThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_PolymorphicBaseTypeThrown_Passes,
        "SpecificTypeExpected_PolymorphicBaseTypeThrown_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_SpecificTypeExpected_WrongSiblingTypeThrown_Fails,
        "SpecificTypeExpected_WrongSiblingTypeThrown_Fails");
}
//---------------------------------------------------------------------------
// The next four each fail a Check* and then let an exception arrive while one is expected. A std::exception
// matching a generic or a specific expectation, and a non-std::exception object, are each a way Test() can
// otherwise reach a pass, and the earlier Check* failure must still fail the test on every one of them. An
// Assert* failure never counts as the expected exception; that case checks both failures reach the record.
void TFixture_ExceptionExpectations::Test_CheckFailedThenAssertFailedWhileExceptionExpected_Fails()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, then a failed Check and a failed Assert");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    AssertTrue(false, __func__, __LINE__, "deliberate Assert failure while an exception is expected");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_CheckFailedThenGenericExpectedExceptionThrown_Fails()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, then a failed Check");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    throw std::runtime_error("the expected exception");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_CheckFailedThenNonStdExpectedExceptionThrown_Fails()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, then a failed Check");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    throw 42; // Not derived from std::exception at all.
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_CheckFailedThenSpecificExpectedExceptionThrown_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "specific expectation, then a failed Check");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    throw TFixtureSpecificError("the expected exception");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "expected an exception that never comes");
    // No throw here: Test() should fail this once control returns without one.
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_GenericExceptionExpected_AssertFails_Fails()
{
    // An Assert* failure is a test failure, never the exception a test is waiting for.
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, then a failed Assert");
    AssertTrue(false, __func__, __LINE__, "deliberate Assert failure while an exception is expected");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_GenericExceptionExpected_NonStdExceptionThrown_Passes()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, non-std::exception thrown");
    throw 42; // Not derived from std::exception at all.
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_GenericExceptionExpected_StdExceptionThrown_Passes()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, any std::exception");
    throw std::runtime_error("whatever");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_AssertFails_Fails()
{
    // Used to pass without the requested type ever being checked.
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "specific expectation, then a failed Assert");
    AssertEquals(1, 2, __func__, __LINE__, "deliberate Assert failure while an exception is expected");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_ExactTypeThrown_Passes()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "exact type match");
    throw TFixtureSpecificError("boom");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_MessageSubstringAbsent_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "message must contain 'needle'", "needle");
    throw TFixtureSpecificError("no match here");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_MessageSubstringPresent_Passes()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "message must contain 'needle'", "needle");
    throw TFixtureSpecificError("hay needle stack");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_NonStdExceptionThrown_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(
        __func__, __LINE__, "specific type expected, non-std::exception thrown");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_PolymorphicBaseTypeThrown_Passes()
{
    SetExceptionExpected<std::runtime_error>(__func__, __LINE__, "base type expected, derived type thrown");
    throw TFixtureSpecificError("boom");
}
//---------------------------------------------------------------------------
void TFixture_ExceptionExpectations::Test_SpecificTypeExpected_WrongSiblingTypeThrown_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "wrong sibling type thrown");
    throw TFixtureOtherError("boom");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_IntegerComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing the integer template overloads
// of AssertEquals()/CheckEquals()/AssertNotEquals()/CheckNotEquals(), for two integer types that no
// fixed-width overload matches exactly. Without those overloads, most of these calls don't compile
// (ambiguous). The "NegativeAndUnsignedMax" tests catch a comparison that lets -1 wrap around to an
// unsigned maximum, and the "Int64Max" tests check the int64_t/uint64_t boundary. Test names
// self-document expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_IntegerComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEquals_IntAndInt64Different_Fails();
    void Test_AssertEquals_IntAndInt64Equal_Passes();
    void Test_AssertNotEquals_IntAndInt64Equal_Fails();
    void Test_AssertNotEquals_NegativeAndUnsignedMax_Passes();
    void Test_CheckEquals_AboveInt64MaxAndInt64Max_Fails();
    void Test_CheckEquals_Int64MaxAndUint64_Passes();
    void Test_CheckEquals_IntAndInt64Different_Fails();
    void Test_CheckEquals_IntAndInt64Equal_Passes();
    void Test_CheckEquals_IntAndShort_Passes();
    void Test_CheckEquals_IntAndUnsigned_Passes();
    void Test_CheckEquals_LongAndLong_Passes();
    void Test_CheckEquals_LongLongAndLongLong_Passes();
    void Test_CheckEquals_NegativeAndUint64Max_Fails();
    void Test_CheckEquals_NegativeAndUnsignedMax_Fails();
    void Test_CheckEquals_UnsignedLongAndUnsignedLong_Passes();
    void Test_CheckNotEquals_IntAndInt64Equal_Fails();
    void Test_CheckNotEquals_NegativeAndUint64Max_Passes();
    void Test_CheckNotEquals_NegativeAndUnsignedMax_Passes();

public:
    TFixture_IntegerComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_IntegerComparisons::TFixture_IntegerComparisons()
    : inherited("Fixture_IntegerComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_IntegerComparisons::Test_AssertEquals_IntAndInt64Different_Fails,
        "AssertEquals_IntAndInt64Different_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_AssertEquals_IntAndInt64Equal_Passes,
        "AssertEquals_IntAndInt64Equal_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_AssertNotEquals_IntAndInt64Equal_Fails,
        "AssertNotEquals_IntAndInt64Equal_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_AssertNotEquals_NegativeAndUnsignedMax_Passes,
        "AssertNotEquals_NegativeAndUnsignedMax_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_AboveInt64MaxAndInt64Max_Fails,
        "CheckEquals_AboveInt64MaxAndInt64Max_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_Int64MaxAndUint64_Passes,
        "CheckEquals_Int64MaxAndUint64_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_IntAndInt64Different_Fails,
        "CheckEquals_IntAndInt64Different_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_IntAndInt64Equal_Passes,
        "CheckEquals_IntAndInt64Equal_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_IntAndShort_Passes, "CheckEquals_IntAndShort_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_IntAndUnsigned_Passes,
        "CheckEquals_IntAndUnsigned_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_LongAndLong_Passes, "CheckEquals_LongAndLong_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_LongLongAndLongLong_Passes,
        "CheckEquals_LongLongAndLongLong_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_NegativeAndUint64Max_Fails,
        "CheckEquals_NegativeAndUint64Max_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_NegativeAndUnsignedMax_Fails,
        "CheckEquals_NegativeAndUnsignedMax_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckEquals_UnsignedLongAndUnsignedLong_Passes,
        "CheckEquals_UnsignedLongAndUnsignedLong_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckNotEquals_IntAndInt64Equal_Fails,
        "CheckNotEquals_IntAndInt64Equal_Fails");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckNotEquals_NegativeAndUint64Max_Passes,
        "CheckNotEquals_NegativeAndUint64Max_Passes");
    RegisterTest(&TFixture_IntegerComparisons::Test_CheckNotEquals_NegativeAndUnsignedMax_Passes,
        "CheckNotEquals_NegativeAndUnsignedMax_Passes");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_AssertEquals_IntAndInt64Different_Fails()
{
    AssertEquals(5, int64_t{ 6 }, __func__, __LINE__, "different values");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_AssertEquals_IntAndInt64Equal_Passes()
{
    AssertEquals(5, int64_t{ 5 }, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_AssertNotEquals_IntAndInt64Equal_Fails()
{
    AssertNotEquals(5, int64_t{ 5 }, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_AssertNotEquals_NegativeAndUnsignedMax_Passes()
{
    AssertNotEquals(-1, std::numeric_limits<unsigned int>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_AboveInt64MaxAndInt64Max_Fails()
{
    uint64_t const aboveInt64Max = static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
    CheckEquals(aboveInt64Max, std::numeric_limits<int64_t>::max(), __func__, __LINE__, "differ by one");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_Int64MaxAndUint64_Passes()
{
    uint64_t const int64Max = static_cast<uint64_t>(std::numeric_limits<int64_t>::max());
    CheckEquals(std::numeric_limits<int64_t>::max(), int64Max, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_IntAndInt64Different_Fails()
{
    CheckEquals(5, int64_t{ 6 }, __func__, __LINE__, "different values");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_IntAndInt64Equal_Passes()
{
    CheckEquals(5, int64_t{ 5 }, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_IntAndShort_Passes()
{
    short const value = 5;
    CheckEquals(5, value, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_IntAndUnsigned_Passes()
{
    CheckEquals(3, 3u, __func__, __LINE__, "same value, different signedness");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_LongAndLong_Passes()
{
    long const value = 7;
    CheckEquals(value, value, __func__, __LINE__, "long matches no fixed-width overload on Windows");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_LongLongAndLongLong_Passes()
{
    long long const value = 7;
    CheckEquals(value, value, __func__, __LINE__, "long long matches no fixed-width overload on Linux");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_NegativeAndUint64Max_Fails()
{
    CheckEquals(int64_t{ -1 }, std::numeric_limits<uint64_t>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_NegativeAndUnsignedMax_Fails()
{
    CheckEquals(-1, std::numeric_limits<unsigned int>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckEquals_UnsignedLongAndUnsignedLong_Passes()
{
    unsigned long const value = 7;
    CheckEquals(value, value, __func__, __LINE__, "unsigned long matches no fixed-width overload on Windows");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckNotEquals_IntAndInt64Equal_Fails()
{
    CheckNotEquals(5, int64_t{ 5 }, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckNotEquals_NegativeAndUint64Max_Passes()
{
    CheckNotEquals(int64_t{ -1 }, std::numeric_limits<uint64_t>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_IntegerComparisons::Test_CheckNotEquals_NegativeAndUnsignedMax_Passes()
{
    CheckNotEquals(-1, std::numeric_limits<unsigned int>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_MixedOutcomes
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test method per outcome
// TTestGroupBase can produce, plus one that fails twice in a row to prove a Check failure doesn't
// abort the rest of the test the way an Assert failure does, and one whose Check failure is
// followed by an Assert failure, so both end up in its record's detail. Constructed and run directly by
// TTest_ASWUnitTests_TestBase's own test methods below, so its deliberate failures/skip only ever
// affect this fixture's own Results() - never the real, self-registered suite running it.
/////////////////////////////////////////////////////////////////////////////
class TFixture_MixedOutcomes : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_ContinuesAfterCheckFailure();
    void Test_FailViaAssert();
    void Test_FailViaCheck();
    void Test_FailViaCheckThenAssert();
    void Test_Pass();
    void Test_Skip();

public:
    bool ReachedSecondCheck = false;

public:
    explicit TFixture_MixedOutcomes(bool suppressLog = true);

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_MixedOutcomes::TFixture_MixedOutcomes(bool suppressLog)
    : inherited("Fixture_MixedOutcomes")
{
    SetLogSuppressed(suppressLog);

    RegisterTest(&TFixture_MixedOutcomes::Test_ContinuesAfterCheckFailure, "ContinuesAfterCheckFailure");
    RegisterTest(&TFixture_MixedOutcomes::Test_FailViaAssert, "FailViaAssert");
    RegisterTest(&TFixture_MixedOutcomes::Test_FailViaCheck, "FailViaCheck");
    RegisterTest(&TFixture_MixedOutcomes::Test_FailViaCheckThenAssert, "FailViaCheckThenAssert");
    RegisterTest(&TFixture_MixedOutcomes::Test_Pass, "Pass");
    RegisterTest(&TFixture_MixedOutcomes::Test_Skip, "Skip");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_ContinuesAfterCheckFailure()
{
    CheckEquals(1, 2, __func__, __LINE__, "deliberate failure, fixture test");
    ReachedSecondCheck = true; // Only reached if CheckEquals() did not abort the test like Assert* does.
    CheckEquals(3, 4, __func__, __LINE__, "deliberate second failure, fixture test");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_FailViaAssert()
{
    AssertEquals(1, 2, __func__, __LINE__, "deliberate failure, fixture test");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_FailViaCheck()
{
    CheckEquals(1, 2, __func__, __LINE__, "deliberate failure, fixture test");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_FailViaCheckThenAssert()
{
    CheckEquals(1, 2, __func__, __LINE__, "deliberate Check failure before an Assert, fixture test");
    AssertEquals(3, 4, __func__, __LINE__, "deliberate Assert failure after a Check, fixture test");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_Pass()
{
    CheckTrue(true, __func__, __LINE__, "trivially passes");
}
//---------------------------------------------------------------------------
void TFixture_MixedOutcomes::Test_Skip()
{
    Skip(__func__, __LINE__, "deliberate skip, fixture test");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_NearComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing CheckNear()/AssertNear()/
// CheckNotNear()'s tolerance boundary: the implementation fails only when diff > tolerance, so a
// diff exactly equal to tolerance must still count as "near". Test names self-document expected
// outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_NearComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNear_JustOutsideTolerance_Fails();
    void Test_CheckNear_AtToleranceBoundary_Passes();
    void Test_CheckNear_Double_AtToleranceBoundary_Passes();
    void Test_CheckNear_JustOutsideTolerance_Fails();
    void Test_CheckNotNear_AtToleranceBoundary_Fails();
    void Test_CheckNotNear_JustOutsideTolerance_Passes();

public:
    TFixture_NearComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_NearComparisons::TFixture_NearComparisons()
    : inherited("Fixture_NearComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_NearComparisons::Test_AssertNear_JustOutsideTolerance_Fails,
        "AssertNear_JustOutsideTolerance_Fails");
    RegisterTest(&TFixture_NearComparisons::Test_CheckNear_AtToleranceBoundary_Passes,
        "CheckNear_AtToleranceBoundary_Passes");
    RegisterTest(&TFixture_NearComparisons::Test_CheckNear_Double_AtToleranceBoundary_Passes,
        "CheckNear_Double_AtToleranceBoundary_Passes");
    RegisterTest(&TFixture_NearComparisons::Test_CheckNear_JustOutsideTolerance_Fails,
        "CheckNear_JustOutsideTolerance_Fails");
    RegisterTest(&TFixture_NearComparisons::Test_CheckNotNear_AtToleranceBoundary_Fails,
        "CheckNotNear_AtToleranceBoundary_Fails");
    RegisterTest(&TFixture_NearComparisons::Test_CheckNotNear_JustOutsideTolerance_Passes,
        "CheckNotNear_JustOutsideTolerance_Passes");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_AssertNear_JustOutsideTolerance_Fails()
{
    AssertNear(1.0f, 1.51f, 0.5f, __func__, __LINE__, "diff (0.51) exceeds tolerance (0.5)");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_CheckNear_AtToleranceBoundary_Passes()
{
    CheckNear(1.0f, 1.5f, 0.5f, __func__, __LINE__, "diff (0.5) exactly equals tolerance (0.5): inclusive boundary");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_CheckNear_Double_AtToleranceBoundary_Passes()
{
    CheckNear(1.0, 1.5, 0.5, __func__, __LINE__, "double overload, same inclusive boundary");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_CheckNear_JustOutsideTolerance_Fails()
{
    CheckNear(1.0f, 1.51f, 0.5f, __func__, __LINE__, "diff (0.51) exceeds tolerance (0.5)");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_CheckNotNear_AtToleranceBoundary_Fails()
{
    CheckNotNear(1.0f, 1.5f, 0.5f, __func__, __LINE__, "diff (0.5) still counts as 'near' at the boundary");
}
//---------------------------------------------------------------------------
void TFixture_NearComparisons::Test_CheckNotNear_JustOutsideTolerance_Passes()
{
    CheckNotNear(1.0f, 1.51f, 0.5f, __func__, __LINE__, "diff (0.51) exceeds tolerance: correctly 'not near'");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_OrderRecorder
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group of 8 trivially-passing tests (A-H)
// that each record their own name into a caller-owned vector when run, for verifying Run()'s
// --shuffle ordering and TestFilter application. 8 tests keeps the odds of a shuffled order
// coincidentally matching the unshuffled one at 1-in-40320 or better, low enough that
// Test_Run_ShuffleSeedProducesDeterministicOrder below won't flake.
/////////////////////////////////////////////////////////////////////////////
class TFixture_OrderRecorder : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    std::vector<std::string>& m_ExecutionOrder;

private:
    void Test_A();
    void Test_B();
    void Test_C();
    void Test_D();
    void Test_E();
    void Test_F();
    void Test_G();
    void Test_H();

public:
    explicit TFixture_OrderRecorder(std::vector<std::string>& executionOrder);

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_OrderRecorder::TFixture_OrderRecorder(std::vector<std::string>& executionOrder)
    : inherited("Fixture_OrderRecorder"),
      m_ExecutionOrder(executionOrder)
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_OrderRecorder::Test_A, "A");
    RegisterTest(&TFixture_OrderRecorder::Test_B, "B");
    RegisterTest(&TFixture_OrderRecorder::Test_C, "C");
    RegisterTest(&TFixture_OrderRecorder::Test_D, "D");
    RegisterTest(&TFixture_OrderRecorder::Test_E, "E");
    RegisterTest(&TFixture_OrderRecorder::Test_F, "F");
    RegisterTest(&TFixture_OrderRecorder::Test_G, "G");
    RegisterTest(&TFixture_OrderRecorder::Test_H, "H");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_A()
{
    m_ExecutionOrder.push_back("A");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_B()
{
    m_ExecutionOrder.push_back("B");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_C()
{
    m_ExecutionOrder.push_back("C");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_D()
{
    m_ExecutionOrder.push_back("D");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_E()
{
    m_ExecutionOrder.push_back("E");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_F()
{
    m_ExecutionOrder.push_back("F");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_G()
{
    m_ExecutionOrder.push_back("G");
}
//---------------------------------------------------------------------------
void TFixture_OrderRecorder::Test_H()
{
    m_ExecutionOrder.push_back("H");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_OrderingComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing the GreaterThan/GreaterThanOrEqual/
// LessThan/LessThanOrEqual methods, each called as (value, bound). The "Negative...Max" and "...MaxAndNegative" tests
// catch a comparison that lets -1 wrap around to an unsigned maximum (the built-in -1 < 4294967295u is false), the
// "Uint64Max" ones the case where neither value fits the other's type, and the NaN ones that a NaN fails every check.
// Test names self-document expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_OrderingComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertGreaterThanOrEqual_Equal_Passes();
    void Test_AssertGreaterThanOrEqual_Less_Fails();
    void Test_AssertGreaterThan_Equal_Fails();
    void Test_AssertGreaterThan_Greater_Passes();
    void Test_AssertLessThanOrEqual_Equal_Passes();
    void Test_AssertLessThanOrEqual_Greater_Fails();
    void Test_AssertLessThan_Equal_Fails();
    void Test_AssertLessThan_Less_Passes();
    void Test_CheckGreaterThanOrEqual_Equal_Passes();
    void Test_CheckGreaterThanOrEqual_Greater_Passes();
    void Test_CheckGreaterThanOrEqual_Less_Fails();
    void Test_CheckGreaterThanOrEqual_NaN_Fails();
    void Test_CheckGreaterThan_AboveInt64MaxAndInt64Max_Passes();
    void Test_CheckGreaterThan_DoubleAndInt_Passes();
    void Test_CheckGreaterThan_Double_Fails();
    void Test_CheckGreaterThan_Equal_Fails();
    void Test_CheckGreaterThan_Float_Fails();
    void Test_CheckGreaterThan_Greater_Passes();
    void Test_CheckGreaterThan_Infinity_Passes();
    void Test_CheckGreaterThan_LongDouble_Fails();
    void Test_CheckGreaterThan_Less_Fails();
    void Test_CheckGreaterThan_Uint64MaxAndNegative_Passes();
    void Test_CheckGreaterThan_UnsignedMaxAndNegative_Passes();
    void Test_CheckLessThanOrEqual_Equal_Passes();
    void Test_CheckLessThanOrEqual_Greater_Fails();
    void Test_CheckLessThanOrEqual_IntAndInt64_Passes();
    void Test_CheckLessThanOrEqual_Less_Passes();
    void Test_CheckLessThanOrEqual_NaNBound_Fails();
    void Test_CheckLessThan_CloseDoubles_Fails();
    void Test_CheckLessThan_Equal_Fails();
    void Test_CheckLessThan_Greater_Fails();
    void Test_CheckLessThan_Less_Passes();
    void Test_CheckLessThan_NaN_Fails();
    void Test_CheckLessThan_NegativeAndUint64Max_Passes();
    void Test_CheckLessThan_NegativeAndUnsignedMax_Passes();
    void Test_CheckLessThan_Uint64MaxAndNegative_Fails();

public:
    TFixture_OrderingComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_OrderingComparisons::TFixture_OrderingComparisons()
    : inherited("Fixture_OrderingComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_OrderingComparisons::Test_AssertGreaterThanOrEqual_Equal_Passes,
        "AssertGreaterThanOrEqual_Equal_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertGreaterThanOrEqual_Less_Fails,
        "AssertGreaterThanOrEqual_Less_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertGreaterThan_Equal_Fails, "AssertGreaterThan_Equal_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertGreaterThan_Greater_Passes,
        "AssertGreaterThan_Greater_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertLessThanOrEqual_Equal_Passes,
        "AssertLessThanOrEqual_Equal_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertLessThanOrEqual_Greater_Fails,
        "AssertLessThanOrEqual_Greater_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertLessThan_Equal_Fails, "AssertLessThan_Equal_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_AssertLessThan_Less_Passes, "AssertLessThan_Less_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Equal_Passes,
        "CheckGreaterThanOrEqual_Equal_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Greater_Passes,
        "CheckGreaterThanOrEqual_Greater_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Less_Fails,
        "CheckGreaterThanOrEqual_Less_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_NaN_Fails,
        "CheckGreaterThanOrEqual_NaN_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_AboveInt64MaxAndInt64Max_Passes,
        "CheckGreaterThan_AboveInt64MaxAndInt64Max_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_DoubleAndInt_Passes,
        "CheckGreaterThan_DoubleAndInt_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Double_Fails, "CheckGreaterThan_Double_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Equal_Fails, "CheckGreaterThan_Equal_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Float_Fails, "CheckGreaterThan_Float_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Greater_Passes,
        "CheckGreaterThan_Greater_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Infinity_Passes,
        "CheckGreaterThan_Infinity_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_LongDouble_Fails,
        "CheckGreaterThan_LongDouble_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Less_Fails, "CheckGreaterThan_Less_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_Uint64MaxAndNegative_Passes,
        "CheckGreaterThan_Uint64MaxAndNegative_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckGreaterThan_UnsignedMaxAndNegative_Passes,
        "CheckGreaterThan_UnsignedMaxAndNegative_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Equal_Passes,
        "CheckLessThanOrEqual_Equal_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Greater_Fails,
        "CheckLessThanOrEqual_Greater_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_IntAndInt64_Passes,
        "CheckLessThanOrEqual_IntAndInt64_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Less_Passes,
        "CheckLessThanOrEqual_Less_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_NaNBound_Fails,
        "CheckLessThanOrEqual_NaNBound_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_CloseDoubles_Fails,
        "CheckLessThan_CloseDoubles_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_Equal_Fails, "CheckLessThan_Equal_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_Greater_Fails, "CheckLessThan_Greater_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_Less_Passes, "CheckLessThan_Less_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_NaN_Fails, "CheckLessThan_NaN_Fails");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_NegativeAndUint64Max_Passes,
        "CheckLessThan_NegativeAndUint64Max_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_NegativeAndUnsignedMax_Passes,
        "CheckLessThan_NegativeAndUnsignedMax_Passes");
    RegisterTest(&TFixture_OrderingComparisons::Test_CheckLessThan_Uint64MaxAndNegative_Fails,
        "CheckLessThan_Uint64MaxAndNegative_Fails");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertGreaterThanOrEqual_Equal_Passes()
{
    AssertGreaterThanOrEqual(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertGreaterThanOrEqual_Less_Fails()
{
    AssertGreaterThanOrEqual(1, 2, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertGreaterThan_Equal_Fails()
{
    AssertGreaterThan(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertGreaterThan_Greater_Passes()
{
    AssertGreaterThan(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertLessThanOrEqual_Equal_Passes()
{
    AssertLessThanOrEqual(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertLessThanOrEqual_Greater_Fails()
{
    AssertLessThanOrEqual(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertLessThan_Equal_Fails()
{
    AssertLessThan(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_AssertLessThan_Less_Passes()
{
    AssertLessThan(3, 4, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Equal_Passes()
{
    CheckGreaterThanOrEqual(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Greater_Passes()
{
    CheckGreaterThanOrEqual(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_Less_Fails()
{
    CheckGreaterThanOrEqual(3, 4, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThanOrEqual_NaN_Fails()
{
    double const nan = std::numeric_limits<double>::quiet_NaN();
    CheckGreaterThanOrEqual(nan, nan, __func__, __LINE__, "a NaN is never equal, even to itself");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_AboveInt64MaxAndInt64Max_Passes()
{
    uint64_t const aboveInt64Max = static_cast<uint64_t>(std::numeric_limits<int64_t>::max()) + 1;
    CheckGreaterThan(aboveInt64Max, std::numeric_limits<int64_t>::max(), __func__, __LINE__, "greater by one");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_DoubleAndInt_Passes()
{
    CheckGreaterThan(0.5, 0, __func__, __LINE__, "the int is compared as a double");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Double_Fails()
{
    CheckGreaterThan(0.1, 0.25, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Equal_Fails()
{
    CheckGreaterThan(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Float_Fails()
{
    CheckGreaterThan(0.1f, 0.25f, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Greater_Passes()
{
    CheckGreaterThan(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Infinity_Passes()
{
    CheckGreaterThan(std::numeric_limits<double>::infinity(), std::numeric_limits<double>::max(), __func__, __LINE__,
        "infinity is greater than any finite value");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_LongDouble_Fails()
{
    CheckGreaterThan(1.5L, 2.5L, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Less_Fails()
{
    CheckGreaterThan(3, 4, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_Uint64MaxAndNegative_Passes()
{
    CheckGreaterThan(std::numeric_limits<uint64_t>::max(), int64_t{ -1 }, __func__, __LINE__, "-1 must not wrap");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckGreaterThan_UnsignedMaxAndNegative_Passes()
{
    CheckGreaterThan(std::numeric_limits<unsigned int>::max(), -1, __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Equal_Passes()
{
    CheckLessThanOrEqual(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Greater_Fails()
{
    CheckLessThanOrEqual(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_IntAndInt64_Passes()
{
    CheckLessThanOrEqual(5, int64_t{ 5 }, __func__, __LINE__, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_Less_Passes()
{
    CheckLessThanOrEqual(3, 4, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThanOrEqual_NaNBound_Fails()
{
    CheckLessThanOrEqual(1.0, std::numeric_limits<double>::quiet_NaN(), __func__, __LINE__, "nothing compares to NaN");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_CloseDoubles_Fails()
{
    // The smallest double above 1, which looks like 1 at the usual 15 significant digits.
    CheckLessThan(std::nextafter(1.0, 2.0), 1.0, __func__, __LINE__, "just greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_Equal_Fails()
{
    CheckLessThan(4, 4, __func__, __LINE__, "equal");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_Greater_Fails()
{
    CheckLessThan(5, 4, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_Less_Passes()
{
    CheckLessThan(3, 4, __func__, __LINE__, "less");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_NaN_Fails()
{
    CheckLessThan(std::numeric_limits<double>::quiet_NaN(), 1.0, __func__, __LINE__, "NaN compares to nothing");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_NegativeAndUint64Max_Passes()
{
    CheckLessThan(int64_t{ -1 }, std::numeric_limits<uint64_t>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_NegativeAndUnsignedMax_Passes()
{
    CheckLessThan(-1, std::numeric_limits<unsigned int>::max(), __func__, __LINE__, "-1 must not wrap around");
}
//---------------------------------------------------------------------------
void TFixture_OrderingComparisons::Test_CheckLessThan_Uint64MaxAndNegative_Fails()
{
    CheckLessThan(std::numeric_limits<uint64_t>::max(), int64_t{ -1 }, __func__, __LINE__, "greater");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_SlowTest
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test that hangs forever,
// followed by one that must never run if RunWithTimeout() correctly abandons the hung test and
// TTestGroupBase::Run() correctly aborts the rest of the group afterward. Used by
// Test_Run_AbandonsHungTestAndAbortsGroupOnTimeout below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SlowTest : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_HangsForever();
    void Test_NeverRuns();

public:
    bool NeverRunReached = false;

public:
    TFixture_SlowTest();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_SlowTest::TFixture_SlowTest()
    : inherited("Fixture_SlowTest")
{
    SetLogSuppressed(true);

    // Registration order matters here (unlike the other fixtures above): the hung test must run
    // first so the second one is still pending when the timeout fires.
    RegisterTest(&TFixture_SlowTest::Test_HangsForever, "HangsForever");
    RegisterTest(&TFixture_SlowTest::Test_NeverRuns, "NeverRuns");
}
//---------------------------------------------------------------------------
void TFixture_SlowTest::Test_HangsForever()
{
    for (;;)
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
//---------------------------------------------------------------------------
void TFixture_SlowTest::Test_NeverRuns()
{
    NeverRunReached = true;
}
//---------------------------------------------------------------------------


#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
/////////////////////////////////////////////////////////////////////////////
// TFixture_SourceLocations
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group calling each std::source_location
// overload. Each "_Fails"/"_Skips" test records the line of its call in ExpectedLines, so
// Test_SourceLocation_ReportsCallerFunctionAndLine below can check the failure names that test's
// function and line. "_Passes" tests check that arguments forward to the same overloads as the
// method/line form (C strings, mixed integers, wide substrings, exception expectations), and the "IC" ones that
// they reach the case-insensitive overloads. CheckIsEven() is a custom helper of the kind a test author might write,
// passing its caller's location through.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SourceLocations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void CheckIsEven(int value, std::source_location loc = std::source_location::current());

    void Test_AssertContainsIC_Passes();
    void Test_AssertContains_Fails();
    void Test_AssertEqualsIC_Passes();
    void Test_AssertEquals_CStrings_Passes();
    void Test_AssertEquals_Fails();
    void Test_AssertFalse_Fails();
    void Test_AssertGreaterThanOrEqual_Passes();
    void Test_AssertGreaterThan_Fails();
    void Test_AssertLessThanOrEqual_Passes();
    void Test_AssertLessThan_Fails();
    void Test_AssertNear_Fails();
    void Test_AssertNotContainsIC_Fails();
    void Test_AssertNotContains_Fails();
    void Test_AssertNotEqualsIC_Fails();
    void Test_AssertNotEquals_Fails();
    void Test_AssertNotNear_Fails();
    void Test_AssertTrue_Fails();
    void Test_CheckContainsIC_Passes();
    void Test_CheckContains_Fails();
    void Test_CheckContains_Wide_Passes();
    void Test_CheckEqualsIC_Passes();
    void Test_CheckEquals_Fails();
    void Test_CheckEquals_MixedIntegers_Passes();
    void Test_CheckFalse_Fails();
    void Test_CheckGreaterThanOrEqual_Passes();
    void Test_CheckGreaterThan_Fails();
    void Test_CheckLessThanOrEqual_Passes();
    void Test_CheckLessThan_Fails();
    void Test_CheckNear_Fails();
    void Test_CheckNear_Passes();
    void Test_CheckNotContainsIC_Fails();
    void Test_CheckNotContains_Fails();
    void Test_CheckNotEqualsIC_Fails();
    void Test_CheckNotEquals_Fails();
    void Test_CheckNotNear_Fails();
    void Test_CheckTrue_Fails();
    void Test_CheckTrue_ThroughHelper_Fails();
    void Test_SetExceptionExpected_Bool_NoneThrown_Fails();
    void Test_SetExceptionExpected_Bool_Passes();
    void Test_SetExceptionExpected_Type_NoneThrown_Fails();
    void Test_SetExceptionExpected_TypeAndMessage_Passes();
    void Test_Skip_Skips();

public:
    std::map<std::string, int> ExpectedLines; // Test name to the line of its failing/skipping call.

    TFixture_SourceLocations();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_SourceLocations::TFixture_SourceLocations()
    : inherited("Fixture_SourceLocations")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_SourceLocations::Test_AssertContainsIC_Passes, "AssertContainsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertContains_Fails, "AssertContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEqualsIC_Passes, "AssertEqualsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEquals_CStrings_Passes, "AssertEquals_CStrings_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEquals_Fails, "AssertEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertFalse_Fails, "AssertFalse_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertGreaterThanOrEqual_Passes, "AssertGreaterThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertGreaterThan_Fails, "AssertGreaterThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertLessThanOrEqual_Passes, "AssertLessThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertLessThan_Fails, "AssertLessThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNear_Fails, "AssertNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotContainsIC_Fails, "AssertNotContainsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotContains_Fails, "AssertNotContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEqualsIC_Fails, "AssertNotEqualsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEquals_Fails, "AssertNotEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotNear_Fails, "AssertNotNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertTrue_Fails, "AssertTrue_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContainsIC_Passes, "CheckContainsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContains_Fails, "CheckContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContains_Wide_Passes, "CheckContains_Wide_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEqualsIC_Passes, "CheckEqualsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEquals_Fails, "CheckEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEquals_MixedIntegers_Passes, "CheckEquals_MixedIntegers_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckFalse_Fails, "CheckFalse_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckGreaterThanOrEqual_Passes, "CheckGreaterThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckGreaterThan_Fails, "CheckGreaterThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckLessThanOrEqual_Passes, "CheckLessThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckLessThan_Fails, "CheckLessThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNear_Fails, "CheckNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNear_Passes, "CheckNear_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotContainsIC_Fails, "CheckNotContainsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotContains_Fails, "CheckNotContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEqualsIC_Fails, "CheckNotEqualsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEquals_Fails, "CheckNotEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotNear_Fails, "CheckNotNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckTrue_Fails, "CheckTrue_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckTrue_ThroughHelper_Fails, "CheckTrue_ThroughHelper_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Bool_NoneThrown_Fails,
        "SetExceptionExpected_Bool_NoneThrown_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Bool_Passes,
        "SetExceptionExpected_Bool_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Type_NoneThrown_Fails,
        "SetExceptionExpected_Type_NoneThrown_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_TypeAndMessage_Passes,
        "SetExceptionExpected_TypeAndMessage_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_Skip_Skips, "Skip_Skips");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::CheckIsEven(int value, std::source_location loc)
{
    CheckTrue(value % 2 == 0, std::to_string(value) + " should be even", loc);
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertContainsIC_Passes()
{
    AssertContainsIC(std::string("abc"), "B", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertContains_Fails()
{
    ExpectedLines["AssertContains_Fails"] = __LINE__ + 1;
    AssertContains(std::string("abc"), "x", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEqualsIC_Passes()
{
    AssertEqualsIC(std::string("abc"), std::string("ABC"), "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEquals_CStrings_Passes()
{
    char const buffer[] = "abc";
    AssertEquals("abc", buffer, "same text in a separate buffer");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEquals_Fails()
{
    ExpectedLines["AssertEquals_Fails"] = __LINE__ + 1;
    AssertEquals(1, 2, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertFalse_Fails()
{
    ExpectedLines["AssertFalse_Fails"] = __LINE__ + 1;
    AssertFalse(true, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertGreaterThanOrEqual_Passes()
{
    AssertGreaterThanOrEqual(4, 4, "equal");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertGreaterThan_Fails()
{
    ExpectedLines["AssertGreaterThan_Fails"] = __LINE__ + 1;
    AssertGreaterThan(4, 4, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertLessThanOrEqual_Passes()
{
    AssertLessThanOrEqual(4, 4, "equal");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertLessThan_Fails()
{
    ExpectedLines["AssertLessThan_Fails"] = __LINE__ + 1;
    AssertLessThan(4, 4, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNear_Fails()
{
    ExpectedLines["AssertNear_Fails"] = __LINE__ + 1;
    AssertNear(1.0, 2.0, 0.5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotContainsIC_Fails()
{
    ExpectedLines["AssertNotContainsIC_Fails"] = __LINE__ + 1;
    AssertNotContainsIC(std::string("abc"), "B", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotContains_Fails()
{
    ExpectedLines["AssertNotContains_Fails"] = __LINE__ + 1;
    AssertNotContains(std::string("abc"), "b", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEqualsIC_Fails()
{
    ExpectedLines["AssertNotEqualsIC_Fails"] = __LINE__ + 1;
    AssertNotEqualsIC(std::string("abc"), std::string("ABC"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEquals_Fails()
{
    ExpectedLines["AssertNotEquals_Fails"] = __LINE__ + 1;
    AssertNotEquals(std::string("abc"), std::string("abc"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotNear_Fails()
{
    ExpectedLines["AssertNotNear_Fails"] = __LINE__ + 1;
    AssertNotNear(1.0f, 1.25f, 0.5f, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertTrue_Fails()
{
    ExpectedLines["AssertTrue_Fails"] = __LINE__ + 1;
    AssertTrue(false, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckContainsIC_Passes()
{
    CheckContainsIC(std::string("abc"), "B", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckContains_Fails()
{
    ExpectedLines["CheckContains_Fails"] = __LINE__ + 1;
    CheckContains(std::string("abc"), "x", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckContains_Wide_Passes()
{
    CheckContains(std::wstring(L"abc"), L"b", "wide substring present");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEqualsIC_Passes()
{
    CheckEqualsIC(std::string("abc"), std::string("ABC"), "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEquals_Fails()
{
    ExpectedLines["CheckEquals_Fails"] = __LINE__ + 1;
    CheckEquals(std::string("abc"), std::string("xyz"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEquals_MixedIntegers_Passes()
{
    CheckEquals(5, int64_t{ 5 }, "same value, different types");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckFalse_Fails()
{
    ExpectedLines["CheckFalse_Fails"] = __LINE__ + 1;
    CheckFalse(true, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckGreaterThanOrEqual_Passes()
{
    CheckGreaterThanOrEqual(2.0, 2, "equal, double and int");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckGreaterThan_Fails()
{
    ExpectedLines["CheckGreaterThan_Fails"] = __LINE__ + 1;
    CheckGreaterThan(4u, 4, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckLessThanOrEqual_Passes()
{
    CheckLessThanOrEqual(int64_t{ 4 }, 4u, "equal, int64_t and unsigned int");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckLessThan_Fails()
{
    ExpectedLines["CheckLessThan_Fails"] = __LINE__ + 1;
    CheckLessThan(4.0f, 4, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNear_Fails()
{
    ExpectedLines["CheckNear_Fails"] = __LINE__ + 1;
    CheckNear(1.0, 2.0, 0.5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNear_Passes()
{
    CheckNear(1.0f, 1.25f, 0.5f, "within tolerance");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotContainsIC_Fails()
{
    ExpectedLines["CheckNotContainsIC_Fails"] = __LINE__ + 1;
    CheckNotContainsIC(std::string("abc"), "B", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotContains_Fails()
{
    ExpectedLines["CheckNotContains_Fails"] = __LINE__ + 1;
    CheckNotContains(std::string("abc"), "b", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEqualsIC_Fails()
{
    ExpectedLines["CheckNotEqualsIC_Fails"] = __LINE__ + 1;
    CheckNotEqualsIC(std::string("abc"), std::string("ABC"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEquals_Fails()
{
    ExpectedLines["CheckNotEquals_Fails"] = __LINE__ + 1;
    CheckNotEquals(5, 5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotNear_Fails()
{
    ExpectedLines["CheckNotNear_Fails"] = __LINE__ + 1;
    CheckNotNear(1.0, 1.25, 0.5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckTrue_Fails()
{
    ExpectedLines["CheckTrue_Fails"] = __LINE__ + 1;
    CheckTrue(false, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckTrue_ThroughHelper_Fails()
{
    ExpectedLines["CheckTrue_ThroughHelper_Fails"] = __LINE__ + 1;
    CheckIsEven(3);
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_SetExceptionExpected_Bool_NoneThrown_Fails()
{
    ExpectedLines["SetExceptionExpected_Bool_NoneThrown_Fails"] = __LINE__ + 1;
    SetExceptionExpected(true, "deliberately nothing thrown");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_SetExceptionExpected_Bool_Passes()
{
    SetExceptionExpected(true, "any exception");
    throw std::runtime_error("boom");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_SetExceptionExpected_Type_NoneThrown_Fails()
{
    ExpectedLines["SetExceptionExpected_Type_NoneThrown_Fails"] = __LINE__ + 1;
    SetExceptionExpected<std::invalid_argument>("deliberately nothing thrown");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_SetExceptionExpected_TypeAndMessage_Passes()
{
    SetExceptionExpected<std::invalid_argument>("matching type and message", "boom");
    throw std::invalid_argument("boom");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_Skip_Skips()
{
    ExpectedLines["Skip_Skips"] = __LINE__ + 1;
    Skip("deliberate skip");
}
//---------------------------------------------------------------------------
#endif // #if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)


/////////////////////////////////////////////////////////////////////////////
// TFixture_TrueFalseChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group calling AssertTrue()/CheckTrue()/AssertFalse()/
// CheckFalse() with a value that passes and one that fails, so Test_TrueFalse_FailureNamesTheExpectedValue below can
// check that each failure states the value that was expected, not the opposite. Test names self-document expected
// outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_TrueFalseChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertFalse_False_Passes();
    void Test_AssertFalse_True_Fails();
    void Test_AssertTrue_False_Fails();
    void Test_AssertTrue_True_Passes();
    void Test_CheckFalse_False_Passes();
    void Test_CheckFalse_True_Fails();
    void Test_CheckTrue_False_Fails();
    void Test_CheckTrue_True_Passes();

public:
    TFixture_TrueFalseChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_TrueFalseChecks::TFixture_TrueFalseChecks()
    : inherited("Fixture_TrueFalseChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_TrueFalseChecks::Test_AssertFalse_False_Passes, "AssertFalse_False_Passes");
    RegisterTest(&TFixture_TrueFalseChecks::Test_AssertFalse_True_Fails, "AssertFalse_True_Fails");
    RegisterTest(&TFixture_TrueFalseChecks::Test_AssertTrue_False_Fails, "AssertTrue_False_Fails");
    RegisterTest(&TFixture_TrueFalseChecks::Test_AssertTrue_True_Passes, "AssertTrue_True_Passes");
    RegisterTest(&TFixture_TrueFalseChecks::Test_CheckFalse_False_Passes, "CheckFalse_False_Passes");
    RegisterTest(&TFixture_TrueFalseChecks::Test_CheckFalse_True_Fails, "CheckFalse_True_Fails");
    RegisterTest(&TFixture_TrueFalseChecks::Test_CheckTrue_False_Fails, "CheckTrue_False_Fails");
    RegisterTest(&TFixture_TrueFalseChecks::Test_CheckTrue_True_Passes, "CheckTrue_True_Passes");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_AssertFalse_False_Passes()
{
    AssertFalse(false, __func__, __LINE__, "value is false");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_AssertFalse_True_Fails()
{
    AssertFalse(true, __func__, __LINE__, "value is true");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_AssertTrue_False_Fails()
{
    AssertTrue(false, __func__, __LINE__, "value is false");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_AssertTrue_True_Passes()
{
    AssertTrue(true, __func__, __LINE__, "value is true");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_CheckFalse_False_Passes()
{
    CheckFalse(false, __func__, __LINE__, "value is false");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_CheckFalse_True_Fails()
{
    CheckFalse(true, __func__, __LINE__, "value is true");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_CheckTrue_False_Fails()
{
    CheckTrue(false, __func__, __LINE__, "value is false");
}
//---------------------------------------------------------------------------
void TFixture_TrueFalseChecks::Test_CheckTrue_True_Passes()
{
    CheckTrue(true, __func__, __LINE__, "value is true");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_CrashingTest
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test that crashes (an
// integer divide by zero), followed by one that records whether it ran. Used by
// Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet below to prove --catch-crashes' central
// behavior: unlike a timeout, which can only abandon-and-abort, a caught crash lets the group
// continue running its remaining tests. Deliberately not a null-pointer write: that's a SIGSEGV on
// POSIX, and TCrashGuard::Run() treats every SIGSEGV there as abort-worthy (it can't tell an
// ordinary one apart from a stack overflow - see its comment), which would make this fixture's
// crashing test abort the group instead of letting it continue, defeating the point of this test.
// An integer divide by zero (SIGFPE on POSIX, EXCEPTION_INT_DIVIDE_BY_ZERO on Windows) is never
// abort-worthy on either platform, so it exercises "ordinary crash, group continues" portably.
/////////////////////////////////////////////////////////////////////////////
class TFixture_CrashingTest : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_CrashesButDoesNotAbort();
    void Test_RunsAfterTheCrash();

public:
    bool RunsAfterTheCrashReached = false;

public:
    TFixture_CrashingTest();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_CrashingTest::TFixture_CrashingTest()
    : inherited("Fixture_CrashingTest")
{
    SetLogSuppressed(true);

    // Registration order matters here (unlike most of the other fixtures above): the crashing test
    // must run first so the second one is still pending when it crashes.
    RegisterTest(&TFixture_CrashingTest::Test_CrashesButDoesNotAbort, "CrashesButDoesNotAbort");
    RegisterTest(&TFixture_CrashingTest::Test_RunsAfterTheCrash, "RunsAfterTheCrash");
}
//---------------------------------------------------------------------------
void TFixture_CrashingTest::Test_CrashesButDoesNotAbort()
{
    // volatile so an optimizing build can't prove the division is undefined behavior and remove it
    // entirely, silently turning this into a false pass instead of a crash - see
    // Test_ASWUnitTests_CrashGuard.cpp's identical fix for the same real issue, hit building this
    // project's own RAD Studio target in a Release configuration.
    volatile int numerator = 42;
    volatile int denominator = 0;
    volatile int const quotient = numerator / denominator;
    (void)quotient;
}
//---------------------------------------------------------------------------
void TFixture_CrashingTest::Test_RunsAfterTheCrash()
{
    RunsAfterTheCrashReached = true;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_TestBase
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_TestBase::TTest_ASWUnitTests_TestBase()
    : inherited("ASWUnitTests_TestBase_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_CheckNear_ToleranceBoundaryIsInclusive,
        "CheckNear_ToleranceBoundaryIsInclusive");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Check_ContinuesButAssert_Aborts, "Check_ContinuesButAssert_Aborts");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_ContainsIC_IgnoresASCIICaseOnly, "ContainsIC_IgnoresASCIICaseOnly");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Contains_ShowsTextAndSubstring, "Contains_ShowsTextAndSubstring");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_EqualsIC_IgnoresASCIICaseOnly, "EqualsIC_IgnoresASCIICaseOnly");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesCStringsByContent, "Equals_ComparesCStringsByContent");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesMixedIntegerTypesByValue,
        "Equals_ComparesMixedIntegerTypesByValue");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ShowsBoolValuesAsTrueOrFalse,
        "Equals_ShowsBoolValuesAsTrueOrFalse");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Ordering_ComparesByValueAndShowsBoth,
        "Ordering_ComparesByValueAndShowsBoth");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AbandonsHungTestAndAbortsGroupOnTimeout,
        "Run_AbandonsHungTestAndAbortsGroupOnTimeout");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AppliesFilterToSkipNonMatchingTests,
        "Run_AppliesFilterToSkipNonMatchingTests");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet,
        "Run_ContinuesAfterCrashWhenCatchCrashesIsSet");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_LogsEachCheckFailureOnce, "Run_LogsEachCheckFailureOnce");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsCheckFailuresInFailedTestDetail,
        "Run_RecordsCheckFailuresInFailedTestDetail");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords,
        "Run_RecordsOutcomeCountsAndCaseRecords");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ReportsEachTestToRunObserver,
        "Run_ReportsEachTestToRunObserver");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ReportsRunObserverEventsOnCallingThreadUnderTimeout,
        "Run_ReportsRunObserverEventsOnCallingThreadUnderTimeout");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ReportsTimedOutTestToRunObserver,
        "Run_ReportsTimedOutTestToRunObserver");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ResetsResultsBetweenRuns, "Run_ResetsResultsBetweenRuns");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ShuffleSeedProducesDeterministicOrder,
        "Run_ShuffleSeedProducesDeterministicOrder");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_StopsWhenRunObserverRequests,
        "Run_StopsWhenRunObserverRequests");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_AssertFailureStillFailsAndIsRecorded,
        "SetExceptionExpected_AssertFailureStillFailsAndIsRecorded");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_EarlierCheckFailureStillFailsAndIsRecorded,
        "SetExceptionExpected_EarlierCheckFailureStillFailsAndIsRecorded");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_MatchesTypeAndMessage,
        "SetExceptionExpected_MatchesTypeAndMessage");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetLogSuppressed_SilencesFixtureOutput,
        "SetLogSuppressed_SilencesFixtureOutput");
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SourceLocation_ReportsCallerFunctionAndLine,
        "SourceLocation_ReportsCallerFunctionAndLine");
#endif
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_TrueFalse_FailureNamesTheExpectedValue,
        "TrueFalse_FailureNamesTheExpectedValue");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_TestBase::~TTest_ASWUnitTests_TestBase()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_CheckNear_ToleranceBoundaryIsInclusive()
{
    // Arrange
    TFixture_NearComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(6), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Check_ContinuesButAssert_Aborts()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    CheckTrue(fixture.ReachedSecondCheck, __func__, __LINE__,
        "a Check failure does not abort the rest of the test, unlike Assert");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_ContainsIC_IgnoresASCIICaseOnly()
{
    // Arrange
    TFixture_ContainsICComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(25), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertContains = FindRecord(results, "AssertContainsIC_Absent_Fails");
    TTestCaseRecord const* const assertNotContains =
        FindRecord(results, "AssertNotContainsIC_Wide_DifferentCase_Fails");
    TTestCaseRecord const* const checkContains = FindRecord(results, "CheckContainsIC_Absent_Fails");
    TTestCaseRecord const* const checkNotContains = FindRecord(results, "CheckNotContainsIC_DifferentCase_Fails");
    AssertTrue(assertContains != nullptr && assertNotContains != nullptr && checkContains != nullptr &&
        checkNotContains != nullptr, __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. The
    // text and substring are shown as given, not case-folded.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };
    std::string const containsDetail =
        "): Expected \"Hello World\" to contain \"xyz\" (ignoring case). substring absent";
    std::string const notContainsDetail =
        "): Expected \"Hello World\" not to contain \"WORLD\" (ignoring case). case differs";

    CheckTrue(messageMatches(*assertContains, "Substring not found: Test_AssertContainsIC_Absent_Fails (",
        containsDetail), __func__, __LINE__,
        "AssertContainsIC shows the text and substring: " + assertContains->Message);
    CheckTrue(messageMatches(*assertNotContains, "Substring found: Test_AssertNotContainsIC_Wide_DifferentCase_Fails (",
        notContainsDetail), __func__, __LINE__,
        "AssertNotContainsIC shows the wide text and substring: " + assertNotContains->Message);
    CheckTrue(messageMatches(*checkContains, "Check failed for: \"Test_CheckContainsIC_Absent_Fails\" (",
        containsDetail), __func__, __LINE__, "CheckContainsIC shows the text and substring: " + checkContains->Message);
    CheckTrue(messageMatches(*checkNotContains, "Check failed for: \"Test_CheckNotContainsIC_DifferentCase_Fails\" (",
        notContainsDetail), __func__, __LINE__,
        "CheckNotContainsIC shows the text and substring: " + checkNotContains->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Contains_ShowsTextAndSubstring()
{
    // Arrange
    TFixture_ContainsComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(21), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertContains = FindRecord(results, "AssertContains_Absent_Fails");
    TTestCaseRecord const* const assertNotContains = FindRecord(results, "AssertNotContains_Wide_Present_Fails");
    TTestCaseRecord const* const checkContains = FindRecord(results, "CheckContains_Absent_Fails");
    TTestCaseRecord const* const checkNonASCII = FindRecord(results, "CheckContains_Wide_NonASCII_Fails");
    TTestCaseRecord const* const checkNotContains = FindRecord(results, "CheckNotContains_Present_Fails");
    AssertTrue(assertContains != nullptr && assertNotContains != nullptr && checkContains != nullptr &&
        checkNonASCII != nullptr && checkNotContains != nullptr, __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    std::string const containsDetail = "): Expected \"hello world\" to contain \"xyz\". substring absent";
    std::string const notContainsDetail = "): Expected \"hello world\" not to contain \"world\". substring present";
    // "cafe" with an e-acute, a space and U+1F600, then u-umlaut, as UTF-8.
    std::string const nonASCIIDetail = "): Expected \"caf\xC3\xA9 \xF0\x9F\x98\x80\" to contain \"\xC3\xBC\". not present";

    CheckTrue(assertContains->Message.find("Substring not found: Test_AssertContains_Absent_Fails (") == 0 &&
        NameEndsWith(assertContains->Message, containsDetail), __func__, __LINE__,
        "AssertContains shows the text and substring: " + assertContains->Message);
    CheckTrue(assertNotContains->Message.find("Substring found: Test_AssertNotContains_Wide_Present_Fails (") == 0 &&
        NameEndsWith(assertNotContains->Message, notContainsDetail), __func__, __LINE__,
        "AssertNotContains shows the wide text and substring: " + assertNotContains->Message);
    CheckTrue(checkContains->Message.find("Check failed for: \"Test_CheckContains_Absent_Fails\" (") == 0 &&
        NameEndsWith(checkContains->Message, containsDetail), __func__, __LINE__,
        "CheckContains shows the text and substring: " + checkContains->Message);
    CheckTrue(checkNotContains->Message.find("Check failed for: \"Test_CheckNotContains_Present_Fails\" (") == 0 &&
        NameEndsWith(checkNotContains->Message, notContainsDetail), __func__, __LINE__,
        "CheckNotContains shows the text and substring: " + checkNotContains->Message);
    CheckTrue(NameEndsWith(checkNonASCII->Message, nonASCIIDetail), __func__, __LINE__,
        "a wide failure shows its text as UTF-8: " + checkNonASCII->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_EqualsIC_IgnoresASCIICaseOnly()
{
    // Arrange
    TFixture_EqualsICComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(23), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertEquals = FindRecord(results, "AssertEqualsIC_DifferentText_Fails");
    TTestCaseRecord const* const assertNotEquals = FindRecord(results, "AssertNotEqualsIC_Wide_DifferentCase_Fails");
    TTestCaseRecord const* const checkEquals = FindRecord(results, "CheckEqualsIC_DifferentText_Fails");
    TTestCaseRecord const* const checkNonASCII = FindRecord(results, "CheckEqualsIC_Wide_NonASCII_Fails");
    TTestCaseRecord const* const checkNotEquals = FindRecord(results, "CheckNotEqualsIC_DifferentCase_Fails");
    AssertTrue(assertEquals != nullptr && assertNotEquals != nullptr && checkEquals != nullptr &&
        checkNonASCII != nullptr && checkNotEquals != nullptr, __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. The
    // values are shown as given, not case-folded, and wide ones as UTF-8.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };

    CheckTrue(messageMatches(*assertEquals, "Values not equal: Test_AssertEqualsIC_DifferentText_Fails (",
        "): Expected: \"Hello\" but was \"Help\" (ignoring case). different text"), __func__, __LINE__,
        "AssertEqualsIC shows both values: " + assertEquals->Message);
    CheckTrue(messageMatches(*assertNotEquals, "Values are equal: Test_AssertNotEqualsIC_Wide_DifferentCase_Fails (",
        "): Values: \"Hello World\" and \"hELLO wORLD\" (ignoring case). case differs"), __func__, __LINE__,
        "AssertNotEqualsIC shows both wide values: " + assertNotEquals->Message);
    CheckTrue(messageMatches(*checkEquals, "Check failed for: \"Test_CheckEqualsIC_DifferentText_Fails\" (",
        "): Expected \"Hello\" but was \"Help\" (ignoring case). different text"), __func__, __LINE__,
        "CheckEqualsIC shows both values: " + checkEquals->Message);
    CheckTrue(messageMatches(*checkNonASCII, "Check failed for: \"Test_CheckEqualsIC_Wide_NonASCII_Fails\" (",
        "): Expected \"caf\xC3\x89\" but was \"caf\xC3\xA9\" (ignoring case). only A-Z are case-folded"), __func__,
        __LINE__, "CheckEqualsIC shows wide values as UTF-8: " + checkNonASCII->Message);
    CheckTrue(messageMatches(*checkNotEquals, "Check failed for: \"Test_CheckNotEqualsIC_DifferentCase_Fails\" (",
        "): Both values equal: \"Hello World\" and \"hELLO wORLD\" (ignoring case). case differs"), __func__,
        __LINE__, "CheckNotEqualsIC shows both values: " + checkNotEquals->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ComparesCStringsByContent()
{
    // Arrange
    TFixture_CStringComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(26), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertNull = FindRecord(results, "AssertEquals_NullAndNonNull_Fails");
    TTestCaseRecord const* const checkNull = FindRecord(results, "CheckEquals_NullAndEmpty_Fails");
    AssertTrue(assertNull != nullptr && checkNull != nullptr, __func__, __LINE__, "every expected record exists");

    CheckTrue(assertNull->Message.find("\"(null)\"") != std::string::npos, __func__, __LINE__,
        "an Assert failure shows a null C string as (null)");
    CheckTrue(checkNull->Message.find("Expected \"(null)\" but was \"\"") != std::string::npos, __func__, __LINE__,
        "a Check failure shows a null C string as (null), distinct from an empty one");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ComparesMixedIntegerTypesByValue()
{
    // Arrange
    TFixture_IntegerComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(18), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const unsignedMax = FindRecord(results, "CheckEquals_NegativeAndUnsignedMax_Fails");
    TTestCaseRecord const* const uint64Max = FindRecord(results, "CheckEquals_NegativeAndUint64Max_Fails");
    AssertTrue(unsignedMax != nullptr && uint64Max != nullptr, __func__, __LINE__, "every expected record exists");

    CheckTrue(unsignedMax->Message.find("Expected \"-1\" but was \"4294967295\"") != std::string::npos, __func__,
        __LINE__, "a failure shows both values as they are, without wrapping either");
    CheckTrue(uint64Max->Message.find("Expected \"-1\" but was \"18446744073709551615\"") != std::string::npos,
        __func__, __LINE__, "a failure beyond int64_t's range shows both values as they are");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ShowsBoolValuesAsTrueOrFalse()
{
    // Arrange
    TFixture_BoolComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    TTestCaseRecord const* const assertEquals = FindRecord(results, "AssertEquals_Different_Fails");
    TTestCaseRecord const* const assertNotEquals = FindRecord(results, "AssertNotEquals_Same_Fails");
    TTestCaseRecord const* const checkEquals = FindRecord(results, "CheckEquals_Different_Fails");
    TTestCaseRecord const* const checkNotEquals = FindRecord(results, "CheckNotEquals_Same_Fails");
    AssertTrue(assertEquals != nullptr && assertNotEquals != nullptr && checkEquals != nullptr &&
        checkNotEquals != nullptr, __func__, __LINE__, "every expected record exists");

    CheckTrue(assertEquals->Message.find("\"true\" but was \"false\"") != std::string::npos, __func__, __LINE__,
        "AssertEquals shows bools as true/false");
    CheckTrue(assertNotEquals->Message.find("Value: \"true\"") != std::string::npos, __func__, __LINE__,
        "AssertNotEquals shows bools as true/false");
    CheckTrue(checkEquals->Message.find("Expected \"true\" but was \"false\"") != std::string::npos, __func__,
        __LINE__, "CheckEquals shows bools as true/false");
    CheckTrue(checkNotEquals->Message.find("Both values equal: \"true\"") != std::string::npos, __func__, __LINE__,
        "CheckNotEquals shows bools as true/false");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Ordering_ComparesByValueAndShowsBoth()
{
    // Arrange
    TFixture_OrderingComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(36), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    std::vector<std::pair<std::string, std::string> > const expectedDetails = {
        { "AssertGreaterThanOrEqual_Less_Fails", "): Expected 1 to be >= 2. less" },
        { "CheckGreaterThan_Double_Fails", "): Expected 0.1 to be > 0.25. less" },
        { "CheckGreaterThan_Equal_Fails", "): Expected 4 to be > 4. equal" },
        { "CheckGreaterThan_Float_Fails", "): Expected 0.1 to be > 0.25. less" },
        { "CheckGreaterThan_LongDouble_Fails", "): Expected 1.5 to be > 2.5. less" },
        { "CheckLessThanOrEqual_Greater_Fails", "): Expected 5 to be <= 4. greater" },
        { "CheckLessThan_CloseDoubles_Fails", "): Expected 1.0000000000000002 to be < 1. just greater" },
        { "CheckLessThan_Uint64MaxAndNegative_Fails", "): Expected 18446744073709551615 to be < -1. greater" },
    };

    for (std::pair<std::string, std::string> const& expected : expectedDetails)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.first);
        AssertTrue(record != nullptr, __func__, __LINE__, expected.first + " has a record");

        std::string const prefix = (expected.first.compare(0, 6, "Assert") == 0) ?
                "Values out of order: Test_" + expected.first + " (" :
                "Check failed for: \"Test_" + expected.first + "\" (";
        CheckTrue(record->Message.find(prefix) == 0 && NameEndsWith(record->Message, expected.second), __func__,
            __LINE__, expected.first + " shows the value and the bound: " + record->Message);
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_AbandonsHungTestAndAbortsGroupOnTimeout()
{
    // Arrange
    TFixture_SlowTest fixture;
    bool timedOutThrown = false;

    // Act
    try
    {
        fixture.Run(TestFilter(), std::nullopt, 1u, false); // 1 second timeout; the fixture hangs forever.
    }
    catch (TExceptTestTimedOut const&)
    {
        timedOutThrown = true;
    }

    // Assert
    CheckTrue(timedOutThrown, __func__, __LINE__, "Run() throws TExceptTestTimedOut once the timeout is exceeded");
    CheckFalse(fixture.NeverRunReached, __func__, __LINE__,
        "the test registered after the slow one never runs; the group is abandoned, not just that one test");

    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(1), results.CaseRecords.size(), __func__, __LINE__,
        "only a single synthetic record exists, for the abandoned test");
    CheckEquals(1u, results.FailedCount, __func__, __LINE__, "the abandoned test counts as failed");

    TTestCaseRecord const& record = results.CaseRecords.front();
    CheckEquals(std::string("HangsForever"), record.TestName, __func__, __LINE__,
        "the synthetic record names the test that actually timed out");
    CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, "recorded as Fail, not Skip");
    CheckTrue(record.Message.find("timeout") != std::string::npos, __func__, __LINE__,
        "the failure message explains why: it exceeded its timeout");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_AppliesFilterToSkipNonMatchingTests()
{
    // Arrange
    std::vector<std::string> executedTests;
    TFixture_OrderRecorder fixture(executedTests);

    TestFilter const filter = [](std::string const& fullTestName)
        {
            return fullTestName == "Fixture_OrderRecorder.C" || fullTestName == "Fixture_OrderRecorder.E";
        };

    // Act
    fixture.Run(filter, std::nullopt, std::nullopt, false);

    // Assert
    std::vector<std::string> sortedExecuted = executedTests;
    std::sort(sortedExecuted.begin(), sortedExecuted.end());
    std::vector<std::string> const expected = { "C", "E" };
    CheckTrue(sortedExecuted == expected, __func__, __LINE__, "only the filter-matching tests ran, and none else");

    TTestResults const& results = fixture.Results();
    CheckEquals(2u, results.SuccessCount, __func__, __LINE__, "Results() reflects only the tests that actually ran");
    CheckEquals(static_cast<size_t>(2), results.CaseRecords.size(), __func__, __LINE__,
        "CaseRecords has an entry only for the tests that ran, not the full registered set");
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_TestBase::Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet

    Skip()ped on RAD Studio's 32-bit compiler (bcc32c) specifically: the crash here is caught and
    reported correctly (this is not the same failure as Test_ASWUnitTests_CrashGuard.cpp's Skip()ped
    tests, which involve much deeper recursion), but the process still crashes moments later, before
    ever reaching this method's own assertions - confirmed to happen even with nothing else running
    before or after the one crashing test, so it is not about cumulative state or later tests
    specifically. This means --catch-crashes' core "catch and continue" behavior, run through the
    real test-execution pipeline (TTestGroupBase::Run() and beyond) rather than a bare, isolated
    TCrashGuard::Run() call, is not currently safe to rely on for real use on 32-bit RAD Studio at
    all, not just for the deep/stack-overflow edge cases 64-bit and bcc32c share - see the commit
    that added this Skip() for the full investigation. 64-bit RAD Studio and MinGW are unaffected.
*/
void TTest_ASWUnitTests_TestBase::Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet()
{
#if defined(__BORLANDC__) && defined(_WIN32) && !defined(_WIN64)
    Skip(__func__, __LINE__,
        "Unreliable on RAD Studio's 32-bit compiler (bcc32c): the crash is caught correctly, but the "
        "process crashes moments later regardless, even with nothing else running before or after. "
        "MinGW and 64-bit RAD Studio are unaffected; investigation ongoing.");
#endif

    // Arrange
    TFixture_CrashingTest fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, true); // catchCrashes = true

    // Assert
    CheckTrue(fixture.RunsAfterTheCrashReached, __func__, __LINE__,
        "unlike a timeout, an ordinary caught crash doesn't abort the group: the next test still runs");

    TTestResults const& results = fixture.Results();
    CheckFalse(results.Crashed, __func__, __LINE__,
        "Crashed is only set when a crash forces an abort, not for one that was safely continued past");
    CheckEquals(static_cast<size_t>(2), results.CaseRecords.size(), __func__, __LINE__,
        "one record for the crashed test, one for the test that ran normally after it");
    CheckEquals(1u, results.FailedCount, __func__, __LINE__, "the crashed test counts as failed");
    CheckEquals(1u, results.SuccessCount, __func__, __LINE__, "the test after it passed normally");

    TTestCaseRecord const& crashedRecord = results.CaseRecords.front();
    CheckEquals(std::string("CrashesButDoesNotAbort"), crashedRecord.TestName, __func__, __LINE__,
        "the synthetic record names the test that actually crashed");
    CheckTrue(crashedRecord.Outcome == TTestOutcome::Fail, __func__, __LINE__, "recorded as Fail, not Skip");
    CheckTrue(crashedRecord.Message.find("crashed") != std::string::npos, __func__, __LINE__,
        "the failure message explains why: it crashed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_LogsEachCheckFailureOnce()
{
    // Arrange
    TFixture_MixedOutcomes fixture(false); // Logging on, so the console output can be inspected.
    std::string output;

    // Act
    {
        TStdOutRedirect redirect;
        fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        output = redirect.Str();
    }

    // Assert
    // A failed test's record carries its Check* failures, but each one was already logged as it happened, so
    // the "***Test failed" line must not repeat them.
    CheckEquals(static_cast<size_t>(1), CountOccurrences(output, "Check failed for: \"Test_FailViaCheck\""),
        __func__, __LINE__, "FailViaCheck's Check failure is logged exactly once");
    CheckEquals(static_cast<size_t>(1), CountOccurrences(output, "Check failed for: \"Test_FailViaCheckThenAssert\""),
        __func__, __LINE__, "FailViaCheckThenAssert's Check failure is logged exactly once");
    CheckEquals(static_cast<size_t>(2),
        CountOccurrences(output, "Check failed for: \"Test_ContinuesAfterCheckFailure\""), __func__, __LINE__,
        "each of ContinuesAfterCheckFailure's two Check failures is logged exactly once");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsCheckFailuresInFailedTestDetail()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    TTestCaseRecord const* const failViaCheck = FindRecord(results, "FailViaCheck");
    TTestCaseRecord const* const continuesAfterCheck = FindRecord(results, "ContinuesAfterCheckFailure");
    TTestCaseRecord const* const checkThenAssert = FindRecord(results, "FailViaCheckThenAssert");
    TTestCaseRecord const* const failViaAssert = FindRecord(results, "FailViaAssert");
    TTestCaseRecord const* const pass = FindRecord(results, "Pass");

    AssertTrue(failViaCheck != nullptr && continuesAfterCheck != nullptr && checkThenAssert != nullptr &&
        failViaAssert != nullptr && pass != nullptr, __func__, __LINE__, "every expected record exists");

    CheckEquals(static_cast<size_t>(0), failViaCheck->Message.find("Check failed for: \"Test_FailViaCheck\""),
        __func__, __LINE__, "a Check-only failure's detail is its Check failure, not empty");
    CheckEquals(std::string::npos, failViaCheck->Message.find('\n'), __func__, __LINE__,
        "one Check failure is one line, with no other test's Check failures carried over");

    CheckEquals(static_cast<size_t>(2),
        CountOccurrences(continuesAfterCheck->Message, "Check failed for: \"Test_ContinuesAfterCheckFailure\""),
        __func__, __LINE__, "both Check failures are in the detail");
    CheckTrue(continuesAfterCheck->Message.find('\n') != std::string::npos, __func__, __LINE__,
        "multiple Check failures are separated by newlines");

    size_t const checkPos = checkThenAssert->Message.find("deliberate Check failure before an Assert");
    size_t const assertPos = checkThenAssert->Message.find("deliberate Assert failure after a Check");
    CheckTrue(checkPos != std::string::npos && assertPos != std::string::npos, __func__, __LINE__,
        "a Check failure followed by an Assert failure records both");
    CheckTrue(checkPos < assertPos, __func__, __LINE__, "in the order they happened");

    CheckFalse(failViaAssert->Message.empty(), __func__, __LINE__, "an Assert failure still records its message");
    CheckEquals(std::string::npos, failViaAssert->Message.find("Check failed for"), __func__, __LINE__,
        "a test with no Check failures gets none in its detail");
    CheckTrue(pass->Message.empty(), __func__, __LINE__, "a passing test's detail stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();

    CheckEquals(1u, results.SuccessCount, __func__, __LINE__, "only the trivially-passing test succeeded");
    CheckEquals(4u, results.FailedCount, __func__, __LINE__,
        "the Check-failing, Assert-failing, Check-then-Assert, and continues-after-Check tests are all counted as failed");
    CheckEquals(1u, results.SkippedCount, __func__, __LINE__, "the skipped test is counted separately from failures");
    CheckEquals(static_cast<size_t>(6), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (record.TestName == "Pass")
        {
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, "Pass recorded as Pass");
        }
        else if (record.TestName == "Skip")
        {
            CheckTrue(record.Outcome == TTestOutcome::Skip, __func__, __LINE__, "Skip recorded as Skip");
        }
        else
        {
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " recorded as Fail");
        }
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_ReportsEachTestToRunObserver()
{
    // Arrange
    TFixture_MixedOutcomes fixture(false); // Logging on, so where the log output goes can be checked.
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);
    std::string consoleOutput;

    // Act
    {
        TStdOutRedirect redirect;
        fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        consoleOutput = redirect.Str();
    }

    // Assert
    TTestResults const& results = fixture.Results();
    AssertEquals(static_cast<size_t>(6), results.CaseRecords.size(), __func__, __LINE__, "the fixture ran all 6 tests");
    AssertEquals(results.CaseRecords.size() * 2, observer.Events.size(), __func__, __LINE__,
        "one start and one finish event per test");

    for (size_t i = 0; i < results.CaseRecords.size(); ++i)
    {
        TTestCaseRecord const& record = results.CaseRecords[i];
        CheckEquals("started:" + record.TestName, observer.Events[i * 2], __func__, __LINE__,
            "each test's start comes before its finish, in run order");
        CheckEquals("finished:" + record.TestName, observer.Events[i * 2 + 1], __func__, __LINE__,
            "and its finish comes before the next test starts");
        CheckTrue(observer.FinishedRecords[i].Outcome == record.Outcome, __func__, __LINE__,
            record.TestName + ": the finish event carries the test's recorded outcome");
        CheckEquals(record.Message, observer.FinishedRecords[i].Message, __func__, __LINE__,
            record.TestName + ": and its recorded detail");
    }

    CheckTrue(observer.LogText().find("Running test: Fixture_MixedOutcomes.Pass") != std::string::npos, __func__,
        __LINE__, "the group's log output goes to the observer");
    CheckTrue(consoleOutput.empty(), __func__, __LINE__, "and none of it goes to std::cout");
    CheckFalse(results.Stopped, __func__, __LINE__, "a run the observer never asked to stop isn't marked stopped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_ReportsRunObserverEventsOnCallingThreadUnderTimeout()
{
    // Arrange
    // A timeout runs each test on a worker thread, but a GUI observer needs its events on the thread that
    // called Run(), so they're raised there regardless.
    TFixture_MixedOutcomes fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);

    // Act
    fixture.Run(TestFilter(), std::nullopt, 30u, false); // Only long enough never to fire.

    // Assert
    CheckEquals(static_cast<size_t>(6), observer.FinishedRecords.size(), __func__, __LINE__, "every test reported");
    CheckTrue(observer.EventsOnConstructingThread, __func__, __LINE__,
        "OnTestStarted/OnTestFinished/StopRequested all arrived on the calling thread");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_ReportsTimedOutTestToRunObserver()
{
    // Arrange
    TFixture_SlowTest fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);
    bool timedOutThrown = false;

    // Act
    try
    {
        fixture.Run(TestFilter(), std::nullopt, 1u, false); // 1 second timeout; the fixture hangs forever.
    }
    catch (TExceptTestTimedOut const&)
    {
        timedOutThrown = true;
    }

    // Assert
    AssertTrue(timedOutThrown, __func__, __LINE__, "the run was aborted by the timeout");
    CheckEquals(static_cast<size_t>(2), observer.Events.size(), __func__, __LINE__,
        "the hung test started and finished; the test after it never started");
    AssertEquals(static_cast<size_t>(1), observer.FinishedRecords.size(), __func__, __LINE__,
        "the timed-out test is reported even though an exception ended the run");
    CheckEquals(std::string("HangsForever"), observer.FinishedRecords.front().TestName, __func__, __LINE__,
        "the report names the test that timed out");
    CheckTrue(observer.FinishedRecords.front().Outcome == TTestOutcome::Fail, __func__, __LINE__,
        "and records it as failed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_ResetsResultsBetweenRuns()
{
    // Arrange
    TFixture_MixedOutcomes fixture;
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
    TTestResults const firstRun = fixture.Results();

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& secondRun = fixture.Results();
    CheckEquals(firstRun.SuccessCount, secondRun.SuccessCount, __func__, __LINE__,
        "the second run's pass count is its own, not added to the first run's");
    CheckEquals(firstRun.FailedCount, secondRun.FailedCount, __func__, __LINE__, "same for failures");
    CheckEquals(firstRun.SkippedCount, secondRun.SkippedCount, __func__, __LINE__, "same for skips");
    CheckEquals(firstRun.Messages.size(), secondRun.Messages.size(), __func__, __LINE__,
        "the second run's messages don't include the first run's");
    CheckEquals(firstRun.CaseRecords.size(), secondRun.CaseRecords.size(), __func__, __LINE__,
        "one record per test, not one per test per run");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_ShuffleSeedProducesDeterministicOrder()
{
    // Arrange
    std::vector<std::string> unshuffledOrder;
    std::vector<std::string> shuffledOrderFirstRun;
    std::vector<std::string> shuffledOrderSecondRun;

    // Act
    {
        TFixture_OrderRecorder fixture(unshuffledOrder);
        fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
    }
    {
        TFixture_OrderRecorder fixture(shuffledOrderFirstRun);
        fixture.Run(TestFilter(), 12345u, std::nullopt, false);
    }
    {
        TFixture_OrderRecorder fixture(shuffledOrderSecondRun);
        fixture.Run(TestFilter(), 12345u, std::nullopt, false);
    }

    // Assert
    CheckEquals(static_cast<size_t>(8), unshuffledOrder.size(), __func__, __LINE__, "all 8 tests ran");
    CheckTrue(shuffledOrderFirstRun == shuffledOrderSecondRun, __func__, __LINE__,
        "the same shuffle seed produces the same execution order every time");
    CheckTrue(unshuffledOrder != shuffledOrderFirstRun, __func__, __LINE__,
        "a shuffle seed actually reorders execution, rather than being a no-op");

    std::vector<std::string> sortedUnshuffled = unshuffledOrder;
    std::vector<std::string> sortedShuffled = shuffledOrderFirstRun;
    std::sort(sortedUnshuffled.begin(), sortedUnshuffled.end());
    std::sort(sortedShuffled.begin(), sortedShuffled.end());
    CheckTrue(sortedUnshuffled == sortedShuffled, __func__, __LINE__,
        "shuffling reorders tests without dropping or duplicating any of them");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_StopsWhenRunObserverRequests()
{
    // Arrange
    TFixture_MixedOutcomes fixture;
    TRecordingObserver observer;
    observer.StopAfterFinishedCount = 2;
    fixture.SetRunObserver(&observer);

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckTrue(results.Stopped, __func__, __LINE__, "the results say the run was stopped");
    CheckEquals(static_cast<size_t>(2), results.CaseRecords.size(), __func__, __LINE__,
        "only the tests that finished before the stop request were run");
    CheckEquals(static_cast<size_t>(4), observer.Events.size(), __func__, __LINE__,
        "and no further test was even started");
    CheckEquals(static_cast<size_t>(3), observer.StopRequestedCalls, __func__, __LINE__,
        "the observer was asked before each test, up to and including the one that stopped the run");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_AssertFailureStillFailsAndIsRecorded()
{
    // Arrange
    TFixture_ExceptionExpectations fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    // Test_SetExceptionExpected_MatchesTypeAndMessage already checks these tests fail; this checks the Assert*
    // failure itself is what's recorded, as it would be with no exception expected.
    size_t assertFailedTests = 0;

    for (TTestCaseRecord const& record : fixture.Results().CaseRecords)
    {
        if (!NameEndsWith(record.TestName, "_AssertFails_Fails"))
            continue;

        ++assertFailedTests;
        CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " fails");
        CheckTrue(record.Message.find("deliberate Assert failure while an exception is expected") != std::string::npos,
            __func__, __LINE__, record.TestName + "'s record carries its Assert failure");
    }

    CheckEquals(static_cast<size_t>(2), assertFailedTests, __func__, __LINE__,
        "both Assert-while-exception-expected tests ran");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_EarlierCheckFailureStillFailsAndIsRecorded()
{
    // Arrange
    TFixture_ExceptionExpectations fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    // Test_SetExceptionExpected_MatchesTypeAndMessage already checks these tests fail; this checks why is
    // recorded, since the logged "***Test failed" line for a Check-only failure has no detail of its own.
    size_t checkFailedTests = 0;

    for (TTestCaseRecord const& record : fixture.Results().CaseRecords)
    {
        if (record.TestName.rfind("CheckFailedThen", 0) != 0)
            continue;

        ++checkFailedTests;
        CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " fails");
        CheckTrue(record.Message.find("deliberate Check failure before the expected exception") != std::string::npos,
            __func__, __LINE__, record.TestName + "'s record carries its Check failure");
    }

    CheckEquals(static_cast<size_t>(4), checkFailedTests, __func__, __LINE__,
        "all four Check-then-expected-exception tests ran");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_MatchesTypeAndMessage()
{
    // Arrange
    TFixture_ExceptionExpectations fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(15), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_SetLogSuppressed_SilencesFixtureOutput()
{
    // Arrange
    TFixture_MixedOutcomes verboseFixture(false);
    TFixture_MixedOutcomes suppressedFixture(true);
    std::string verboseOutput;
    std::string suppressedOutput;

    // Act
    {
        TStdOutRedirect redirect;
        verboseFixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        verboseOutput = redirect.Str();
    }
    {
        TStdOutRedirect redirect;
        suppressedFixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        suppressedOutput = redirect.Str();
    }

    // Assert
    CheckFalse(verboseOutput.empty(), __func__, __LINE__, "an unsuppressed fixture logs its test run as usual");
    CheckTrue(suppressedOutput.empty(), __func__, __LINE__,
        "SetLogSuppressed(true) silences the fixture's own Log()/LogAppend()");
}
//---------------------------------------------------------------------------
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
void TTest_ASWUnitTests_TestBase::Test_SourceLocation_ReportsCallerFunctionAndLine()
{
    // Arrange
    TFixture_SourceLocations fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(42), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
        {
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
            continue;
        }

        if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else if (NameEndsWith(record.TestName, "_Skips"))
            CheckTrue(record.Outcome == TTestOutcome::Skip, __func__, __LINE__, record.TestName + " should skip");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes, _Fails or _Skips");

        // function_name() is compiler-specific (e.g. "void NS::TClass::Test_X()" on GCC), but always contains the
        // function's own name.
        auto const expectedLine = fixture.ExpectedLines.find(record.TestName);
        AssertTrue(expectedLine != fixture.ExpectedLines.end(), __func__, __LINE__,
            record.TestName + " recorded the line of its call");
        CheckTrue(record.Message.find("Test_" + record.TestName) != std::string::npos, __func__, __LINE__,
            record.TestName + " reports its own function: " + record.Message);
        CheckTrue(record.Message.find("(" + std::to_string(expectedLine->second) + ")") != std::string::npos,
            __func__, __LINE__, record.TestName + " reports the line of its call: " + record.Message);
    }

    TTestCaseRecord const* const throughHelper = FindRecord(results, "CheckTrue_ThroughHelper_Fails");
    AssertTrue(throughHelper != nullptr, __func__, __LINE__, "the helper test's record exists");
    CheckTrue(throughHelper->Message.find("CheckIsEven") == std::string::npos, __func__, __LINE__,
        "a helper that passes its caller's location through isn't itself reported");
}
//---------------------------------------------------------------------------
#endif // #if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
void TTest_ASWUnitTests_TestBase::Test_TrueFalse_FailureNamesTheExpectedValue()
{
    // Arrange
    TFixture_TrueFalseChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(8), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckTrue(record.Outcome == TTestOutcome::Pass, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckTrue(record.Outcome == TTestOutcome::Fail, __func__, __LINE__, record.TestName + " should fail");
        else
            AssertTrue(false, __func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertFalse = FindRecord(results, "AssertFalse_True_Fails");
    TTestCaseRecord const* const assertTrue = FindRecord(results, "AssertTrue_False_Fails");
    TTestCaseRecord const* const checkFalse = FindRecord(results, "CheckFalse_True_Fails");
    TTestCaseRecord const* const checkTrue = FindRecord(results, "CheckTrue_False_Fails");
    AssertTrue(assertFalse != nullptr && assertTrue != nullptr && checkFalse != nullptr && checkTrue != nullptr,
        __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };

    CheckTrue(messageMatches(*assertFalse, "Expected false but was true: Test_AssertFalse_True_Fails (",
        "): value is true"), __func__, __LINE__, "AssertFalse expects false: " + assertFalse->Message);
    CheckTrue(messageMatches(*assertTrue, "Expected true but was false: Test_AssertTrue_False_Fails (",
        "): value is false"), __func__, __LINE__, "AssertTrue expects true: " + assertTrue->Message);
    CheckTrue(messageMatches(*checkFalse, "Check failed for: \"Test_CheckFalse_True_Fails\" (",
        "): Expected false but was true: \"value is true\""), __func__, __LINE__,
        "CheckFalse expects false: " + checkFalse->Message);
    CheckTrue(messageMatches(*checkTrue, "Check failed for: \"Test_CheckTrue_False_Fails\" (",
        "): Expected true but was false: \"value is false\""), __func__, __LINE__,
        "CheckTrue expects true: " + checkTrue->Message);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_TestBase)
