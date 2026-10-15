/* **************************************************************************
Test_ASWUnitTests_Handler.cpp
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
#include "Test_ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Console.h"
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_StdOutRedirect.h"
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

std::string JoinIndices(std::vector<size_t> const& indices);
TestFilter WildcardMatchTestsFilter(bool alsoStdOutRedirectTests);

//---------------------------------------------------------------------------

// 'indices' joined with ",", so a whole order can be compared, and shown on failure, as one string.
std::string JoinIndices(std::vector<size_t> const& indices)
{
    std::string joined;

    for (size_t i = 0; i < indices.size(); ++i)
        joined += (i == 0 ? std::string() : std::string(",")) + std::to_string(indices[i]);

    return joined;
}

//---------------------------------------------------------------------------

// Matches only this group's own WildcardMatch_* tests (pure static-function checks, so safe to run from a
// nested TTestHandler inside this group, and never including the test doing the nesting), plus, when asked,
// every ASWUnitTests_StdOutRedirect_Tests test, a second group that runs after this one.
TestFilter WildcardMatchTestsFilter(bool alsoStdOutRedirectTests)
{
    return [alsoStdOutRedirectTests](std::string const& fullName)
        {
            return fullName.rfind("ASWUnitTests_Handler_Tests.WildcardMatch_", 0) == 0 ||
                (alsoStdOutRedirectTests && fullName.rfind("ASWUnitTests_StdOutRedirect_Tests.", 0) == 0);
        };
}

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_GroupConstructorThrows
//
// A group registered in suite "HandlerFixtures", so a nested TTestHandler creates it, whose constructor throws what
// ConstructorThrows says, for Test_Initialize_ReplacesGroupWhoseConstructorThrows below. Otherwise (as in every
// ordinary run) it's created as usual and its one test passes.
/////////////////////////////////////////////////////////////////////////////
class TFixture_GroupConstructorThrows : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_Passes();

public:
    enum class TThrow
    {
        Nothing,
        StdException,     // A std::runtime_error.
        RTLException,     // An EConvertError, in RTL builds.
        DuplicateTestName // Registers its test twice, so RegisterTest() throws TExceptDuplicateTestName.
    };

    static TThrow ConstructorThrows;

public:
    TFixture_GroupConstructorThrows();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

TFixture_GroupConstructorThrows::TThrow TFixture_GroupConstructorThrows::ConstructorThrows = TThrow::Nothing;

//---------------------------------------------------------------------------
TFixture_GroupConstructorThrows::TFixture_GroupConstructorThrows()
    : inherited("ASWUnitTests_Handler_ConstructorFixture")
{
    if (ConstructorThrows == TThrow::StdException)
        throw std::runtime_error("deliberate constructor failure");

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    if (ConstructorThrows == TThrow::RTLException)
        throw System::Sysutils::EConvertError("deliberate constructor failure");
#endif

    RegisterTest(&TFixture_GroupConstructorThrows::Test_Passes, "Passes");

    if (ConstructorThrows == TThrow::DuplicateTestName)
        RegisterTest(&TFixture_GroupConstructorThrows::Test_Passes, "Passes");
}
//---------------------------------------------------------------------------
void TFixture_GroupConstructorThrows::Test_Passes()
{
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_GroupSetUpThrows
//
// A registered group, so a nested TTestHandler creates and runs it, whose SetUp_Group() throws what SetUpGroupThrows
// says, for Test_Run_FailsGroupTestsWhenSetUpGroupThrows below. Otherwise (as in every ordinary run) its set-up does
// nothing and its two tests pass.
/////////////////////////////////////////////////////////////////////////////
class TFixture_GroupSetUpThrows : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_First();
    void Test_Second();

public:
    enum class TThrow
    {
        Nothing,
        StdException, // A std::runtime_error.
        RTLException  // An EConvertError, in RTL builds.
    };

    static TThrow SetUpGroupThrows;
    // Counts set by the latest instance, for the test to check.
    static size_t TearDownGroupCalls;
    static size_t TestsRun;

public:
    TFixture_GroupSetUpThrows();

    void SetUp_Group() override;
    void TearDown_Group() override;
};

TFixture_GroupSetUpThrows::TThrow TFixture_GroupSetUpThrows::SetUpGroupThrows = TThrow::Nothing;
size_t TFixture_GroupSetUpThrows::TearDownGroupCalls = 0;
size_t TFixture_GroupSetUpThrows::TestsRun = 0;

//---------------------------------------------------------------------------
TFixture_GroupSetUpThrows::TFixture_GroupSetUpThrows()
    : inherited("ASWUnitTests_Handler_GroupSetUpFixture")
{
    RegisterTest(&TFixture_GroupSetUpThrows::Test_First, "First");
    RegisterTest(&TFixture_GroupSetUpThrows::Test_Second, "Second");
}
//---------------------------------------------------------------------------
void TFixture_GroupSetUpThrows::SetUp_Group()
{
    if (SetUpGroupThrows == TThrow::StdException)
        throw std::runtime_error("deliberate SetUp_Group() failure");

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
    if (SetUpGroupThrows == TThrow::RTLException)
        throw System::Sysutils::EConvertError("deliberate SetUp_Group() failure");
#endif
}
//---------------------------------------------------------------------------
void TFixture_GroupSetUpThrows::TearDown_Group()
{
    ++TearDownGroupCalls;
}
//---------------------------------------------------------------------------
void TFixture_GroupSetUpThrows::Test_First()
{
    ++TestsRun;
}
//---------------------------------------------------------------------------
void TFixture_GroupSetUpThrows::Test_Second()
{
    ++TestsRun;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TRecordingObserver
//
// An ITestRunObserver that records what it's told, for the SetRunObserver_* tests below. No test here
// uses a timeout, so every call arrives on the calling thread and no locking is needed.
/////////////////////////////////////////////////////////////////////////////
class TRecordingObserver : public ITestRunObserver
{
public:
    std::vector<std::string> FinishedGroupNames;
    std::string LogText;
    size_t StopAfterFinishedCount = static_cast<size_t>(-1);

public:
    void OnLog(std::string const& text) override
    {
        LogText += text;
    }

    void OnTestFinished(TTestCaseRecord const& record) override
    {
        FinishedGroupNames.push_back(record.GroupName);
    }

    void OnTestStarted(std::string const& /*groupName*/, std::string const& /*testName*/) override
    {
    }

    bool StopRequested() override
    {
        return FinishedGroupNames.size() >= StopAfterFinishedCount;
    }
};

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Handler
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Handler::TTest_ASWUnitTests_Handler()
    : inherited("ASWUnitTests_Handler_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_KeepsSuitesTogether,
        "CanonicalGroupOrder_KeepsSuitesTogether");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_RejectsInvalidSuitePath,
        "CanonicalGroupOrder_RejectsInvalidSuitePath");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_WithoutSuites_SortsByOrderThenName,
        "CanonicalGroupOrder_WithoutSuites_SortsByOrderThenName");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_GetTests_MatchesGetAllTestFullNames,
        "GetTests_MatchesGetAllTestFullNames");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_Initialize_ReplacesGroupWhoseConstructorThrows,
        "Initialize_ReplacesGroupWhoseConstructorThrows");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_Run_FailsGroupTestsWhenSetUpGroupThrows,
        "Run_FailsGroupTestsWhenSetUpGroupThrows");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_Run_ShuffleKeepsOrderWhenFiltered,
        "Run_ShuffleKeepsOrderWhenFiltered");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesInitializeAndRunOutput,
        "SetRunObserver_ReceivesInitializeAndRunOutput");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesLogWithoutColor,
        "SetRunObserver_ReceivesLogWithoutColor");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_SetRunObserver_StopsBetweenGroups, "SetRunObserver_StopsBetweenGroups");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_ShuffledGroupOrder_KeepsSuitesTogether,
        "ShuffledGroupOrder_KeepsSuitesTogether");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_ShuffledGroupOrder_WithoutSuites_MatchesShuffledIndices,
        "ShuffledGroupOrder_WithoutSuites_MatchesShuffledIndices");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_CaseSensitivity, "WildcardMatch_CaseSensitivity");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_ExactAndStar, "WildcardMatch_ExactAndStar");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_QuestionMark, "WildcardMatch_QuestionMark");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Handler::~TTest_ASWUnitTests_Handler()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_KeepsSuitesTogether()
{
    // Arrange: groups at the top level, in suite "S", and in suite "S.T", plus a group named "T" in "S" beside
    // suite "S.T", given out of order.
    std::vector<TTestGroupOrderKey> const groups = {
        TTestGroupOrderKey{ "", "Zed", 0 },     // 0
        TTestGroupOrderKey{ "S", "G2", 0 },     // 1
        TTestGroupOrderKey{ "", "Alpha", 0 },   // 2
        TTestGroupOrderKey{ "S.T", "G3", 0 },   // 3
        TTestGroupOrderKey{ "S", "G1", 0 },     // 4
        TTestGroupOrderKey{ "", "Early", -1 },  // 5
        TTestGroupOrderKey{ "S", "Late", 1 },   // 6
        TTestGroupOrderKey{ "S", "T", 0 },      // 7
        TTestGroupOrderKey{ "S", "First", -5 }  // 8
    };

    // Act
    std::vector<size_t> const order = TTestHandler::CanonicalGroupOrder(groups);

    // Assert
    CheckEquals(std::string("5,2,8,4,1,7,3,6,0"), JoinIndices(order), __func__, __LINE__,
        "Early (order -1) first, then Alpha, then all of suite S before Zed; within S, First (order -5), G1, G2, "
        "group T before suite T's G3, then Late (order 1). An order only sorts a group among its siblings, so "
        "First doesn't move S ahead of Alpha");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_RejectsInvalidSuitePath()
{
    CheckThrows<TExceptInvalidSuitePath>([]()
        {
            return TTestHandler::CanonicalGroupOrder({ TTestGroupOrderKey{ "S..T", "G", 0 } });
        }, __func__, __LINE__, "a suite path with an empty suite name, from an ITestGroup's GetSuitePath()",
        "Suite path \"S..T\" has an empty suite name");
    CheckThrows<TExceptInvalidSuitePath>([]()
        {
            return TTestHandler::ShuffledGroupOrder({ "", "S." }, 1);
        }, __func__, __LINE__, "ShuffledGroupOrder() checks suite paths the same way",
        "Suite path \"S.\" has an empty suite name");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_CanonicalGroupOrder_WithoutSuites_SortsByOrderThenName()
{
    // Arrange
    std::vector<TTestGroupOrderKey> const groups = {
        TTestGroupOrderKey{ "", "Bravo", 0 },    // 0
        TTestGroupOrderKey{ "", "Alpha", 0 },    // 1
        TTestGroupOrderKey{ "", "Charlie", -1 }, // 2
        TTestGroupOrderKey{ "", "Alpha", 0 },    // 3
        TTestGroupOrderKey{ "", "Delta", 1 }     // 4
    };

    // Act
    std::vector<size_t> const order = TTestHandler::CanonicalGroupOrder(groups);

    // Assert
    CheckEquals(std::string("2,1,3,0,4"), JoinIndices(order), __func__, __LINE__,
        "by order, then name, with tied groups kept in registration order, as before suites existed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_GetTests_MatchesGetAllTestFullNames()
{
    // Arrange
    TTestHandler handler;
    {
        TStdOutRedirect const suppressOutput; // Initialize() logs its own version/registration banner.
        handler.Initialize("Test_GetTests_MatchesGetAllTestFullNames");
    }

    // Act
    std::vector<TTestId> const tests = handler.GetTests();
    std::vector<std::string> const fullNames = handler.GetAllTestFullNames();

    // Assert
    AssertEquals(fullNames.size(), tests.size(), __func__, __LINE__, "one entry per registered test in each");
    CheckNotEmpty(tests, __func__, __LINE__, "the real, self-registered suite is listed");

    bool foundThisTest = false;

    for (size_t i = 0; i < tests.size(); ++i)
    {
        CheckEquals(fullNames[i], tests[i].GroupName + "." + tests[i].TestName, __func__, __LINE__,
            "same tests, in the same canonical order, just with group and test names kept separate");

        if (tests[i].GroupName == "ASWUnitTests_Handler_Tests" &&
            tests[i].TestName == "GetTests_MatchesGetAllTestFullNames")
            foundThisTest = true;
    }

    CheckTrue(foundThisTest, __func__, __LINE__, "this very test is listed, under its own group");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_Initialize_ReplacesGroupWhoseConstructorThrows()
{
    typedef TFixture_GroupConstructorThrows::TThrow TThrow;

    // Cleared again however each scenario ends, so the group's own run in this suite isn't affected.
    struct TResetThrow
    {
        ~TResetThrow()
        {
            TFixture_GroupConstructorThrows::ConstructorThrows = TThrow::Nothing;
        }
    } const resetThrow;

    // What the constructor throws, and how the failure describes it.
    std::vector<std::pair<TThrow, std::string> > const scenarios =
    {
        { TThrow::StdException, "deliberate constructor failure" },
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        {
            TThrow::RTLException, "EConvertError: deliberate constructor failure"
        },
#endif
    };

    for (std::pair<TThrow, std::string> const& scenario : scenarios)
    {
        std::string const& description = scenario.second;
        std::string const detail = "Test group class \"TFixture_GroupConstructorThrows\" could not be created: " +
            description;

        // Act
        TTestHandler handler;
        std::string log;
        TTestResults results;
        {
            TFixture_GroupConstructorThrows::ConstructorThrows = scenario.first;
            TStdOutRedirect const captureOutput;
            handler.Initialize("Test_Initialize_ReplacesGroupWhoseConstructorThrows");
            log = captureOutput.Str();
            TFixture_GroupConstructorThrows::ConstructorThrows = TThrow::Nothing;
            results = handler.Run([](std::string const& fullName)
                {
                    return fullName.rfind("HandlerFixtures.", 0) == 0;
                }, "the HandlerFixtures suite");
        }

        // Assert
        CheckContains(log, "!!ERROR!!: " + detail, __func__, __LINE__, description + ": the failure is logged");

        bool foundStandIn = false;
        for (TTestId const& test : handler.GetTests())
        {
            CheckNotEquals(std::string("HandlerFixtures.ASWUnitTests_Handler_ConstructorFixture"), test.GroupName,
                __func__, __LINE__, description + ": the group itself wasn't created");

            if (test.GroupName == "HandlerFixtures.TFixture_GroupConstructorThrows")
            {
                foundStandIn = true;
                CheckEquals(std::string("(construction)"), test.TestName, __func__, __LINE__,
                    description + ": the stand-in's one test");
                CheckEquals(std::string("HandlerFixtures"), test.SuitePath, __func__, __LINE__,
                    description + ": the stand-in is in the registration's suite");
            }
        }

        CheckTrue(foundStandIn, __func__, __LINE__, description + ": a group named after the class stands in for it");
        CheckEquals(1u, results.FailedCount, __func__, __LINE__, description + ": the stand-in's test fails");
        CheckEquals(0u, results.SuccessCount, __func__, __LINE__, description + ": and nothing else in the suite ran");
        AssertEquals(static_cast<size_t>(1), results.CaseRecords.size(), __func__, __LINE__,
            description + ": one record");
        CheckEquals(std::string("(construction)"), results.CaseRecords.front().TestName, __func__, __LINE__,
            description + ": for the stand-in's test");
        CheckEquals(detail, results.CaseRecords.front().Message, __func__, __LINE__,
            description + ": saying why the group couldn't be created");
    }

    // A mistake in how a group registers its tests still stops the run at startup.
    TTestHandler handler;
    bool duplicateThrown = false;
    {
        TFixture_GroupConstructorThrows::ConstructorThrows = TThrow::DuplicateTestName;
        TStdOutRedirect const suppressOutput;

        try
        {
            handler.Initialize("Test_Initialize_ReplacesGroupWhoseConstructorThrows");
        }
        catch (TExceptDuplicateTestName const&)
        {
            duplicateThrown = true;
        }

        TFixture_GroupConstructorThrows::ConstructorThrows = TThrow::Nothing;
    }

    CheckTrue(duplicateThrown, __func__, __LINE__, "a TExceptDuplicateTestName from a constructor isn't replaced");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_Run_FailsGroupTestsWhenSetUpGroupThrows()
{
    // Arrange: the fixture group's first test only, so its other test checks the filter still applies, then a
    // group of pure-function tests that must still run after it.
    TestFilter const filter = [](std::string const& fullName)
        {
            return fullName == "ASWUnitTests_Handler_GroupSetUpFixture.First" ||
                fullName.rfind("ASWUnitTests_Version_Tests.", 0) == 0;
        };
    TTestHandler handler;
    {
        TStdOutRedirect const suppressOutput;
        handler.Initialize("Test_Run_FailsGroupTestsWhenSetUpGroupThrows");
    }

    // What SetUp_Group() throws, and how the failure describes it.
    std::vector<std::pair<TFixture_GroupSetUpThrows::TThrow, std::string> > const scenarios =
    {
        { TFixture_GroupSetUpThrows::TThrow::StdException, "deliberate SetUp_Group() failure" },
#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
        {
            TFixture_GroupSetUpThrows::TThrow::RTLException, "EConvertError: deliberate SetUp_Group() failure"
        },
#endif
    };

    for (std::pair<TFixture_GroupSetUpThrows::TThrow, std::string> const& scenario : scenarios)
    {
        std::string const& description = scenario.second;
        TFixture_GroupSetUpThrows::TearDownGroupCalls = 0;
        TFixture_GroupSetUpThrows::TestsRun = 0;

        // Act
        TTestResults results;
        std::string log;
        {
            // Cleared again however Run() ends, so the group's own run in this suite isn't affected.
            struct TResetThrow
            {
                ~TResetThrow()
                {
                    TFixture_GroupSetUpThrows::SetUpGroupThrows = TFixture_GroupSetUpThrows::TThrow::Nothing;
                }
            } const resetThrow;

            TFixture_GroupSetUpThrows::SetUpGroupThrows = scenario.first;
            TStdOutRedirect const captureOutput;
            results = handler.Run(filter, "the fixture group and the version tests");
            log = captureOutput.Str();
        }

        // Assert
        CheckEquals(static_cast<size_t>(0), TFixture_GroupSetUpThrows::TestsRun, __func__, __LINE__,
            description + ": none of the group's tests ran");
        CheckEquals(static_cast<size_t>(1), TFixture_GroupSetUpThrows::TearDownGroupCalls, __func__, __LINE__,
            description + ": TearDown_Group() still ran");
        CheckEquals(1u, results.FailedCount, __func__, __LINE__,
            description + ": one failure, for the one test the filter matches");
        CheckGreaterThan(results.SuccessCount, 0u, __func__, __LINE__,
            description + ": the next group's tests still ran and passed");

        size_t fixtureRecords = 0;
        for (TTestCaseRecord const& record : results.CaseRecords)
        {
            if (record.GroupName != "ASWUnitTests_Handler_GroupSetUpFixture")
                continue;

            ++fixtureRecords;
            CheckEquals(std::string("First"), record.TestName, __func__, __LINE__,
                description + ": the test the filter matches");
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, description + ": recorded as failed");
            CheckEquals("SetUp_Group() exception: " + description, record.Message, __func__, __LINE__,
                description + ": its detail names SetUp_Group() and the exception");
        }

        CheckEquals(static_cast<size_t>(1), fixtureRecords, __func__, __LINE__,
            description + ": one record for the group");
        CheckContains(log, "!!ERROR!!: SetUp_Group() exception for group \"ASWUnitTests_Handler_GroupSetUpFixture\": " +
            description, __func__, __LINE__, description + ": the exception is logged");
        CheckContains(log, "\"ASWUnitTests_Handler_GroupSetUpFixture.First\": SetUp_Group() exception: ", __func__,
            __LINE__, description + ": and so is the test's failure");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_Run_ShuffleKeepsOrderWhenFiltered()
{
    // Arrange: tests from three groups that only call pure functions, so they're safe to run from a nested
    // TTestHandler. The subset drops a whole group and one test from another, as rerunning failures would.
    TestFilter const threeGroups = [](std::string const& fullName)
        {
            return fullName.rfind("ASWUnitTests_Handler_Tests.WildcardMatch_", 0) == 0 ||
                fullName.rfind("ASWUnitTests_Utils_Tests.", 0) == 0 ||
                fullName.rfind("ASWUnitTests_Version_Tests.", 0) == 0;
        };
    TestFilter const subset = [&threeGroups](std::string const& fullName)
        {
            return threeGroups(fullName) && fullName.rfind("ASWUnitTests_Version_Tests.", 0) != 0 &&
                fullName != "ASWUnitTests_Handler_Tests.WildcardMatch_ExactAndStar";
        };
    TTestHandler handler;
    {
        TStdOutRedirect const suppressOutput;
        handler.Initialize("Test_Run_ShuffleKeepsOrderWhenFiltered");
    }

    // Several seeds, so the subset matching the full run's order by chance can't hide a regression.
    for (unsigned int seed = 1; seed <= 8; ++seed)
    {
        // Act
        TTestResults fullRun;
        TTestResults subsetRun;
        {
            TStdOutRedirect const suppressOutput;
            fullRun = handler.Run(threeGroups, "three groups", true, seed);
            subsetRun = handler.Run(subset, "subset", true, seed);
        }

        // Assert
        std::vector<std::string> expected;
        std::vector<std::string> actual;

        for (TTestCaseRecord const& record : fullRun.CaseRecords)
        {
            std::string const fullName = record.GroupName + "." + record.TestName;

            if (subset(fullName))
                expected.push_back(fullName);
        }

        for (TTestCaseRecord const& record : subsetRun.CaseRecords)
            actual.push_back(record.GroupName + "." + record.TestName);

        CheckNotEmpty(expected, __func__, __LINE__, "the subset has tests to compare (seed " +
            std::to_string(seed) + ")");
        CheckTrue(expected == actual, __func__, __LINE__, "the subset runs in the full run's order, groups and "
            "tests alike (seed " + std::to_string(seed) + ")");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesInitializeAndRunOutput()
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer); // Before Initialize(), so its output and the groups it creates are covered.
    std::string consoleOutput;
    TTestResults results;

    // Act
    {
        TStdOutRedirect redirect;
        handler.Initialize("ObserverTestProject");
        results = handler.Run(WildcardMatchTestsFilter(false), "WildcardMatch_* tests only");
        consoleOutput = redirect.Str();
    }

    // Assert
    CheckEquals(3u, results.SuccessCount, __func__, __LINE__, "the three WildcardMatch_* tests ran and passed");
    CheckEquals(static_cast<size_t>(3), observer.FinishedGroupNames.size(), __func__, __LINE__,
        "each was reported to the observer, through the groups Initialize() created after it was set");
    CheckContains(observer.LogText, "registering test groups for ObserverTestProject", __func__, __LINE__,
        "Initialize()'s output went to the observer");
    CheckContains(observer.LogText, "Tests done", __func__, __LINE__, "and so did Run()'s own summary");
    CheckEmpty(consoleOutput, __func__, __LINE__, "none of it went to std::cout");
    CheckFalse(results.Stopped, __func__, __LINE__, "a run the observer never asked to stop isn't marked stopped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesLogWithoutColor()
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer);
    TColorMode const savedMode = TConsole::GetColorMode();
    TConsole::SetColorMode(TColorMode::Always); // So the console would get color codes.

    // Act
    handler.Initialize("ObserverTestProject");
    handler.Run(WildcardMatchTestsFilter(false), "WildcardMatch_* tests only");
    TConsole::SetColorMode(savedMode);

    // Assert
    CheckContains(observer.LogText, "Done. Succeeded: 3", __func__, __LINE__, "the group's colored totals are logged");
    CheckContains(observer.LogText, "Totals: succeeded: 3", __func__, __LINE__, "and so are the run's");
    CheckTrue(observer.LogText.find('\x1b') == std::string::npos, __func__, __LINE__, "with no color codes");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_SetRunObserver_StopsBetweenGroups()
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    observer.StopAfterFinishedCount = 3; // Exactly this group's WildcardMatch_* tests, then stop.
    handler.SetRunObserver(&observer);
    TTestResults results;

    // Act
    handler.Initialize("ObserverTestProject");
    results = handler.Run(WildcardMatchTestsFilter(true), "WildcardMatch_* and StdOutRedirect tests");

    // Assert
    CheckTrue(results.Stopped, __func__, __LINE__, "the results say the run was stopped");
    CheckEquals(static_cast<size_t>(3), results.CaseRecords.size(), __func__, __LINE__,
        "only the first group's tests ran");
    CheckNotContains(observer.LogText, "\"ASWUnitTests_StdOutRedirect_Tests\"", __func__, __LINE__,
        "the second group was never even set up");
    CheckContains(observer.LogText, "Run stopped on request", __func__, __LINE__, "the log says the run was stopped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_ShuffledGroupOrder_KeepsSuitesTogether()
{
    // Arrange: groups in canonical order, with suite "A" (holding suite "A.B") and suite "C" among top-level ones.
    std::vector<std::string> const suitePaths = { "", "A", "A", "A.B", "A.B", "A", "", "C", "C" };
    std::vector<std::string> const suites = { "A", "A.B", "C" };
    std::vector<std::string> const oneSuite(5, "A");
    bool anyReordered = false;

    for (unsigned int seed = 1; seed <= 20; ++seed)
    {
        // Act
        std::vector<size_t> const order = TTestHandler::ShuffledGroupOrder(suitePaths, seed);

        // Assert
        std::vector<size_t> sorted = order;
        std::sort(sorted.begin(), sorted.end());
        CheckEquals(std::string("0,1,2,3,4,5,6,7,8"), JoinIndices(sorted), __func__, __LINE__,
            "every group appears exactly once (seed " + std::to_string(seed) + ")");
        CheckEquals(JoinIndices(order), JoinIndices(TTestHandler::ShuffledGroupOrder(suitePaths, seed)), __func__,
            __LINE__, "the same seed gives the same order (seed " + std::to_string(seed) + ")");

        for (std::string const& suite : suites)
        {
            size_t first = order.size();
            size_t last = 0;
            size_t count = 0;

            for (size_t position = 0; position < order.size(); ++position)
            {
                std::string const& path = suitePaths[order[position]];

                if (path == suite || path.rfind(suite + ".", 0) == 0)
                {
                    first = std::min(first, position);
                    last = std::max(last, position);
                    ++count;
                }
            }

            CheckEquals(count, last - first + 1, __func__, __LINE__, "suite " + suite + "'s groups run together (seed " +
                std::to_string(seed) + ")");
        }

        if (JoinIndices(order) != "0,1,2,3,4,5,6,7,8")
            anyReordered = true;

        CheckEquals(JoinIndices(ShuffledIndices(oneSuite.size(), seed + FNV1aHash("A"))),
            JoinIndices(TTestHandler::ShuffledGroupOrder(oneSuite, seed)), __func__, __LINE__,
            "a suite's groups are shuffled with the seed plus the FNV-1a hash of its path (seed " +
            std::to_string(seed) + ")");
    }

    CheckTrue(anyReordered, __func__, __LINE__, "some seed actually shuffles the groups");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_ShuffledGroupOrder_WithoutSuites_MatchesShuffledIndices()
{
    std::vector<std::string> const suitePaths(7, std::string());

    for (unsigned int seed = 1; seed <= 8; ++seed)
    {
        CheckEquals(JoinIndices(ShuffledIndices(suitePaths.size(), seed)),
            JoinIndices(TTestHandler::ShuffledGroupOrder(suitePaths, seed)), __func__, __LINE__,
            "without suites, a seed gives the same group order as before suites existed (seed " +
            std::to_string(seed) + ")");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_CaseSensitivity()
{
    // Act & Assert
    CheckFalse(TTestHandler::WildcardMatch("ABC", "abc"), __func__, __LINE__, "case-sensitive by default");
    CheckTrue(TTestHandler::WildcardMatch("ABC", "abc", true), __func__, __LINE__, "case-insensitive when requested");
    CheckTrue(TTestHandler::WildcardMatch("*STRING*", "Test_ASWTools_String.cpp", true),
        __func__, __LINE__, "case-insensitive substring search");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_ExactAndStar()
{
    // Act & Assert
    CheckTrue(TTestHandler::WildcardMatch("abc", "abc"), __func__, __LINE__, "exact match");
    CheckFalse(TTestHandler::WildcardMatch("abc", "abd"), __func__, __LINE__, "exact mismatch");
    CheckTrue(TTestHandler::WildcardMatch("*", "anything"), __func__, __LINE__, "bare star matches everything");
    CheckTrue(TTestHandler::WildcardMatch("*", ""), __func__, __LINE__, "bare star matches empty text");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "abc"), __func__, __LINE__, "star matches one char");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "abbbbc"), __func__, __LINE__, "star matches many chars");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "ac"), __func__, __LINE__, "star matches zero chars");
    CheckTrue(TTestHandler::WildcardMatch("*.cpp", "Test_ASWTools_String.cpp"), __func__, __LINE__, "leading star");
    CheckTrue(TTestHandler::WildcardMatch("Test_*", "Test_ASWTools_String.cpp"), __func__, __LINE__, "trailing star");
    CheckFalse(TTestHandler::WildcardMatch("String", "Test_ASWTools_String.cpp"),
        __func__, __LINE__, "a substring alone does not match - the whole name must match");
    CheckTrue(TTestHandler::WildcardMatch("*String*", "Test_ASWTools_String.cpp"),
        __func__, __LINE__, "explicit substring search via surrounding stars");
    CheckTrue(TTestHandler::WildcardMatch("*", "*abc"), __func__, __LINE__, "bare star matches text starting with a star");
    CheckTrue(TTestHandler::WildcardMatch("*c", "*abc"), __func__, __LINE__, "leading star matches a star in the text");
    CheckTrue(TTestHandler::WildcardMatch("a*", "a*"), __func__, __LINE__, "trailing star matches a star in the text");
    CheckTrue(TTestHandler::WildcardMatch("*b*", "a*b*c"), __func__, __LINE__, "stars around a name holding stars");
    CheckFalse(TTestHandler::WildcardMatch("*d", "*abc"), __func__, __LINE__, "a star in the text is still just a char");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_QuestionMark()
{
    // Act & Assert
    CheckTrue(TTestHandler::WildcardMatch("a?c", "abc"), __func__, __LINE__, "question mark matches one char");
    CheckFalse(TTestHandler::WildcardMatch("a?c", "ac"), __func__, __LINE__, "question mark requires a char, not zero");
    CheckFalse(TTestHandler::WildcardMatch("a?c", "abbc"), __func__, __LINE__, "question mark matches one char, not two");
    CheckTrue(TTestHandler::WildcardMatch("???", "abc"), __func__, __LINE__, "three question marks match three chars");
    CheckFalse(TTestHandler::WildcardMatch("??", "abc"), __func__, __LINE__, "too few question marks for the text length");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Handler)
ASW_REGISTER_TEST_GROUP_IN_SUITE(TFixture_GroupConstructorThrows, "HandlerFixtures")
ASW_REGISTER_TEST_GROUP(TFixture_GroupSetUpThrows)
