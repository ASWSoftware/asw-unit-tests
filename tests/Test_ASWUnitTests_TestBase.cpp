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
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <iostream>
#include <sstream>
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

// Temporarily redirects std::cout into an in-memory buffer for its lifetime, restoring the
// original stream buffer on destruction. Used to verify SetLogSuppressed(false) still logs
// normally (i.e. that the suppression test below isn't trivially passing because nothing ever
// logs at all).
class TStdoutRedirect
{
private:
    // Declaration order matters here: members initialize in this order regardless of the
    // constructor's initializer-list order, and m_OriginalBuffer's initializer reads m_Captured.
    std::ostringstream m_Captured;
    std::streambuf* m_OriginalBuffer;

public:
    TStdoutRedirect()
        : m_OriginalBuffer(std::cout.rdbuf(m_Captured.rdbuf()))
    {
    }

    ~TStdoutRedirect()
    {
        std::cout.rdbuf(m_OriginalBuffer);
    }

    std::string Str() const
    {
        return m_Captured.str();
    }
};

// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test method per outcome
// TTestGroupBase can produce, plus one that fails twice in a row to prove a Check failure doesn't
// abort the rest of the test the way an Assert failure does. Constructed and run directly by
// TTest_ASWUnitTests_TestBase's own test methods below, so its deliberate failures/skip only ever
// affect this fixture's own Results() - never the real, self-registered suite running it.
class TFixture_MixedOutcomes : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_ContinuesAfterCheckFailure();
    void Test_FailViaAssert();
    void Test_FailViaCheck();
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
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Check_ContinuesButAssert_Aborts, "Check_ContinuesButAssert_Aborts");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords,
        "Run_RecordsOutcomeCountsAndCaseRecords");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetLogSuppressed_SilencesFixtureOutput,
        "SetLogSuppressed_SilencesFixtureOutput");
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
void TTest_ASWUnitTests_TestBase::Test_Check_ContinuesButAssert_Aborts()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt);

    // Assert
    CheckTrue(fixture.ReachedSecondCheck, __func__, __LINE__,
        "a Check failure does not abort the rest of the test, unlike Assert");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt);

    // Assert
    TTestResults const& results = fixture.Results();

    CheckEquals(1u, results.SuccessCount, __func__, __LINE__, "only the trivially-passing test succeeded");
    CheckEquals(3u, results.FailedCount, __func__, __LINE__,
        "the Check-failing, Assert-failing, and continues-after-Check tests are all counted as failed");
    CheckEquals(1u, results.SkippedCount, __func__, __LINE__, "the skipped test is counted separately from failures");
    CheckEquals(static_cast<size_t>(5), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

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
void TTest_ASWUnitTests_TestBase::Test_SetLogSuppressed_SilencesFixtureOutput()
{
    // Arrange
    TFixture_MixedOutcomes verboseFixture(false);
    TFixture_MixedOutcomes suppressedFixture(true);
    std::string verboseOutput;
    std::string suppressedOutput;

    // Act
    {
        TStdoutRedirect redirect;
        verboseFixture.Run(TestFilter(), std::nullopt);
        verboseOutput = redirect.Str();
    }
    {
        TStdoutRedirect redirect;
        suppressedFixture.Run(TestFilter(), std::nullopt);
        suppressedOutput = redirect.Str();
    }

    // Assert
    CheckFalse(verboseOutput.empty(), __func__, __LINE__, "an unsuppressed fixture logs its test run as usual");
    CheckTrue(suppressedOutput.empty(), __func__, __LINE__,
        "SetLogSuppressed(true) silences the fixture's own Log()/LogAppend()");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_TestBase)
