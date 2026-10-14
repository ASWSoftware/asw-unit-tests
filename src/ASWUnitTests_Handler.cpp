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
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
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
void TTestHandler::ApplyRegisteredSuitePath(ITestGroup& group, std::string const& registeredPath)
{
    if (registeredPath.empty())
        return;

    TSuitePath const checked(registeredPath); // Throws TExceptInvalidSuitePath for an invalid path.
    std::string const groupPath = group.GetSuitePath();

    if (groupPath == registeredPath)
        return;

    TTestGroupBase* const groupBase = dynamic_cast<TTestGroupBase*>(&group);

    if (!groupPath.empty() || groupBase == nullptr)
        throw TExceptInvalidSuitePath::Conflicting(group.GetTestGroupName(), registeredPath, groupPath);

    groupBase->SetSuitePath(registeredPath);
}
//---------------------------------------------------------------------------
/*
    TTestHandler::CanonicalGroupOrder

    Gives each group a sort key with one entry per level of its path: one for each suite it's in, outermost
    first, then one for the group itself. Each entry is (order, name, kind), where a suite's entry is
    (0, its suite name, 1) and the group's is (its order, its name, 0), so comparing two keys entry by entry
    compares the groups, or the suites holding them, at the first level where their paths differ. A group and a
    suite that tie on order and name at the same level sort the group first. Without suites, each key is a
    single (order, name, 0) entry, which sorts exactly as order then name did before suites existed.
*/
std::vector<size_t> TTestHandler::CanonicalGroupOrder(std::vector<TTestGroupOrderKey> const& groups)
{
    typedef std::tuple<int, std::string, int> TLevelKey;

    std::vector<std::vector<TLevelKey> > sortKeys;

    for (TTestGroupOrderKey const& group : groups)
    {
        TSuitePath const suitePath(group.SuitePath);
        std::vector<TLevelKey> sortKey;

        for (std::string const& suiteName : suitePath.GetNames())
            sortKey.push_back(TLevelKey(0, suiteName, 1));

        sortKey.push_back(TLevelKey(group.Order, group.Name, 0));
        sortKeys.push_back(sortKey);
    }

    std::vector<size_t> order(groups.size());

    for (size_t i = 0; i < order.size(); ++i)
        order[i] = i;

    std::stable_sort(order.begin(), order.end(), [&sortKeys](size_t a, size_t b)
        {
            return sortKeys[a] < sortKeys[b];
        });

    return order;
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
        std::string const groupName = testGroup.GetQualifiedTestGroupName();

        for (ITestGroup::TestCallbackList::iterator testIt = testGroup.GetTestCallbackList().begin();
             testIt != testGroup.GetTestCallbackList().end(); testIt++)
        {
            ITestCase& testCase = *testIt->get();
            tests.push_back(TTestId{ groupName, testCase.GetName(), testGroup.GetSuitePath() });
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
    TTestHandler::ShuffledGroupOrder

    Builds the suite tree from the groups' suite paths: each suite's children are its groups and the suites
    inside it, in the order they first appear in 'suitePaths', with the top level as the root. Then walks the
    tree, visiting each node's children in an order from ShuffledIndices(), so each suite's groups come out
    together.
*/
std::vector<size_t> TTestHandler::ShuffledGroupOrder(std::vector<std::string> const& suitePaths, unsigned int seed)
{
    // A child of a suite (or of the top level): a group, by its index into 'suitePaths', or a suite, by its index
    // into 'suites'.
    struct TChild
    {
        bool IsSuite;
        size_t Index;
    };

    struct TSuite
    {
        std::string Path; // Empty for the top level.
        std::vector<TChild> Children;
    };

    std::vector<TSuite> suites(1); // suites[0] is the top level.
    std::map<std::string, size_t> suiteIndexByPath;

    for (size_t groupIndex = 0; groupIndex < suitePaths.size(); ++groupIndex)
    {
        TSuitePath const suitePath(suitePaths[groupIndex]);
        size_t parent = 0;
        std::string path;

        for (std::string const& suiteName : suitePath.GetNames())
        {
            path += (path.empty() ? std::string() : std::string(".")) + suiteName;

            std::map<std::string, size_t>::const_iterator const found = suiteIndexByPath.find(path);
            if (found != suiteIndexByPath.end())
            {
                parent = found->second;
                continue;
            }

            size_t const suiteIndex = suites.size();
            suites.push_back(TSuite{ path, std::vector<TChild>() });
            suites[parent].Children.push_back(TChild{ true, suiteIndex });
            suiteIndexByPath[path] = suiteIndex;
            parent = suiteIndex;
        }

        suites[parent].Children.push_back(TChild{ false, groupIndex });
    }

    std::vector<size_t> order;
    std::function<void (size_t)> appendShuffled = [&](size_t suiteIndex)
        {
            TSuite const& suite = suites[suiteIndex];
            unsigned int const suiteSeed = suite.Path.empty() ? seed : (seed + FNV1aHash(suite.Path));

            for (size_t childIndex : ShuffledIndices(suite.Children.size(), suiteSeed))
            {
                TChild const& child = suite.Children[childIndex];

                if (child.IsSuite)
                    appendShuffled(child.Index);
                else
                    order.push_back(child.Index);
            }
        };

    appendShuffled(0);
    return order;
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
        // A '*' is checked first, so it's always a wildcard, even where the text has a '*' too.
        if (p < pattern.size() && pattern[p] == '*')
        {
            starIdx = p;
            matchIdx = t;
            ++p;
        }
        else if (p < pattern.size() && (pattern[p] == '?' || pattern[p] == text[t]))
        {
            ++p;
            ++t;
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
        std::string const groupName = testGroup.GetQualifiedTestGroupName();
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
    ASW_REGISTER_TEST_GROUP macros (see ASWUnitTests_Registry.h), and puts
    each in the suite its registration names, if any (see
    ApplyRegisteredSuitePath()). Test modules are added or removed from their
    own .cpp files; this method never needs to change.

    Groups are kept in their canonical order (see CanonicalGroupOrder()):
    by default alphabetical, with each suite's groups together, and
    deterministic across compilers/linkers.
*/
void TTestHandler::RegisterTestGroups()
{
    std::vector<std::unique_ptr<ITestGroup> > groups;
    std::vector<TTestGroupOrderKey> orderKeys;

    for (TRegisteredTestGroupFactory const& registered : TTestGroupRegistry::Factories())
    {
        groups.push_back(registered.Factory());
        ApplyRegisteredSuitePath(*groups.back(), registered.SuitePath);
        orderKeys.push_back(TTestGroupOrderKey{ groups.back()->GetSuitePath(), groups.back()->GetTestGroupName(),
                                                registered.Order });
    }

    for (size_t index : CanonicalGroupOrder(orderKeys))
    {
        groups[index]->SetRunObserver(m_RunObserver);
        m_TestGroups.push_back(std::move(groups[index]));
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

    When 'shuffle' is set, group order is randomized (see
    ShuffledGroupOrder(), which keeps each suite's groups together), and
    each group's test order is independently randomized too (each from a
    seed derived from the master seed and the group's qualified name, so
    results stay reproducible without
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
        std::vector<std::string> suitePaths;

        for (std::unique_ptr<ITestGroup> const& testGroup : m_TestGroups)
            suitePaths.push_back(testGroup->GetSuitePath());

        resolvedSeed = shuffleSeed.has_value() ? shuffleSeed : std::random_device{}();
        groupOrder = ShuffledGroupOrder(suitePaths, *resolvedSeed);
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

        std::string const groupName = testGroup.GetQualifiedTestGroupName();
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
        std::string const name = testGroup.GetQualifiedTestGroupName();
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
