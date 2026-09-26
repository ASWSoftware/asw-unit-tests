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
#include <stdexcept>
#include <thread>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Registry.h"
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

bool NameEndsWith(std::string const& name, std::string const& suffix);

//---------------------------------------------------------------------------

// True if 'name' ends with 'suffix'. Used below so a fixture test's own name ("..._Passes"/
// "..._Fails") documents its expected outcome, and the outer verifying test can check every
// registered test generically instead of hand-maintaining a separate expected-outcome table.
bool NameEndsWith(std::string const& name, std::string const& suffix)
{
    return name.size() >= suffix.size() && name.compare(name.size() - suffix.size(), suffix.size(), suffix) == 0;
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
// throw, and a message substring that's present or absent. Every test name ends with "_Passes" or
// "_Fails", read generically by Test_SetExceptionExpected_MatchesTypeAndMessage below via
// NameEndsWith() rather than a hand-maintained table.
/////////////////////////////////////////////////////////////////////////////
class TFixture_ExceptionExpectations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_ExceptionExpectedButNoneThrown_Fails();
    void Test_GenericExceptionExpected_NonStdExceptionThrown_Passes();
    void Test_GenericExceptionExpected_StdExceptionThrown_Passes();
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

    RegisterTest(&TFixture_ExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails,
        "ExceptionExpectedButNoneThrown_Fails");
    RegisterTest(&TFixture_ExceptionExpectations::Test_GenericExceptionExpected_NonStdExceptionThrown_Passes,
        "GenericExceptionExpected_NonStdExceptionThrown_Passes");
    RegisterTest(&TFixture_ExceptionExpectations::Test_GenericExceptionExpected_StdExceptionThrown_Passes,
        "GenericExceptionExpected_StdExceptionThrown_Passes");
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
void TFixture_ExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails()
{
    SetExceptionExpected<TFixtureSpecificError>(__func__, __LINE__, "expected an exception that never comes");
    // No throw here: Test() should fail this once control returns without one.
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
// TFixture_MixedOutcomes
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test method per outcome
// TTestGroupBase can produce, plus one that fails twice in a row to prove a Check failure doesn't
// abort the rest of the test the way an Assert failure does. Constructed and run directly by
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
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AbandonsHungTestAndAbortsGroupOnTimeout,
        "Run_AbandonsHungTestAndAbortsGroupOnTimeout");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_AppliesFilterToSkipNonMatchingTests,
        "Run_AppliesFilterToSkipNonMatchingTests");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ContinuesAfterCrashWhenCatchCrashesIsSet,
        "Run_ContinuesAfterCrashWhenCatchCrashesIsSet");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords,
        "Run_RecordsOutcomeCountsAndCaseRecords");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_Run_ShuffleSeedProducesDeterministicOrder,
        "Run_ShuffleSeedProducesDeterministicOrder");
    RegisterTest(&TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_MatchesTypeAndMessage,
        "SetExceptionExpected_MatchesTypeAndMessage");
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
void TTest_ASWUnitTests_TestBase::Test_Run_RecordsOutcomeCountsAndCaseRecords()
{
    // Arrange
    TFixture_MixedOutcomes fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

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
void TTest_ASWUnitTests_TestBase::Test_SetExceptionExpected_MatchesTypeAndMessage()
{
    // Arrange
    TFixture_ExceptionExpectations fixture;

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(9), results.CaseRecords.size(), __func__, __LINE__, "one record per registered test");

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

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_TestBase)
