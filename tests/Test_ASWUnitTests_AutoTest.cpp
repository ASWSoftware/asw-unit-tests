/* **************************************************************************
Test_ASWUnitTests_AutoTest.cpp
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
#include "Test_ASWUnitTests_AutoTest.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

std::string JoinNames(std::vector<std::string> const& names);
std::vector<std::string> RegisteredNames(ITestGroup& group);

//---------------------------------------------------------------------------
/*
    JoinNames

    'names' joined with ", ", so a whole list can be compared, and shown on failure, as one string.
*/
std::string JoinNames(std::vector<std::string> const& names)
{
    std::string joined;

    for (std::string const& name : names)
    {
        if (!joined.empty())
            joined += ", ";
        joined += name;
    }

    return joined;
}
//---------------------------------------------------------------------------
/*
    RegisteredNames

    The registered test names of 'group', in registration order.
*/
std::vector<std::string> RegisteredNames(ITestGroup& group)
{
    std::vector<std::string> names;

    for (std::unique_ptr<ITestCase> const& testCase : group.GetTestCallbackList())
        names.push_back(testCase->GetName());

    return names;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoAndExplicitDuplicate
//
// A never-registered fixture group that registers the ASW_TEST test "Same"
// again in its constructor, so constructing it throws.
/////////////////////////////////////////////////////////////////////////////
class TFixture_AutoAndExplicitDuplicate : public TAutoTestGroup<TFixture_AutoAndExplicitDuplicate>
{
private:
    typedef TAutoTestGroup<TFixture_AutoAndExplicitDuplicate> inherited;

public:
    void Test_Same();

public:
    TFixture_AutoAndExplicitDuplicate();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoTests
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with three
// ASW_TEST tests, defined out of alphabetical order, and two tests its
// constructor registers itself, also out of alphabetical order. Each test
// appends its name to Calls, on the instance it ran on.
/////////////////////////////////////////////////////////////////////////////
class TFixture_AutoTests : public TAutoTestGroup<TFixture_AutoTests>
{
private:
    typedef TAutoTestGroup<TFixture_AutoTests> inherited;

public:
    std::vector<std::string> Calls;

public:
    void Test_Alpha();
    void Test_Explicit_A();
    void Test_Explicit_B();
    void Test_Mike();
    void Test_Zulu();

public:
    TFixture_AutoTests();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_ExplicitDuplicate
//
// A never-registered fixture group, not a TAutoTestGroup, whose constructor
// registers two different methods under the same test name, so
// constructing it throws.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ExplicitDuplicate : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_First() {}
    void Test_Second() {}

public:
    TFixture_ExplicitDuplicate();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_TestCasesDuplicate
//
// A never-registered fixture group whose RegisterTestCases() name generator
// gives two rows the same label, so constructing it throws.
/////////////////////////////////////////////////////////////////////////////
class TFixture_TestCasesDuplicate : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_Case(int const& /*value*/) {}

public:
    TFixture_TestCasesDuplicate();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoAndExplicitDuplicate
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_AutoAndExplicitDuplicate::TFixture_AutoAndExplicitDuplicate()
    : inherited("Fixture_AutoAndExplicitDuplicate")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_AutoAndExplicitDuplicate::Test_Same, "Same");
}
//---------------------------------------------------------------------------
ASW_TEST(TFixture_AutoAndExplicitDuplicate, Same)
{
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_AutoTests
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_AutoTests::TFixture_AutoTests()
    : inherited("Fixture_AutoTests")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_AutoTests::Test_Explicit_B, "Explicit_B");
    RegisterTest(&TFixture_AutoTests::Test_Explicit_A, "Explicit_A");
}
//---------------------------------------------------------------------------
ASW_TEST(TFixture_AutoTests, Zulu)
{
    Calls.push_back("Zulu");
}
//---------------------------------------------------------------------------
ASW_TEST(TFixture_AutoTests, Alpha)
{
    Calls.push_back("Alpha");
}
//---------------------------------------------------------------------------
ASW_TEST(TFixture_AutoTests, Mike)
{
    Calls.push_back("Mike");
}
//---------------------------------------------------------------------------
void TFixture_AutoTests::Test_Explicit_A()
{
    Calls.push_back("Explicit_A");
}
//---------------------------------------------------------------------------
void TFixture_AutoTests::Test_Explicit_B()
{
    Calls.push_back("Explicit_B");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_ExplicitDuplicate
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_ExplicitDuplicate::TFixture_ExplicitDuplicate()
    : inherited("Fixture_ExplicitDuplicate")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_ExplicitDuplicate::Test_First, "Same");
    RegisterTest(&TFixture_ExplicitDuplicate::Test_Second, "Same");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_TestCasesDuplicate
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_TestCasesDuplicate::TFixture_TestCasesDuplicate()
    : inherited("Fixture_TestCasesDuplicate")
{
    SetLogSuppressed(true);

    std::vector<int> const values = { 1, 2, 3 };
    RegisterTestCases(&TFixture_TestCasesDuplicate::Test_Case, "Case", values,
        [](int const& value)
        {
            return std::to_string(value % 2);
        });
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_AutoTest
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_AutoTest::TTest_ASWUnitTests_AutoTest()
    : inherited("ASWUnitTests_AutoTest_Tests")
{
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_AutoTest::~TTest_ASWUnitTests_AutoTest()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_AutoTest::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_AutoTest::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_AutoTest::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_AutoTest::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, AutoTests_AreRegisteredAlphabeticallyBeforeExplicitTests)
{
    // Arrange
    TFixture_AutoTests fixture;

    // Act
    std::string const names = JoinNames(RegisteredNames(fixture));

    // Assert
    CheckEquals(std::string("Alpha, Mike, Zulu, Explicit_B, Explicit_A"), names, __func__, __LINE__,
        "ASW_TEST tests in alphabetical order, then the constructor's tests in the order it registered them");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, AutoTests_AreRegisteredForEveryInstance)
{
    // Arrange
    TFixture_AutoTests first;

    // Act
    TFixture_AutoTests second;

    // Assert
    CheckEquals(JoinNames(RegisteredNames(first)), JoinNames(RegisteredNames(second)), __func__, __LINE__,
        "constructing a group doesn't use up its ASW_TEST list");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, AutoTests_RunOnTheirOwnInstance)
{
    // Arrange
    TFixture_AutoTests fixture;
    TFixture_AutoTests other;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    CheckEquals(std::string("Alpha, Mike, Zulu, Explicit_B, Explicit_A"), JoinNames(fixture.Calls), __func__,
        __LINE__, "every test runs once, on the group that was run, in registration order");
    CheckEquals(5u, fixture.Results().SuccessCount, __func__, __LINE__, "every test passes");
    CheckTrue(other.Calls.empty(), __func__, __LINE__, "no test runs on another instance of the group");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, AutoTests_RunShuffled)
{
    // Arrange
    TFixture_AutoTests fixture;
    std::vector<std::string> const names = RegisteredNames(fixture);
    unsigned int const seed = 12345;

    std::vector<std::string> expected;
    for (std::size_t index : ShuffledIndices(names.size(), seed))
        expected.push_back(names[index]);

    // Act
    fixture.Run(TestFilter(), seed, std::nullopt, false);

    // Assert
    CheckEquals(JoinNames(expected), JoinNames(fixture.Calls), __func__, __LINE__,
        "--shuffle permutes ASW_TEST and explicitly registered tests together");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, AutoTests_RunWithFilter)
{
    // Arrange
    TFixture_AutoTests fixture;
    TestFilter const filter = [](std::string const& fullTestName)
        {
            return fullTestName == "Fixture_AutoTests.Mike" || fullTestName == "Fixture_AutoTests.Explicit_A";
        };

    // Act
    fixture.Run(filter, std::nullopt, std::nullopt, false);

    // Assert
    CheckEquals(std::string("Mike, Explicit_A"), JoinNames(fixture.Calls), __func__, __LINE__,
        "a filter matches an ASW_TEST test by \"GroupName.TestName\", like any other test");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, DuplicateName_AutoAndExplicit_Throws)
{
    CheckThrows<TExceptDuplicateTestName>([]()
        {
            return std::make_unique<TFixture_AutoAndExplicitDuplicate>();
        }, __func__, __LINE__, "registering an ASW_TEST test's name again in the constructor",
        "Test group \"Fixture_AutoAndExplicitDuplicate\" registers more than one test named \"Same\"");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, DuplicateName_ExplicitTwice_Throws)
{
    CheckThrows<TExceptDuplicateTestName>([]()
        {
            return std::make_unique<TFixture_ExplicitDuplicate>();
        }, __func__, __LINE__, "registering two methods under one name",
        "Test group \"Fixture_ExplicitDuplicate\" registers more than one test named \"Same\"");
}
//---------------------------------------------------------------------------
ASW_TEST(TTest_ASWUnitTests_AutoTest, DuplicateName_RegisterTestCases_Throws)
{
    CheckThrows<TExceptDuplicateTestName>([]()
        {
            return std::make_unique<TFixture_TestCasesDuplicate>();
        }, __func__, __LINE__, "a name generator that gives two rows the same label",
        "Test group \"Fixture_TestCasesDuplicate\" registers more than one test named \"Case[1]\"");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_AutoTest)
