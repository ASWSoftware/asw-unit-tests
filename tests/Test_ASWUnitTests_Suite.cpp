/* **************************************************************************
Test_ASWUnitTests_Suite.cpp
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
#include "Test_ASWUnitTests_Suite.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// This group's suite path and qualified name, as the Handler_* tests expect to find them.
char const* const ThisSuitePath = "Suites.Nested";
char const* const ThisQualifiedName = "Suites.Nested.ASWUnitTests_Suite_Tests";

//---------------------------------------------------------------------------

std::string JoinNames(std::vector<std::string> const& names);
TestFilter WildcardFilter(std::string const& pattern);

//---------------------------------------------------------------------------
/*
    JoinNames

    'names' joined with "|", so a whole list can be compared, and shown on failure, as one string.
*/
std::string JoinNames(std::vector<std::string> const& names)
{
    std::string joined;

    for (std::size_t i = 0; i < names.size(); ++i)
        joined += (i == 0 ? std::string() : std::string("|")) + names[i];

    return joined;
}
//---------------------------------------------------------------------------
/*
    WildcardFilter

    A filter matching full names against 'pattern' the way --filter does.
*/
TestFilter WildcardFilter(std::string const& pattern)
{
    return [pattern](std::string const& fullTestName)
        {
            return TTestHandler::WildcardMatch(pattern, fullTestName);
        };
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoSuiteGroup
//
// A never-registered TAutoTestGroup in suite "Outer", with one ASW_TEST test.
/////////////////////////////////////////////////////////////////////////////
class TFixture_AutoSuiteGroup : public TAutoTestGroup<TFixture_AutoSuiteGroup>
{
private:
    typedef TAutoTestGroup<TFixture_AutoSuiteGroup> inherited;

public:
    std::size_t Calls = 0;

public:
    void Test_Auto();

public:
    TFixture_AutoSuiteGroup();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_MinimalGroup
//
// An ITestGroup implemented directly, without TTestGroupBase, that doesn't
// override GetSuitePath(), as one written before suites existed wouldn't.
/////////////////////////////////////////////////////////////////////////////
class TFixture_MinimalGroup : public ITestGroup
{
private:
    std::string m_Name = "Fixture_MinimalGroup";
    TTestResults m_Results;
    TestCallbackList m_Tests;

public:
    TestCallbackList& GetTestCallbackList() override
    {
        return m_Tests;
    }
    std::string const& GetTestGroupName() const override
    {
        return m_Name;
    }
    TTestResults const& Results() const override
    {
        return m_Results;
    }
    void Run(TestFilter const& /*filter*/, std::optional<unsigned int> /*shuffleSeed*/,
        std::optional<unsigned int> /*testTimeoutSeconds*/, bool /*catchCrashes*/) override
    {
    }
    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_PlainGroup
//
// A never-registered group outside any suite, with one test.
/////////////////////////////////////////////////////////////////////////////
class TFixture_PlainGroup : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

public:
    void Test_Only() {}

public:
    TFixture_PlainGroup()
        : inherited("Fixture_PlainGroup")
    {
        SetLogSuppressed(true);
        RegisterTest(&TFixture_PlainGroup::Test_Only, "Only");
    }

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_SuiteDuplicate
//
// A never-registered group in suite "Outer" that registers one test name
// twice, so constructing it throws TExceptDuplicateTestName.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SuiteDuplicate : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

public:
    void Test_Same() {}

public:
    TFixture_SuiteDuplicate()
        : inherited(TSuitePath("Outer"), "Fixture_SuiteDuplicate")
    {
        SetLogSuppressed(true);
        RegisterTest(&TFixture_SuiteDuplicate::Test_Same, "Same");
        RegisterTest(&TFixture_SuiteDuplicate::Test_Same, "Same");
    }

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_SuiteGroup
//
// A never-registered group in nested suite "Outer.Inner", with two tests.
/////////////////////////////////////////////////////////////////////////////
class TFixture_SuiteGroup : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

public:
    void Test_Alpha() {}
    void Test_Beta() {}

public:
    TFixture_SuiteGroup()
        : inherited(TSuitePath("Outer.Inner"), "Fixture_SuiteGroup")
    {
        SetLogSuppressed(true);
        RegisterTest(&TFixture_SuiteGroup::Test_Alpha, "Alpha");
        RegisterTest(&TFixture_SuiteGroup::Test_Beta, "Beta");
    }

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoSuiteGroup
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_AutoSuiteGroup::TFixture_AutoSuiteGroup()
    : inherited(TSuitePath("Outer"), "Fixture_AutoSuiteGroup")
{
    SetLogSuppressed(true);
}
//---------------------------------------------------------------------------
ASW_TEST(TFixture_AutoSuiteGroup, Auto)
{
    ++Calls;
}
//---------------------------------------------------------------------------

namespace
{

/////////////////////////////////////////////////////////////////////////////
// TRecordingObserver
//
// An ITestRunObserver that records the log text and the group name each test
// started with. No test here uses a timeout, so every call arrives on the
// calling thread and no locking is needed.
/////////////////////////////////////////////////////////////////////////////
class TRecordingObserver : public ITestRunObserver
{
public:
    std::string LogText;
    std::vector<std::string> StartedGroupNames;

public:
    void OnLog(std::string const& text) override
    {
        LogText += text;
    }

    void OnTestFinished(TTestCaseRecord const& /*record*/) override
    {
    }

    void OnTestStarted(std::string const& groupName, std::string const& /*testName*/) override
    {
        StartedGroupNames.push_back(groupName);
    }

    bool StopRequested() override
    {
        return false;
    }
};

} // namespace

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Suite
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Suite::TTest_ASWUnitTests_Suite()
    : inherited(TSuitePath(ThisSuitePath), "ASWUnitTests_Suite_Tests")
{
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Suite::~TTest_ASWUnitTests_Suite()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Suite::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Suite::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Suite::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Suite::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, ApplyRegisteredSuitePath_KeepsMatchingOrNoSuite)
{
    // Arrange
    TFixture_PlainGroup plain;
    TFixture_SuiteGroup inSuite;
    TFixture_MinimalGroup minimal;

    // Act
    TTestHandler::ApplyRegisteredSuitePath(plain, "");
    TTestHandler::ApplyRegisteredSuitePath(inSuite, "Outer.Inner");
    TTestHandler::ApplyRegisteredSuitePath(minimal, "");

    // Assert
    CheckEmpty(plain.GetSuitePath(), __func__, __LINE__, "a registration naming no suite leaves the group as it is");
    CheckEquals(std::string("Outer.Inner.Fixture_SuiteGroup"), inSuite.GetQualifiedTestGroupName(), __func__,
        __LINE__, "a registration may name the suite the constructor gives too");
    CheckEmpty(minimal.GetSuitePath(), __func__, __LINE__,
        "an ITestGroup not derived from TTestGroupBase is fine while its registration names no suite");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, ApplyRegisteredSuitePath_PutsGroupInSuite)
{
    // Arrange
    TFixture_PlainGroup fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);

    // Act
    TTestHandler::ApplyRegisteredSuitePath(fixture, "Outer.Inner");
    fixture.Run(WildcardFilter("Outer.*"), std::nullopt, std::nullopt, false);

    // Assert
    CheckEquals(std::string("Outer.Inner"), fixture.GetSuitePath(), __func__, __LINE__,
        "the group is in the registration's suite");
    CheckEquals(std::string("Fixture_PlainGroup"), fixture.GetTestGroupName(), __func__, __LINE__,
        "its own name is unchanged");
    CheckEquals(std::string("Outer.Inner.Fixture_PlainGroup"), fixture.GetQualifiedTestGroupName(), __func__,
        __LINE__, "its qualified name starts with the registration's suite path");
    AssertEquals(static_cast<std::size_t>(1), fixture.Results().CaseRecords.size(), __func__, __LINE__,
        "the suite's filter runs its test");
    CheckEquals(std::string("Outer.Inner.Fixture_PlainGroup"), fixture.Results().CaseRecords[0].GroupName, __func__,
        __LINE__, "the record names the group by its qualified name");
    CheckEquals(std::string("Outer.Inner"), fixture.Results().CaseRecords[0].SuitePath, __func__, __LINE__,
        "the record carries the suite path");
    CheckEquals(std::string("Outer.Inner.Fixture_PlainGroup"), JoinNames(observer.StartedGroupNames), __func__,
        __LINE__, "OnTestStarted() is given the qualified name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, ApplyRegisteredSuitePath_RejectsConflictingSuite)
{
    CheckThrows<TExceptInvalidSuitePath>([]()
        {
            TFixture_SuiteGroup fixture;
            TTestHandler::ApplyRegisteredSuitePath(fixture, "Other");
        }, __func__, __LINE__, "the constructor gives a different suite",
        "Test group \"Fixture_SuiteGroup\" is registered in suite \"Other\", but is in suite \"Outer.Inner\" by its "
        "own definition");
    CheckThrows<TExceptInvalidSuitePath>([]()
        {
            TFixture_MinimalGroup fixture;
            TTestHandler::ApplyRegisteredSuitePath(fixture, "Outer");
        }, __func__, __LINE__, "an ITestGroup not derived from TTestGroupBase can't be put in a suite",
        "Test group \"Fixture_MinimalGroup\" is registered in suite \"Outer\", but is in suite \"\" by its own "
        "definition");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, ApplyRegisteredSuitePath_RejectsInvalidPath)
{
    CheckThrows<TExceptInvalidSuitePath>([]()
        {
            TFixture_PlainGroup fixture;
            TTestHandler::ApplyRegisteredSuitePath(fixture, "Outer.");
        }, __func__, __LINE__, "a registration's suite path is checked like any other",
        "Suite path \"Outer.\" has an empty suite name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, AutoTestGroup_InSuite)
{
    // Arrange
    TFixture_AutoSuiteGroup fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    CheckEquals(std::string("Outer"), fixture.GetSuitePath(), __func__, __LINE__,
        "TAutoTestGroup's suite constructor passes the suite path on");
    CheckEquals(std::string("Outer.Fixture_AutoSuiteGroup"), fixture.GetQualifiedTestGroupName(), __func__, __LINE__,
        "the qualified name is the suite path, then the group's own name");
    CheckEquals(static_cast<std::size_t>(1), fixture.Calls, __func__, __LINE__, "its ASW_TEST test still runs");
    AssertEquals(static_cast<std::size_t>(1), fixture.Results().CaseRecords.size(), __func__, __LINE__,
        "one record for its one test");
    CheckEquals(std::string("Outer.Fixture_AutoSuiteGroup"), fixture.Results().CaseRecords[0].GroupName, __func__,
        __LINE__, "the record names the group by its qualified name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, DuplicateName_ReportsQualifiedName)
{
    CheckThrows<TExceptDuplicateTestName>([]()
        {
            return std::make_unique<TFixture_SuiteDuplicate>();
        }, __func__, __LINE__, "a group in a suite registering one name twice",
        "Test group \"Outer.Fixture_SuiteDuplicate\" registers more than one test named \"Same\"");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, GroupInSuite_FilterMatchesQualifiedName)
{
    // Arrange
    TFixture_SuiteGroup byFullName;
    TFixture_SuiteGroup bySuite;
    TFixture_SuiteGroup byOwnName;

    // Act
    byFullName.Run(WildcardFilter("Outer.Inner.Fixture_SuiteGroup.Beta"), std::nullopt, std::nullopt, false);
    bySuite.Run(WildcardFilter("Outer.*"), std::nullopt, std::nullopt, false);
    byOwnName.Run(WildcardFilter("Fixture_SuiteGroup.*"), std::nullopt, std::nullopt, false);

    // Assert
    AssertEquals(static_cast<std::size_t>(1), byFullName.Results().CaseRecords.size(), __func__, __LINE__,
        "a test's full name starts with its suite path");
    CheckEquals(std::string("Beta"), byFullName.Results().CaseRecords[0].TestName, __func__, __LINE__,
        "the full name selects just that test");
    CheckEquals(static_cast<std::size_t>(2), bySuite.Results().CaseRecords.size(), __func__, __LINE__,
        "a pattern on the outer suite selects every test in it");
    CheckEmpty(byOwnName.Results().CaseRecords, __func__, __LINE__,
        "the group's own name without its suite path doesn't match");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, GroupInSuite_ReportsQualifiedName)
{
    // Arrange
    TFixture_SuiteGroup fixture;
    TRecordingObserver observer;
    fixture.SetRunObserver(&observer);

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    CheckEquals(std::string("Fixture_SuiteGroup"), fixture.GetTestGroupName(), __func__, __LINE__,
        "GetTestGroupName() is still the group's own name");
    CheckEquals(std::string("Outer.Inner"), fixture.GetSuitePath(), __func__, __LINE__,
        "GetSuitePath() is the path it was constructed with");
    CheckEquals(std::string("Outer.Inner.Fixture_SuiteGroup"), fixture.GetQualifiedTestGroupName(), __func__,
        __LINE__, "the qualified name is the suite path, then the group's own name");

    AssertEquals(static_cast<std::size_t>(2), fixture.Results().CaseRecords.size(), __func__, __LINE__,
        "one record per test");
    for (TTestCaseRecord const& record : fixture.Results().CaseRecords)
    {
        CheckEquals(std::string("Outer.Inner.Fixture_SuiteGroup"), record.GroupName, __func__, __LINE__,
            "a record names the group by its qualified name");
        CheckEquals(std::string("Outer.Inner"), record.SuitePath, __func__, __LINE__,
            "a record carries the suite path on its own");
    }

    CheckEquals(std::string("Outer.Inner.Fixture_SuiteGroup|Outer.Inner.Fixture_SuiteGroup"),
        JoinNames(observer.StartedGroupNames), __func__, __LINE__,
        "OnTestStarted() is given the qualified name, like the records");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, GroupWithoutSuite_KeepsItsOwnName)
{
    // Arrange
    TFixture_PlainGroup fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    CheckEmpty(fixture.GetSuitePath(), __func__, __LINE__, "the name-only constructor puts the group in no suite");
    CheckEquals(std::string("Fixture_PlainGroup"), fixture.GetQualifiedTestGroupName(), __func__, __LINE__,
        "without a suite, the qualified name is just the group's own name");
    AssertEquals(static_cast<std::size_t>(1), fixture.Results().CaseRecords.size(), __func__, __LINE__,
        "one record for its one test");
    CheckEquals(std::string("Fixture_PlainGroup"), fixture.Results().CaseRecords[0].GroupName, __func__, __LINE__,
        "the record's group name is unchanged from before suites existed");
    CheckEmpty(fixture.Results().CaseRecords[0].SuitePath, __func__, __LINE__, "the record has no suite path");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, Handler_ListsGroupUnderItsSuite)
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer); // Before Initialize(), so its banner goes to the observer, not std::cout.
    handler.Initialize("Test_Handler_ListsGroupUnderItsSuite");

    // Act
    std::vector<TTestId> const tests = handler.GetTests();
    handler.ListTests(WildcardFilter("Suites.*"), "Suites.*");

    // Assert
    CheckEquals(std::string(ThisQualifiedName), GetQualifiedTestGroupName(), __func__, __LINE__,
        "this group is in the suite the other checks look for");

    std::size_t listed = 0;
    for (TTestId const& test : tests)
    {
        if (test.GroupName != ThisQualifiedName)
            continue;

        ++listed;
        CheckEquals(std::string(ThisSuitePath), test.SuitePath, __func__, __LINE__,
            "GetTests() gives each test's suite path on its own");
    }

    CheckEquals(GetTestCallbackList().size(), listed, __func__, __LINE__,
        "GetTests() lists every test in this group under its qualified name");
    CheckContains(observer.LogText, std::string("\n") + ThisQualifiedName + ".TSuitePath_SplitsNames\n", __func__,
        __LINE__, "--list shows a test by its full name, suite path first");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, Handler_PutsGroupInItsRegisteredSuite)
{
    // Arrange: the example group TTest_TMersenneTwisterRandom names its suite only in its registration (see
    // Test_ASWTools_Random.cpp).
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer);
    handler.Initialize("Test_Handler_PutsGroupInItsRegisteredSuite");

    // Act
    std::vector<TTestId> const tests = handler.GetTests();

    // Assert
    std::size_t found = 0;
    for (TTestId const& test : tests)
    {
        if (test.GroupName != "ASWExamples.ASWTools_Random_TMersenneTwisterRandom_Tests")
            continue;

        ++found;
        CheckEquals(std::string("ASWExamples"), test.SuitePath, __func__, __LINE__,
            "ASW_REGISTER_TEST_GROUP_IN_SUITE put the group in its suite");
    }

    CheckTrue(found > 0, __func__, __LINE__, "the group's tests are listed under its qualified name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, Handler_RunsSuiteByFilter)
{
    // Arrange: only this group's TSuitePath_* tests, which are pure, so they're safe to run from a nested
    // TTestHandler.
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer);
    handler.Initialize("Test_Handler_RunsSuiteByFilter");

    // Act
    TTestResults const results = handler.Run(WildcardFilter("Suites.*.TSuitePath_*"), "Suites.*.TSuitePath_*");

    // Assert
    CheckEquals(2u, results.SuccessCount, __func__, __LINE__, "both TSuitePath_* tests ran and passed");
    AssertEquals(static_cast<std::size_t>(2), results.CaseRecords.size(), __func__, __LINE__,
        "nothing outside the filter ran");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        CheckEquals(std::string(ThisQualifiedName), record.GroupName, __func__, __LINE__,
            "the handler's records name the group by its qualified name");
        CheckEquals(std::string(ThisSuitePath), record.SuitePath, __func__, __LINE__,
            "the handler's records carry the suite path");
    }

    CheckContains(observer.LogText, std::string("Setting up group 1 of 1: \"") + ThisQualifiedName + "\"", __func__,
        __LINE__, "the console log names the group by its qualified name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, ITestGroup_DefaultsToNoSuite)
{
    // Arrange
    TFixture_MinimalGroup fixture;

    // Act
    std::string const suitePath = fixture.GetSuitePath();
    std::string const qualifiedName = fixture.GetQualifiedTestGroupName();

    // Assert
    CheckEmpty(suitePath, __func__, __LINE__, "an ITestGroup that doesn't override GetSuitePath() is in no suite");
    CheckEquals(std::string("Fixture_MinimalGroup"), qualifiedName, __func__, __LINE__,
        "so its qualified name is just its own name");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, TSuitePath_RejectsEmptyNames)
{
    std::vector<std::string> const invalidPaths = { ".", "..", ".Framework", "Framework.", "Framework..Console" };

    for (std::string const& path : invalidPaths)
    {
        CheckThrows<TExceptInvalidSuitePath>([&path]()
            {
                return TSuitePath(path);
            }, __func__, __LINE__, "suite path \"" + path + "\" has an empty suite name",
            "Suite path \"" + path + "\" has an empty suite name");
    }
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_Suite, TSuitePath_SplitsNames)
{
    // Arrange
    TSuitePath const empty("");
    TSuitePath const single("Framework");
    TSuitePath const nested("Framework.Console.Colors");
    TSuitePath const spaced("My Suite.Sub Suite");

    // Assert
    CheckEmpty(empty.GetPath(), __func__, __LINE__, "an empty path is allowed, for no suite");
    CheckEmpty(empty.GetNames(), __func__, __LINE__, "an empty path has no suite names");
    CheckEquals(std::string("Framework"), JoinNames(single.GetNames()), __func__, __LINE__,
        "a path without '.' is one suite name");
    CheckEquals(std::string("Framework.Console.Colors"), nested.GetPath(), __func__, __LINE__,
        "GetPath() is the path as given");
    CheckEquals(std::string("Framework|Console|Colors"), JoinNames(nested.GetNames()), __func__, __LINE__,
        "a nested path splits at each '.', outermost first");
    CheckEquals(std::string("My Suite|Sub Suite"), JoinNames(spaced.GetNames()), __func__, __LINE__,
        "only '.' separates suite names");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Suite)
