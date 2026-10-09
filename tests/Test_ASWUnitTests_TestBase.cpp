/* **************************************************************************
Test_ASWUnitTests_TestBase.cpp
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
#include "Test_ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <forward_list>
#include <functional>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>
// The test header's ASWUnitTests_TestBase.h includes <version>, where available, for this feature-test macro.
#if defined(__cpp_lib_string_view)
#  include <string_view>
#endif
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_JUnitReport.h"
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
// TFixture_EmptyChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEmpty()/CheckEmpty()/
// AssertNotEmpty()/CheckNotEmpty() with strings, wide strings, containers with and without size(), and (where the
// library has it) std::string_view, so Test_Empty_ShowsContentsOnFailure below can check what each failure shows.
// Test names self-document expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_EmptyChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEmpty_EmptyString_Passes();
    void Test_AssertEmpty_String_Fails();
    void Test_AssertNotEmpty_EmptyVector_Fails();
    void Test_AssertNotEmpty_Vector_Passes();
    void Test_CheckEmpty_EmptyMap_Passes();
    void Test_CheckEmpty_ForwardList_Fails();
    void Test_CheckEmpty_OneElement_Fails();
#if defined(__cpp_lib_string_view)
    void Test_CheckEmpty_StringView_Fails();
#endif
    void Test_CheckEmpty_Vector_Fails();
    void Test_CheckEmpty_WideString_Fails();
    void Test_CheckNotEmpty_EmptyString_Fails();
    void Test_CheckNotEmpty_String_Passes();

public:
    TFixture_EmptyChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_EmptyChecks::TFixture_EmptyChecks()
    : inherited("Fixture_EmptyChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_EmptyChecks::Test_AssertEmpty_EmptyString_Passes, "AssertEmpty_EmptyString_Passes");
    RegisterTest(&TFixture_EmptyChecks::Test_AssertEmpty_String_Fails, "AssertEmpty_String_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_AssertNotEmpty_EmptyVector_Fails, "AssertNotEmpty_EmptyVector_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_AssertNotEmpty_Vector_Passes, "AssertNotEmpty_Vector_Passes");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_EmptyMap_Passes, "CheckEmpty_EmptyMap_Passes");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_ForwardList_Fails, "CheckEmpty_ForwardList_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_OneElement_Fails, "CheckEmpty_OneElement_Fails");
#if defined(__cpp_lib_string_view)
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_StringView_Fails, "CheckEmpty_StringView_Fails");
#endif
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_Vector_Fails, "CheckEmpty_Vector_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckEmpty_WideString_Fails, "CheckEmpty_WideString_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckNotEmpty_EmptyString_Fails, "CheckNotEmpty_EmptyString_Fails");
    RegisterTest(&TFixture_EmptyChecks::Test_CheckNotEmpty_String_Passes, "CheckNotEmpty_String_Passes");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_AssertEmpty_EmptyString_Passes()
{
    AssertEmpty(std::string(), __func__, __LINE__, "empty string");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_AssertEmpty_String_Fails()
{
    AssertEmpty(std::string("abc"), __func__, __LINE__, "has text");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_AssertNotEmpty_EmptyVector_Fails()
{
    AssertNotEmpty(std::vector<int>(), __func__, __LINE__, "no elements");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_AssertNotEmpty_Vector_Passes()
{
    AssertNotEmpty(std::vector<int>{ 1 }, __func__, __LINE__, "one element");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckEmpty_EmptyMap_Passes()
{
    CheckEmpty(std::map<std::string, int>(), __func__, __LINE__, "empty map");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckEmpty_ForwardList_Fails()
{
    // std::forward_list has empty() but no size().
    CheckEmpty(std::forward_list<int>{ 1, 2 }, __func__, __LINE__, "no size to show");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckEmpty_OneElement_Fails()
{
    CheckEmpty(std::vector<int>{ 7 }, __func__, __LINE__, "one element");
}
//---------------------------------------------------------------------------
#if defined(__cpp_lib_string_view)
void TFixture_EmptyChecks::Test_CheckEmpty_StringView_Fails()
{
    CheckEmpty(std::string_view("xyz"), __func__, __LINE__, "shown as text, not an element count");
}
//---------------------------------------------------------------------------
#endif
void TFixture_EmptyChecks::Test_CheckEmpty_Vector_Fails()
{
    CheckEmpty(std::vector<int>{ 1, 2, 3 }, __func__, __LINE__, "three elements");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckEmpty_WideString_Fails()
{
    // U+00E9 (e with acute accent) is shown as its UTF-8 bytes.
    CheckEmpty(std::wstring(L"caf\x00E9"), __func__, __LINE__, "wide text");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckNotEmpty_EmptyString_Fails()
{
    CheckNotEmpty(std::string(), __func__, __LINE__, "empty string");
}
//---------------------------------------------------------------------------
void TFixture_EmptyChecks::Test_CheckNotEmpty_String_Passes()
{
    CheckNotEmpty(std::string("abc"), __func__, __LINE__, "has text");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_EndsWithComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEndsWith()/CheckEndsWith()/
// AssertNotEndsWith()/CheckNotEndsWith(), narrow and wide. "SuffixOnlyAtStart" and "SuffixOnlyInMiddle" catch a
// check that matches anywhere but the end, and "SuffixLongerThanText" one that doesn't check the length first.
// "Wide_NonASCII" checks that a wide failure shows its text as UTF-8. Test names self-document expected outcome via
// NameEndsWith(), same as TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_EndsWithComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEndsWith_Absent_Fails();
    void Test_AssertEndsWith_Present_Passes();
    void Test_AssertEndsWith_Wide_Absent_Fails();
    void Test_AssertEndsWith_Wide_Present_Passes();
    void Test_AssertNotEndsWith_Absent_Passes();
    void Test_AssertNotEndsWith_Present_Fails();
    void Test_AssertNotEndsWith_Wide_Absent_Passes();
    void Test_AssertNotEndsWith_Wide_Present_Fails();
    void Test_CheckEndsWith_Absent_Fails();
    void Test_CheckEndsWith_BothEmpty_Passes();
    void Test_CheckEndsWith_DifferentCase_Fails();
    void Test_CheckEndsWith_EmptySuffix_Passes();
    void Test_CheckEndsWith_EmptyText_Fails();
    void Test_CheckEndsWith_Present_Passes();
    void Test_CheckEndsWith_SuffixLongerThanText_Fails();
    void Test_CheckEndsWith_SuffixOnlyAtStart_Fails();
    void Test_CheckEndsWith_SuffixOnlyInMiddle_Fails();
    void Test_CheckEndsWith_Wide_Absent_Fails();
    void Test_CheckEndsWith_Wide_NonASCII_Fails();
    void Test_CheckEndsWith_Wide_Present_Passes();
    void Test_CheckNotEndsWith_Absent_Passes();
    void Test_CheckNotEndsWith_BothEmpty_Fails();
    void Test_CheckNotEndsWith_EmptySuffix_Fails();
    void Test_CheckNotEndsWith_Present_Fails();
    void Test_CheckNotEndsWith_SuffixLongerThanText_Passes();
    void Test_CheckNotEndsWith_SuffixOnlyAtStart_Passes();
    void Test_CheckNotEndsWith_SuffixOnlyInMiddle_Passes();
    void Test_CheckNotEndsWith_Wide_Absent_Passes();
    void Test_CheckNotEndsWith_Wide_Present_Fails();

public:
    TFixture_EndsWithComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_EndsWithComparisons::TFixture_EndsWithComparisons()
    : inherited("Fixture_EndsWithComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertEndsWith_Absent_Fails, "AssertEndsWith_Absent_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertEndsWith_Present_Passes, "AssertEndsWith_Present_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertEndsWith_Wide_Absent_Fails,
        "AssertEndsWith_Wide_Absent_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertEndsWith_Wide_Present_Passes,
        "AssertEndsWith_Wide_Present_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Absent_Passes,
        "AssertNotEndsWith_Absent_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Present_Fails,
        "AssertNotEndsWith_Present_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Wide_Absent_Passes,
        "AssertNotEndsWith_Wide_Absent_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Wide_Present_Fails,
        "AssertNotEndsWith_Wide_Present_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_Absent_Fails, "CheckEndsWith_Absent_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_BothEmpty_Passes, "CheckEndsWith_BothEmpty_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_DifferentCase_Fails,
        "CheckEndsWith_DifferentCase_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_EmptySuffix_Passes,
        "CheckEndsWith_EmptySuffix_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_EmptyText_Fails, "CheckEndsWith_EmptyText_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_Present_Passes, "CheckEndsWith_Present_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixLongerThanText_Fails,
        "CheckEndsWith_SuffixLongerThanText_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixOnlyAtStart_Fails,
        "CheckEndsWith_SuffixOnlyAtStart_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixOnlyInMiddle_Fails,
        "CheckEndsWith_SuffixOnlyInMiddle_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_Absent_Fails,
        "CheckEndsWith_Wide_Absent_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_NonASCII_Fails,
        "CheckEndsWith_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_Present_Passes,
        "CheckEndsWith_Wide_Present_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Absent_Passes, "CheckNotEndsWith_Absent_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_BothEmpty_Fails,
        "CheckNotEndsWith_BothEmpty_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_EmptySuffix_Fails,
        "CheckNotEndsWith_EmptySuffix_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Present_Fails, "CheckNotEndsWith_Present_Fails");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixLongerThanText_Passes,
        "CheckNotEndsWith_SuffixLongerThanText_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixOnlyAtStart_Passes,
        "CheckNotEndsWith_SuffixOnlyAtStart_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixOnlyInMiddle_Passes,
        "CheckNotEndsWith_SuffixOnlyInMiddle_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Wide_Absent_Passes,
        "CheckNotEndsWith_Wide_Absent_Passes");
    RegisterTest(&TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Wide_Present_Fails,
        "CheckNotEndsWith_Wide_Present_Fails");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertEndsWith_Absent_Fails()
{
    AssertEndsWith(std::string("hello world"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertEndsWith_Present_Passes()
{
    AssertEndsWith(std::string("hello world"), "world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertEndsWith_Wide_Absent_Fails()
{
    AssertEndsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertEndsWith_Wide_Present_Passes()
{
    AssertEndsWith(std::wstring(L"hello world"), L"world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Absent_Passes()
{
    AssertNotEndsWith(std::string("hello world"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Present_Fails()
{
    AssertNotEndsWith(std::string("hello world"), "world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Wide_Absent_Passes()
{
    AssertNotEndsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_AssertNotEndsWith_Wide_Present_Fails()
{
    AssertNotEndsWith(std::wstring(L"hello world"), L"world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_Absent_Fails()
{
    CheckEndsWith(std::string("hello world"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_BothEmpty_Passes()
{
    CheckEndsWith(std::string(), "", __func__, __LINE__, "even an empty string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_DifferentCase_Fails()
{
    CheckEndsWith(std::string("hello world"), "World", __func__, __LINE__, "the comparison is case-sensitive");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_EmptySuffix_Passes()
{
    CheckEndsWith(std::string("hello world"), "", __func__, __LINE__, "every string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_EmptyText_Fails()
{
    CheckEndsWith(std::string(), "d", __func__, __LINE__, "an empty string ends with no letter");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_Present_Passes()
{
    CheckEndsWith(std::string("hello world"), "hello world", __func__, __LINE__, "the whole text is a suffix");
    CheckEndsWith(std::string("hello world"), "world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixLongerThanText_Fails()
{
    CheckEndsWith(std::string("world"), "hello world", __func__, __LINE__, "the text has the end only");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixOnlyAtStart_Fails()
{
    CheckEndsWith(std::string("hello world"), "hello", __func__, __LINE__, "the suffix is at the start");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_SuffixOnlyInMiddle_Fails()
{
    CheckEndsWith(std::string("hello world"), "lo wo", __func__, __LINE__, "the suffix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_Absent_Fails()
{
    CheckEndsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_NonASCII_Fails()
{
    // "cafe" with an e-acute, a space, and U+1F600 (a surrogate pair where wchar_t is 16 bits), then u-umlaut.
    CheckEndsWith(std::wstring(L"caf" L"\x00E9" L" \U0001F600"), L"\x00FC", __func__, __LINE__, "not present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckEndsWith_Wide_Present_Passes()
{
    CheckEndsWith(std::wstring(L"hello world"), L"world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Absent_Passes()
{
    CheckNotEndsWith(std::string("hello world"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_BothEmpty_Fails()
{
    CheckNotEndsWith(std::string(), "", __func__, __LINE__, "even an empty string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_EmptySuffix_Fails()
{
    CheckNotEndsWith(std::string("hello world"), "", __func__, __LINE__, "every string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Present_Fails()
{
    CheckNotEndsWith(std::string("hello world"), "world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixLongerThanText_Passes()
{
    CheckNotEndsWith(std::string("world"), "hello world", __func__, __LINE__, "the text has the end only");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixOnlyAtStart_Passes()
{
    CheckNotEndsWith(std::string("hello world"), "hello", __func__, __LINE__, "the suffix is at the start");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_SuffixOnlyInMiddle_Passes()
{
    CheckNotEndsWith(std::string("hello world"), "lo wo", __func__, __LINE__, "the suffix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Wide_Absent_Passes()
{
    CheckNotEndsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithComparisons::Test_CheckNotEndsWith_Wide_Present_Fails()
{
    CheckNotEndsWith(std::wstring(L"hello world"), L"world", __func__, __LINE__, "suffix present");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_EndsWithICComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEndsWithIC()/CheckEndsWithIC()/
// AssertNotEndsWithIC()/CheckNotEndsWithIC(), narrow and wide. Only the ASCII letters A-Z are case-folded, as in
// TFixture_ContainsICComparisons above. Test names self-document expected outcome via NameEndsWith(), same as
// TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_EndsWithICComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEndsWithIC_Absent_Fails();
    void Test_AssertEndsWithIC_DifferentCase_Passes();
    void Test_AssertEndsWithIC_Wide_Absent_Fails();
    void Test_AssertEndsWithIC_Wide_DifferentCase_Passes();
    void Test_AssertNotEndsWithIC_Absent_Passes();
    void Test_AssertNotEndsWithIC_DifferentCase_Fails();
    void Test_AssertNotEndsWithIC_Wide_Absent_Passes();
    void Test_AssertNotEndsWithIC_Wide_DifferentCase_Fails();
    void Test_CheckEndsWithIC_Absent_Fails();
    void Test_CheckEndsWithIC_BothEmpty_Passes();
    void Test_CheckEndsWithIC_DifferentCase_Passes();
    void Test_CheckEndsWithIC_EmptySuffix_Passes();
    void Test_CheckEndsWithIC_EmptyText_Fails();
    void Test_CheckEndsWithIC_NonASCII_Fails();
    void Test_CheckEndsWithIC_NonLetters_Fails();
    void Test_CheckEndsWithIC_SuffixLongerThanText_Fails();
    void Test_CheckEndsWithIC_SuffixOnlyAtStart_Fails();
    void Test_CheckEndsWithIC_SuffixOnlyInMiddle_Fails();
    void Test_CheckEndsWithIC_Wide_Absent_Fails();
    void Test_CheckEndsWithIC_Wide_DifferentCase_Passes();
    void Test_CheckEndsWithIC_Wide_NonASCII_Fails();
    void Test_CheckNotEndsWithIC_Absent_Passes();
    void Test_CheckNotEndsWithIC_DifferentCase_Fails();
    void Test_CheckNotEndsWithIC_EmptySuffix_Fails();
    void Test_CheckNotEndsWithIC_SuffixLongerThanText_Passes();
    void Test_CheckNotEndsWithIC_SuffixOnlyAtStart_Passes();
    void Test_CheckNotEndsWithIC_Wide_Absent_Passes();
    void Test_CheckNotEndsWithIC_Wide_DifferentCase_Fails();

public:
    TFixture_EndsWithICComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_EndsWithICComparisons::TFixture_EndsWithICComparisons()
    : inherited("Fixture_EndsWithICComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Absent_Fails, "AssertEndsWithIC_Absent_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_DifferentCase_Passes,
        "AssertEndsWithIC_DifferentCase_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Wide_Absent_Fails,
        "AssertEndsWithIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Wide_DifferentCase_Passes,
        "AssertEndsWithIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Absent_Passes,
        "AssertNotEndsWithIC_Absent_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_DifferentCase_Fails,
        "AssertNotEndsWithIC_DifferentCase_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Wide_Absent_Passes,
        "AssertNotEndsWithIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Wide_DifferentCase_Fails,
        "AssertNotEndsWithIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Absent_Fails, "CheckEndsWithIC_Absent_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_BothEmpty_Passes,
        "CheckEndsWithIC_BothEmpty_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_DifferentCase_Passes,
        "CheckEndsWithIC_DifferentCase_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_EmptySuffix_Passes,
        "CheckEndsWithIC_EmptySuffix_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_EmptyText_Fails,
        "CheckEndsWithIC_EmptyText_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_NonASCII_Fails,
        "CheckEndsWithIC_NonASCII_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_NonLetters_Fails,
        "CheckEndsWithIC_NonLetters_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixLongerThanText_Fails,
        "CheckEndsWithIC_SuffixLongerThanText_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixOnlyAtStart_Fails,
        "CheckEndsWithIC_SuffixOnlyAtStart_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixOnlyInMiddle_Fails,
        "CheckEndsWithIC_SuffixOnlyInMiddle_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_Absent_Fails,
        "CheckEndsWithIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_DifferentCase_Passes,
        "CheckEndsWithIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_NonASCII_Fails,
        "CheckEndsWithIC_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Absent_Passes,
        "CheckNotEndsWithIC_Absent_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_DifferentCase_Fails,
        "CheckNotEndsWithIC_DifferentCase_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_EmptySuffix_Fails,
        "CheckNotEndsWithIC_EmptySuffix_Fails");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_SuffixLongerThanText_Passes,
        "CheckNotEndsWithIC_SuffixLongerThanText_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_SuffixOnlyAtStart_Passes,
        "CheckNotEndsWithIC_SuffixOnlyAtStart_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Wide_Absent_Passes,
        "CheckNotEndsWithIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Wide_DifferentCase_Fails,
        "CheckNotEndsWithIC_Wide_DifferentCase_Fails");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Absent_Fails()
{
    AssertEndsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_DifferentCase_Passes()
{
    AssertEndsWithIC(std::string("Hello World"), "o wORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Wide_Absent_Fails()
{
    AssertEndsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertEndsWithIC_Wide_DifferentCase_Passes()
{
    AssertEndsWithIC(std::wstring(L"Hello World"), L"o wORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Absent_Passes()
{
    AssertNotEndsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_DifferentCase_Fails()
{
    AssertNotEndsWithIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Wide_Absent_Passes()
{
    AssertNotEndsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_AssertNotEndsWithIC_Wide_DifferentCase_Fails()
{
    AssertNotEndsWithIC(std::wstring(L"Hello World"), L"WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Absent_Fails()
{
    CheckEndsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_BothEmpty_Passes()
{
    CheckEndsWithIC(std::string(), "", __func__, __LINE__, "even an empty string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_DifferentCase_Passes()
{
    CheckEndsWithIC(std::string("Hello World"), "o wORLD", __func__, __LINE__, "case differs");
    CheckEndsWithIC(std::string("Hello World"), "hello world", __func__, __LINE__, "the whole text, case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_EmptySuffix_Passes()
{
    CheckEndsWithIC(std::string("Hello World"), "", __func__, __LINE__, "every string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_EmptyText_Fails()
{
    CheckEndsWithIC(std::string(), "d", __func__, __LINE__, "an empty string ends with no letter");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_NonASCII_Fails()
{
    // UTF-8 E-acute and e-acute.
    CheckEndsWithIC(std::string("caf\xC3\x89"), "F\xC3\xA9", __func__, __LINE__, "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_NonLetters_Fails()
{
    CheckEndsWithIC(std::string("a@b[c"), "`b{c", __func__, __LINE__, "@ and `, and [ and {, are not letters");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixLongerThanText_Fails()
{
    CheckEndsWithIC(std::string("World"), "HELLO WORLD", __func__, __LINE__, "the text has the end only");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixOnlyAtStart_Fails()
{
    CheckEndsWithIC(std::string("Hello World"), "HELLO", __func__, __LINE__, "the suffix is at the start");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_SuffixOnlyInMiddle_Fails()
{
    CheckEndsWithIC(std::string("Hello World"), "LO WO", __func__, __LINE__, "the suffix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_Absent_Fails()
{
    CheckEndsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_DifferentCase_Passes()
{
    CheckEndsWithIC(std::wstring(L"Hello World"), L"o wORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckEndsWithIC_Wide_NonASCII_Fails()
{
    // E-acute and e-acute.
    CheckEndsWithIC(std::wstring(L"caf" L"\x00C9"), L"F" L"\x00E9", __func__, __LINE__, "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Absent_Passes()
{
    CheckNotEndsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_DifferentCase_Fails()
{
    CheckNotEndsWithIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_EmptySuffix_Fails()
{
    CheckNotEndsWithIC(std::string("Hello World"), "", __func__, __LINE__, "every string ends with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_SuffixLongerThanText_Passes()
{
    CheckNotEndsWithIC(std::string("World"), "HELLO WORLD", __func__, __LINE__, "the text has the end only");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_SuffixOnlyAtStart_Passes()
{
    CheckNotEndsWithIC(std::string("Hello World"), "HELLO", __func__, __LINE__, "the suffix is at the start");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Wide_Absent_Passes()
{
    CheckNotEndsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "suffix absent");
}
//---------------------------------------------------------------------------
void TFixture_EndsWithICComparisons::Test_CheckNotEndsWithIC_Wide_DifferentCase_Fails()
{
    CheckNotEndsWithIC(std::wstring(L"Hello World"), L"WORLD", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_EnumComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEquals()/CheckEquals()/
// AssertNotEquals()/CheckNotEquals() with scoped enums (enum class), compared and shown by their underlying values,
// including char, signed and 64-bit unsigned underlying types; plus an unscoped enum with an int, which must still
// compile and compare as before. Test names self-document expected outcome via NameEndsWith(), same as
// TFixture_ExceptionExpectations below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_EnumComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

    enum class TBig : uint64_t
    {
        One = 1,
        Max = std::numeric_limits<uint64_t>::max()
    };

    enum class TColor
    {
        Red,
        Green,
        Blue
    };

    enum class TLetter : char
    {
        A = 'A',
        B = 'B'
    };

    enum TPlain
    {
        PlainA,
        PlainB
    };

    enum class TSmall : int8_t
    {
        Negative = -1,
        Positive = 1
    };

private:
    void Test_AssertEquals_Different_Fails();
    void Test_AssertEquals_Same_Passes();
    void Test_AssertNotEquals_Different_Passes();
    void Test_AssertNotEquals_Same_Fails();
    void Test_CheckEquals_CharUnderlying_Fails();
    void Test_CheckEquals_Different_Fails();
    void Test_CheckEquals_NegativeUnderlying_Fails();
    void Test_CheckEquals_Same_Passes();
    void Test_CheckEquals_UInt64Underlying_Fails();
    void Test_CheckEquals_UnscopedEnumAndInt_Passes();
    void Test_CheckNotEquals_Different_Passes();
    void Test_CheckNotEquals_Same_Fails();

public:
    TFixture_EnumComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_EnumComparisons::TFixture_EnumComparisons()
    : inherited("Fixture_EnumComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_EnumComparisons::Test_AssertEquals_Different_Fails, "AssertEquals_Different_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_AssertEquals_Same_Passes, "AssertEquals_Same_Passes");
    RegisterTest(&TFixture_EnumComparisons::Test_AssertNotEquals_Different_Passes, "AssertNotEquals_Different_Passes");
    RegisterTest(&TFixture_EnumComparisons::Test_AssertNotEquals_Same_Fails, "AssertNotEquals_Same_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_CharUnderlying_Fails, "CheckEquals_CharUnderlying_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_Different_Fails, "CheckEquals_Different_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_NegativeUnderlying_Fails,
        "CheckEquals_NegativeUnderlying_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_Same_Passes, "CheckEquals_Same_Passes");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_UInt64Underlying_Fails,
        "CheckEquals_UInt64Underlying_Fails");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckEquals_UnscopedEnumAndInt_Passes,
        "CheckEquals_UnscopedEnumAndInt_Passes");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckNotEquals_Different_Passes, "CheckNotEquals_Different_Passes");
    RegisterTest(&TFixture_EnumComparisons::Test_CheckNotEquals_Same_Fails, "CheckNotEquals_Same_Fails");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_AssertEquals_Different_Fails()
{
    AssertEquals(TColor::Red, TColor::Blue, __func__, __LINE__, "different colors");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_AssertEquals_Same_Passes()
{
    AssertEquals(TColor::Green, TColor::Green, __func__, __LINE__, "same color");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_AssertNotEquals_Different_Passes()
{
    AssertNotEquals(TColor::Red, TColor::Blue, __func__, __LINE__, "different colors");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_AssertNotEquals_Same_Fails()
{
    AssertNotEquals(TColor::Green, TColor::Green, __func__, __LINE__, "same color");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_CharUnderlying_Fails()
{
    CheckEquals(TLetter::A, TLetter::B, __func__, __LINE__, "shown as numbers, not characters");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_Different_Fails()
{
    CheckEquals(TColor::Red, TColor::Blue, __func__, __LINE__, "different colors");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_NegativeUnderlying_Fails()
{
    CheckEquals(TSmall::Negative, TSmall::Positive, __func__, __LINE__, "a negative value stays negative");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_Same_Passes()
{
    CheckEquals(TColor::Green, TColor::Green, __func__, __LINE__, "same color");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_UInt64Underlying_Fails()
{
    CheckEquals(TBig::Max, TBig::One, __func__, __LINE__, "a value above INT64_MAX doesn't wrap");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckEquals_UnscopedEnumAndInt_Passes()
{
    CheckEquals(1, PlainB, __func__, __LINE__, "an unscoped enum still converts to its integer value");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckNotEquals_Different_Passes()
{
    CheckNotEquals(TColor::Red, TColor::Blue, __func__, __LINE__, "different colors");
}
//---------------------------------------------------------------------------
void TFixture_EnumComparisons::Test_CheckNotEquals_Same_Fails()
{
    CheckNotEquals(TColor::Green, TColor::Green, __func__, __LINE__, "same color");
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
// TFixture_FailCalls
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group calling Fail(), so Test_Fail_AbortsTestAsFailed
// below can check it fails the test, stops it (ReachedAfterFail stays false), still fails while an exception is
// expected, and keeps an earlier Check failure in the record. Test names self-document expected outcome via
// NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_FailCalls : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_Fail_AfterCheckFailure_Fails();
    void Test_Fail_Fails();
    void Test_Fail_WhileExceptionExpected_Fails();

public:
    bool ReachedAfterFail = false;

    TFixture_FailCalls();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_FailCalls::TFixture_FailCalls()
    : inherited("Fixture_FailCalls")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_FailCalls::Test_Fail_AfterCheckFailure_Fails, "Fail_AfterCheckFailure_Fails");
    RegisterTest(&TFixture_FailCalls::Test_Fail_Fails, "Fail_Fails");
    RegisterTest(&TFixture_FailCalls::Test_Fail_WhileExceptionExpected_Fails, "Fail_WhileExceptionExpected_Fails");
}
//---------------------------------------------------------------------------
void TFixture_FailCalls::Test_Fail_AfterCheckFailure_Fails()
{
    CheckTrue(false, __func__, __LINE__, "earlier check failure");
    Fail(__func__, __LINE__, "then an unconditional failure");
}
//---------------------------------------------------------------------------
void TFixture_FailCalls::Test_Fail_Fails()
{
    Fail(__func__, __LINE__, "unconditional failure");
    ReachedAfterFail = true;
}
//---------------------------------------------------------------------------
void TFixture_FailCalls::Test_Fail_WhileExceptionExpected_Fails()
{
    SetExceptionExpected(true, __func__, __LINE__, "any exception");
    Fail(__func__, __LINE__, "not the exception the test expects");
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
// TFixture_MatchesChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertMatches()/CheckMatches()/
// AssertNotMatches()/CheckNotMatches(), narrow and wide, so Test_Matches_MatchesWholeTextAndShowsPattern below can
// check their outcomes and messages. "PartialMatch" shows the whole text must match, and "InvalidPattern" that a bad
// pattern fails the check instead of throwing std::regex_error. Test names self-document expected outcome via
// NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_MatchesChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertMatches_InvalidPattern_Fails();
    void Test_AssertMatches_Matching_Passes();
    void Test_AssertMatches_NotMatching_Fails();
    void Test_AssertNotMatches_Matching_Fails();
    void Test_AssertNotMatches_NotMatching_Passes();
    void Test_CheckMatches_InvalidPattern_Fails();
    void Test_CheckMatches_Matching_Passes();
    void Test_CheckMatches_PartialMatch_Fails();
    void Test_CheckMatches_Wide_Matching_Passes();
    void Test_CheckMatches_Wide_NotMatching_Fails();
    void Test_CheckNotMatches_InvalidPattern_Fails();
    void Test_CheckNotMatches_Matching_Fails();
    void Test_CheckNotMatches_NotMatching_Passes();

public:
    TFixture_MatchesChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_MatchesChecks::TFixture_MatchesChecks()
    : inherited("Fixture_MatchesChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_MatchesChecks::Test_AssertMatches_InvalidPattern_Fails,
        "AssertMatches_InvalidPattern_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_AssertMatches_Matching_Passes, "AssertMatches_Matching_Passes");
    RegisterTest(&TFixture_MatchesChecks::Test_AssertMatches_NotMatching_Fails, "AssertMatches_NotMatching_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_AssertNotMatches_Matching_Fails, "AssertNotMatches_Matching_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_AssertNotMatches_NotMatching_Passes,
        "AssertNotMatches_NotMatching_Passes");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckMatches_InvalidPattern_Fails, "CheckMatches_InvalidPattern_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckMatches_Matching_Passes, "CheckMatches_Matching_Passes");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckMatches_PartialMatch_Fails, "CheckMatches_PartialMatch_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckMatches_Wide_Matching_Passes, "CheckMatches_Wide_Matching_Passes");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckMatches_Wide_NotMatching_Fails,
        "CheckMatches_Wide_NotMatching_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckNotMatches_InvalidPattern_Fails,
        "CheckNotMatches_InvalidPattern_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckNotMatches_Matching_Fails, "CheckNotMatches_Matching_Fails");
    RegisterTest(&TFixture_MatchesChecks::Test_CheckNotMatches_NotMatching_Passes,
        "CheckNotMatches_NotMatching_Passes");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_AssertMatches_InvalidPattern_Fails()
{
    AssertMatches(std::string("abc"), std::string("a("), __func__, __LINE__, "unbalanced parenthesis");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_AssertMatches_Matching_Passes()
{
    AssertMatches(std::string("2026-10-07"), std::string(R"(\d{4}-\d{2}-\d{2})"), __func__, __LINE__, "a date");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_AssertMatches_NotMatching_Fails()
{
    AssertMatches(std::string("2026-1-7"), std::string(R"(\d{4}-\d{2}-\d{2})"), __func__, __LINE__, "not a date");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_AssertNotMatches_Matching_Fails()
{
    AssertNotMatches(std::string("abc123"), std::string(R"([a-z]+\d+)"), __func__, __LINE__, "matches");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_AssertNotMatches_NotMatching_Passes()
{
    AssertNotMatches(std::string("abc"), std::string(R"(\d+)"), __func__, __LINE__, "no digits");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckMatches_InvalidPattern_Fails()
{
    CheckMatches(std::string("abc"), std::string("[a-"), __func__, __LINE__, "unterminated bracket");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckMatches_Matching_Passes()
{
    CheckMatches(std::string("Error 42: disk full"), std::string(R"(Error \d+: .*)"), __func__, __LINE__,
        "an error message");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckMatches_PartialMatch_Fails()
{
    // The digits match part of the text, but not all of it.
    CheckMatches(std::string("abc123"), std::string(R"(\d+)"), __func__, __LINE__, "only part matches");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckMatches_Wide_Matching_Passes()
{
    // With std::wregex, '.' matches U+00E9 (e with acute accent) as a single character.
    CheckMatches(std::wstring(L"caf\x00E9"), std::wstring(L"caf."), __func__, __LINE__, "one wide character");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckMatches_Wide_NotMatching_Fails()
{
    CheckMatches(std::wstring(L"caf\x00E9"), std::wstring(L"tea"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckNotMatches_InvalidPattern_Fails()
{
    CheckNotMatches(std::string("abc"), std::string("a("), __func__, __LINE__, "an invalid pattern never passes");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckNotMatches_Matching_Fails()
{
    CheckNotMatches(std::string("abc123"), std::string(R"([a-z]+\d+)"), __func__, __LINE__, "matches");
}
//---------------------------------------------------------------------------
void TFixture_MatchesChecks::Test_CheckNotMatches_NotMatching_Passes()
{
    CheckNotMatches(std::string("abc"), std::string(R"(\d+)"), __func__, __LINE__, "no digits");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_MemoryComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEqualsMem()/CheckEqualsMem()/
// AssertNotEqualsMem()/CheckNotEqualsMem(), so Test_EqualsMem_ShowsFirstDifferingBytes below can check their outcomes
// and messages: a difference at the start, in the middle and past 16 bytes, zero bytes, the same pointer twice, and
// null pointers. Test names self-document expected outcome via NameEndsWith(), same as
// TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_MemoryComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEqualsMem_Different_Fails();
    void Test_AssertEqualsMem_Same_Passes();
    void Test_AssertNotEqualsMem_Different_Passes();
    void Test_AssertNotEqualsMem_Same_Fails();
    void Test_CheckEqualsMem_ActualNull_Fails();
    void Test_CheckEqualsMem_BothNull_Passes();
    void Test_CheckEqualsMem_DifferentLater_Fails();
    void Test_CheckEqualsMem_LongDifference_Fails();
    void Test_CheckEqualsMem_SameBuffer_Passes();
    void Test_CheckEqualsMem_ZeroSize_Passes();
    void Test_CheckNotEqualsMem_Different_Passes();
    void Test_CheckNotEqualsMem_NullAndBuffer_Passes();
    void Test_CheckNotEqualsMem_Same_Fails();

public:
    TFixture_MemoryComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_MemoryComparisons::TFixture_MemoryComparisons()
    : inherited("Fixture_MemoryComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_MemoryComparisons::Test_AssertEqualsMem_Different_Fails, "AssertEqualsMem_Different_Fails");
    RegisterTest(&TFixture_MemoryComparisons::Test_AssertEqualsMem_Same_Passes, "AssertEqualsMem_Same_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_AssertNotEqualsMem_Different_Passes,
        "AssertNotEqualsMem_Different_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_AssertNotEqualsMem_Same_Fails, "AssertNotEqualsMem_Same_Fails");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_ActualNull_Fails, "CheckEqualsMem_ActualNull_Fails");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_BothNull_Passes, "CheckEqualsMem_BothNull_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_DifferentLater_Fails,
        "CheckEqualsMem_DifferentLater_Fails");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_LongDifference_Fails,
        "CheckEqualsMem_LongDifference_Fails");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_SameBuffer_Passes,
        "CheckEqualsMem_SameBuffer_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckEqualsMem_ZeroSize_Passes, "CheckEqualsMem_ZeroSize_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckNotEqualsMem_Different_Passes,
        "CheckNotEqualsMem_Different_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckNotEqualsMem_NullAndBuffer_Passes,
        "CheckNotEqualsMem_NullAndBuffer_Passes");
    RegisterTest(&TFixture_MemoryComparisons::Test_CheckNotEqualsMem_Same_Fails, "CheckNotEqualsMem_Same_Fails");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_AssertEqualsMem_Different_Fails()
{
    unsigned char const expected[] = { 0x01, 0x02, 0x03, 0x04 };
    unsigned char const actual[] = { 0x01, 0x02, 0xFF, 0x04 };
    AssertEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "third byte differs");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_AssertEqualsMem_Same_Passes()
{
    unsigned char const expected[] = { 0x01, 0x02, 0x03, 0x04 };
    unsigned char const actual[] = { 0x01, 0x02, 0x03, 0x04 };
    AssertEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "same bytes, different buffers");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_AssertNotEqualsMem_Different_Passes()
{
    unsigned char const expected[] = { 0x01, 0x02, 0x03, 0x04 };
    unsigned char const actual[] = { 0x01, 0x02, 0x03, 0x05 };
    AssertNotEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "last byte differs");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_AssertNotEqualsMem_Same_Fails()
{
    unsigned char const expected[] = { 0x01, 0x02, 0x03, 0x04 };
    unsigned char const actual[] = { 0x01, 0x02, 0x03, 0x04 };
    AssertNotEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "same bytes");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_ActualNull_Fails()
{
    unsigned char const expected[] = { 0x01, 0x02, 0x03, 0x04 };
    CheckEqualsMem(expected, nullptr, sizeof(expected), __func__, __LINE__, "nothing to compare with");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_BothNull_Passes()
{
    CheckEqualsMem(nullptr, nullptr, 4, __func__, __LINE__, "the same pointer, so never read");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_DifferentLater_Fails()
{
    unsigned char expected[24];
    unsigned char actual[24];
    for (unsigned char i = 0; i < 24; ++i)
    {
        expected[i] = i;
        actual[i] = i;
    }

    actual[20] = 0xAA;
    CheckEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "byte 20 differs");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_LongDifference_Fails()
{
    std::vector<unsigned char> const expected(20, 0x00);
    std::vector<unsigned char> const actual(20, 0x11);
    CheckEqualsMem(expected.data(), actual.data(), expected.size(), __func__, __LINE__, "shows 16 bytes, then ...");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_SameBuffer_Passes()
{
    int const values[] = { 1, 2, 3 };
    CheckEqualsMem(values, values, sizeof(values), __func__, __LINE__, "the same buffer");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckEqualsMem_ZeroSize_Passes()
{
    unsigned char const expected[] = { 0x01 };
    unsigned char const actual[] = { 0x02 };
    CheckEqualsMem(expected, actual, 0, __func__, __LINE__, "zero bytes are always equal");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckNotEqualsMem_Different_Passes()
{
    unsigned char const expected[] = { 0x01, 0x02 };
    unsigned char const actual[] = { 0x02, 0x01 };
    CheckNotEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "different order");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckNotEqualsMem_NullAndBuffer_Passes()
{
    unsigned char const actual[] = { 0x01, 0x02 };
    CheckNotEqualsMem(nullptr, actual, sizeof(actual), __func__, __LINE__, "null isn't equal to any memory");
}
//---------------------------------------------------------------------------
void TFixture_MemoryComparisons::Test_CheckNotEqualsMem_Same_Fails()
{
    unsigned char const expected[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    unsigned char const actual[] = { 0xDE, 0xAD, 0xBE, 0xEF };
    CheckNotEqualsMem(expected, actual, sizeof(expected), __func__, __LINE__, "same bytes");
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
// TFixture_NullChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group calling AssertNull()/CheckNull()/AssertNotNull()/
// CheckNotNull() with null and non-null raw pointers, smart pointers, std::function objects, C strings and a nullptr
// literal, so Test_NullNotNull_FailureNamesTheExpectedValue below can check each is compared with nullptr and that
// each failure states the value that was expected. Test names self-document expected outcome via NameEndsWith(), same
// as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_NullChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNotNull_NonNull_Passes();
    void Test_AssertNotNull_Null_Fails();
    void Test_AssertNull_NonNull_Fails();
    void Test_AssertNull_Null_Passes();
    void Test_CheckNotNull_EmptyCString_Passes();
    void Test_CheckNotNull_EmptyFunction_Fails();
    void Test_CheckNotNull_NonNull_Passes();
    void Test_CheckNotNull_Null_Fails();
    void Test_CheckNotNull_SharedPtr_Passes();
    void Test_CheckNull_EmptyFunction_Passes();
    void Test_CheckNull_NonNull_Fails();
    void Test_CheckNull_NullCString_Passes();
    void Test_CheckNull_Null_Passes();
    void Test_CheckNull_NullptrLiteral_Passes();
    void Test_CheckNull_UniquePtr_Fails();

public:
    TFixture_NullChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_NullChecks::TFixture_NullChecks()
    : inherited("Fixture_NullChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_NullChecks::Test_AssertNotNull_NonNull_Passes, "AssertNotNull_NonNull_Passes");
    RegisterTest(&TFixture_NullChecks::Test_AssertNotNull_Null_Fails, "AssertNotNull_Null_Fails");
    RegisterTest(&TFixture_NullChecks::Test_AssertNull_NonNull_Fails, "AssertNull_NonNull_Fails");
    RegisterTest(&TFixture_NullChecks::Test_AssertNull_Null_Passes, "AssertNull_Null_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNotNull_EmptyCString_Passes, "CheckNotNull_EmptyCString_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNotNull_EmptyFunction_Fails, "CheckNotNull_EmptyFunction_Fails");
    RegisterTest(&TFixture_NullChecks::Test_CheckNotNull_NonNull_Passes, "CheckNotNull_NonNull_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNotNull_Null_Fails, "CheckNotNull_Null_Fails");
    RegisterTest(&TFixture_NullChecks::Test_CheckNotNull_SharedPtr_Passes, "CheckNotNull_SharedPtr_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_EmptyFunction_Passes, "CheckNull_EmptyFunction_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_NonNull_Fails, "CheckNull_NonNull_Fails");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_NullCString_Passes, "CheckNull_NullCString_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_Null_Passes, "CheckNull_Null_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_NullptrLiteral_Passes, "CheckNull_NullptrLiteral_Passes");
    RegisterTest(&TFixture_NullChecks::Test_CheckNull_UniquePtr_Fails, "CheckNull_UniquePtr_Fails");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_AssertNotNull_NonNull_Passes()
{
    int value = 0;
    AssertNotNull(&value, __func__, __LINE__, "pointer is not null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_AssertNotNull_Null_Fails()
{
    int* const pointer = nullptr;
    AssertNotNull(pointer, __func__, __LINE__, "pointer is null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_AssertNull_NonNull_Fails()
{
    int value = 0;
    AssertNull(&value, __func__, __LINE__, "pointer is not null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_AssertNull_Null_Passes()
{
    int* const pointer = nullptr;
    AssertNull(pointer, __func__, __LINE__, "pointer is null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNotNull_EmptyCString_Passes()
{
    char const* const text = "";
    CheckNotNull(text, __func__, __LINE__, "empty text is not a null pointer");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNotNull_EmptyFunction_Fails()
{
    std::function<void()> const callback;
    CheckNotNull(callback, __func__, __LINE__, "function is empty");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNotNull_NonNull_Passes()
{
    int value = 0;
    CheckNotNull(&value, __func__, __LINE__, "pointer is not null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNotNull_Null_Fails()
{
    int* const pointer = nullptr;
    CheckNotNull(pointer, __func__, __LINE__, "pointer is null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNotNull_SharedPtr_Passes()
{
    std::shared_ptr<int> const pointer = std::make_shared<int>(0);
    CheckNotNull(pointer, __func__, __LINE__, "shared_ptr owns an int");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_EmptyFunction_Passes()
{
    std::function<void()> const callback;
    CheckNull(callback, __func__, __LINE__, "function is empty");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_NonNull_Fails()
{
    int value = 0;
    CheckNull(&value, __func__, __LINE__, "pointer is not null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_NullCString_Passes()
{
    char const* const text = nullptr;
    CheckNull(text, __func__, __LINE__, "a null C string is a null pointer, not empty text");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_Null_Passes()
{
    int* const pointer = nullptr;
    CheckNull(pointer, __func__, __LINE__, "pointer is null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_NullptrLiteral_Passes()
{
    CheckNull(nullptr, __func__, __LINE__, "nullptr is null");
}
//---------------------------------------------------------------------------
void TFixture_NullChecks::Test_CheckNull_UniquePtr_Fails()
{
    std::unique_ptr<int> const pointer = std::make_unique<int>(0);
    CheckNull(pointer, __func__, __LINE__, "unique_ptr owns an int");
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
// TFixture_PointerComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertEquals()/CheckEquals()/
// AssertNotEquals()/CheckNotEquals() with two pointers. Without the pointer overloads, two pointers convert to bool
// and match the bool overload, so two different non-null pointers compare equal and the "DifferentPointers" tests
// pass/fail the wrong way round. The tests compare the addresses of First and Second, so
// Test_Equals_ComparesPointersByAddress below can check the addresses a failure shows. Test names self-document
// expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_PointerComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

    struct TBase
    {
        int Value = 0;
    };

    struct TDerived : TBase
    {
    };

    // Different bodies, so a linker that merges identical functions (e.g. MSVC's /OPT:ICF) can't give them one address.
    static int FirstFunction() { return 1; }
    static int SecondFunction() { return 2; }

private:
    void Test_AssertEquals_DifferentPointers_Fails();
    void Test_AssertEquals_SamePointer_Passes();
    void Test_AssertNotEquals_DifferentPointers_Passes();
    void Test_AssertNotEquals_SamePointer_Fails();
    void Test_CheckEquals_BaseAndDerived_Passes();
    void Test_CheckEquals_DifferentFunctions_Fails();
    void Test_CheckEquals_DifferentPointers_Fails();
    void Test_CheckEquals_MutableCStringBuffer_Passes();
    void Test_CheckEquals_NullAndNonNull_Fails();
    void Test_CheckEquals_SamePointer_Passes();
    void Test_CheckEquals_VoidAndTyped_Passes();
    void Test_CheckNotEquals_DifferentPointers_Passes();
    void Test_CheckNotEquals_SamePointer_Fails();

public:
    int First = 0;
    int Second = 0;

    TFixture_PointerComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_PointerComparisons::TFixture_PointerComparisons()
    : inherited("Fixture_PointerComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_PointerComparisons::Test_AssertEquals_DifferentPointers_Fails,
        "AssertEquals_DifferentPointers_Fails");
    RegisterTest(&TFixture_PointerComparisons::Test_AssertEquals_SamePointer_Passes, "AssertEquals_SamePointer_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_AssertNotEquals_DifferentPointers_Passes,
        "AssertNotEquals_DifferentPointers_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_AssertNotEquals_SamePointer_Fails,
        "AssertNotEquals_SamePointer_Fails");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_BaseAndDerived_Passes,
        "CheckEquals_BaseAndDerived_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_DifferentFunctions_Fails,
        "CheckEquals_DifferentFunctions_Fails");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_DifferentPointers_Fails,
        "CheckEquals_DifferentPointers_Fails");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_MutableCStringBuffer_Passes,
        "CheckEquals_MutableCStringBuffer_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_NullAndNonNull_Fails,
        "CheckEquals_NullAndNonNull_Fails");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_SamePointer_Passes, "CheckEquals_SamePointer_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckEquals_VoidAndTyped_Passes, "CheckEquals_VoidAndTyped_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckNotEquals_DifferentPointers_Passes,
        "CheckNotEquals_DifferentPointers_Passes");
    RegisterTest(&TFixture_PointerComparisons::Test_CheckNotEquals_SamePointer_Fails,
        "CheckNotEquals_SamePointer_Fails");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_AssertEquals_DifferentPointers_Fails()
{
    AssertEquals(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_AssertEquals_SamePointer_Passes()
{
    AssertEquals(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_AssertNotEquals_DifferentPointers_Passes()
{
    AssertNotEquals(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_AssertNotEquals_SamePointer_Fails()
{
    AssertNotEquals(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_BaseAndDerived_Passes()
{
    TDerived derived;
    TBase* const base = &derived;
    CheckEquals(base, &derived, __func__, __LINE__, "a base pointer to the derived object");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_DifferentFunctions_Fails()
{
    CheckEquals(&FirstFunction, &SecondFunction, __func__, __LINE__, "different functions");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_DifferentPointers_Fails()
{
    CheckEquals(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_MutableCStringBuffer_Passes()
{
    // A char* (not char const*) must still reach the C string overload, which compares by content.
    char buffer[] = "abc";
    CheckEquals("abc", buffer, __func__, __LINE__, "same text in a different, non-const buffer");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_NullAndNonNull_Fails()
{
    int* const pointer = nullptr;
    CheckEquals(pointer, &First, __func__, __LINE__, "null and non-null");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_SamePointer_Passes()
{
    CheckEquals(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckEquals_VoidAndTyped_Passes()
{
    void const* const untyped = &First;
    CheckEquals(untyped, &First, __func__, __LINE__, "void pointer to the same object");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckNotEquals_DifferentPointers_Passes()
{
    CheckNotEquals(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_PointerComparisons::Test_CheckNotEquals_SamePointer_Fails()
{
    CheckNotEquals(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_SameChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertSame()/CheckSame()/AssertNotSame()/
// CheckNotSame() with raw pointers, arrays, smart pointers and C strings, which unlike CheckEquals() are compared by
// address, not content. The basic tests compare the addresses of First and Second, so
// Test_Same_ComparesAddressesNotContent below can check the addresses a failure shows. Test names self-document
// expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SameChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNotSame_DifferentObjects_Passes();
    void Test_AssertNotSame_SameObject_Fails();
    void Test_AssertSame_DifferentObjects_Fails();
    void Test_AssertSame_SameObject_Passes();
    void Test_CheckNotSame_CStringsSameContent_Passes();
    void Test_CheckNotSame_SameObject_Fails();
    void Test_CheckSame_ArrayAndPointer_Passes();
    void Test_CheckSame_BothNull_Passes();
    void Test_CheckSame_CStringsSameContent_Fails();
    void Test_CheckSame_DifferentObjects_Fails();
    void Test_CheckSame_RawAndSharedPtr_Passes();
    void Test_CheckSame_SameObject_Passes();
    void Test_CheckSame_SharedPtrCopies_Passes();
    void Test_CheckSame_UniquePtrs_Fails();

public:
    int First = 0;
    int Second = 0;

    TFixture_SameChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_SameChecks::TFixture_SameChecks()
    : inherited("Fixture_SameChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_SameChecks::Test_AssertNotSame_DifferentObjects_Passes,
        "AssertNotSame_DifferentObjects_Passes");
    RegisterTest(&TFixture_SameChecks::Test_AssertNotSame_SameObject_Fails, "AssertNotSame_SameObject_Fails");
    RegisterTest(&TFixture_SameChecks::Test_AssertSame_DifferentObjects_Fails, "AssertSame_DifferentObjects_Fails");
    RegisterTest(&TFixture_SameChecks::Test_AssertSame_SameObject_Passes, "AssertSame_SameObject_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckNotSame_CStringsSameContent_Passes,
        "CheckNotSame_CStringsSameContent_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckNotSame_SameObject_Fails, "CheckNotSame_SameObject_Fails");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_ArrayAndPointer_Passes, "CheckSame_ArrayAndPointer_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_BothNull_Passes, "CheckSame_BothNull_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_CStringsSameContent_Fails, "CheckSame_CStringsSameContent_Fails");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_DifferentObjects_Fails, "CheckSame_DifferentObjects_Fails");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_RawAndSharedPtr_Passes, "CheckSame_RawAndSharedPtr_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_SameObject_Passes, "CheckSame_SameObject_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_SharedPtrCopies_Passes, "CheckSame_SharedPtrCopies_Passes");
    RegisterTest(&TFixture_SameChecks::Test_CheckSame_UniquePtrs_Fails, "CheckSame_UniquePtrs_Fails");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_AssertNotSame_DifferentObjects_Passes()
{
    AssertNotSame(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_AssertNotSame_SameObject_Fails()
{
    AssertNotSame(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_AssertSame_DifferentObjects_Fails()
{
    AssertSame(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_AssertSame_SameObject_Passes()
{
    AssertSame(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckNotSame_CStringsSameContent_Passes()
{
    char const buffer[] = "abc";
    CheckNotSame("abc", buffer, __func__, __LINE__, "same text in a different buffer is a different object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckNotSame_SameObject_Fails()
{
    CheckNotSame(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_ArrayAndPointer_Passes()
{
    char buffer[] = "abc";
    char const* const pointer = buffer;
    CheckSame(buffer, pointer, __func__, __LINE__, "a pointer to the array's first element");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_BothNull_Passes()
{
    int* const pointer = nullptr;
    std::shared_ptr<int> const empty;
    CheckSame(pointer, empty, __func__, __LINE__, "both null");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_CStringsSameContent_Fails()
{
    char const buffer[] = "abc";
    CheckSame("abc", buffer, __func__, __LINE__, "same text in a different buffer");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_DifferentObjects_Fails()
{
    CheckSame(&First, &Second, __func__, __LINE__, "different objects");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_RawAndSharedPtr_Passes()
{
    std::shared_ptr<int> const shared = std::make_shared<int>(0);
    int const* const raw = shared.get();
    CheckSame(raw, shared, __func__, __LINE__, "a raw pointer to the shared object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_SameObject_Passes()
{
    CheckSame(&First, &First, __func__, __LINE__, "same object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_SharedPtrCopies_Passes()
{
    std::shared_ptr<int> const original = std::make_shared<int>(0);
    std::shared_ptr<int> const copy = original;
    CheckSame(original, copy, __func__, __LINE__, "copies share one object");
}
//---------------------------------------------------------------------------
void TFixture_SameChecks::Test_CheckSame_UniquePtrs_Fails()
{
    std::unique_ptr<int> const first = std::make_unique<int>(0);
    std::unique_ptr<int> const second = std::make_unique<int>(0);
    CheckSame(first, second, __func__, __LINE__, "two objects with the same value");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ShortTest
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test that busy-waits for
// BusyWaitDuration, far less than a millisecond, timed with std::chrono::steady_clock. Logging is left on, for a
// run observer to collect. Used by Test_Run_RecordsDurationOfShortTest below to prove a short test's duration is
// measured, rather than read from a clock too coarse to see it (RAD Studio's 32-bit high_resolution_clock only
// advances about every 10 ms).
/////////////////////////////////////////////////////////////////////////////
class TFixture_ShortTest : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_BusyWaits();

public:
    static constexpr std::chrono::microseconds BusyWaitDuration{ 200 };

public:
    TFixture_ShortTest();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_ShortTest::TFixture_ShortTest()
    : inherited("Fixture_ShortTest")
{
    RegisterTest(&TFixture_ShortTest::Test_BusyWaits, "BusyWaits");
}
//---------------------------------------------------------------------------
void TFixture_ShortTest::Test_BusyWaits()
{
    // Busy-waits rather than sleeping, since a sleep can last a whole scheduler tick, which would hide the problem.
    std::chrono::steady_clock::time_point const start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < BusyWaitDuration)
    {
    }
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
// they reach the case-insensitive overloads. The StartsWith/EndsWith texts are chosen so that forwarding to the wrong
// overload (IC or not, start or end, Not or not) changes the outcome of at least one test. CheckIsEven() is a custom
// helper of the kind a test author might write, passing its caller's location through.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SourceLocations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void CheckIsEven(int value, std::source_location loc = std::source_location::current());

    void Test_AssertContainsIC_Passes();
    void Test_AssertContains_Fails();
    void Test_AssertEmpty_Fails();
    void Test_AssertEndsWithIC_Passes();
    void Test_AssertEndsWith_Fails();
    void Test_AssertEqualsIC_Passes();
    void Test_AssertEqualsMem_Fails();
    void Test_AssertEquals_CStrings_Passes();
    void Test_AssertEquals_Fails();
    void Test_AssertFalse_Fails();
    void Test_AssertGreaterThanOrEqual_Passes();
    void Test_AssertGreaterThan_Fails();
    void Test_AssertIsNotType_Fails();
    void Test_AssertIsType_Fails();
    void Test_AssertLessThanOrEqual_Passes();
    void Test_AssertLessThan_Fails();
    void Test_AssertMatches_Fails();
    void Test_AssertNear_Fails();
    void Test_AssertNoThrow_Fails();
    void Test_AssertNotContainsIC_Fails();
    void Test_AssertNotContains_Fails();
    void Test_AssertNotEmpty_Fails();
    void Test_AssertNotEndsWithIC_Fails();
    void Test_AssertNotEndsWith_DifferentCase_Passes();
    void Test_AssertNotEndsWith_Fails();
    void Test_AssertNotEqualsIC_Fails();
    void Test_AssertNotEqualsMem_Fails();
    void Test_AssertNotEquals_Fails();
    void Test_AssertNotMatches_Fails();
    void Test_AssertNotNear_Fails();
    void Test_AssertNotNull_Fails();
    void Test_AssertNotSame_Fails();
    void Test_AssertNotStartsWithIC_Fails();
    void Test_AssertNotStartsWith_DifferentCase_Passes();
    void Test_AssertNotStartsWith_Fails();
    void Test_AssertNull_Fails();
    void Test_AssertSame_Fails();
    void Test_AssertStartsWithIC_Passes();
    void Test_AssertStartsWith_Fails();
    void Test_AssertThrows_Fails();
    void Test_AssertTrue_Fails();
    void Test_CheckContainsIC_Passes();
    void Test_CheckContains_Fails();
    void Test_CheckContains_Wide_Passes();
    void Test_CheckEmpty_Fails();
    void Test_CheckEndsWithIC_Passes();
    void Test_CheckEndsWith_Fails();
    void Test_CheckEndsWith_Wide_Passes();
    void Test_CheckEqualsIC_Passes();
    void Test_CheckEqualsMem_Fails();
    void Test_CheckEquals_EnumClass_Fails();
    void Test_CheckEquals_Fails();
    void Test_CheckEquals_MixedIntegers_Passes();
    void Test_CheckFalse_Fails();
    void Test_CheckGreaterThanOrEqual_Passes();
    void Test_CheckGreaterThan_Fails();
    void Test_CheckIsNotType_Fails();
    void Test_CheckIsType_Fails();
    void Test_CheckLessThanOrEqual_Passes();
    void Test_CheckLessThan_Fails();
    void Test_CheckMatches_Fails();
    void Test_CheckMatches_Wide_Passes();
    void Test_CheckNear_Fails();
    void Test_CheckNear_Passes();
    void Test_CheckNoThrow_Fails();
    void Test_CheckNotContainsIC_Fails();
    void Test_CheckNotContains_Fails();
    void Test_CheckNotEmpty_Fails();
    void Test_CheckNotEndsWithIC_Fails();
    void Test_CheckNotEndsWith_DifferentCase_Passes();
    void Test_CheckNotEndsWith_Fails();
    void Test_CheckNotEqualsIC_Fails();
    void Test_CheckNotEqualsMem_Fails();
    void Test_CheckNotEquals_Fails();
    void Test_CheckNotMatches_Fails();
    void Test_CheckNotNear_Fails();
    void Test_CheckNotNull_Fails();
    void Test_CheckNotSame_Fails();
    void Test_CheckNotSame_UniquePtrs_Passes();
    void Test_CheckNotStartsWithIC_Fails();
    void Test_CheckNotStartsWith_DifferentCase_Passes();
    void Test_CheckNotStartsWith_Fails();
    void Test_CheckNull_Fails();
    void Test_CheckNull_UniquePtr_Passes();
    void Test_CheckSame_Fails();
    void Test_CheckStartsWithIC_Passes();
    void Test_CheckStartsWith_Fails();
    void Test_CheckStartsWith_Wide_Passes();
    void Test_CheckThrows_Fails();
    void Test_CheckThrows_MessageMatches_Passes();
    void Test_CheckTrue_Fails();
    void Test_CheckTrue_ThroughHelper_Fails();
    void Test_Fail_Fails();
    void Test_SetExceptionExpected_Bool_NoneThrown_Fails();
    void Test_SetExceptionExpected_Bool_Passes();
    void Test_SetExceptionExpected_TypeAndMessage_Passes();
    void Test_SetExceptionExpected_Type_NoneThrown_Fails();
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
    RegisterTest(&TFixture_SourceLocations::Test_AssertEmpty_Fails, "AssertEmpty_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEndsWithIC_Passes, "AssertEndsWithIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEndsWith_Fails, "AssertEndsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEqualsIC_Passes, "AssertEqualsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEqualsMem_Fails, "AssertEqualsMem_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEquals_CStrings_Passes, "AssertEquals_CStrings_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertEquals_Fails, "AssertEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertFalse_Fails, "AssertFalse_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertGreaterThanOrEqual_Passes, "AssertGreaterThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertGreaterThan_Fails, "AssertGreaterThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertIsNotType_Fails, "AssertIsNotType_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertIsType_Fails, "AssertIsType_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertLessThanOrEqual_Passes, "AssertLessThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertLessThan_Fails, "AssertLessThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertMatches_Fails, "AssertMatches_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNear_Fails, "AssertNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNoThrow_Fails, "AssertNoThrow_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotContainsIC_Fails, "AssertNotContainsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotContains_Fails, "AssertNotContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEmpty_Fails, "AssertNotEmpty_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEndsWithIC_Fails, "AssertNotEndsWithIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEndsWith_DifferentCase_Passes,
        "AssertNotEndsWith_DifferentCase_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEndsWith_Fails, "AssertNotEndsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEqualsIC_Fails, "AssertNotEqualsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEqualsMem_Fails, "AssertNotEqualsMem_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotEquals_Fails, "AssertNotEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotMatches_Fails, "AssertNotMatches_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotNear_Fails, "AssertNotNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotNull_Fails, "AssertNotNull_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotSame_Fails, "AssertNotSame_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotStartsWithIC_Fails, "AssertNotStartsWithIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotStartsWith_DifferentCase_Passes,
        "AssertNotStartsWith_DifferentCase_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNotStartsWith_Fails, "AssertNotStartsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertNull_Fails, "AssertNull_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertSame_Fails, "AssertSame_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertStartsWithIC_Passes, "AssertStartsWithIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_AssertStartsWith_Fails, "AssertStartsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertThrows_Fails, "AssertThrows_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_AssertTrue_Fails, "AssertTrue_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContainsIC_Passes, "CheckContainsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContains_Fails, "CheckContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckContains_Wide_Passes, "CheckContains_Wide_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEmpty_Fails, "CheckEmpty_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEndsWithIC_Passes, "CheckEndsWithIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEndsWith_Fails, "CheckEndsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEndsWith_Wide_Passes, "CheckEndsWith_Wide_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEqualsIC_Passes, "CheckEqualsIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEqualsMem_Fails, "CheckEqualsMem_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEquals_EnumClass_Fails, "CheckEquals_EnumClass_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEquals_Fails, "CheckEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckEquals_MixedIntegers_Passes, "CheckEquals_MixedIntegers_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckFalse_Fails, "CheckFalse_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckGreaterThanOrEqual_Passes, "CheckGreaterThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckGreaterThan_Fails, "CheckGreaterThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckIsNotType_Fails, "CheckIsNotType_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckIsType_Fails, "CheckIsType_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckLessThanOrEqual_Passes, "CheckLessThanOrEqual_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckLessThan_Fails, "CheckLessThan_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckMatches_Fails, "CheckMatches_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckMatches_Wide_Passes, "CheckMatches_Wide_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNear_Fails, "CheckNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNear_Passes, "CheckNear_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNoThrow_Fails, "CheckNoThrow_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotContainsIC_Fails, "CheckNotContainsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotContains_Fails, "CheckNotContains_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEmpty_Fails, "CheckNotEmpty_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEndsWithIC_Fails, "CheckNotEndsWithIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEndsWith_DifferentCase_Passes,
        "CheckNotEndsWith_DifferentCase_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEndsWith_Fails, "CheckNotEndsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEqualsIC_Fails, "CheckNotEqualsIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEqualsMem_Fails, "CheckNotEqualsMem_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotEquals_Fails, "CheckNotEquals_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotMatches_Fails, "CheckNotMatches_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotNear_Fails, "CheckNotNear_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotNull_Fails, "CheckNotNull_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotSame_Fails, "CheckNotSame_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotSame_UniquePtrs_Passes, "CheckNotSame_UniquePtrs_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotStartsWithIC_Fails, "CheckNotStartsWithIC_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotStartsWith_DifferentCase_Passes,
        "CheckNotStartsWith_DifferentCase_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNotStartsWith_Fails, "CheckNotStartsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNull_Fails, "CheckNull_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckNull_UniquePtr_Passes, "CheckNull_UniquePtr_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckSame_Fails, "CheckSame_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckStartsWithIC_Passes, "CheckStartsWithIC_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckStartsWith_Fails, "CheckStartsWith_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckStartsWith_Wide_Passes, "CheckStartsWith_Wide_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckThrows_Fails, "CheckThrows_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckThrows_MessageMatches_Passes,
        "CheckThrows_MessageMatches_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_CheckTrue_Fails, "CheckTrue_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_CheckTrue_ThroughHelper_Fails, "CheckTrue_ThroughHelper_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_Fail_Fails, "Fail_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Bool_NoneThrown_Fails,
        "SetExceptionExpected_Bool_NoneThrown_Fails");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Bool_Passes,
        "SetExceptionExpected_Bool_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_TypeAndMessage_Passes,
        "SetExceptionExpected_TypeAndMessage_Passes");
    RegisterTest(&TFixture_SourceLocations::Test_SetExceptionExpected_Type_NoneThrown_Fails,
        "SetExceptionExpected_Type_NoneThrown_Fails");
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
void TFixture_SourceLocations::Test_AssertEmpty_Fails()
{
    ExpectedLines["AssertEmpty_Fails"] = __LINE__ + 1;
    AssertEmpty(std::string("abc"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEndsWithIC_Passes()
{
    AssertEndsWithIC(std::string("abc"), "BC", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEndsWith_Fails()
{
    ExpectedLines["AssertEndsWith_Fails"] = __LINE__ + 1;
    AssertEndsWith(std::string("BC-bc"), "BC", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEqualsIC_Passes()
{
    AssertEqualsIC(std::string("abc"), std::string("ABC"), "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertEqualsMem_Fails()
{
    int const expected = 1;
    int const actual = 2;
    ExpectedLines["AssertEqualsMem_Fails"] = __LINE__ + 1;
    AssertEqualsMem(&expected, &actual, sizeof(expected), "deliberate failure");
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
void TFixture_SourceLocations::Test_AssertIsNotType_Fails()
{
    std::out_of_range const error("index");
    std::exception const& ex = error;
    ExpectedLines["AssertIsNotType_Fails"] = __LINE__ + 1;
    AssertIsNotType<std::logic_error>(ex, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertIsType_Fails()
{
    std::runtime_error const error("boom");
    std::exception const& ex = error;
    ExpectedLines["AssertIsType_Fails"] = __LINE__ + 1;
    AssertIsType<std::logic_error>(ex, "deliberate failure");
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
void TFixture_SourceLocations::Test_AssertMatches_Fails()
{
    ExpectedLines["AssertMatches_Fails"] = __LINE__ + 1;
    AssertMatches(std::string("abc"), std::string(R"(\d+)"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNear_Fails()
{
    ExpectedLines["AssertNear_Fails"] = __LINE__ + 1;
    AssertNear(1.0, 2.0, 0.5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNoThrow_Fails()
{
    ExpectedLines["AssertNoThrow_Fails"] = __LINE__ + 1;
    AssertNoThrow([] {
            throw std::runtime_error("boom");
        }, "deliberate failure");
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
void TFixture_SourceLocations::Test_AssertNotEmpty_Fails()
{
    ExpectedLines["AssertNotEmpty_Fails"] = __LINE__ + 1;
    AssertNotEmpty(std::vector<int>(), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEndsWithIC_Fails()
{
    ExpectedLines["AssertNotEndsWithIC_Fails"] = __LINE__ + 1;
    AssertNotEndsWithIC(std::string("abc"), "BC", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEndsWith_DifferentCase_Passes()
{
    AssertNotEndsWith(std::string("abc"), "BC", "matches only ignoring case");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEndsWith_Fails()
{
    ExpectedLines["AssertNotEndsWith_Fails"] = __LINE__ + 1;
    AssertNotEndsWith(std::string("BC-bc"), "bc", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEqualsIC_Fails()
{
    ExpectedLines["AssertNotEqualsIC_Fails"] = __LINE__ + 1;
    AssertNotEqualsIC(std::string("abc"), std::string("ABC"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEqualsMem_Fails()
{
    int const expected = 1;
    int const actual = 1;
    ExpectedLines["AssertNotEqualsMem_Fails"] = __LINE__ + 1;
    AssertNotEqualsMem(&expected, &actual, sizeof(expected), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotEquals_Fails()
{
    ExpectedLines["AssertNotEquals_Fails"] = __LINE__ + 1;
    AssertNotEquals(std::string("abc"), std::string("abc"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotMatches_Fails()
{
    ExpectedLines["AssertNotMatches_Fails"] = __LINE__ + 1;
    AssertNotMatches(std::string("123"), std::string(R"(\d+)"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotNear_Fails()
{
    ExpectedLines["AssertNotNear_Fails"] = __LINE__ + 1;
    AssertNotNear(1.0f, 1.25f, 0.5f, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotNull_Fails()
{
    ExpectedLines["AssertNotNull_Fails"] = __LINE__ + 1;
    AssertNotNull(nullptr, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotSame_Fails()
{
    int value = 0;
    ExpectedLines["AssertNotSame_Fails"] = __LINE__ + 1;
    AssertNotSame(&value, &value, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotStartsWithIC_Fails()
{
    ExpectedLines["AssertNotStartsWithIC_Fails"] = __LINE__ + 1;
    AssertNotStartsWithIC(std::string("abc"), "AB", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotStartsWith_DifferentCase_Passes()
{
    AssertNotStartsWith(std::string("abc"), "AB", "matches only ignoring case");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNotStartsWith_Fails()
{
    ExpectedLines["AssertNotStartsWith_Fails"] = __LINE__ + 1;
    AssertNotStartsWith(std::string("ab-AB"), "ab", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertNull_Fails()
{
    int value = 0;
    ExpectedLines["AssertNull_Fails"] = __LINE__ + 1;
    AssertNull(&value, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertSame_Fails()
{
    int first = 0;
    int second = 0;
    ExpectedLines["AssertSame_Fails"] = __LINE__ + 1;
    AssertSame(&first, &second, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertStartsWithIC_Passes()
{
    AssertStartsWithIC(std::string("abc"), "AB", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertStartsWith_Fails()
{
    ExpectedLines["AssertStartsWith_Fails"] = __LINE__ + 1;
    AssertStartsWith(std::string("ab-AB"), "AB", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_AssertThrows_Fails()
{
    ExpectedLines["AssertThrows_Fails"] = __LINE__ + 1;
    AssertThrows<std::runtime_error>([] {
        }, "deliberate failure");
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
void TFixture_SourceLocations::Test_CheckEmpty_Fails()
{
    ExpectedLines["CheckEmpty_Fails"] = __LINE__ + 1;
    CheckEmpty(std::vector<int>{ 1 }, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEndsWithIC_Passes()
{
    CheckEndsWithIC(std::string("abc"), "BC", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEndsWith_Fails()
{
    ExpectedLines["CheckEndsWith_Fails"] = __LINE__ + 1;
    CheckEndsWith(std::string("BC-bc"), "BC", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEndsWith_Wide_Passes()
{
    CheckEndsWith(std::wstring(L"abc"), L"bc", "wide suffix present");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEqualsIC_Passes()
{
    CheckEqualsIC(std::string("abc"), std::string("ABC"), "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEqualsMem_Fails()
{
    int const expected = 1;
    int const actual = 2;
    ExpectedLines["CheckEqualsMem_Fails"] = __LINE__ + 1;
    CheckEqualsMem(&expected, &actual, sizeof(expected), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckEquals_EnumClass_Fails()
{
    enum class TState
    {
        Off,
        On
    };

    ExpectedLines["CheckEquals_EnumClass_Fails"] = __LINE__ + 1;
    CheckEquals(TState::Off, TState::On, "deliberate failure");
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
void TFixture_SourceLocations::Test_CheckIsNotType_Fails()
{
    std::out_of_range const error("index");
    std::exception const* const ex = &error;
    ExpectedLines["CheckIsNotType_Fails"] = __LINE__ + 1;
    CheckIsNotType<std::logic_error>(ex, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckIsType_Fails()
{
    std::runtime_error const error("boom");
    std::exception const* const ex = &error;
    ExpectedLines["CheckIsType_Fails"] = __LINE__ + 1;
    CheckIsType<std::logic_error>(ex, "deliberate failure");
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
void TFixture_SourceLocations::Test_CheckMatches_Fails()
{
    ExpectedLines["CheckMatches_Fails"] = __LINE__ + 1;
    CheckMatches(std::string("abc"), std::string(R"(\d+)"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckMatches_Wide_Passes()
{
    CheckMatches(std::wstring(L"abc"), std::wstring(L"[a-c]+"), "wide text and pattern");
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
void TFixture_SourceLocations::Test_CheckNoThrow_Fails()
{
    ExpectedLines["CheckNoThrow_Fails"] = __LINE__ + 1;
    CheckNoThrow([] {
            throw std::runtime_error("boom");
        }, "deliberate failure");
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
void TFixture_SourceLocations::Test_CheckNotEmpty_Fails()
{
    ExpectedLines["CheckNotEmpty_Fails"] = __LINE__ + 1;
    CheckNotEmpty(std::string(), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEndsWithIC_Fails()
{
    ExpectedLines["CheckNotEndsWithIC_Fails"] = __LINE__ + 1;
    CheckNotEndsWithIC(std::string("abc"), "BC", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEndsWith_DifferentCase_Passes()
{
    CheckNotEndsWith(std::string("abc"), "BC", "matches only ignoring case");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEndsWith_Fails()
{
    ExpectedLines["CheckNotEndsWith_Fails"] = __LINE__ + 1;
    CheckNotEndsWith(std::string("BC-bc"), "bc", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEqualsIC_Fails()
{
    ExpectedLines["CheckNotEqualsIC_Fails"] = __LINE__ + 1;
    CheckNotEqualsIC(std::string("abc"), std::string("ABC"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEqualsMem_Fails()
{
    int const expected = 1;
    int const actual = 1;
    ExpectedLines["CheckNotEqualsMem_Fails"] = __LINE__ + 1;
    CheckNotEqualsMem(&expected, &actual, sizeof(expected), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotEquals_Fails()
{
    ExpectedLines["CheckNotEquals_Fails"] = __LINE__ + 1;
    CheckNotEquals(5, 5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotMatches_Fails()
{
    ExpectedLines["CheckNotMatches_Fails"] = __LINE__ + 1;
    CheckNotMatches(std::string("123"), std::string(R"(\d+)"), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotNear_Fails()
{
    ExpectedLines["CheckNotNear_Fails"] = __LINE__ + 1;
    CheckNotNear(1.0, 1.25, 0.5, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotNull_Fails()
{
    ExpectedLines["CheckNotNull_Fails"] = __LINE__ + 1;
    CheckNotNull(std::shared_ptr<int>(), "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotSame_Fails()
{
    int value = 0;
    ExpectedLines["CheckNotSame_Fails"] = __LINE__ + 1;
    CheckNotSame(&value, &value, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotSame_UniquePtrs_Passes()
{
    CheckNotSame(std::make_unique<int>(0), std::make_unique<int>(0),
        "move-only rvalues forward to the method/line form");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotStartsWithIC_Fails()
{
    ExpectedLines["CheckNotStartsWithIC_Fails"] = __LINE__ + 1;
    CheckNotStartsWithIC(std::string("abc"), "AB", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotStartsWith_DifferentCase_Passes()
{
    CheckNotStartsWith(std::string("abc"), "AB", "matches only ignoring case");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNotStartsWith_Fails()
{
    ExpectedLines["CheckNotStartsWith_Fails"] = __LINE__ + 1;
    CheckNotStartsWith(std::string("ab-AB"), "ab", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNull_Fails()
{
    int value = 0;
    ExpectedLines["CheckNull_Fails"] = __LINE__ + 1;
    CheckNull(&value, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckNull_UniquePtr_Passes()
{
    CheckNull(std::unique_ptr<int>(), "a move-only rvalue forwards to the method/line overload");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckSame_Fails()
{
    int first = 0;
    int second = 0;
    ExpectedLines["CheckSame_Fails"] = __LINE__ + 1;
    CheckSame(&first, &second, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckStartsWithIC_Passes()
{
    CheckStartsWithIC(std::string("abc"), "AB", "case differs");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckStartsWith_Fails()
{
    ExpectedLines["CheckStartsWith_Fails"] = __LINE__ + 1;
    CheckStartsWith(std::string("ab-AB"), "AB", "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckStartsWith_Wide_Passes()
{
    CheckStartsWith(std::wstring(L"abc"), L"ab", "wide prefix present");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckThrows_Fails()
{
    ExpectedLines["CheckThrows_Fails"] = __LINE__ + 1;
    CheckThrows<std::out_of_range>([] {
            throw std::runtime_error("boom");
        }, "deliberate failure");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_CheckThrows_MessageMatches_Passes()
{
    // The optional expected message comes after 'msg', as in the method/line form.
    CheckThrows<std::runtime_error>([] {
            throw std::runtime_error("disk full");
        }, "message checked", "full");
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
void TFixture_SourceLocations::Test_Fail_Fails()
{
    ExpectedLines["Fail_Fails"] = __LINE__ + 1;
    Fail("deliberate failure");
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
void TFixture_SourceLocations::Test_SetExceptionExpected_TypeAndMessage_Passes()
{
    SetExceptionExpected<std::invalid_argument>("matching type and message", "boom");
    throw std::invalid_argument("boom");
}
//---------------------------------------------------------------------------
void TFixture_SourceLocations::Test_SetExceptionExpected_Type_NoneThrown_Fails()
{
    ExpectedLines["SetExceptionExpected_Type_NoneThrown_Fails"] = __LINE__ + 1;
    SetExceptionExpected<std::invalid_argument>("deliberately nothing thrown");
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
// TFixture_StartsWithComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertStartsWith()/CheckStartsWith()/
// AssertNotStartsWith()/CheckNotStartsWith(), narrow and wide. "PrefixOnlyAtEnd" and "PrefixOnlyInMiddle" catch a
// check that matches anywhere but the start, and "PrefixLongerThanText" one that doesn't check the length first.
// "Wide_NonASCII" checks that a wide failure shows its text as UTF-8. Test names self-document expected outcome via
// NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_StartsWithComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNotStartsWith_Absent_Passes();
    void Test_AssertNotStartsWith_Present_Fails();
    void Test_AssertNotStartsWith_Wide_Absent_Passes();
    void Test_AssertNotStartsWith_Wide_Present_Fails();
    void Test_AssertStartsWith_Absent_Fails();
    void Test_AssertStartsWith_Present_Passes();
    void Test_AssertStartsWith_Wide_Absent_Fails();
    void Test_AssertStartsWith_Wide_Present_Passes();
    void Test_CheckNotStartsWith_Absent_Passes();
    void Test_CheckNotStartsWith_BothEmpty_Fails();
    void Test_CheckNotStartsWith_EmptyPrefix_Fails();
    void Test_CheckNotStartsWith_PrefixLongerThanText_Passes();
    void Test_CheckNotStartsWith_PrefixOnlyAtEnd_Passes();
    void Test_CheckNotStartsWith_PrefixOnlyInMiddle_Passes();
    void Test_CheckNotStartsWith_Present_Fails();
    void Test_CheckNotStartsWith_Wide_Absent_Passes();
    void Test_CheckNotStartsWith_Wide_Present_Fails();
    void Test_CheckStartsWith_Absent_Fails();
    void Test_CheckStartsWith_BothEmpty_Passes();
    void Test_CheckStartsWith_DifferentCase_Fails();
    void Test_CheckStartsWith_EmptyPrefix_Passes();
    void Test_CheckStartsWith_EmptyText_Fails();
    void Test_CheckStartsWith_PrefixLongerThanText_Fails();
    void Test_CheckStartsWith_PrefixOnlyAtEnd_Fails();
    void Test_CheckStartsWith_PrefixOnlyInMiddle_Fails();
    void Test_CheckStartsWith_Present_Passes();
    void Test_CheckStartsWith_Wide_Absent_Fails();
    void Test_CheckStartsWith_Wide_NonASCII_Fails();
    void Test_CheckStartsWith_Wide_Present_Passes();

public:
    TFixture_StartsWithComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_StartsWithComparisons::TFixture_StartsWithComparisons()
    : inherited("Fixture_StartsWithComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Absent_Passes,
        "AssertNotStartsWith_Absent_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Present_Fails,
        "AssertNotStartsWith_Present_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Wide_Absent_Passes,
        "AssertNotStartsWith_Wide_Absent_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Wide_Present_Fails,
        "AssertNotStartsWith_Wide_Present_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertStartsWith_Absent_Fails, "AssertStartsWith_Absent_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertStartsWith_Present_Passes,
        "AssertStartsWith_Present_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertStartsWith_Wide_Absent_Fails,
        "AssertStartsWith_Wide_Absent_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_AssertStartsWith_Wide_Present_Passes,
        "AssertStartsWith_Wide_Present_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Absent_Passes,
        "CheckNotStartsWith_Absent_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_BothEmpty_Fails,
        "CheckNotStartsWith_BothEmpty_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_EmptyPrefix_Fails,
        "CheckNotStartsWith_EmptyPrefix_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixLongerThanText_Passes,
        "CheckNotStartsWith_PrefixLongerThanText_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixOnlyAtEnd_Passes,
        "CheckNotStartsWith_PrefixOnlyAtEnd_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixOnlyInMiddle_Passes,
        "CheckNotStartsWith_PrefixOnlyInMiddle_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Present_Fails,
        "CheckNotStartsWith_Present_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Wide_Absent_Passes,
        "CheckNotStartsWith_Wide_Absent_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Wide_Present_Fails,
        "CheckNotStartsWith_Wide_Present_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_Absent_Fails, "CheckStartsWith_Absent_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_BothEmpty_Passes,
        "CheckStartsWith_BothEmpty_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_DifferentCase_Fails,
        "CheckStartsWith_DifferentCase_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_EmptyPrefix_Passes,
        "CheckStartsWith_EmptyPrefix_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_EmptyText_Fails,
        "CheckStartsWith_EmptyText_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixLongerThanText_Fails,
        "CheckStartsWith_PrefixLongerThanText_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixOnlyAtEnd_Fails,
        "CheckStartsWith_PrefixOnlyAtEnd_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixOnlyInMiddle_Fails,
        "CheckStartsWith_PrefixOnlyInMiddle_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_Present_Passes,
        "CheckStartsWith_Present_Passes");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_Absent_Fails,
        "CheckStartsWith_Wide_Absent_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_NonASCII_Fails,
        "CheckStartsWith_Wide_NonASCII_Fails");
    RegisterTest(&TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_Present_Passes,
        "CheckStartsWith_Wide_Present_Passes");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Absent_Passes()
{
    AssertNotStartsWith(std::string("hello world"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Present_Fails()
{
    AssertNotStartsWith(std::string("hello world"), "hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Wide_Absent_Passes()
{
    AssertNotStartsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertNotStartsWith_Wide_Present_Fails()
{
    AssertNotStartsWith(std::wstring(L"hello world"), L"hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertStartsWith_Absent_Fails()
{
    AssertStartsWith(std::string("hello world"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertStartsWith_Present_Passes()
{
    AssertStartsWith(std::string("hello world"), "hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertStartsWith_Wide_Absent_Fails()
{
    AssertStartsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_AssertStartsWith_Wide_Present_Passes()
{
    AssertStartsWith(std::wstring(L"hello world"), L"hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Absent_Passes()
{
    CheckNotStartsWith(std::string("hello world"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_BothEmpty_Fails()
{
    CheckNotStartsWith(std::string(), "", __func__, __LINE__, "even an empty string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_EmptyPrefix_Fails()
{
    CheckNotStartsWith(std::string("hello world"), "", __func__, __LINE__, "every string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixLongerThanText_Passes()
{
    CheckNotStartsWith(std::string("hello"), "hello world", __func__, __LINE__, "the text has the start only");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixOnlyAtEnd_Passes()
{
    CheckNotStartsWith(std::string("hello world"), "world", __func__, __LINE__, "the prefix is at the end");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_PrefixOnlyInMiddle_Passes()
{
    CheckNotStartsWith(std::string("hello world"), "lo wo", __func__, __LINE__, "the prefix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Present_Fails()
{
    CheckNotStartsWith(std::string("hello world"), "hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Wide_Absent_Passes()
{
    CheckNotStartsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckNotStartsWith_Wide_Present_Fails()
{
    CheckNotStartsWith(std::wstring(L"hello world"), L"hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_Absent_Fails()
{
    CheckStartsWith(std::string("hello world"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_BothEmpty_Passes()
{
    CheckStartsWith(std::string(), "", __func__, __LINE__, "even an empty string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_DifferentCase_Fails()
{
    CheckStartsWith(std::string("hello world"), "Hello", __func__, __LINE__, "the comparison is case-sensitive");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_EmptyPrefix_Passes()
{
    CheckStartsWith(std::string("hello world"), "", __func__, __LINE__, "every string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_EmptyText_Fails()
{
    CheckStartsWith(std::string(), "h", __func__, __LINE__, "an empty string starts with no letter");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixLongerThanText_Fails()
{
    CheckStartsWith(std::string("hello"), "hello world", __func__, __LINE__, "the text has the start only");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixOnlyAtEnd_Fails()
{
    CheckStartsWith(std::string("hello world"), "world", __func__, __LINE__, "the prefix is at the end");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_PrefixOnlyInMiddle_Fails()
{
    CheckStartsWith(std::string("hello world"), "lo wo", __func__, __LINE__, "the prefix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_Present_Passes()
{
    CheckStartsWith(std::string("hello world"), "hello world", __func__, __LINE__, "the whole text is a prefix");
    CheckStartsWith(std::string("hello world"), "hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_Absent_Fails()
{
    CheckStartsWith(std::wstring(L"hello world"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_NonASCII_Fails()
{
    // "cafe" with an e-acute, a space, and U+1F600 (a surrogate pair where wchar_t is 16 bits), then u-umlaut.
    CheckStartsWith(std::wstring(L"caf" L"\x00E9" L" \U0001F600"), L"\x00FC", __func__, __LINE__, "not present");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithComparisons::Test_CheckStartsWith_Wide_Present_Passes()
{
    CheckStartsWith(std::wstring(L"hello world"), L"hello", __func__, __LINE__, "prefix present");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_StartsWithICComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertStartsWithIC()/CheckStartsWithIC()/
// AssertNotStartsWithIC()/CheckNotStartsWithIC(), narrow and wide. Only the ASCII letters A-Z are case-folded, as in
// TFixture_ContainsICComparisons above. Test names self-document expected outcome via NameEndsWith(), same as
// TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_StartsWithICComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNotStartsWithIC_Absent_Passes();
    void Test_AssertNotStartsWithIC_DifferentCase_Fails();
    void Test_AssertNotStartsWithIC_Wide_Absent_Passes();
    void Test_AssertNotStartsWithIC_Wide_DifferentCase_Fails();
    void Test_AssertStartsWithIC_Absent_Fails();
    void Test_AssertStartsWithIC_DifferentCase_Passes();
    void Test_AssertStartsWithIC_Wide_Absent_Fails();
    void Test_AssertStartsWithIC_Wide_DifferentCase_Passes();
    void Test_CheckNotStartsWithIC_Absent_Passes();
    void Test_CheckNotStartsWithIC_DifferentCase_Fails();
    void Test_CheckNotStartsWithIC_EmptyPrefix_Fails();
    void Test_CheckNotStartsWithIC_PrefixLongerThanText_Passes();
    void Test_CheckNotStartsWithIC_PrefixOnlyAtEnd_Passes();
    void Test_CheckNotStartsWithIC_Wide_Absent_Passes();
    void Test_CheckNotStartsWithIC_Wide_DifferentCase_Fails();
    void Test_CheckStartsWithIC_Absent_Fails();
    void Test_CheckStartsWithIC_BothEmpty_Passes();
    void Test_CheckStartsWithIC_DifferentCase_Passes();
    void Test_CheckStartsWithIC_EmptyPrefix_Passes();
    void Test_CheckStartsWithIC_EmptyText_Fails();
    void Test_CheckStartsWithIC_NonASCII_Fails();
    void Test_CheckStartsWithIC_NonLetters_Fails();
    void Test_CheckStartsWithIC_PrefixLongerThanText_Fails();
    void Test_CheckStartsWithIC_PrefixOnlyAtEnd_Fails();
    void Test_CheckStartsWithIC_PrefixOnlyInMiddle_Fails();
    void Test_CheckStartsWithIC_Wide_Absent_Fails();
    void Test_CheckStartsWithIC_Wide_DifferentCase_Passes();
    void Test_CheckStartsWithIC_Wide_NonASCII_Fails();

public:
    TFixture_StartsWithICComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_StartsWithICComparisons::TFixture_StartsWithICComparisons()
    : inherited("Fixture_StartsWithICComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Absent_Passes,
        "AssertNotStartsWithIC_Absent_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_DifferentCase_Fails,
        "AssertNotStartsWithIC_DifferentCase_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Wide_Absent_Passes,
        "AssertNotStartsWithIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Wide_DifferentCase_Fails,
        "AssertNotStartsWithIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Absent_Fails,
        "AssertStartsWithIC_Absent_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_DifferentCase_Passes,
        "AssertStartsWithIC_DifferentCase_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Wide_Absent_Fails,
        "AssertStartsWithIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Wide_DifferentCase_Passes,
        "AssertStartsWithIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Absent_Passes,
        "CheckNotStartsWithIC_Absent_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_DifferentCase_Fails,
        "CheckNotStartsWithIC_DifferentCase_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_EmptyPrefix_Fails,
        "CheckNotStartsWithIC_EmptyPrefix_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_PrefixLongerThanText_Passes,
        "CheckNotStartsWithIC_PrefixLongerThanText_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_PrefixOnlyAtEnd_Passes,
        "CheckNotStartsWithIC_PrefixOnlyAtEnd_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Wide_Absent_Passes,
        "CheckNotStartsWithIC_Wide_Absent_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Wide_DifferentCase_Fails,
        "CheckNotStartsWithIC_Wide_DifferentCase_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Absent_Fails,
        "CheckStartsWithIC_Absent_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_BothEmpty_Passes,
        "CheckStartsWithIC_BothEmpty_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_DifferentCase_Passes,
        "CheckStartsWithIC_DifferentCase_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_EmptyPrefix_Passes,
        "CheckStartsWithIC_EmptyPrefix_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_EmptyText_Fails,
        "CheckStartsWithIC_EmptyText_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_NonASCII_Fails,
        "CheckStartsWithIC_NonASCII_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_NonLetters_Fails,
        "CheckStartsWithIC_NonLetters_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixLongerThanText_Fails,
        "CheckStartsWithIC_PrefixLongerThanText_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixOnlyAtEnd_Fails,
        "CheckStartsWithIC_PrefixOnlyAtEnd_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixOnlyInMiddle_Fails,
        "CheckStartsWithIC_PrefixOnlyInMiddle_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_Absent_Fails,
        "CheckStartsWithIC_Wide_Absent_Fails");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_DifferentCase_Passes,
        "CheckStartsWithIC_Wide_DifferentCase_Passes");
    RegisterTest(&TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_NonASCII_Fails,
        "CheckStartsWithIC_Wide_NonASCII_Fails");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Absent_Passes()
{
    AssertNotStartsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_DifferentCase_Fails()
{
    AssertNotStartsWithIC(std::string("Hello World"), "HELLO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Wide_Absent_Passes()
{
    AssertNotStartsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertNotStartsWithIC_Wide_DifferentCase_Fails()
{
    AssertNotStartsWithIC(std::wstring(L"Hello World"), L"HELLO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Absent_Fails()
{
    AssertStartsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_DifferentCase_Passes()
{
    AssertStartsWithIC(std::string("Hello World"), "hELLO w", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Wide_Absent_Fails()
{
    AssertStartsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_AssertStartsWithIC_Wide_DifferentCase_Passes()
{
    AssertStartsWithIC(std::wstring(L"Hello World"), L"hELLO w", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Absent_Passes()
{
    CheckNotStartsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_DifferentCase_Fails()
{
    CheckNotStartsWithIC(std::string("Hello World"), "HELLO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_EmptyPrefix_Fails()
{
    CheckNotStartsWithIC(std::string("Hello World"), "", __func__, __LINE__,
        "every string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_PrefixLongerThanText_Passes()
{
    CheckNotStartsWithIC(std::string("Hello"), "HELLO WORLD", __func__, __LINE__, "the text has the start only");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_PrefixOnlyAtEnd_Passes()
{
    CheckNotStartsWithIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "the prefix is at the end");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Wide_Absent_Passes()
{
    CheckNotStartsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckNotStartsWithIC_Wide_DifferentCase_Fails()
{
    CheckNotStartsWithIC(std::wstring(L"Hello World"), L"HELLO", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Absent_Fails()
{
    CheckStartsWithIC(std::string("Hello World"), "xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_BothEmpty_Passes()
{
    CheckStartsWithIC(std::string(), "", __func__, __LINE__, "even an empty string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_DifferentCase_Passes()
{
    CheckStartsWithIC(std::string("Hello World"), "hELLO w", __func__, __LINE__, "case differs");
    CheckStartsWithIC(std::string("Hello World"), "hello world", __func__, __LINE__, "the whole text, case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_EmptyPrefix_Passes()
{
    CheckStartsWithIC(std::string("Hello World"), "", __func__, __LINE__, "every string starts with the empty string");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_EmptyText_Fails()
{
    CheckStartsWithIC(std::string(), "h", __func__, __LINE__, "an empty string starts with no letter");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_NonASCII_Fails()
{
    // UTF-8 E-acute and e-acute.
    CheckStartsWithIC(std::string("caf\xC3\x89s"), "CAF\xC3\xA9", __func__, __LINE__, "only A-Z are case-folded");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_NonLetters_Fails()
{
    CheckStartsWithIC(std::string("a@b[c"), "a`b{", __func__, __LINE__, "@ and `, and [ and {, are not letters");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixLongerThanText_Fails()
{
    CheckStartsWithIC(std::string("Hello"), "HELLO WORLD", __func__, __LINE__, "the text has the start only");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixOnlyAtEnd_Fails()
{
    CheckStartsWithIC(std::string("Hello World"), "WORLD", __func__, __LINE__, "the prefix is at the end");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_PrefixOnlyInMiddle_Fails()
{
    CheckStartsWithIC(std::string("Hello World"), "LO WO", __func__, __LINE__, "the prefix is in the middle");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_Absent_Fails()
{
    CheckStartsWithIC(std::wstring(L"Hello World"), L"xyz", __func__, __LINE__, "prefix absent");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_DifferentCase_Passes()
{
    CheckStartsWithIC(std::wstring(L"Hello World"), L"hELLO w", __func__, __LINE__, "case differs");
}
//---------------------------------------------------------------------------
void TFixture_StartsWithICComparisons::Test_CheckStartsWithIC_Wide_NonASCII_Fails()
{
    // E-acute and e-acute.
    CheckStartsWithIC(std::wstring(L"caf" L"\x00C9" L"s"), L"CAF" L"\x00E9", __func__, __LINE__,
        "only A-Z are case-folded");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_StringComparisons
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one deliberately failing test per string
// overload of AssertEquals()/CheckEquals()/AssertNotEquals()/CheckNotEquals() whose failure message didn't use to
// show the values: wide text (shown as UTF-8), the NotEquals overloads (which show the shared value), and null wide
// C strings (shown as "(null)", like narrow ones). Used by Test_Equals_ShowsStringValues below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_StringComparisons : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertEquals_WideCStringNull_Fails();
    void Test_AssertEquals_Wide_Fails();
    void Test_AssertNotEquals_CStringBothNull_Fails();
    void Test_AssertNotEquals_String_Fails();
    void Test_AssertNotEquals_WideCStringBothNull_Fails();
    void Test_AssertNotEquals_Wide_Fails();
    void Test_CheckEquals_WideCStringNull_Fails();
    void Test_CheckEquals_WideCString_Fails();
    void Test_CheckEquals_Wide_Fails();
    void Test_CheckNotEquals_CStringBothNull_Fails();
    void Test_CheckNotEquals_String_Fails();
    void Test_CheckNotEquals_WideCStringBothNull_Fails();
    void Test_CheckNotEquals_Wide_Fails();

public:
    TFixture_StringComparisons();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_StringComparisons::TFixture_StringComparisons()
    : inherited("Fixture_StringComparisons")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_StringComparisons::Test_AssertEquals_WideCStringNull_Fails,
        "AssertEquals_WideCStringNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_AssertEquals_Wide_Fails, "AssertEquals_Wide_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_AssertNotEquals_CStringBothNull_Fails,
        "AssertNotEquals_CStringBothNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_AssertNotEquals_String_Fails, "AssertNotEquals_String_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_AssertNotEquals_WideCStringBothNull_Fails,
        "AssertNotEquals_WideCStringBothNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_AssertNotEquals_Wide_Fails, "AssertNotEquals_Wide_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckEquals_WideCStringNull_Fails,
        "CheckEquals_WideCStringNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckEquals_WideCString_Fails, "CheckEquals_WideCString_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckEquals_Wide_Fails, "CheckEquals_Wide_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckNotEquals_CStringBothNull_Fails,
        "CheckNotEquals_CStringBothNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckNotEquals_String_Fails, "CheckNotEquals_String_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckNotEquals_WideCStringBothNull_Fails,
        "CheckNotEquals_WideCStringBothNull_Fails");
    RegisterTest(&TFixture_StringComparisons::Test_CheckNotEquals_Wide_Fails, "CheckNotEquals_Wide_Fails");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertEquals_WideCStringNull_Fails()
{
    wchar_t const* const nullStr = nullptr;
    AssertEquals(L"abc", nullStr, __func__, __LINE__, "null never matches");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertEquals_Wide_Fails()
{
    // "cafe" with an e-acute.
    AssertEquals(std::wstring(L"caf" L"\x00E9"), std::wstring(L"cafe"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertNotEquals_CStringBothNull_Fails()
{
    char const* const nullStr = nullptr;
    AssertNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertNotEquals_String_Fails()
{
    AssertNotEquals(std::string("abc"), std::string("abc"), __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertNotEquals_WideCStringBothNull_Fails()
{
    wchar_t const* const nullStr = nullptr;
    AssertNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_AssertNotEquals_Wide_Fails()
{
    // "ete" with two e-acutes.
    std::wstring const text = L"\x00E9" L"t" L"\x00E9";
    AssertNotEquals(text, text, __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckEquals_WideCStringNull_Fails()
{
    wchar_t const* const nullStr = nullptr;
    CheckEquals(nullStr, L"", __func__, __LINE__, "null never matches, even an empty string");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckEquals_WideCString_Fails()
{
    CheckEquals(L"abc", L"xyz", __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckEquals_Wide_Fails()
{
    // "cafe" with an e-acute.
    CheckEquals(std::wstring(L"caf" L"\x00E9"), std::wstring(L"cafe"), __func__, __LINE__, "different text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckNotEquals_CStringBothNull_Fails()
{
    char const* const nullStr = nullptr;
    CheckNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckNotEquals_String_Fails()
{
    CheckNotEquals(std::string("abc"), std::string("abc"), __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckNotEquals_WideCStringBothNull_Fails()
{
    wchar_t const* const nullStr = nullptr;
    CheckNotEquals(nullStr, nullStr, __func__, __LINE__, "two nulls are equal");
}
//---------------------------------------------------------------------------
void TFixture_StringComparisons::Test_CheckNotEquals_Wide_Fails()
{
    // "ete" with two e-acutes.
    std::wstring const text = L"\x00E9" L"t" L"\x00E9";
    CheckNotEquals(text, text, __func__, __LINE__, "same text");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ThrowingSetUpTearDown
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group whose SetUp_Test() or TearDown_Test(), as chosen
// by 'ThrowFrom', throws a std::runtime_error for its first test. That test expects an exception and throws one,
// so the exception from TearDown_Test() arrives while one is expected, and must not be taken for it. The second
// test records whether it ran. Used by Test_Run_EndsRunOnSetUpOrTearDownTestException below.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ThrowingSetUpTearDown : public TTestGroupBase
{
public:
    enum class TThrowFrom
    {
        SetUp,
        TearDown
    };

private:
    typedef TTestGroupBase inherited;

private:
    void Test_RunsAfterThrowingTest();
    void Test_ThrowsExpectedException();

public:
    bool RunsAfterThrowingTestReached = false;
    TThrowFrom const ThrowFrom;

public:
    explicit TFixture_ThrowingSetUpTearDown(TThrowFrom throwFrom);

    void SetUp_Group() override {}
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override {}
    void TearDown_Test(ITestCase& testCase) override;
};

//---------------------------------------------------------------------------
TFixture_ThrowingSetUpTearDown::TFixture_ThrowingSetUpTearDown(TThrowFrom throwFrom)
    : inherited("Fixture_ThrowingSetUpTearDown"),
      ThrowFrom(throwFrom)
{
    SetLogSuppressed(true);

    // Registration order matters here: the throwing test must run first.
    RegisterTest(&TFixture_ThrowingSetUpTearDown::Test_ThrowsExpectedException, "ThrowsExpectedException");
    RegisterTest(&TFixture_ThrowingSetUpTearDown::Test_RunsAfterThrowingTest, "RunsAfterThrowingTest");
}
//---------------------------------------------------------------------------
void TFixture_ThrowingSetUpTearDown::SetUp_Test(ITestCase& testCase)
{
    if (ThrowFrom == TThrowFrom::SetUp && testCase.GetName() == "ThrowsExpectedException")
        throw std::runtime_error("SetUp_Test failed");
}
//---------------------------------------------------------------------------
void TFixture_ThrowingSetUpTearDown::TearDown_Test(ITestCase& testCase)
{
    if (ThrowFrom == TThrowFrom::TearDown && testCase.GetName() == "ThrowsExpectedException")
        throw std::runtime_error("TearDown_Test failed");
}
//---------------------------------------------------------------------------
void TFixture_ThrowingSetUpTearDown::Test_RunsAfterThrowingTest()
{
    RunsAfterThrowingTestReached = true;
}
//---------------------------------------------------------------------------
void TFixture_ThrowingSetUpTearDown::Test_ThrowsExpectedException()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, matching any exception");
    throw std::logic_error("the expected exception");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ThrowsChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertThrows()/CheckThrows()/
// AssertNoThrow()/CheckNoThrow(), so Test_Throws_ChecksTypeAndMessageAndContinues below can check their outcomes
// and messages. ReachedAfterCheckNoThrow shows a failed Check lets the test continue; ReachedAfterInnerAssert shows
// an Assert failure inside the callable still ends the test instead of counting as the exception thrown. Test names
// self-document expected outcome via NameEndsWith(), same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ThrowsChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertNoThrow_NoException_Passes();
    void Test_AssertNoThrow_Throws_Fails();
    void Test_AssertThrows_NoException_Fails();
    void Test_AssertThrows_RightType_Passes();
    void Test_CheckNoThrow_Throws_Fails();
    void Test_CheckThrows_AssertFailureInside_Fails();
    void Test_CheckThrows_FrameworkTypeRequested_Passes();
    void Test_CheckThrows_MessageMatches_Passes();
    void Test_CheckThrows_MessageMismatch_Fails();
    void Test_CheckThrows_NoException_Fails();
    void Test_CheckThrows_NonStdException_Fails();
    void Test_CheckThrows_SeveralCalls_Passes();
    void Test_CheckThrows_Subclass_Passes();
    void Test_CheckThrows_WrongType_Fails();

public:
    bool ReachedAfterCheckNoThrow = false;
    bool ReachedAfterInnerAssert = false;

    TFixture_ThrowsChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_ThrowsChecks::TFixture_ThrowsChecks()
    : inherited("Fixture_ThrowsChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_ThrowsChecks::Test_AssertNoThrow_NoException_Passes, "AssertNoThrow_NoException_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_AssertNoThrow_Throws_Fails, "AssertNoThrow_Throws_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_AssertThrows_NoException_Fails, "AssertThrows_NoException_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_AssertThrows_RightType_Passes, "AssertThrows_RightType_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckNoThrow_Throws_Fails, "CheckNoThrow_Throws_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_AssertFailureInside_Fails,
        "CheckThrows_AssertFailureInside_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_FrameworkTypeRequested_Passes,
        "CheckThrows_FrameworkTypeRequested_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_MessageMatches_Passes, "CheckThrows_MessageMatches_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_MessageMismatch_Fails, "CheckThrows_MessageMismatch_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_NoException_Fails, "CheckThrows_NoException_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_NonStdException_Fails, "CheckThrows_NonStdException_Fails");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_SeveralCalls_Passes, "CheckThrows_SeveralCalls_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_Subclass_Passes, "CheckThrows_Subclass_Passes");
    RegisterTest(&TFixture_ThrowsChecks::Test_CheckThrows_WrongType_Fails, "CheckThrows_WrongType_Fails");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_AssertNoThrow_NoException_Passes()
{
    AssertNoThrow([] {
        }, __func__, __LINE__, "must not throw");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_AssertNoThrow_Throws_Fails()
{
    AssertNoThrow([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "must not throw");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_AssertThrows_NoException_Fails()
{
    AssertThrows<std::runtime_error>([] {
        }, __func__, __LINE__, "must throw");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_AssertThrows_RightType_Passes()
{
    AssertThrows<std::runtime_error>([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "must throw");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckNoThrow_Throws_Fails()
{
    CheckNoThrow([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "must not throw");
    ReachedAfterCheckNoThrow = true;
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_AssertFailureInside_Fails()
{
    CheckThrows<std::exception>([this] {
            AssertTrue(false, __func__, __LINE__, "inner assert");
        }, __func__, __LINE__,
        "an Assert failure isn't the exception being checked for");
    ReachedAfterInnerAssert = true;
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_FrameworkTypeRequested_Passes()
{
    CheckThrows<TExceptTrue>([this] {
            AssertTrue(false, __func__, __LINE__, "inner assert");
        }, __func__, __LINE__,
        "asking for TExceptTrue catches the Assert failure");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_MessageMatches_Passes()
{
    CheckThrows<std::runtime_error>([] {
            throw std::runtime_error("disk full");
        }, __func__, __LINE__,
        "message contains the substring", "full");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_MessageMismatch_Fails()
{
    CheckThrows<std::runtime_error>([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "wrong message",
        "bang");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_NoException_Fails()
{
    CheckThrows<std::runtime_error>([] {
        }, __func__, __LINE__, "must throw");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_NonStdException_Fails()
{
    CheckThrows<std::runtime_error>([] {
            throw 42;
        }, __func__, __LINE__, "an int isn't a runtime_error");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_SeveralCalls_Passes()
{
    std::vector<int> values{ 1, 2, 3 };
    CheckThrows<std::out_of_range>([&values] {
            static_cast<void>(values.at(3));
        }, __func__, __LINE__, "one past");
    CheckThrows<std::out_of_range>([&values] {
            static_cast<void>(values.at(10));
        }, __func__, __LINE__, "far past");
    CheckEquals(static_cast<size_t>(3), values.size(), __func__, __LINE__, "the test carries on and checks the state");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_Subclass_Passes()
{
    CheckThrows<std::logic_error>([] {
            throw std::out_of_range("index");
        }, __func__, __LINE__,
        "out_of_range is a logic_error");
}
//---------------------------------------------------------------------------
void TFixture_ThrowsChecks::Test_CheckThrows_WrongType_Fails()
{
    CheckThrows<std::out_of_range>([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "wrong type");
}
//---------------------------------------------------------------------------


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
// TFixture_TypeChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing AssertIsType()/CheckIsType()/
// AssertIsNotType()/CheckIsNotType() with a small class hierarchy, through raw and smart pointers and a reference, so
// Test_IsType_UsesDynamicTypeAndShowsNames below can check their outcomes and messages. A subclass counts as its base
// class's type, and a null pointer is never any type. Test names self-document expected outcome via NameEndsWith(),
// same as TFixture_ExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_TypeChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

    struct TShape
    {
        virtual ~TShape() = default;
    };

    struct TCircle : TShape
    {
    };

    struct TBigCircle : TCircle
    {
    };

    struct TSquare : TShape
    {
    };

private:
    void Test_AssertIsNotType_OtherType_Passes();
    void Test_AssertIsNotType_SameType_Fails();
    void Test_AssertIsType_RightType_Passes();
    void Test_AssertIsType_WrongType_Fails();
    void Test_CheckIsNotType_Null_Passes();
    void Test_CheckIsNotType_OtherType_Passes();
    void Test_CheckIsNotType_Subclass_Fails();
    void Test_CheckIsType_BaseObject_Fails();
    void Test_CheckIsType_Null_Fails();
    void Test_CheckIsType_Reference_Passes();
    void Test_CheckIsType_RightType_Passes();
    void Test_CheckIsType_SharedPtr_Passes();
    void Test_CheckIsType_Subclass_Passes();
    void Test_CheckIsType_UniquePtr_Fails();

public:
    TFixture_TypeChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_TypeChecks::TFixture_TypeChecks()
    : inherited("Fixture_TypeChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_TypeChecks::Test_AssertIsNotType_OtherType_Passes, "AssertIsNotType_OtherType_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_AssertIsNotType_SameType_Fails, "AssertIsNotType_SameType_Fails");
    RegisterTest(&TFixture_TypeChecks::Test_AssertIsType_RightType_Passes, "AssertIsType_RightType_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_AssertIsType_WrongType_Fails, "AssertIsType_WrongType_Fails");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsNotType_Null_Passes, "CheckIsNotType_Null_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsNotType_OtherType_Passes, "CheckIsNotType_OtherType_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsNotType_Subclass_Fails, "CheckIsNotType_Subclass_Fails");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_BaseObject_Fails, "CheckIsType_BaseObject_Fails");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_Null_Fails, "CheckIsType_Null_Fails");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_Reference_Passes, "CheckIsType_Reference_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_RightType_Passes, "CheckIsType_RightType_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_SharedPtr_Passes, "CheckIsType_SharedPtr_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_Subclass_Passes, "CheckIsType_Subclass_Passes");
    RegisterTest(&TFixture_TypeChecks::Test_CheckIsType_UniquePtr_Fails, "CheckIsType_UniquePtr_Fails");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_AssertIsNotType_OtherType_Passes()
{
    TSquare square;
    TShape const* const shape = &square;
    AssertIsNotType<TCircle>(shape, __func__, __LINE__, "a square isn't a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_AssertIsNotType_SameType_Fails()
{
    TCircle circle;
    TShape const* const shape = &circle;
    AssertIsNotType<TCircle>(shape, __func__, __LINE__, "it is a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_AssertIsType_RightType_Passes()
{
    TCircle circle;
    TShape const* const shape = &circle;
    AssertIsType<TCircle>(shape, __func__, __LINE__, "a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_AssertIsType_WrongType_Fails()
{
    TSquare square;
    TShape const* const shape = &square;
    AssertIsType<TCircle>(shape, __func__, __LINE__, "wrong type");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsNotType_Null_Passes()
{
    TShape const* const shape = nullptr;
    CheckIsNotType<TCircle>(shape, __func__, __LINE__, "a null pointer isn't a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsNotType_OtherType_Passes()
{
    TSquare square;
    TShape* const shape = &square;
    CheckIsNotType<TCircle>(shape, __func__, __LINE__, "a square isn't a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsNotType_Subclass_Fails()
{
    TBigCircle bigCircle;
    TShape* const shape = &bigCircle;
    CheckIsNotType<TCircle>(shape, __func__, __LINE__, "a big circle is a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_BaseObject_Fails()
{
    TShape shape;
    CheckIsType<TCircle>(&shape, __func__, __LINE__, "a plain shape isn't a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_Null_Fails()
{
    TShape const* const shape = nullptr;
    CheckIsType<TCircle>(shape, __func__, __LINE__, "null");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_Reference_Passes()
{
    TCircle circle;
    TShape const& shape = circle;
    CheckIsType<TCircle>(shape, __func__, __LINE__, "an object, not a pointer");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_RightType_Passes()
{
    TCircle circle;
    TShape* const shape = &circle;
    CheckIsType<TCircle>(shape, __func__, __LINE__, "a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_SharedPtr_Passes()
{
    std::shared_ptr<TShape> const shape = std::make_shared<TCircle>();
    CheckIsType<TCircle>(shape, __func__, __LINE__, "through a shared_ptr");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_Subclass_Passes()
{
    TBigCircle bigCircle;
    TShape* const shape = &bigCircle;
    CheckIsType<TCircle>(shape, __func__, __LINE__, "a big circle is a circle");
}
//---------------------------------------------------------------------------
void TFixture_TypeChecks::Test_CheckIsType_UniquePtr_Fails()
{
    std::unique_ptr<TShape> const shape = std::make_unique<TSquare>();
    CheckIsType<TCircle>(shape, __func__, __LINE__, "through a unique_ptr");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_UnexpectedExceptions
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group whose tests throw without expecting it: a
// std::exception, an object that isn't an exception at all, and a std::exception after a failed Check. Each of
// the first two is followed by a test that passes. Registration order matters: each throwing test must run before
// the test after it. Used by the Run_*UnexpectedException* tests below to prove an unexpected exception only fails
// the test that threw it, after which TearDown_Test() and the remaining tests still run.
/////////////////////////////////////////////////////////////////////////////
class TFixture_UnexpectedExceptions : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_CheckFailsThenThrowsStdException();
    void Test_RunsAfterNonException();
    void Test_RunsAfterStdException();
    void Test_ThrowsNonException();
    void Test_ThrowsStdException();

public:
    std::vector<std::string> TornDownTests; // The tests TearDown_Test() ran for, in order.

public:
    TFixture_UnexpectedExceptions();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
    void TearDown_Test(ITestCase& testCase) override;
};

//---------------------------------------------------------------------------
TFixture_UnexpectedExceptions::TFixture_UnexpectedExceptions()
    : inherited("Fixture_UnexpectedExceptions")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_UnexpectedExceptions::Test_ThrowsStdException, "ThrowsStdException");
    RegisterTest(&TFixture_UnexpectedExceptions::Test_RunsAfterStdException, "RunsAfterStdException");
    RegisterTest(&TFixture_UnexpectedExceptions::Test_ThrowsNonException, "ThrowsNonException");
    RegisterTest(&TFixture_UnexpectedExceptions::Test_RunsAfterNonException, "RunsAfterNonException");
    RegisterTest(&TFixture_UnexpectedExceptions::Test_CheckFailsThenThrowsStdException,
        "CheckFailsThenThrowsStdException");
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::TearDown_Test(ITestCase& testCase)
{
    TornDownTests.push_back(testCase.GetName());
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::Test_CheckFailsThenThrowsStdException()
{
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the unexpected exception");
    throw std::runtime_error("thrown after a failed Check");
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::Test_RunsAfterNonException()
{
    CheckTrue(true, __func__, __LINE__, "trivially passes");
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::Test_RunsAfterStdException()
{
    CheckTrue(true, __func__, __LINE__, "trivially passes");
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::Test_ThrowsNonException()
{
    throw 42; // Not derived from std::exception at all.
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedExceptions::Test_ThrowsStdException()
{
    // Like a constructor in the code under test rejecting its argument.
    throw std::invalid_argument("bad placeholder");
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
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Empty_ShowsContentsOnFailure, "Empty_ShowsContentsOnFailure");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_EndsWithIC_IgnoresASCIICaseOnly, "EndsWithIC_IgnoresASCIICaseOnly");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_EndsWith_ShowsTextAndSuffix, "EndsWith_ShowsTextAndSuffix");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_EqualsIC_IgnoresASCIICaseOnly, "EqualsIC_IgnoresASCIICaseOnly");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_EqualsMem_ShowsFirstDifferingBytes,
        "EqualsMem_ShowsFirstDifferingBytes");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesCStringsByContent, "Equals_ComparesCStringsByContent");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesEnumClassByUnderlyingValue,
        "Equals_ComparesEnumClassByUnderlyingValue");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesMixedIntegerTypesByValue,
        "Equals_ComparesMixedIntegerTypesByValue");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ComparesPointersByAddress,
        "Equals_ComparesPointersByAddress");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ShowsBoolValuesAsTrueOrFalse,
        "Equals_ShowsBoolValuesAsTrueOrFalse");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Equals_ShowsStringValues, "Equals_ShowsStringValues");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Fail_AbortsTestAsFailed, "Fail_AbortsTestAsFailed");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_IsType_UsesDynamicTypeAndShowsNames,
        "IsType_UsesDynamicTypeAndShowsNames");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Matches_MatchesWholeTextAndShowsPattern,
        "Matches_MatchesWholeTextAndShowsPattern");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_NullNotNull_FailureNamesTheExpectedValue,
        "NullNotNull_FailureNamesTheExpectedValue");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Ordering_ComparesByValueAndShowsBoth,
        "Ordering_ComparesByValueAndShowsBoth");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AbandonsHungTestAndAbortsGroupOnTimeout,
        "Run_AbandonsHungTestAndAbortsGroupOnTimeout");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AppliesFilterToSkipNonMatchingTests,
        "Run_AppliesFilterToSkipNonMatchingTests");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet,
        "Run_ContinuesAfterCrashWhenCatchCrashesIsSet");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_EndsRunOnSetUpOrTearDownTestException,
        "Run_EndsRunOnSetUpOrTearDownTestException");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_LogsEachCheckFailureOnce, "Run_LogsEachCheckFailureOnce");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsCheckFailuresInFailedTestDetail,
        "Run_RecordsCheckFailuresInFailedTestDetail");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsDurationOfShortTest,
        "Run_RecordsDurationOfShortTest");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords,
        "Run_RecordsOutcomeCountsAndCaseRecords");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsUnexpectedExceptionAsFailureAndContinues,
        "Run_RecordsUnexpectedExceptionAsFailureAndContinues");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsUnexpectedExceptionWithRunOptions,
        "Run_RecordsUnexpectedExceptionWithRunOptions");
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
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Same_ComparesAddressesNotContent,
        "Same_ComparesAddressesNotContent");
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
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_StartsWithIC_IgnoresASCIICaseOnly,
        "StartsWithIC_IgnoresASCIICaseOnly");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_StartsWith_ShowsTextAndPrefix, "StartsWith_ShowsTextAndPrefix");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Throws_ChecksTypeAndMessageAndContinues,
        "Throws_ChecksTypeAndMessageAndContinues");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_TrueFalse_FailureNamesTheExpectedValue,
        "TrueFalse_FailureNamesTheExpectedValue");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_TestBase::~TTest_ASWUnitTests_TestBase()
{
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_TestBase::CheckUnexpectedExceptionRecords

    Checks the records a run of every TFixture_UnexpectedExceptions test leaves in 'results', whatever order the
    tests ran in: each throwing test failed with an "Unexpected exception: " detail describing what it threw, and
    each other test passed. 'method'/'line' identify the calling test in any failure message.
*/
void TTest_ASWUnitTests_TestBase::CheckUnexpectedExceptionRecords(TTestResults const& results,
    std::string const& method, int line)
{
    // The same description CheckNoThrow() gives an object that isn't an exception.
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    std::string const nonExceptionDescription = "an object that is neither a std::exception nor an RTL Exception";
#else
    std::string const nonExceptionDescription = "a non-std::exception object";
#endif

    struct TExpectedRecord
    {
        std::string TestName;
        TTestOutcome Outcome;
        std::string Message;
    };

    TExpectedRecord const expectedRecords[] =
    {
        { "ThrowsStdException", TTestOutcome::Fail, "Unexpected exception: bad placeholder" },
        { "RunsAfterStdException", TTestOutcome::Pass, "" },
        { "ThrowsNonException", TTestOutcome::Fail, "Unexpected exception: " + nonExceptionDescription },
        { "RunsAfterNonException", TTestOutcome::Pass, "" },
    };

    CheckEquals(static_cast<size_t>(5), results.CaseRecords.size(), method, line,
        "one record per test, including each that threw");
    CheckEquals(3u, results.FailedCount, method, line, "each throwing test failed");
    CheckEquals(2u, results.SuccessCount, method, line, "each test after one that threw still ran, and passed");

    for (TExpectedRecord const& expected : expectedRecords)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, method, line, expected.TestName + " has a record");
        CheckEquals(expected.Outcome, record->Outcome, method, line, expected.TestName + "'s outcome");
        CheckEquals(expected.Message, record->Message, method, line, expected.TestName + "'s detail");
    }

    // The record shows the earlier Check failure first, as for any failed test (see
    // Test_Run_RecordsCheckFailuresInFailedTestDetail()).
    TTestCaseRecord const* const checkThenThrow = FindRecord(results, "CheckFailsThenThrowsStdException");
    AssertNotNull(checkThenThrow, method, line, "CheckFailsThenThrowsStdException has a record");
    CheckEquals(TTestOutcome::Fail, checkThenThrow->Outcome, method, line, "CheckFailsThenThrowsStdException failed");
    CheckStartsWith(checkThenThrow->Message, "Check failed for: \"Test_CheckFailsThenThrowsStdException\" (", method,
        line, "the Check failure comes first");
    CheckEndsWith(checkThenThrow->Message, "\nUnexpected exception: thrown after a failed Check", method, line,
        "followed by the unexpected exception");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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
    std::string const containsDetail =
        "): Expected \"Hello World\" to contain \"xyz\" (ignoring case). substring absent";
    std::string const notContainsDetail =
        "): Expected \"Hello World\" not to contain \"WORLD\" (ignoring case). case differs";

    CheckStartsWith(assertContains->Message, "Substring not found: Test_AssertContainsIC_Absent_Fails (", __func__,
        __LINE__, "AssertContainsIC names the failure and test");
    CheckEndsWith(assertContains->Message, containsDetail, __func__, __LINE__,
        "AssertContainsIC shows the text and substring");
    CheckStartsWith(assertNotContains->Message,
        "Substring found: Test_AssertNotContainsIC_Wide_DifferentCase_Fails (", __func__, __LINE__,
        "AssertNotContainsIC names the failure and test");
    CheckEndsWith(assertNotContains->Message, notContainsDetail, __func__, __LINE__,
        "AssertNotContainsIC shows the wide text and substring");
    CheckStartsWith(checkContains->Message, "Check failed for: \"Test_CheckContainsIC_Absent_Fails\" (", __func__,
        __LINE__, "CheckContainsIC names the test");
    CheckEndsWith(checkContains->Message, containsDetail, __func__, __LINE__,
        "CheckContainsIC shows the text and substring");
    CheckStartsWith(checkNotContains->Message, "Check failed for: \"Test_CheckNotContainsIC_DifferentCase_Fails\" (",
        __func__, __LINE__, "CheckNotContainsIC names the test");
    CheckEndsWith(checkNotContains->Message, notContainsDetail, __func__, __LINE__,
        "CheckNotContainsIC shows the text and substring");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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

    CheckStartsWith(assertContains->Message, "Substring not found: Test_AssertContains_Absent_Fails (", __func__,
        __LINE__, "AssertContains names the failure and test");
    CheckEndsWith(assertContains->Message, containsDetail, __func__, __LINE__,
        "AssertContains shows the text and substring");
    CheckStartsWith(assertNotContains->Message, "Substring found: Test_AssertNotContains_Wide_Present_Fails (",
        __func__, __LINE__, "AssertNotContains names the failure and test");
    CheckEndsWith(assertNotContains->Message, notContainsDetail, __func__, __LINE__,
        "AssertNotContains shows the wide text and substring");
    CheckStartsWith(checkContains->Message, "Check failed for: \"Test_CheckContains_Absent_Fails\" (", __func__,
        __LINE__, "CheckContains names the test");
    CheckEndsWith(checkContains->Message, containsDetail, __func__, __LINE__,
        "CheckContains shows the text and substring");
    CheckStartsWith(checkNotContains->Message, "Check failed for: \"Test_CheckNotContains_Present_Fails\" (", __func__,
        __LINE__, "CheckNotContains names the test");
    CheckEndsWith(checkNotContains->Message, notContainsDetail, __func__, __LINE__,
        "CheckNotContains shows the text and substring");
    CheckEndsWith(checkNonASCII->Message, nonASCIIDetail, __func__, __LINE__, "a wide failure shows its text as UTF-8");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Empty_ShowsContentsOnFailure()
{
    // Arrange
    TFixture_EmptyChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
#if defined(__cpp_lib_string_view)
    size_t const expectedCount = 12;
#else
    size_t const expectedCount = 11;
#endif
    CheckEquals(expectedCount, results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each test name, the start of its failure message, and the end after the varying line number.
    struct TExpectedFailure
    {
        std::string TestName;
        std::string Prefix;
        std::string Detail;
    };

    std::vector<TExpectedFailure> const expectedFailures = {
        { "AssertEmpty_String_Fails", "Not empty: Test_AssertEmpty_String_Fails (",
          "): Expected empty but was \"abc\". has text" },
        { "AssertNotEmpty_EmptyVector_Fails", "Empty: Test_AssertNotEmpty_EmptyVector_Fails (",
          "): Expected not empty but was empty. no elements" },
        { "CheckEmpty_ForwardList_Fails", "Check failed for: \"Test_CheckEmpty_ForwardList_Fails\" (",
          "): Expected empty but was not empty. no size to show" },
        { "CheckEmpty_OneElement_Fails", "Check failed for: \"Test_CheckEmpty_OneElement_Fails\" (",
          "): Expected empty but had 1 element. one element" },
#if defined(__cpp_lib_string_view)
        {
            "CheckEmpty_StringView_Fails", "Check failed for: \"Test_CheckEmpty_StringView_Fails\" (",
            "): Expected empty but was \"xyz\". shown as text, not an element count"
        },
#endif
        {
            "CheckEmpty_Vector_Fails", "Check failed for: \"Test_CheckEmpty_Vector_Fails\" (",
            "): Expected empty but had 3 elements. three elements"
        },
        { "CheckEmpty_WideString_Fails", "Check failed for: \"Test_CheckEmpty_WideString_Fails\" (",
          "): Expected empty but was \"caf\xC3\xA9\". wide text" },
        { "CheckNotEmpty_EmptyString_Fails", "Check failed for: \"Test_CheckNotEmpty_EmptyString_Fails\" (",
          "): Expected not empty but was empty. empty string" },
    };

    for (TExpectedFailure const& expected : expectedFailures)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, __func__, __LINE__, expected.TestName + " has a record");
        CheckStartsWith(record->Message, expected.Prefix, __func__, __LINE__, expected.TestName + " names the test");
        CheckEndsWith(record->Message, expected.Detail, __func__, __LINE__, expected.TestName + " shows the contents");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_EndsWithIC_IgnoresASCIICaseOnly()
{
    // Arrange
    TFixture_EndsWithICComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(28), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertEndsWith = FindRecord(results, "AssertEndsWithIC_Absent_Fails");
    TTestCaseRecord const* const assertEndsWithWide = FindRecord(results, "AssertEndsWithIC_Wide_Absent_Fails");
    TTestCaseRecord const* const assertNotEndsWith = FindRecord(results, "AssertNotEndsWithIC_DifferentCase_Fails");
    TTestCaseRecord const* const assertNotEndsWithWide =
        FindRecord(results, "AssertNotEndsWithIC_Wide_DifferentCase_Fails");
    TTestCaseRecord const* const checkEndsWith = FindRecord(results, "CheckEndsWithIC_Absent_Fails");
    TTestCaseRecord const* const checkEndsWithWide = FindRecord(results, "CheckEndsWithIC_Wide_Absent_Fails");
    TTestCaseRecord const* const checkNotEndsWith = FindRecord(results, "CheckNotEndsWithIC_DifferentCase_Fails");
    TTestCaseRecord const* const checkNotEndsWithWide =
        FindRecord(results, "CheckNotEndsWithIC_Wide_DifferentCase_Fails");
    AssertTrue(assertEndsWith != nullptr && assertEndsWithWide != nullptr && assertNotEndsWith != nullptr &&
        assertNotEndsWithWide != nullptr && checkEndsWith != nullptr && checkEndsWithWide != nullptr &&
        checkNotEndsWith != nullptr && checkNotEndsWithWide != nullptr, __func__, __LINE__,
        "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. The
    // text and suffix are shown as given, not case-folded. Each "Wide_" test uses the same text as its narrow twin.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };
    std::string const endsWithDetail = "): Expected \"Hello World\" to end with \"xyz\" (ignoring case). suffix absent";
    std::string const notEndsWithDetail =
        "): Expected \"Hello World\" not to end with \"WORLD\" (ignoring case). case differs";

    CheckTrue(messageMatches(*assertEndsWith, "Suffix not found: Test_AssertEndsWithIC_Absent_Fails (", endsWithDetail),
        __func__, __LINE__, "AssertEndsWithIC shows the text and suffix: " + assertEndsWith->Message);
    CheckTrue(messageMatches(*assertEndsWithWide, "Suffix not found: Test_AssertEndsWithIC_Wide_Absent_Fails (",
        endsWithDetail), __func__, __LINE__,
        "AssertEndsWithIC shows the wide text and suffix: " + assertEndsWithWide->Message);
    CheckTrue(messageMatches(*assertNotEndsWith, "Suffix found: Test_AssertNotEndsWithIC_DifferentCase_Fails (",
        notEndsWithDetail), __func__, __LINE__,
        "AssertNotEndsWithIC shows the text and suffix: " + assertNotEndsWith->Message);
    CheckTrue(messageMatches(*assertNotEndsWithWide,
        "Suffix found: Test_AssertNotEndsWithIC_Wide_DifferentCase_Fails (", notEndsWithDetail), __func__, __LINE__,
        "AssertNotEndsWithIC shows the wide text and suffix: " + assertNotEndsWithWide->Message);
    CheckTrue(messageMatches(*checkEndsWith, "Check failed for: \"Test_CheckEndsWithIC_Absent_Fails\" (",
        endsWithDetail), __func__, __LINE__, "CheckEndsWithIC shows the text and suffix: " + checkEndsWith->Message);
    CheckTrue(messageMatches(*checkEndsWithWide, "Check failed for: \"Test_CheckEndsWithIC_Wide_Absent_Fails\" (",
        endsWithDetail), __func__, __LINE__,
        "CheckEndsWithIC shows the wide text and suffix: " + checkEndsWithWide->Message);
    CheckTrue(messageMatches(*checkNotEndsWith, "Check failed for: \"Test_CheckNotEndsWithIC_DifferentCase_Fails\" (",
        notEndsWithDetail), __func__, __LINE__,
        "CheckNotEndsWithIC shows the text and suffix: " + checkNotEndsWith->Message);
    CheckTrue(messageMatches(*checkNotEndsWithWide,
        "Check failed for: \"Test_CheckNotEndsWithIC_Wide_DifferentCase_Fails\" (", notEndsWithDetail), __func__,
        __LINE__, "CheckNotEndsWithIC shows the wide text and suffix: " + checkNotEndsWithWide->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_EndsWith_ShowsTextAndSuffix()
{
    // Arrange
    TFixture_EndsWithComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(29), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertEndsWith = FindRecord(results, "AssertEndsWith_Absent_Fails");
    TTestCaseRecord const* const assertEndsWithWide = FindRecord(results, "AssertEndsWith_Wide_Absent_Fails");
    TTestCaseRecord const* const assertNotEndsWith = FindRecord(results, "AssertNotEndsWith_Present_Fails");
    TTestCaseRecord const* const assertNotEndsWithWide = FindRecord(results, "AssertNotEndsWith_Wide_Present_Fails");
    TTestCaseRecord const* const checkEndsWith = FindRecord(results, "CheckEndsWith_Absent_Fails");
    TTestCaseRecord const* const checkEndsWithWide = FindRecord(results, "CheckEndsWith_Wide_Absent_Fails");
    TTestCaseRecord const* const checkNonASCII = FindRecord(results, "CheckEndsWith_Wide_NonASCII_Fails");
    TTestCaseRecord const* const checkNotEndsWith = FindRecord(results, "CheckNotEndsWith_Present_Fails");
    TTestCaseRecord const* const checkNotEndsWithWide = FindRecord(results, "CheckNotEndsWith_Wide_Present_Fails");
    AssertTrue(assertEndsWith != nullptr && assertEndsWithWide != nullptr && assertNotEndsWith != nullptr &&
        assertNotEndsWithWide != nullptr && checkEndsWith != nullptr && checkEndsWithWide != nullptr &&
        checkNonASCII != nullptr && checkNotEndsWith != nullptr && checkNotEndsWithWide != nullptr, __func__,
        __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. Each
    // "Wide_" test uses the same text as its narrow twin.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };
    std::string const endsWithDetail = "): Expected \"hello world\" to end with \"xyz\". suffix absent";
    std::string const notEndsWithDetail = "): Expected \"hello world\" not to end with \"world\". suffix present";
    // "cafe" with an e-acute, a space and U+1F600, then u-umlaut, as UTF-8.
    std::string const nonASCIIDetail =
        "): Expected \"caf\xC3\xA9 \xF0\x9F\x98\x80\" to end with \"\xC3\xBC\". not present";

    CheckTrue(messageMatches(*assertEndsWith, "Suffix not found: Test_AssertEndsWith_Absent_Fails (", endsWithDetail),
        __func__, __LINE__, "AssertEndsWith shows the text and suffix: " + assertEndsWith->Message);
    CheckTrue(messageMatches(*assertEndsWithWide, "Suffix not found: Test_AssertEndsWith_Wide_Absent_Fails (",
        endsWithDetail), __func__, __LINE__,
        "AssertEndsWith shows the wide text and suffix: " + assertEndsWithWide->Message);
    CheckTrue(messageMatches(*assertNotEndsWith, "Suffix found: Test_AssertNotEndsWith_Present_Fails (",
        notEndsWithDetail), __func__, __LINE__,
        "AssertNotEndsWith shows the text and suffix: " + assertNotEndsWith->Message);
    CheckTrue(messageMatches(*assertNotEndsWithWide, "Suffix found: Test_AssertNotEndsWith_Wide_Present_Fails (",
        notEndsWithDetail), __func__, __LINE__,
        "AssertNotEndsWith shows the wide text and suffix: " + assertNotEndsWithWide->Message);
    CheckTrue(messageMatches(*checkEndsWith, "Check failed for: \"Test_CheckEndsWith_Absent_Fails\" (", endsWithDetail),
        __func__, __LINE__, "CheckEndsWith shows the text and suffix: " + checkEndsWith->Message);
    CheckTrue(messageMatches(*checkEndsWithWide, "Check failed for: \"Test_CheckEndsWith_Wide_Absent_Fails\" (",
        endsWithDetail), __func__, __LINE__,
        "CheckEndsWith shows the wide text and suffix: " + checkEndsWithWide->Message);
    CheckTrue(messageMatches(*checkNotEndsWith, "Check failed for: \"Test_CheckNotEndsWith_Present_Fails\" (",
        notEndsWithDetail), __func__, __LINE__,
        "CheckNotEndsWith shows the text and suffix: " + checkNotEndsWith->Message);
    CheckTrue(messageMatches(*checkNotEndsWithWide, "Check failed for: \"Test_CheckNotEndsWith_Wide_Present_Fails\" (",
        notEndsWithDetail), __func__, __LINE__,
        "CheckNotEndsWith shows the wide text and suffix: " + checkNotEndsWithWide->Message);
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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
    CheckStartsWith(assertEquals->Message, "Values not equal: Test_AssertEqualsIC_DifferentText_Fails (", __func__,
        __LINE__, "AssertEqualsIC names the failure and test");
    CheckEndsWith(assertEquals->Message, "): Expected: \"Hello\" but was \"Help\" (ignoring case). different text",
        __func__, __LINE__, "AssertEqualsIC shows both values");
    CheckStartsWith(assertNotEquals->Message, "Values are equal: Test_AssertNotEqualsIC_Wide_DifferentCase_Fails (",
        __func__, __LINE__, "AssertNotEqualsIC names the failure and test");
    CheckEndsWith(assertNotEquals->Message,
        "): Values: \"Hello World\" and \"hELLO wORLD\" (ignoring case). case differs", __func__, __LINE__,
        "AssertNotEqualsIC shows both wide values");
    CheckStartsWith(checkEquals->Message, "Check failed for: \"Test_CheckEqualsIC_DifferentText_Fails\" (", __func__,
        __LINE__, "CheckEqualsIC names the test");
    CheckEndsWith(checkEquals->Message, "): Expected \"Hello\" but was \"Help\" (ignoring case). different text",
        __func__, __LINE__, "CheckEqualsIC shows both values");
    CheckStartsWith(checkNonASCII->Message, "Check failed for: \"Test_CheckEqualsIC_Wide_NonASCII_Fails\" (",
        __func__, __LINE__, "CheckEqualsIC names the test");
    CheckEndsWith(checkNonASCII->Message,
        "): Expected \"caf\xC3\x89\" but was \"caf\xC3\xA9\" (ignoring case). only A-Z are case-folded", __func__,
        __LINE__, "CheckEqualsIC shows wide values as UTF-8");
    CheckStartsWith(checkNotEquals->Message, "Check failed for: \"Test_CheckNotEqualsIC_DifferentCase_Fails\" (",
        __func__, __LINE__, "CheckNotEqualsIC names the test");
    CheckEndsWith(checkNotEquals->Message,
        "): Both values equal: \"Hello World\" and \"hELLO wORLD\" (ignoring case). case differs", __func__, __LINE__,
        "CheckNotEqualsIC shows both values");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_EqualsMem_ShowsFirstDifferingBytes()
{
    // Arrange
    TFixture_MemoryComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(13), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each test name, the start of its failure message, and the end after the varying line number.
    struct TExpectedFailure
    {
        std::string TestName;
        std::string Prefix;
        std::string Detail;
    };

    std::vector<TExpectedFailure> const expectedFailures = {
        { "AssertEqualsMem_Different_Fails", "Memory not equal: Test_AssertEqualsMem_Different_Fails (",
          "): Bytes differ at offset 2 of 4: expected \"03 04\" but was \"FF 04\". third byte differs" },
        { "AssertNotEqualsMem_Same_Fails", "Memory equal: Test_AssertNotEqualsMem_Same_Fails (",
          "): Expected the 4 bytes to differ, but both are \"01 02 03 04\". same bytes" },
        { "CheckEqualsMem_ActualNull_Fails", "Check failed for: \"Test_CheckEqualsMem_ActualNull_Fails\" (",
          "): Cannot compare 4 bytes: actual is null. nothing to compare with" },
        { "CheckEqualsMem_DifferentLater_Fails", "Check failed for: \"Test_CheckEqualsMem_DifferentLater_Fails\" (",
          "): Bytes differ at offset 20 of 24: expected \"14 15 16 17\" but was \"AA 15 16 17\". byte 20 differs" },
        { "CheckEqualsMem_LongDifference_Fails", "Check failed for: \"Test_CheckEqualsMem_LongDifference_Fails\" (",
          "): Bytes differ at offset 0 of 20: expected \"00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 ...\" but was "
          "\"11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11 ...\". shows 16 bytes, then ..." },
        { "CheckNotEqualsMem_Same_Fails", "Check failed for: \"Test_CheckNotEqualsMem_Same_Fails\" (",
          "): Expected the 4 bytes to differ, but both are \"DE AD BE EF\". same bytes" },
    };

    for (TExpectedFailure const& expected : expectedFailures)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, __func__, __LINE__, expected.TestName + " has a record");
        CheckStartsWith(record->Message, expected.Prefix, __func__, __LINE__, expected.TestName + " names the test");
        CheckEndsWith(record->Message, expected.Detail, __func__, __LINE__, expected.TestName + " shows the bytes");
    }
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertNull = FindRecord(results, "AssertEquals_NullAndNonNull_Fails");
    TTestCaseRecord const* const checkNull = FindRecord(results, "CheckEquals_NullAndEmpty_Fails");
    AssertTrue(assertNull != nullptr && checkNull != nullptr, __func__, __LINE__, "every expected record exists");

    CheckContains(assertNull->Message, "\"(null)\"", __func__, __LINE__,
        "an Assert failure shows a null C string as (null)");
    CheckContains(checkNull->Message, "Expected \"(null)\" but was \"\"", __func__, __LINE__,
        "a Check failure shows a null C string as (null), distinct from an empty one");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ComparesEnumClassByUnderlyingValue()
{
    // Arrange
    TFixture_EnumComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(12), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each test name and the end of its failure message, after the varying line number.
    std::vector<std::pair<std::string, std::string> > const expectedDetails = {
        { "AssertEquals_Different_Fails", "): Expected: \"0\" but was \"2\". different colors" },
        { "AssertNotEquals_Same_Fails", "): Value: \"1\". same color" },
        { "CheckEquals_CharUnderlying_Fails", "): Expected \"65\" but was \"66\". shown as numbers, not characters" },
        { "CheckEquals_Different_Fails", "): Expected \"0\" but was \"2\". different colors" },
        { "CheckEquals_NegativeUnderlying_Fails", "): Expected \"-1\" but was \"1\". a negative value stays negative" },
        { "CheckEquals_UInt64Underlying_Fails",
          "): Expected \"18446744073709551615\" but was \"1\". a value above INT64_MAX doesn't wrap" },
        { "CheckNotEquals_Same_Fails", "): Both values equal: \"1\". same color" },
    };

    for (std::pair<std::string, std::string> const& expected : expectedDetails)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.first);
        AssertNotNull(record, __func__, __LINE__, expected.first + " has a record");
        CheckEndsWith(record->Message, expected.second, __func__, __LINE__, expected.first + " shows the values");
    }
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const unsignedMax = FindRecord(results, "CheckEquals_NegativeAndUnsignedMax_Fails");
    TTestCaseRecord const* const uint64Max = FindRecord(results, "CheckEquals_NegativeAndUint64Max_Fails");
    AssertTrue(unsignedMax != nullptr && uint64Max != nullptr, __func__, __LINE__, "every expected record exists");

    CheckContains(unsignedMax->Message, "Expected \"-1\" but was \"4294967295\"", __func__, __LINE__,
        "a failure shows both values as they are, without wrapping either");
    CheckContains(uint64Max->Message, "Expected \"-1\" but was \"18446744073709551615\"", __func__, __LINE__,
        "a failure beyond int64_t's range shows both values as they are");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ComparesPointersByAddress()
{
    // Arrange
    TFixture_PointerComparisons fixture;
    auto const formatAddress = [](void const* pointer)
        {
            std::ostringstream stream;
            stream << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(pointer);
            return stream.str();
        };
    std::string const first = formatAddress(&fixture.First);
    std::string const second = formatAddress(&fixture.Second);

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(13), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertEquals = FindRecord(results, "AssertEquals_DifferentPointers_Fails");
    TTestCaseRecord const* const assertNotEquals = FindRecord(results, "AssertNotEquals_SamePointer_Fails");
    TTestCaseRecord const* const checkEquals = FindRecord(results, "CheckEquals_DifferentPointers_Fails");
    TTestCaseRecord const* const checkFunctions = FindRecord(results, "CheckEquals_DifferentFunctions_Fails");
    TTestCaseRecord const* const checkNull = FindRecord(results, "CheckEquals_NullAndNonNull_Fails");
    TTestCaseRecord const* const checkNotEquals = FindRecord(results, "CheckNotEquals_SamePointer_Fails");
    AssertTrue(assertEquals != nullptr && assertNotEquals != nullptr && checkEquals != nullptr &&
        checkFunctions != nullptr && checkNull != nullptr && checkNotEquals != nullptr, __func__, __LINE__,
        "every expected record exists");

    CheckEndsWith(assertEquals->Message, "): Expected: \"" + first + "\" but was \"" + second + "\". different objects",
        __func__, __LINE__, "AssertEquals shows both addresses");
    CheckEndsWith(assertNotEquals->Message, "): Value: \"" + first + "\". same object", __func__, __LINE__,
        "AssertNotEquals shows the shared address");
    CheckEndsWith(checkEquals->Message, "): Expected \"" + first + "\" but was \"" + second + "\". different objects",
        __func__, __LINE__, "CheckEquals shows both addresses");
    CheckContains(checkFunctions->Message, "Expected \"0x", __func__, __LINE__,
        "a function pointer's address is shown too");
    CheckEndsWith(checkNull->Message, "): Expected \"(null)\" but was \"" + first + "\". null and non-null", __func__,
        __LINE__, "a null pointer is shown as (null)");
    CheckEndsWith(checkNotEquals->Message, "): Both values equal: \"" + first + "\". same object", __func__, __LINE__,
        "CheckNotEquals shows the shared address");
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

    CheckContains(assertEquals->Message, "\"true\" but was \"false\"", __func__, __LINE__,
        "AssertEquals shows bools as true/false");
    CheckContains(assertNotEquals->Message, "Value: \"true\"", __func__, __LINE__,
        "AssertNotEquals shows bools as true/false");
    CheckContains(checkEquals->Message, "Expected \"true\" but was \"false\"", __func__, __LINE__,
        "CheckEquals shows bools as true/false");
    CheckContains(checkNotEquals->Message, "Both values equal: \"true\"", __func__, __LINE__,
        "CheckNotEquals shows bools as true/false");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Equals_ShowsStringValues()
{
    // Arrange
    TFixture_StringComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(13), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. Wide
    // text is shown as UTF-8 ("caf\xC3\xA9" is "cafe" with an e-acute).
    std::vector<std::pair<std::string, std::string> > const expectedDetails = {
        { "AssertEquals_WideCStringNull_Fails", "): Expected: \"abc\" but was \"(null)\". null never matches" },
        { "AssertEquals_Wide_Fails", "): Expected: \"caf\xC3\xA9\" but was \"cafe\". different text" },
        { "AssertNotEquals_CStringBothNull_Fails", "): Value: \"(null)\". two nulls are equal" },
        { "AssertNotEquals_String_Fails", "): Value: \"abc\". same text" },
        { "AssertNotEquals_WideCStringBothNull_Fails", "): Value: \"(null)\". two nulls are equal" },
        { "AssertNotEquals_Wide_Fails", "): Value: \"\xC3\xA9t\xC3\xA9\". same text" },
        { "CheckEquals_WideCStringNull_Fails",
          "): Expected \"(null)\" but was \"\". null never matches, even an empty string" },
        { "CheckEquals_WideCString_Fails", "): Expected \"abc\" but was \"xyz\". different text" },
        { "CheckEquals_Wide_Fails", "): Expected \"caf\xC3\xA9\" but was \"cafe\". different text" },
        { "CheckNotEquals_CStringBothNull_Fails", "): Both values equal: \"(null)\". two nulls are equal" },
        { "CheckNotEquals_String_Fails", "): Both values equal: \"abc\". same text" },
        { "CheckNotEquals_WideCStringBothNull_Fails", "): Both values equal: \"(null)\". two nulls are equal" },
        { "CheckNotEquals_Wide_Fails", "): Both values equal: \"\xC3\xA9t\xC3\xA9\". same text" },
    };

    for (std::pair<std::string, std::string> const& expected : expectedDetails)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.first);
        AssertNotNull(record, __func__, __LINE__, expected.first + " has a record");
        CheckEquals(TTestOutcome::Fail, record->Outcome, __func__, __LINE__, expected.first + " should fail");

        std::string prefix = "Check failed for: \"Test_" + expected.first + "\" (";
        if (expected.first.compare(0, 12, "AssertEquals") == 0)
            prefix = "Values not equal: Test_" + expected.first + " (";
        else if (expected.first.compare(0, 15, "AssertNotEquals") == 0)
            prefix = "Values are equal: Test_" + expected.first + " (";

        CheckStartsWith(record->Message, prefix, __func__, __LINE__, expected.first + " names the test");
        CheckEndsWith(record->Message, expected.second, __func__, __LINE__, expected.first + " shows the values");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Fail_AbortsTestAsFailed()
{
    // Arrange
    TFixture_FailCalls fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(3), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");
    CheckEquals(3u, results.FailedCount, __func__, __LINE__, "every test fails");
    CheckFalse(fixture.ReachedAfterFail, __func__, __LINE__, "Fail aborts the test");

    TTestCaseRecord const* const fail = FindRecord(results, "Fail_Fails");
    TTestCaseRecord const* const afterCheck = FindRecord(results, "Fail_AfterCheckFailure_Fails");
    TTestCaseRecord const* const whileExpected = FindRecord(results, "Fail_WhileExceptionExpected_Fails");
    AssertTrue(fail != nullptr && afterCheck != nullptr && whileExpected != nullptr, __func__, __LINE__,
        "every expected record exists");

    // The message is "Failed: <method> (<line>): <msg>"; the line varies, so the parts either side of it are checked.
    CheckEquals(TTestOutcome::Fail, fail->Outcome, __func__, __LINE__, "Fail fails the test");
    CheckStartsWith(fail->Message, "Failed: Test_Fail_Fails (", __func__, __LINE__, "Fail names the test");
    CheckEndsWith(fail->Message, "): unconditional failure", __func__, __LINE__, "Fail shows the message");
    CheckStartsWith(afterCheck->Message, "Check failed for: \"Test_Fail_AfterCheckFailure_Fails\" (", __func__,
        __LINE__, "the earlier check failure comes first");
    CheckEndsWith(afterCheck->Message, "): then an unconditional failure", __func__, __LINE__,
        "followed by the Fail message");
    CheckStartsWith(whileExpected->Message, "Failed: Test_Fail_WhileExceptionExpected_Fails (", __func__, __LINE__,
        "Fail isn't taken as the expected exception");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_IsType_UsesDynamicTypeAndShowsNames()
{
    // Arrange
    TFixture_TypeChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(14), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Type names vary by compiler (e.g. a namespace or "struct " prefix), but always end with the class's own name,
    // so each check is of the text around a name: '"' + <prefix> + "TCircle" + '"' and so on.
    struct TExpectedFailure
    {
        std::string TestName;
        std::string Prefix;
        std::vector<std::string> Parts; // In order, each found after the previous one.
    };

    std::vector<TExpectedFailure> const expectedFailures = {
        { "AssertIsNotType_SameType_Fails", "Type matched: Test_AssertIsNotType_SameType_Fails (",
          { "): Expected not type \"", "TCircle\" but was \"", "TCircle\". it is a circle" } },
        { "AssertIsType_WrongType_Fails", "Type mismatch: Test_AssertIsType_WrongType_Fails (",
          { "): Expected type \"", "TCircle\" but was \"", "TSquare\". wrong type" } },
        { "CheckIsNotType_Subclass_Fails", "Check failed for: \"Test_CheckIsNotType_Subclass_Fails\" (",
          { "): Expected not type \"", "TCircle\" but was \"", "TBigCircle\". a big circle is a circle" } },
        { "CheckIsType_BaseObject_Fails", "Check failed for: \"Test_CheckIsType_BaseObject_Fails\" (",
          { "): Expected type \"", "TCircle\" but was \"", "TShape\". a plain shape isn't a circle" } },
        { "CheckIsType_Null_Fails", "Check failed for: \"Test_CheckIsType_Null_Fails\" (",
          { "): Expected type \"", "TCircle\" but was null. null" } },
        { "CheckIsType_UniquePtr_Fails", "Check failed for: \"Test_CheckIsType_UniquePtr_Fails\" (",
          { "): Expected type \"", "TCircle\" but was \"", "TSquare\". through a unique_ptr" } },
    };

    for (TExpectedFailure const& expected : expectedFailures)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, __func__, __LINE__, expected.TestName + " has a record");
        CheckStartsWith(record->Message, expected.Prefix, __func__, __LINE__, expected.TestName + " names the test");

        size_t position = 0;
        for (std::string const& part : expected.Parts)
        {
            position = record->Message.find(part, position);
            if (position == std::string::npos)
            {
                Fail(__func__, __LINE__, expected.TestName + " should contain \"" + part + "\", in order: " +
                    record->Message);
            }

            position += part.size();
        }

        CheckEquals(record->Message.size(), position, __func__, __LINE__, expected.TestName + " ends with the message");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Matches_MatchesWholeTextAndShowsPattern()
{
    // Arrange
    TFixture_MatchesChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(13), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // Each test name, the start of its failure message, and the end after the varying line number.
    struct TExpectedFailure
    {
        std::string TestName;
        std::string Prefix;
        std::string Detail;
    };

    std::vector<TExpectedFailure> const expectedFailures = {
        { "AssertMatches_NotMatching_Fails", "Pattern not matched: Test_AssertMatches_NotMatching_Fails (",
          "): Expected \"2026-1-7\" to match \"\\d{4}-\\d{2}-\\d{2}\". not a date" },
        { "AssertNotMatches_Matching_Fails", "Pattern matched: Test_AssertNotMatches_Matching_Fails (",
          "): Expected \"abc123\" not to match \"[a-z]+\\d+\". matches" },
        { "CheckMatches_PartialMatch_Fails", "Check failed for: \"Test_CheckMatches_PartialMatch_Fails\" (",
          "): Expected \"abc123\" to match \"\\d+\". only part matches" },
        { "CheckMatches_Wide_NotMatching_Fails", "Check failed for: \"Test_CheckMatches_Wide_NotMatching_Fails\" (",
          "): Expected \"caf\xC3\xA9\" to match \"tea\". different text" },
        { "CheckNotMatches_Matching_Fails", "Check failed for: \"Test_CheckNotMatches_Matching_Fails\" (",
          "): Expected \"abc123\" not to match \"[a-z]+\\d+\". matches" },
    };

    for (TExpectedFailure const& expected : expectedFailures)
    {
        TTestCaseRecord const* const record = FindRecord(results, expected.TestName);
        AssertNotNull(record, __func__, __LINE__, expected.TestName + " has a record");
        CheckStartsWith(record->Message, expected.Prefix, __func__, __LINE__, expected.TestName + " names the test");
        CheckEndsWith(record->Message, expected.Detail, __func__, __LINE__, expected.TestName + " shows the pattern");
    }

    // std::regex_error's own description varies by library, so only the part before it is checked.
    std::vector<std::pair<std::string, std::string> > const invalidPatterns = {
        { "AssertMatches_InvalidPattern_Fails", "a(" },
        { "CheckMatches_InvalidPattern_Fails", "[a-" },
        { "CheckNotMatches_InvalidPattern_Fails", "a(" },
    };

    for (std::pair<std::string, std::string> const& invalid : invalidPatterns)
    {
        TTestCaseRecord const* const record = FindRecord(results, invalid.first);
        AssertNotNull(record, __func__, __LINE__, invalid.first + " has a record");
        CheckContains(record->Message, "): Invalid regular expression \"" + invalid.second + "\": ", __func__,
            __LINE__, invalid.first + " says the pattern is invalid");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_NullNotNull_FailureNamesTheExpectedValue()
{
    // Arrange
    TFixture_NullChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(15), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertNotNull = FindRecord(results, "AssertNotNull_Null_Fails");
    TTestCaseRecord const* const assertNull = FindRecord(results, "AssertNull_NonNull_Fails");
    TTestCaseRecord const* const checkNotNull = FindRecord(results, "CheckNotNull_Null_Fails");
    TTestCaseRecord const* const checkNull = FindRecord(results, "CheckNull_NonNull_Fails");
    AssertTrue(assertNotNull != nullptr && assertNull != nullptr && checkNotNull != nullptr && checkNull != nullptr,
        __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    CheckStartsWith(assertNotNull->Message, "Expected not null but was null: Test_AssertNotNull_Null_Fails (", __func__,
        __LINE__, "AssertNotNull expects not null");
    CheckEndsWith(assertNotNull->Message, "): pointer is null", __func__, __LINE__, "AssertNotNull shows the message");
    CheckStartsWith(assertNull->Message, "Expected null but was not null: Test_AssertNull_NonNull_Fails (", __func__,
        __LINE__, "AssertNull expects null");
    CheckEndsWith(assertNull->Message, "): pointer is not null", __func__, __LINE__, "AssertNull shows the message");
    CheckStartsWith(checkNotNull->Message, "Check failed for: \"Test_CheckNotNull_Null_Fails\" (", __func__, __LINE__,
        "CheckNotNull names the test");
    CheckEndsWith(checkNotNull->Message, "): Expected not null but was null: \"pointer is null\"", __func__, __LINE__,
        "CheckNotNull expects not null");
    CheckStartsWith(checkNull->Message, "Check failed for: \"Test_CheckNull_NonNull_Fails\" (", __func__, __LINE__,
        "CheckNull names the test");
    CheckEndsWith(checkNull->Message, "): Expected null but was not null: \"pointer is not null\"", __func__, __LINE__,
        "CheckNull expects null");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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
        AssertNotNull(record, __func__, __LINE__, expected.first + " has a record");

        std::string const prefix = (expected.first.compare(0, 6, "Assert") == 0) ?
                "Values out of order: Test_" + expected.first + " (" :
                "Check failed for: \"Test_" + expected.first + "\" (";
        CheckStartsWith(record->Message, prefix, __func__, __LINE__, expected.first + " names the test");
        CheckEndsWith(record->Message, expected.second, __func__, __LINE__,
            expected.first + " shows the value and the bound");
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
    CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, "recorded as Fail, not Skip");
    CheckContains(record.Message, "timeout", __func__, __LINE__,
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
    CheckEquals(TTestOutcome::Fail, crashedRecord.Outcome, __func__, __LINE__, "recorded as Fail, not Skip");
    CheckContains(crashedRecord.Message, "crashed", __func__, __LINE__, "the failure message explains why: it crashed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_EndsRunOnSetUpOrTearDownTestException()
{
    // Unlike an unexpected exception from a test itself, one from SetUp_Test() or TearDown_Test() still escapes
    // Run(), ending the run. The one from TearDown_Test() arrives after the test threw the exception it expected,
    // and used to be taken for that exception, passing the test.
    struct TScenario
    {
        TFixture_ThrowingSetUpTearDown::TThrowFrom ThrowFrom;
        std::string ExpectedMessage;
    };

    TScenario const scenarios[] =
    {
        { TFixture_ThrowingSetUpTearDown::TThrowFrom::SetUp, "SetUp_Test failed" },
        { TFixture_ThrowingSetUpTearDown::TThrowFrom::TearDown, "TearDown_Test failed" },
    };

    for (TScenario const& scenario : scenarios)
    {
        // Arrange
        TFixture_ThrowingSetUpTearDown fixture(scenario.ThrowFrom);
        std::string caughtMessage;

        // Act
        try
        {
            fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        }
        catch (std::exception const& ex)
        {
            caughtMessage = ex.what();
        }

        // Assert
        CheckEquals(scenario.ExpectedMessage, caughtMessage, __func__, __LINE__, "the exception escapes Run()");
        CheckFalse(fixture.RunsAfterThrowingTestReached, __func__, __LINE__,
            scenario.ExpectedMessage + ": no later test ran");
        CheckEquals(1u, fixture.Results().FailedCount, __func__, __LINE__,
            scenario.ExpectedMessage + ": the test counts as failed");
    }
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

    CheckStartsWith(failViaCheck->Message, "Check failed for: \"Test_FailViaCheck\"", __func__, __LINE__,
        "a Check-only failure's detail is its Check failure, not empty");
    CheckNotContains(failViaCheck->Message, "\n", __func__, __LINE__,
        "one Check failure is one line, with no other test's Check failures carried over");

    CheckEquals(static_cast<size_t>(2),
        CountOccurrences(continuesAfterCheck->Message, "Check failed for: \"Test_ContinuesAfterCheckFailure\""),
        __func__, __LINE__, "both Check failures are in the detail");
    CheckContains(continuesAfterCheck->Message, "\n", __func__, __LINE__,
        "multiple Check failures are separated by newlines");

    std::string const checkFailure = "deliberate Check failure before an Assert";
    std::string const assertFailure = "deliberate Assert failure after a Check";
    CheckContains(checkThenAssert->Message, checkFailure, __func__, __LINE__,
        "a Check failure followed by an Assert failure records the Check failure");
    CheckContains(checkThenAssert->Message, assertFailure, __func__, __LINE__, "and the Assert failure");
    CheckLessThan(checkThenAssert->Message.find(checkFailure), checkThenAssert->Message.find(assertFailure), __func__,
        __LINE__, "in the order they happened");

    CheckNotEmpty(failViaAssert->Message, __func__, __LINE__, "an Assert failure still records its message");
    CheckNotContains(failViaAssert->Message, "Check failed for", __func__, __LINE__,
        "a test with no Check failures gets none in its detail");
    CheckEmpty(pass->Message, __func__, __LINE__, "a passing test's detail stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsDurationOfShortTest()
{
    // Arrange
    // The recorded duration encloses the test's own busy-wait, so it can't be shorter, with a clock fine enough to
    // see it. RAD Studio's 32-bit high_resolution_clock wasn't, and used to record almost every short test as 0.
    TFixture_ShortTest fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);
    double const busyWaitSeconds = std::chrono::duration<double>(TFixture_ShortTest::BusyWaitDuration).count();

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    AssertEquals(static_cast<size_t>(1), results.CaseRecords.size(), __func__, __LINE__, "one record");
    CheckGreaterThanOrEqual(results.CaseRecords.front().DurationSeconds, busyWaitSeconds, __func__, __LINE__,
        "the recorded duration includes the test's busy-wait");

    std::string const logText = observer.LogText();
    CheckContains(logText, "Finished test: \"Fixture_ShortTest.BusyWaits\" - passed (", __func__, __LINE__,
        "the test's timing line is logged");
    CheckNotContains(logText, "- passed (0.000 ms)", __func__, __LINE__, "and doesn't show it as taking no time");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, "Pass recorded as Pass");
        }
        else if (record.TestName == "Skip")
        {
            CheckEquals(TTestOutcome::Skip, record.Outcome, __func__, __LINE__, "Skip recorded as Skip");
        }
        else
        {
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " recorded as Fail");
        }
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsUnexpectedExceptionAsFailureAndContinues()
{
    // Arrange
    TFixture_UnexpectedExceptions fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);

    // Act
    // An unexpected exception used to escape Run(), ending the whole run after the test that threw it.
    CheckNoThrow([&fixture]() {
            fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
        }, __func__, __LINE__, "an unexpected exception from a test doesn't escape Run()");

    // Assert
    TTestResults const& results = fixture.Results();
    CheckUnexpectedExceptionRecords(results, __func__, __LINE__);

    std::vector<std::string> const allTests{ "ThrowsStdException", "RunsAfterStdException", "ThrowsNonException",
                                             "RunsAfterNonException", "CheckFailsThenThrowsStdException" };
    CheckTrue(fixture.TornDownTests == allTests, __func__, __LINE__,
        "TearDown_Test() ran after every test, including each that threw");
    CheckEquals(allTests.size(), observer.FinishedRecords.size(), __func__, __LINE__,
        "the run observer heard about every test, including each that threw");
    CheckEquals(ExitCode_TestsFailed, ExitCodeForResults(results), __func__, __LINE__,
        "an unexpected exception is an ordinary test failure, with the same exit code");

    std::string const xml = TJUnitReportWriter::BuildXML("Suite", ToJUnitTestCases(results.CaseRecords));
    CheckContains(xml, "<failure message=\"Unexpected exception: bad placeholder\">", __func__, __LINE__,
        "the JUnit report shows the test as a failure, with the exception's message");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsUnexpectedExceptionWithRunOptions()
{
    // Each of these runs the tests differently (in a shuffled order, on a worker thread, or under the crash guard),
    // and an unexpected exception must be caught and recorded the same way under each.
    struct TRunOptions
    {
        std::string Description;
        std::optional<unsigned int> ShuffleSeed;
        std::optional<unsigned int> TestTimeoutSeconds;
        bool CatchCrashes;
    };

    TRunOptions const runs[] =
    {
        { "shuffled", 12345u, std::nullopt, false },
        { "with a timeout", std::nullopt, 30u, false },
        { "catching crashes", std::nullopt, std::nullopt, true },
    };

    for (TRunOptions const& run : runs)
    {
        // Arrange
        TFixture_UnexpectedExceptions fixture;

        // Act
        CheckNoThrow([&fixture, &run]() {
                fixture.Run(TestFilter(), run.ShuffleSeed, run.TestTimeoutSeconds, run.CatchCrashes);
            }, __func__, __LINE__, "an unexpected exception doesn't escape Run() " + run.Description);

        // Assert
        CheckUnexpectedExceptionRecords(fixture.Results(), std::string(__func__) + ", " + run.Description, __LINE__);
    }

    // Arrange
    TFixture_UnexpectedExceptions filtered;
    TestFilter const filter = [](std::string const& fullName)
        {
            return fullName == "Fixture_UnexpectedExceptions.ThrowsStdException" ||
                fullName == "Fixture_UnexpectedExceptions.RunsAfterStdException";
        };

    // Act
    CheckNoThrow([&filtered, &filter]() {
            filtered.Run(filter, std::nullopt, std::nullopt, false);
        }, __func__, __LINE__, "an unexpected exception doesn't escape a filtered Run()");

    // Assert
    TTestResults const& filteredResults = filtered.Results();
    CheckEquals(static_cast<size_t>(2), filteredResults.CaseRecords.size(), __func__, __LINE__,
        "only the two tests the filter matches ran");
    CheckEquals(1u, filteredResults.FailedCount, __func__, __LINE__, "the throwing test failed");
    CheckEquals(1u, filteredResults.SuccessCount, __func__, __LINE__, "the test after it still ran, and passed");
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

    CheckContains(observer.LogText(), "Running test: Fixture_MixedOutcomes.Pass", __func__, __LINE__,
        "the group's log output goes to the observer");
    CheckEmpty(consoleOutput, __func__, __LINE__, "and none of it goes to std::cout");
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
    CheckEquals(TTestOutcome::Fail, observer.FinishedRecords.front().Outcome, __func__, __LINE__,
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
void TTest_ASWUnitTests_TestBase::Test_Same_ComparesAddressesNotContent()
{
    // Arrange
    TFixture_SameChecks fixture;
    auto const formatAddress = [](void const* pointer)
        {
            std::ostringstream stream;
            stream << "0x" << std::hex << reinterpret_cast<std::uintptr_t>(pointer);
            return stream.str();
        };
    std::string const first = formatAddress(&fixture.First);
    std::string const second = formatAddress(&fixture.Second);

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(14), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertNotSame = FindRecord(results, "AssertNotSame_SameObject_Fails");
    TTestCaseRecord const* const assertSame = FindRecord(results, "AssertSame_DifferentObjects_Fails");
    TTestCaseRecord const* const checkNotSame = FindRecord(results, "CheckNotSame_SameObject_Fails");
    TTestCaseRecord const* const checkSame = FindRecord(results, "CheckSame_DifferentObjects_Fails");
    AssertTrue(assertNotSame != nullptr && assertSame != nullptr && checkNotSame != nullptr && checkSame != nullptr,
        __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    CheckStartsWith(assertNotSame->Message, "Same object: Test_AssertNotSame_SameObject_Fails (", __func__, __LINE__,
        "AssertNotSame names the test");
    CheckEndsWith(assertNotSame->Message, "): Expected a different object but both are \"" + first +
        "\". same object", __func__, __LINE__, "AssertNotSame shows the shared address");
    CheckStartsWith(assertSame->Message, "Not the same object: Test_AssertSame_DifferentObjects_Fails (", __func__,
        __LINE__, "AssertSame names the test");
    CheckEndsWith(assertSame->Message, "): Expected the same object as \"" + first + "\" but was \"" + second +
        "\". different objects", __func__, __LINE__, "AssertSame shows both addresses");
    CheckStartsWith(checkNotSame->Message, "Check failed for: \"Test_CheckNotSame_SameObject_Fails\" (", __func__,
        __LINE__, "CheckNotSame names the test");
    CheckEndsWith(checkNotSame->Message, "): Expected a different object but both are \"" + first +
        "\". same object", __func__, __LINE__, "CheckNotSame shows the shared address");
    CheckStartsWith(checkSame->Message, "Check failed for: \"Test_CheckSame_DifferentObjects_Fails\" (", __func__,
        __LINE__, "CheckSame names the test");
    CheckEndsWith(checkSame->Message, "): Expected the same object as \"" + first + "\" but was \"" + second +
        "\". different objects", __func__, __LINE__, "CheckSame shows both addresses");
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
        CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " fails");
        CheckContains(record.Message, "deliberate Assert failure while an exception is expected", __func__, __LINE__,
            record.TestName + "'s record carries its Assert failure");
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
        CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " fails");
        CheckContains(record.Message, "deliberate Check failure before the expected exception", __func__, __LINE__,
            record.TestName + "'s record carries its Check failure");
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
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
    CheckNotEmpty(verboseOutput, __func__, __LINE__, "an unsuppressed fixture logs its test run as usual");
    CheckEmpty(suppressedOutput, __func__, __LINE__,
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
    CheckEquals(static_cast<size_t>(98), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
        {
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
            continue;
        }

        if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else if (NameEndsWith(record.TestName, "_Skips"))
            CheckEquals(TTestOutcome::Skip, record.Outcome, __func__, __LINE__, record.TestName + " should skip");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes, _Fails or _Skips");

        // function_name() is compiler-specific (e.g. "void NS::TClass::Test_X()" on GCC), but always contains the
        // function's own name.
        auto const expectedLine = fixture.ExpectedLines.find(record.TestName);
        AssertTrue(expectedLine != fixture.ExpectedLines.end(), __func__, __LINE__,
            record.TestName + " recorded the line of its call");
        CheckContains(record.Message, "Test_" + record.TestName, __func__, __LINE__,
            record.TestName + " reports its own function");
        CheckContains(record.Message, "(" + std::to_string(expectedLine->second) + ")", __func__, __LINE__,
            record.TestName + " reports the line of its call");
    }

    TTestCaseRecord const* const throughHelper = FindRecord(results, "CheckTrue_ThroughHelper_Fails");
    AssertNotNull(throughHelper, __func__, __LINE__, "the helper test's record exists");
    CheckNotContains(throughHelper->Message, "CheckIsEven", __func__, __LINE__,
        "a helper that passes its caller's location through isn't itself reported");
}
//---------------------------------------------------------------------------
#endif // #if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
void TTest_ASWUnitTests_TestBase::Test_StartsWithIC_IgnoresASCIICaseOnly()
{
    // Arrange
    TFixture_StartsWithICComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(28), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertStartsWith = FindRecord(results, "AssertStartsWithIC_Absent_Fails");
    TTestCaseRecord const* const assertStartsWithWide = FindRecord(results, "AssertStartsWithIC_Wide_Absent_Fails");
    TTestCaseRecord const* const assertNotStartsWith = FindRecord(results, "AssertNotStartsWithIC_DifferentCase_Fails");
    TTestCaseRecord const* const assertNotStartsWithWide =
        FindRecord(results, "AssertNotStartsWithIC_Wide_DifferentCase_Fails");
    TTestCaseRecord const* const checkStartsWith = FindRecord(results, "CheckStartsWithIC_Absent_Fails");
    TTestCaseRecord const* const checkStartsWithWide = FindRecord(results, "CheckStartsWithIC_Wide_Absent_Fails");
    TTestCaseRecord const* const checkNotStartsWith = FindRecord(results, "CheckNotStartsWithIC_DifferentCase_Fails");
    TTestCaseRecord const* const checkNotStartsWithWide =
        FindRecord(results, "CheckNotStartsWithIC_Wide_DifferentCase_Fails");
    AssertTrue(assertStartsWith != nullptr && assertStartsWithWide != nullptr && assertNotStartsWith != nullptr &&
        assertNotStartsWithWide != nullptr && checkStartsWith != nullptr && checkStartsWithWide != nullptr &&
        checkNotStartsWith != nullptr && checkNotStartsWithWide != nullptr, __func__, __LINE__,
        "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. The
    // text and prefix are shown as given, not case-folded. Each "Wide_" test uses the same text as its narrow twin.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };
    std::string const startsWithDetail =
        "): Expected \"Hello World\" to start with \"xyz\" (ignoring case). prefix absent";
    std::string const notStartsWithDetail =
        "): Expected \"Hello World\" not to start with \"HELLO\" (ignoring case). case differs";

    CheckTrue(messageMatches(*assertStartsWith, "Prefix not found: Test_AssertStartsWithIC_Absent_Fails (",
        startsWithDetail), __func__, __LINE__,
        "AssertStartsWithIC shows the text and prefix: " + assertStartsWith->Message);
    CheckTrue(messageMatches(*assertStartsWithWide, "Prefix not found: Test_AssertStartsWithIC_Wide_Absent_Fails (",
        startsWithDetail), __func__, __LINE__,
        "AssertStartsWithIC shows the wide text and prefix: " + assertStartsWithWide->Message);
    CheckTrue(messageMatches(*assertNotStartsWith, "Prefix found: Test_AssertNotStartsWithIC_DifferentCase_Fails (",
        notStartsWithDetail), __func__, __LINE__,
        "AssertNotStartsWithIC shows the text and prefix: " + assertNotStartsWith->Message);
    CheckTrue(messageMatches(*assertNotStartsWithWide,
        "Prefix found: Test_AssertNotStartsWithIC_Wide_DifferentCase_Fails (", notStartsWithDetail), __func__,
        __LINE__, "AssertNotStartsWithIC shows the wide text and prefix: " + assertNotStartsWithWide->Message);
    CheckTrue(messageMatches(*checkStartsWith, "Check failed for: \"Test_CheckStartsWithIC_Absent_Fails\" (",
        startsWithDetail), __func__, __LINE__,
        "CheckStartsWithIC shows the text and prefix: " + checkStartsWith->Message);
    CheckTrue(messageMatches(*checkStartsWithWide, "Check failed for: \"Test_CheckStartsWithIC_Wide_Absent_Fails\" (",
        startsWithDetail), __func__, __LINE__,
        "CheckStartsWithIC shows the wide text and prefix: " + checkStartsWithWide->Message);
    CheckTrue(messageMatches(*checkNotStartsWith,
        "Check failed for: \"Test_CheckNotStartsWithIC_DifferentCase_Fails\" (", notStartsWithDetail), __func__,
        __LINE__, "CheckNotStartsWithIC shows the text and prefix: " + checkNotStartsWith->Message);
    CheckTrue(messageMatches(*checkNotStartsWithWide,
        "Check failed for: \"Test_CheckNotStartsWithIC_Wide_DifferentCase_Fails\" (", notStartsWithDetail), __func__,
        __LINE__, "CheckNotStartsWithIC shows the wide text and prefix: " + checkNotStartsWithWide->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_StartsWith_ShowsTextAndPrefix()
{
    // Arrange
    TFixture_StartsWithComparisons fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(29), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertStartsWith = FindRecord(results, "AssertStartsWith_Absent_Fails");
    TTestCaseRecord const* const assertStartsWithWide = FindRecord(results, "AssertStartsWith_Wide_Absent_Fails");
    TTestCaseRecord const* const assertNotStartsWith = FindRecord(results, "AssertNotStartsWith_Present_Fails");
    TTestCaseRecord const* const assertNotStartsWithWide =
        FindRecord(results, "AssertNotStartsWith_Wide_Present_Fails");
    TTestCaseRecord const* const checkStartsWith = FindRecord(results, "CheckStartsWith_Absent_Fails");
    TTestCaseRecord const* const checkStartsWithWide = FindRecord(results, "CheckStartsWith_Wide_Absent_Fails");
    TTestCaseRecord const* const checkNonASCII = FindRecord(results, "CheckStartsWith_Wide_NonASCII_Fails");
    TTestCaseRecord const* const checkNotStartsWith = FindRecord(results, "CheckNotStartsWith_Present_Fails");
    TTestCaseRecord const* const checkNotStartsWithWide = FindRecord(results, "CheckNotStartsWith_Wide_Present_Fails");
    AssertTrue(assertStartsWith != nullptr && assertStartsWithWide != nullptr && assertNotStartsWith != nullptr &&
        assertNotStartsWithWide != nullptr && checkStartsWith != nullptr && checkStartsWithWide != nullptr &&
        checkNonASCII != nullptr && checkNotStartsWith != nullptr && checkNotStartsWithWide != nullptr, __func__,
        __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked. Each
    // "Wide_" test uses the same text as its narrow twin.
    auto const messageMatches = [](TTestCaseRecord const& record, std::string const& prefix, std::string const& detail)
        {
            return record.Message.find(prefix) == 0 && NameEndsWith(record.Message, detail);
        };
    std::string const startsWithDetail = "): Expected \"hello world\" to start with \"xyz\". prefix absent";
    std::string const notStartsWithDetail = "): Expected \"hello world\" not to start with \"hello\". prefix present";
    // "cafe" with an e-acute, a space and U+1F600, then u-umlaut, as UTF-8.
    std::string const nonASCIIDetail =
        "): Expected \"caf\xC3\xA9 \xF0\x9F\x98\x80\" to start with \"\xC3\xBC\". not present";

    CheckTrue(messageMatches(*assertStartsWith, "Prefix not found: Test_AssertStartsWith_Absent_Fails (",
        startsWithDetail), __func__, __LINE__,
        "AssertStartsWith shows the text and prefix: " + assertStartsWith->Message);
    CheckTrue(messageMatches(*assertStartsWithWide, "Prefix not found: Test_AssertStartsWith_Wide_Absent_Fails (",
        startsWithDetail), __func__, __LINE__,
        "AssertStartsWith shows the wide text and prefix: " + assertStartsWithWide->Message);
    CheckTrue(messageMatches(*assertNotStartsWith, "Prefix found: Test_AssertNotStartsWith_Present_Fails (",
        notStartsWithDetail), __func__, __LINE__,
        "AssertNotStartsWith shows the text and prefix: " + assertNotStartsWith->Message);
    CheckTrue(messageMatches(*assertNotStartsWithWide, "Prefix found: Test_AssertNotStartsWith_Wide_Present_Fails (",
        notStartsWithDetail), __func__, __LINE__,
        "AssertNotStartsWith shows the wide text and prefix: " + assertNotStartsWithWide->Message);
    CheckTrue(messageMatches(*checkStartsWith, "Check failed for: \"Test_CheckStartsWith_Absent_Fails\" (",
        startsWithDetail), __func__, __LINE__,
        "CheckStartsWith shows the text and prefix: " + checkStartsWith->Message);
    CheckTrue(messageMatches(*checkStartsWithWide, "Check failed for: \"Test_CheckStartsWith_Wide_Absent_Fails\" (",
        startsWithDetail), __func__, __LINE__,
        "CheckStartsWith shows the wide text and prefix: " + checkStartsWithWide->Message);
    CheckTrue(messageMatches(*checkNotStartsWith, "Check failed for: \"Test_CheckNotStartsWith_Present_Fails\" (",
        notStartsWithDetail), __func__, __LINE__,
        "CheckNotStartsWith shows the text and prefix: " + checkNotStartsWith->Message);
    CheckTrue(messageMatches(*checkNotStartsWithWide,
        "Check failed for: \"Test_CheckNotStartsWith_Wide_Present_Fails\" (", notStartsWithDetail), __func__,
        __LINE__, "CheckNotStartsWith shows the wide text and prefix: " + checkNotStartsWithWide->Message);
    CheckTrue(NameEndsWith(checkNonASCII->Message, nonASCIIDetail), __func__, __LINE__,
        "a wide failure shows its text as UTF-8: " + checkNonASCII->Message);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Throws_ChecksTypeAndMessageAndContinues()
{
    // Arrange
    TFixture_ThrowsChecks fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(14), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    CheckTrue(fixture.ReachedAfterCheckNoThrow, __func__, __LINE__, "a failed CheckNoThrow lets the test continue");
    CheckFalse(fixture.ReachedAfterInnerAssert, __func__, __LINE__,
        "an Assert failure inside the callable still ends the test");

    TTestCaseRecord const* const assertNoThrow = FindRecord(results, "AssertNoThrow_Throws_Fails");
    TTestCaseRecord const* const assertThrows = FindRecord(results, "AssertThrows_NoException_Fails");
    TTestCaseRecord const* const checkNoThrow = FindRecord(results, "CheckNoThrow_Throws_Fails");
    TTestCaseRecord const* const innerAssert = FindRecord(results, "CheckThrows_AssertFailureInside_Fails");
    TTestCaseRecord const* const mismatch = FindRecord(results, "CheckThrows_MessageMismatch_Fails");
    TTestCaseRecord const* const nonStd = FindRecord(results, "CheckThrows_NonStdException_Fails");
    TTestCaseRecord const* const wrongType = FindRecord(results, "CheckThrows_WrongType_Fails");
    AssertTrue(assertNoThrow != nullptr && assertThrows != nullptr && checkNoThrow != nullptr &&
        innerAssert != nullptr && mismatch != nullptr && nonStd != nullptr && wrongType != nullptr, __func__,
        __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    CheckStartsWith(assertNoThrow->Message, "Unexpected exception: Test_AssertNoThrow_Throws_Fails (", __func__,
        __LINE__, "AssertNoThrow names the test");
    CheckEndsWith(assertNoThrow->Message, "): Expected no exception but caught: boom. must not throw", __func__,
        __LINE__, "AssertNoThrow shows what was thrown");
    CheckStartsWith(assertThrows->Message, "Expected exception not caught: Test_AssertThrows_NoException_Fails (",
        __func__, __LINE__, "AssertThrows names the test");
    CheckEndsWith(assertThrows->Message, "): Expected an exception but none was thrown. must throw", __func__,
        __LINE__, "AssertThrows says nothing was thrown");
    CheckEndsWith(checkNoThrow->Message, "): Expected no exception but caught: boom. must not throw", __func__,
        __LINE__, "CheckNoThrow shows what was thrown");
    CheckStartsWith(innerAssert->Message, "Expected true but was false: ", __func__, __LINE__,
        "the inner Assert failure is what's reported");
    CheckEndsWith(mismatch->Message, "): Expected the exception message to contain \"bang\" but caught: boom. "
        "wrong message", __func__, __LINE__, "a message mismatch shows the expected substring and what was caught");
    CheckContains(nonStd->Message, "): Expected a different exception type but caught: ", __func__, __LINE__,
        "a non-std::exception object is a wrong type");
    CheckEndsWith(wrongType->Message, "): Expected a different exception type but caught: boom. wrong type",
        __func__, __LINE__, "a wrong type shows what was caught");
}
//---------------------------------------------------------------------------
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
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass");
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    TTestCaseRecord const* const assertFalse = FindRecord(results, "AssertFalse_True_Fails");
    TTestCaseRecord const* const assertTrue = FindRecord(results, "AssertTrue_False_Fails");
    TTestCaseRecord const* const checkFalse = FindRecord(results, "CheckFalse_True_Fails");
    TTestCaseRecord const* const checkTrue = FindRecord(results, "CheckTrue_False_Fails");
    AssertTrue(assertFalse != nullptr && assertTrue != nullptr && checkFalse != nullptr && checkTrue != nullptr,
        __func__, __LINE__, "every expected record exists");

    // Each message is "<prefix> (<line>): <detail>"; the line varies, so the parts either side of it are checked.
    CheckStartsWith(assertFalse->Message, "Expected false but was true: Test_AssertFalse_True_Fails (", __func__,
        __LINE__, "AssertFalse expects false");
    CheckEndsWith(assertFalse->Message, "): value is true", __func__, __LINE__, "AssertFalse shows the message");
    CheckStartsWith(assertTrue->Message, "Expected true but was false: Test_AssertTrue_False_Fails (", __func__,
        __LINE__, "AssertTrue expects true");
    CheckEndsWith(assertTrue->Message, "): value is false", __func__, __LINE__, "AssertTrue shows the message");
    CheckStartsWith(checkFalse->Message, "Check failed for: \"Test_CheckFalse_True_Fails\" (", __func__, __LINE__,
        "CheckFalse names the test");
    CheckEndsWith(checkFalse->Message, "): Expected false but was true: \"value is true\"", __func__, __LINE__,
        "CheckFalse expects false");
    CheckStartsWith(checkTrue->Message, "Check failed for: \"Test_CheckTrue_False_Fails\" (", __func__, __LINE__,
        "CheckTrue names the test");
    CheckEndsWith(checkTrue->Message, "): Expected true but was false: \"value is false\"", __func__, __LINE__,
        "CheckTrue expects true");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_TestBase)
