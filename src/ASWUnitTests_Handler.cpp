/* **************************************************************************
ASWUnitTests_Handler.cpp
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
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cctype>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_Utils.h"
#include "ASWUnitTests_Version.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTestHandler
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTestHandler::TTestHandler()
    : m_RunObserver(nullptr)
{
}
//---------------------------------------------------------------------------
TTestHandler::~TTestHandler()
{
}
//---------------------------------------------------------------------------
/*
    TTestHandler::GetAllTestFullNames

    Returns every registered test's "GroupName.TestName" full name, in the same
    canonical (unshuffled) order used by ListTests() and the default run order.

    Requires Initialize() to have already been called.
*/
std::vector<std::string> TTestHandler::GetAllTestFullNames()
{
    std::vector<std::string> fullNames;

    for (TTestId const& test : GetTests())
        fullNames.push_back(test.GroupName + "." + test.TestName);

    return fullNames;
}
//---------------------------------------------------------------------------
std::string const& TTestHandler::GetProjectName() const
{
    return m_ProjectName;
}
//---------------------------------------------------------------------------
/*
    TTestHandler::GetTests

    Returns every registered test's group and test names, in the same canonical
    (unshuffled) order as GetAllTestFullNames(), for a caller that needs them
    separately (e.g. to build a group/test tree) rather than as one full name,
    which can't be split reliably if a group name contains '.'.

    Requires Initialize() to have already been called.
*/
std::vector<TTestId> TTestHandler::GetTests()
{
    std::vector<TTestId> tests;

    for (ITestGroups::iterator it = m_TestGroups.begin(); it != m_TestGroups.end(); it++)
    {
        ITestGroup& testGroup = *it->get();
        std::string const& groupName = testGroup.GetTestGroupName();

        for (ITestGroup::TestCallbackList::iterator testIt = testGroup.GetTestCallbackList().begin();
             testIt != testGroup.GetTestCallbackList().end(); testIt++)
        {
            ITestCase& testCase = *testIt->get();
            tests.push_back(TTestId{ groupName, testCase.GetName() });
        }
    }

    return tests;
}
//---------------------------------------------------------------------------
std::string TTestHandler::GetUTCTimeISO8601()
{
    // Get the current time as a time_point
    auto now = std::chrono::system_clock::now();

    // Convert to time_t for time manipulation
    std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
    auto milliSecs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    // Convert to UTC time structure
    // std::gmtime() is flagged by MSVC in favor of gmtime_s(), which is Windows-only and has a
    // different signature than POSIX's gmtime_r(), so there's no single portable replacement.
#if defined(_MSC_VER)
#  pragma warning(push)
#  pragma warning(disable: 4996)
#endif
    std::tm utc_tm = *std::gmtime(&now_time_t);
#if defined(_MSC_VER)
#  pragma warning(pop)
#endif

    // Format the time into a string
    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%S") << '.'
    << std::setfill('0') << std::setw(3) << milliSecs.count() << "Z";
    return oss.str();
}
//---------------------------------------------------------------------------
std::string TTestHandler::GetVersionStr()
{
    return Version;
}
//---------------------------------------------------------------------------
std::string TTestHandler::GetVersionFullStr()
{
    return "ASWUnitTests - Version " + GetVersionStr();
}
//---------------------------------------------------------------------------
/*
    TTestHandler::WildcardMatch

    Matches 'text' against 'pattern' in full (not a substring search),
    where '*' matches any sequence of characters (including none) and '?'
    matches exactly one character. When 'ignoreCase' is true, matching is
    ASCII case-insensitive. Used to implement --filter/--filter-ignore-case
    in main.cpp.
*/
bool TTestHandler::WildcardMatch(std::string const& pattern, std::string const& text, bool ignoreCase)
{
    if (ignoreCase)
    {
        std::string lowerPattern = pattern;
        std::string lowerText = text;

        for (char& ch : lowerPattern)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        for (char& ch : lowerText)
            ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));

        return WildcardMatch(lowerPattern, lowerText, false);
    }

    size_t p = 0;
    size_t t = 0;
    size_t starIdx = std::string::npos;
    size_t matchIdx = 0;

    while (t < text.size())
    {
        if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t]))
        {
            ++p;
            ++t;
        }
        else if (p < pattern.size() && pattern[p] == '*')
        {
            starIdx = p;
            matchIdx = t;
            ++p;
        }
        else if (starIdx != std::string::npos)
        {
            p = starIdx + 1;
            t = ++matchIdx;
        }
        else
        {
            return false;
        }
    }

    while (p < pattern.size() && pattern[p] == '*')
        ++p;

    return p == pattern.size();
}
//---------------------------------------------------------------------------
void TTestHandler::Initialize(std::string const& projectName)
{
    m_ProjectName = projectName;

    Log(GetVersionFullStr());

    Log("Initializing test handler and registering test groups for " + m_ProjectName + "...");
    RegisterTestGroups();

    Log("Test groups registered: " + std::to_string(m_TestGroups.size()));

    size_t totalTests = 0;

    for (ITestGroups::iterator it = m_TestGroups.begin(); it != m_TestGroups.end(); it++)
    {
        ITestGroup& testGroup = *it->get();
        totalTests += testGroup.GetTestCallbackList().size();
    }

    Log("Total tests registered: " + std::to_string(totalTests));
}
//---------------------------------------------------------------------------
/*
    TTestHandler::ListTests

    Logs every registered test as "GroupName.TestName", one per line,
    without running any of them. Backs main.cpp's --list option. When
    'filter' is set, only matching tests are listed, so a --list --filter
    combination lets the caller verify a pattern before running it.
*/
void TTestHandler::ListTests(TestFilter const& filter, std::string const& filterDescription)
{
    if (filter != nullptr)
        Log("Listing tests, filtered: " + filterDescription);
    else
        Log("Listing tests, unfiltered.");

    size_t totalTests = 0;
    size_t matchedTests = 0;
    size_t totalGroups = m_TestGroups.size();
    size_t matchedGroups = 0;

    for (ITestGroups::iterator it = m_TestGroups.begin(); it != m_TestGroups.end(); it++)
    {
        ITestGroup& testGroup = *it->get();
        std::string const& groupName = testGroup.GetTestGroupName();
        bool groupHasMatch = false;

        for (ITestGroup::TestCallbackList::iterator testIt = testGroup.GetTestCallbackList().begin();
             testIt != testGroup.GetTestCallbackList().end(); testIt++)
        {
            ITestCase& testCase = *testIt->get();
            std::string const fullName = groupName + "." + testCase.GetName();

            ++totalTests;

            if (filter == nullptr || filter(fullName))
            {
                Log(fullName);
                ++matchedTests;
                groupHasMatch = true;
            }
        }

        if (groupHasMatch)
            ++matchedGroups;
    }

    if (filter != nullptr)
    {
        Log("Listed " + std::to_string(matchedTests) + " of " + std::to_string(totalTests) +
            " registered test(s), in " + std::to_string(matchedGroups) + " of " + std::to_string(totalGroups) +
            " registered group(s), matching the filter.");
    }
    else
    {
        Log("Listed " + std::to_string(totalTests) + " test(s) in " + std::to_string(totalGroups) + " group(s).");
    }
}
//---------------------------------------------------------------------------
void TTestHandler::Log(std::string const& msg)
{
    if (m_RunObserver != nullptr)
    {
        m_RunObserver->OnLog(msg + "\n");
        return;
    }

    std::cout << msg << std::endl;
}
//---------------------------------------------------------------------------
void TTestHandler::LogAppend(std::string const& msg)
{
    if (m_RunObserver != nullptr)
    {
        m_RunObserver->OnLog(msg);
        return;
    }

    std::cout << msg;
}
//---------------------------------------------------------------------------
/*
    TTestHandler::RegisterTestGroups

    Instantiates every test group that self-registered via the
    ASW_REGISTER_TEST_GROUP / ASW_REGISTER_TEST_GROUP_ORDERED macro (see
    ASWUnitTests_Registry.h). Test modules are added or removed from their
    own .cpp files; this method never needs to change.

    Groups run in ascending order by their registered order (default 0),
    with ties broken alphabetically by group name, so the default run
    order is alphabetical and deterministic across compilers/linkers.
*/
void TTestHandler::RegisterTestGroups()
{
    struct TOrderedGroup
    {
        int Order;
        std::unique_ptr<ITestGroup> Group;
    };

    std::vector<TOrderedGroup> orderedGroups;

    for (TRegisteredTestGroupFactory const& registered : TTestGroupRegistry::Factories())
        orderedGroups.push_back(TOrderedGroup{ registered.Order, registered.Factory() });

    std::stable_sort(orderedGroups.begin(), orderedGroups.end(),
        [](TOrderedGroup const& a, TOrderedGroup const& b)
        {
            if (a.Order != b.Order)
                return a.Order < b.Order;
            return a.Group->GetTestGroupName() < b.Group->GetTestGroupName();
        });

    for (TOrderedGroup& orderedGroup : orderedGroups)
    {
        orderedGroup.Group->SetRunObserver(m_RunObserver);
        m_TestGroups.push_back(std::move(orderedGroup.Group));
    }
}
//---------------------------------------------------------------------------
/*
    TTestHandler::Run

    Returns the number of tests that failed. When 'filter' is set, only
    tests whose "GroupName.TestName" full name matches it are run; groups
    with no matching test are skipped entirely (no SetUp_Group/TearDown_Group,
    no log entries), and group numbering in the log reflects only the
    groups that actually run. The active filter (or lack of one) is always
    logged, so redirected output still explains why fewer tests ran.

    When 'shuffle' is set, group order is randomized, and each group's test
    order is independently randomized too (each from a seed derived from the
    master seed and the group's name, so results stay reproducible without
    every equally-sized group shuffling identically). The master seed is
    'shuffleSeed' if given, otherwise one is generated and logged so a
    failure caused by shuffled order can be reproduced. A seed gives the
    same order with every compiler and standard library (see
    ShuffledIndices() and FNV1aHash()). Groups, like each group's tests, are
    shuffled before 'filter' is applied, so a filtered run (e.g. rerunning
    just the failures) keeps the relative order the same seed gives an
    unfiltered one.

    'testTimeoutSeconds' and 'catchCrashes' are both passed through to each group's Run(). If a
    group's Run() throws a TExceptAbortRun (TExceptTestTimedOut or TExceptTestCrashed), that group's
    (now-augmented, see TTestGroupBase::ReportTimedOutTest()/ReportCrashedTest()) results are still
    merged in exactly like a normal completion, but no further groups are run, and the returned
    TTestResults has TimedOut or Crashed set accordingly.

    With an ITestRunObserver set (see SetRunObserver()), StopRequested() is checked before each group
    (and, by each group's Run(), before each test). Once it returns true, no further tests or groups
    run, a group that was part-way through is still torn down and merged in, and the returned
    TTestResults has Stopped set.
*/
TTestResults TTestHandler::Run(TestFilter const& filter, std::string const& filterDescription, bool shuffle,
    std::optional<unsigned int> shuffleSeed, std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes)
{
    TTestResults testResults;
    std::vector<ITestGroup*> groupsToRun;

    if (filter != nullptr)
        Log("Filter: " + filterDescription);
    else
        Log("No filter: running all tests.");

    std::optional<unsigned int> resolvedSeed;
    std::vector<size_t> groupOrder;

    if (shuffle)
    {
        resolvedSeed = shuffleSeed.has_value() ? shuffleSeed : std::random_device{}();
        groupOrder = ShuffledIndices(m_TestGroups.size(), *resolvedSeed);
    }
    else
    {
        groupOrder.resize(m_TestGroups.size());
        for (size_t i = 0; i < groupOrder.size(); ++i)
            groupOrder[i] = i;
    }

    for (size_t groupIndex : groupOrder)
    {
        ITestGroup& testGroup = *m_TestGroups[groupIndex];

        if (filter == nullptr)
        {
            groupsToRun.push_back(&testGroup);
            continue;
        }

        std::string const& groupName = testGroup.GetTestGroupName();
        bool anyMatch = false;

        for (ITestGroup::TestCallbackList::iterator testIt = testGroup.GetTestCallbackList().begin();
             testIt != testGroup.GetTestCallbackList().end(); testIt++)
        {
            ITestCase& testCase = *testIt->get();

            if (filter(groupName + "." + testCase.GetName()))
            {
                anyMatch = true;
                break;
            }
        }

        if (anyMatch)
            groupsToRun.push_back(&testGroup);
    }

    if (filter != nullptr && groupsToRun.empty())
        Log("No registered tests matched the filter.");

    if (resolvedSeed.has_value())
        Log("Shuffle: enabled (seed " + std::to_string(*resolvedSeed) + ")");
    else
        Log("Shuffle: disabled");

    size_t groupNum = 0;
    size_t nGroups = groupsToRun.size();
    std::chrono::steady_clock::time_point const start = std::chrono::steady_clock::now();

    Log("\n[" + GetUTCTimeISO8601() + "] Tests started.\n");

    for (std::vector<ITestGroup*>::iterator it = groupsToRun.begin(); it != groupsToRun.end(); it++)
    {
        ITestGroup& testGroup = **it;
        std::string const& name = testGroup.GetTestGroupName();
        size_t nTestsInGroup = 0;

        for (ITestGroup::TestCallbackList::iterator testIt = testGroup.GetTestCallbackList().begin();
             testIt != testGroup.GetTestCallbackList().end(); testIt++)
        {
            ITestCase& testCase = *testIt->get();

            if (filter == nullptr || filter(name + "." + testCase.GetName()))
                ++nTestsInGroup;
        }

        if (m_RunObserver != nullptr && m_RunObserver->StopRequested())
        {
            testResults.Stopped = true;
            break;
        }

        // set up
        Log("--------------------------------------------------------------------------------");
        Log("[" + GetUTCTimeISO8601() + "] Setting up group " + std::to_string(++groupNum) + " of " +
            std::to_string(nGroups) + ": \"" + name + "\"");
        testGroup.SetUp_Group();

        // run
        Log("Running " + std::to_string(nTestsInGroup) + " test" + std::string((nTestsInGroup == 1) ? "." : "s."));

        std::optional<unsigned int> groupSeed;
        if (resolvedSeed.has_value())
            groupSeed = *resolvedSeed + FNV1aHash(name);

        bool groupTimedOut = false;
        bool groupCrashed = false;
        try
        {
            testGroup.Run(filter, groupSeed, testTimeoutSeconds, catchCrashes);
        }
        catch (TExceptTestTimedOut const&)
        {
            groupTimedOut = true;
        }
        catch (TExceptTestCrashed const&)
        {
            groupCrashed = true;
        }

        TTestResults const& testGroupResults = testGroup.Results();
        Log("Done. " +
            TConsole::Colorize("Succeeded: " + std::to_string(testGroupResults.SuccessCount), TLogKind::Pass) +
            ", " + TConsole::Colorize("failed: " + std::to_string(testGroupResults.FailedCount), TLogKind::Fail) +
            ", " + TConsole::Colorize("skipped: " + std::to_string(testGroupResults.SkippedCount), TLogKind::Skip));

        testResults.FailedCount += testGroupResults.FailedCount;
        testResults.SkippedCount += testGroupResults.SkippedCount;
        testResults.SuccessCount += testGroupResults.SuccessCount;
        testResults.AddMessages(testGroupResults.Messages);
        testResults.CaseRecords.insert(testResults.CaseRecords.end(), testGroupResults.CaseRecords.begin(),
            testGroupResults.CaseRecords.end());

        // tear down
        Log("[" + GetUTCTimeISO8601() + "] Tearing down group: \"" + name + "\"");
        testGroup.TearDown_Group();

        if (groupTimedOut || groupCrashed)
        {
            testResults.TimedOut = groupTimedOut;
            testResults.Crashed = groupCrashed;
            break;
        }

        if (testGroupResults.Stopped)
        {
            testResults.Stopped = true;
            break;
        }
    }

    std::chrono::steady_clock::time_point const end = std::chrono::steady_clock::now();

    Log("--------------------------------------------------------------------------------");

    if (testResults.Stopped)
        Log("Run stopped on request before every test ran.");

    Log("\n[" + GetUTCTimeISO8601() + "] Tests done: Totals: " +
        TConsole::Colorize("succeeded: " + std::to_string(testResults.SuccessCount), TLogKind::Pass) + ", " +
        TConsole::Colorize("failed: " + std::to_string(testResults.FailedCount), TLogKind::Fail) + ", " +
        TConsole::Colorize("skipped: " + std::to_string(testResults.SkippedCount), TLogKind::Skip));

    // Get elapsed time
    std::chrono::milliseconds const elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    // Convert to hours, minutes, seconds, and milliseconds
    long long totalMilliseconds = elapsed.count();
    long long const hours = totalMilliseconds / (1000 * 60 * 60);
    totalMilliseconds %= (1000 * 60 * 60);
    long long const mins = totalMilliseconds / (1000 * 60);
    totalMilliseconds %= (1000 * 60);
    long long const secs = totalMilliseconds / 1000;
    long long const milliseconds = totalMilliseconds % 1000;

    // Display the result
    std::stringstream ss;
    ss << "Elapsed time: " << std::setfill('0') << std::setw(2) << hours << "h:" << std::setfill('0')
    << std::setw(2) << mins << "m:" << std::setfill('0') << std::setw(2) << secs << "s."
    << std::setfill('0') << std::setw(3) << milliseconds << "ms";
    Log(ss.str());

    return testResults;
}
//---------------------------------------------------------------------------
void TTestHandler::SetRunObserver(ITestRunObserver* observer)
{
    m_RunObserver = observer;

    for (ITestGroups::iterator it = m_TestGroups.begin(); it != m_TestGroups.end(); it++)
        (*it)->SetRunObserver(observer);
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
