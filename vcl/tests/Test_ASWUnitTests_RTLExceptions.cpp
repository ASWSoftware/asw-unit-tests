/* **************************************************************************
Test_ASWUnitTests_RTLExceptions.cpp
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
#include "Test_ASWUnitTests_RTLExceptions.h"
//---------------------------------------------------------------------------

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

//---------------------------------------------------------------------------
#include <memory>
#include <stdexcept>
#include <type_traits>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Exception.h"
#include "ASWUnitTests_Registry.h"
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
// TFixture_RTLExceptionExpectations
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with one test method per RTL-related
// SetExceptionExpected()/Test() branch: no throw, a generic expectation, and a specific RTL type crossed
// with an exact match (thrown from C++ and raised from Delphi RTL code), a polymorphic base-type match, a
// wrong sibling type, a message substring that's present, absent, non-ASCII, or only in the class name,
// a std::exception, and a non-exception throw; plus a std::exception type expected but an RTL exception
// thrown, and an earlier Check* failure followed by the expected RTL exception; plus, where supported, the
// std::source_location form of SetExceptionExpected(). Every test name ends with "_Passes" or "_Fails",
// read generically by CheckFixtureOutcomes().
/////////////////////////////////////////////////////////////////////////////
class TFixture_RTLExceptionExpectations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_CheckFailedThenExpectedRTLExceptionThrown_Fails();
    void Test_ExceptionExpectedButNoneThrown_Fails();
    void Test_GenericExceptionExpected_RTLExceptionThrown_Passes();
    void Test_RTLTypeExpected_ExactTypeThrown_Passes();
    void Test_RTLTypeExpected_MessageSubstringAbsent_Fails();
    void Test_RTLTypeExpected_MessageSubstringOnlyInClassName_Fails();
    void Test_RTLTypeExpected_MessageSubstringPresent_Passes();
    void Test_RTLTypeExpected_NonASCIIMessageSubstringPresent_Passes();
    void Test_RTLTypeExpected_NonExceptionThrown_Fails();
    void Test_RTLTypeExpected_PolymorphicBaseTypeThrown_Passes();
    void Test_RTLTypeExpected_RaisedFromDelphiCode_Passes();
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    void Test_RTLTypeExpected_SourceLocation_MessageSubstringPresent_Passes();
    void Test_RTLTypeExpected_SourceLocation_WrongSiblingTypeThrown_Fails();
#endif
    void Test_RTLTypeExpected_StdExceptionThrown_Fails();
    void Test_RTLTypeExpected_WrongSiblingTypeThrown_Fails();
    void Test_StdTypeExpected_RTLExceptionThrown_Fails();

public:
    TFixture_RTLExceptionExpectations();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_RTLExceptionExpectations::TFixture_RTLExceptionExpectations()
    : inherited("Fixture_RTLExceptionExpectations")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_RTLExceptionExpectations::Test_CheckFailedThenExpectedRTLExceptionThrown_Fails,
        "CheckFailedThenExpectedRTLExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails,
        "ExceptionExpectedButNoneThrown_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_GenericExceptionExpected_RTLExceptionThrown_Passes,
        "GenericExceptionExpected_RTLExceptionThrown_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_ExactTypeThrown_Passes,
        "RTLTypeExpected_ExactTypeThrown_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringAbsent_Fails,
        "RTLTypeExpected_MessageSubstringAbsent_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringOnlyInClassName_Fails,
        "RTLTypeExpected_MessageSubstringOnlyInClassName_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringPresent_Passes,
        "RTLTypeExpected_MessageSubstringPresent_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_NonASCIIMessageSubstringPresent_Passes,
        "RTLTypeExpected_NonASCIIMessageSubstringPresent_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_NonExceptionThrown_Fails,
        "RTLTypeExpected_NonExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_PolymorphicBaseTypeThrown_Passes,
        "RTLTypeExpected_PolymorphicBaseTypeThrown_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_RaisedFromDelphiCode_Passes,
        "RTLTypeExpected_RaisedFromDelphiCode_Passes");
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_SourceLocation_MessageSubstringPresent_Passes,
        "RTLTypeExpected_SourceLocation_MessageSubstringPresent_Passes");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_SourceLocation_WrongSiblingTypeThrown_Fails,
        "RTLTypeExpected_SourceLocation_WrongSiblingTypeThrown_Fails");
#endif
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_StdExceptionThrown_Fails,
        "RTLTypeExpected_StdExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_WrongSiblingTypeThrown_Fails,
        "RTLTypeExpected_WrongSiblingTypeThrown_Fails");
    RegisterTest(&TFixture_RTLExceptionExpectations::Test_StdTypeExpected_RTLExceptionThrown_Fails,
        "StdTypeExpected_RTLExceptionThrown_Fails");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_CheckFailedThenExpectedRTLExceptionThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "RTL type expected, then a failed Check");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    throw System::Sysutils::EConvertError(L"the expected exception");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_ExceptionExpectedButNoneThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "expected an exception that never comes");
    // No throw here: Test() should fail this once control returns without one.
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_GenericExceptionExpected_RTLExceptionThrown_Passes()
{
    SetExceptionExpected(true, __func__, __LINE__, "generic expectation, RTL exception thrown");
    throw System::Sysutils::EConvertError(L"whatever");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_ExactTypeThrown_Passes()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "exact type match");
    throw System::Sysutils::EConvertError(L"boom");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringAbsent_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "message must contain 'needle'",
        "needle");
    throw System::Sysutils::EConvertError(L"no match here");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringOnlyInClassName_Fails()
{
    // The substring check applies to Message alone, not the "ClassName: Message" failure description.
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "message must contain the class name",
        "EConvertError");
    throw System::Sysutils::EConvertError(L"boom");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_MessageSubstringPresent_Passes()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "message must contain 'needle'",
        "needle");
    throw System::Sysutils::EConvertError(L"hay needle stack");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_NonASCIIMessageSubstringPresent_Passes()
{
    // U+00E9 (e with acute accent) in the UTF-16 Message must match its UTF-8 bytes in the expectation.
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "message must contain 'caf\\xE9'",
        "caf\xC3\xA9");
    throw System::Sysutils::EConvertError(L"le caf\x00E9 est ferm\x00E9");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_NonExceptionThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "RTL type expected, int thrown");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_PolymorphicBaseTypeThrown_Passes()
{
    SetExceptionExpected<System::Sysutils::Exception>(__func__, __LINE__, "base type expected, derived type thrown");
    throw System::Sysutils::EConvertError(L"boom");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_RaisedFromDelphiCode_Passes()
{
    // Raised inside the Delphi RTL itself, not by a C++ throw, so it arrives through Delphi's own raise mechanism.
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "StrToInt raises EConvertError");
    static_cast<void>(System::Sysutils::StrToInt(L"not a number"));
}
//---------------------------------------------------------------------------
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_SourceLocation_MessageSubstringPresent_Passes()
{
    SetExceptionExpected<System::Sysutils::EConvertError>("message must contain 'needle'", "needle");
    throw System::Sysutils::EConvertError(L"hay needle stack");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_SourceLocation_WrongSiblingTypeThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>("wrong sibling type thrown");
    throw System::Sysutils::EArgumentException(L"boom");
}
//---------------------------------------------------------------------------
#endif
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_StdExceptionThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "RTL type expected, std type thrown");
    throw std::runtime_error("boom");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_RTLTypeExpected_WrongSiblingTypeThrown_Fails()
{
    SetExceptionExpected<System::Sysutils::EConvertError>(__func__, __LINE__, "wrong sibling type thrown");
    throw System::Sysutils::EArgumentException(L"boom");
}
//---------------------------------------------------------------------------
void TFixture_RTLExceptionExpectations::Test_StdTypeExpected_RTLExceptionThrown_Fails()
{
    SetExceptionExpected<std::runtime_error>(__func__, __LINE__, "std type expected, RTL type thrown");
    throw System::Sysutils::EConvertError(L"boom");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_RTLThrowsChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group testing CheckThrows()/AssertThrows()/
// CheckNoThrow() with RTL exceptions: an exact type (thrown from C++ and raised from Delphi RTL code), a base
// type, a message substring that's absent, a wrong sibling type, and RTL and std types crossed either way. Test
// names self-document expected outcome via NameEndsWith(), as in TFixture_RTLExceptionExpectations above.
/////////////////////////////////////////////////////////////////////////////
class TFixture_RTLThrowsChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertThrows_ExactType_Passes();
    void Test_CheckNoThrow_RTLExceptionThrown_Fails();
    void Test_CheckThrows_BaseType_Passes();
    void Test_CheckThrows_MessageSubstringAbsent_Fails();
    void Test_CheckThrows_RaisedFromDelphiCode_Passes();
    void Test_CheckThrows_RTLTypeExpected_StdExceptionThrown_Fails();
    void Test_CheckThrows_StdTypeExpected_RTLExceptionThrown_Fails();
    void Test_CheckThrows_WrongSiblingType_Fails();

public:
    TFixture_RTLThrowsChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_RTLThrowsChecks::TFixture_RTLThrowsChecks()
    : inherited("Fixture_RTLThrowsChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_RTLThrowsChecks::Test_AssertThrows_ExactType_Passes, "AssertThrows_ExactType_Passes");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckNoThrow_RTLExceptionThrown_Fails,
        "CheckNoThrow_RTLExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_BaseType_Passes, "CheckThrows_BaseType_Passes");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_MessageSubstringAbsent_Fails,
        "CheckThrows_MessageSubstringAbsent_Fails");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_RaisedFromDelphiCode_Passes,
        "CheckThrows_RaisedFromDelphiCode_Passes");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_RTLTypeExpected_StdExceptionThrown_Fails,
        "CheckThrows_RTLTypeExpected_StdExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_StdTypeExpected_RTLExceptionThrown_Fails,
        "CheckThrows_StdTypeExpected_RTLExceptionThrown_Fails");
    RegisterTest(&TFixture_RTLThrowsChecks::Test_CheckThrows_WrongSiblingType_Fails,
        "CheckThrows_WrongSiblingType_Fails");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_AssertThrows_ExactType_Passes()
{
    AssertThrows<System::Sysutils::EConvertError>([] {
            throw System::Sysutils::EConvertError(L"boom");
        }, __func__,
        __LINE__, "exact type match");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckNoThrow_RTLExceptionThrown_Fails()
{
    CheckNoThrow([] {
            throw System::Sysutils::EConvertError(L"boom");
        }, __func__, __LINE__, "must not throw");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_BaseType_Passes()
{
    CheckThrows<System::Sysutils::Exception>([] {
            throw System::Sysutils::EConvertError(L"boom");
        }, __func__,
        __LINE__, "base type expected, derived type thrown");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_MessageSubstringAbsent_Fails()
{
    CheckThrows<System::Sysutils::EConvertError>([] {
            throw System::Sysutils::EConvertError(L"boom");
        }, __func__,
        __LINE__, "message must contain 'needle'", "needle");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_RaisedFromDelphiCode_Passes()
{
    // Raised inside the Delphi RTL itself, not by a C++ throw, so it arrives through Delphi's own raise mechanism.
    CheckThrows<System::Sysutils::EConvertError>([] {
            static_cast<void>(System::Sysutils::StrToInt(L"not a number"));
        },
        __func__, __LINE__, "StrToInt raises EConvertError", "not a number");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_RTLTypeExpected_StdExceptionThrown_Fails()
{
    CheckThrows<System::Sysutils::EConvertError>([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__,
        "RTL type expected, std type thrown");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_StdTypeExpected_RTLExceptionThrown_Fails()
{
    CheckThrows<std::runtime_error>([] {
            throw System::Sysutils::EConvertError(L"boom");
        }, __func__, __LINE__,
        "std type expected, RTL type thrown");
}
//---------------------------------------------------------------------------
void TFixture_RTLThrowsChecks::Test_CheckThrows_WrongSiblingType_Fails()
{
    CheckThrows<System::Sysutils::EConvertError>([] {
            throw System::Sysutils::EArgumentException(L"boom");
        },
        __func__, __LINE__, "wrong sibling type thrown");
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_UnexpectedRTLException
//
// A never-registered fixture group with a single test that throws an RTL exception without expecting
// one, so the test verifying it can confirm the exception escapes Run() intact.
/////////////////////////////////////////////////////////////////////////////
class TFixture_UnexpectedRTLException : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_ThrowsUnexpectedly();

public:
    TFixture_UnexpectedRTLException();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_UnexpectedRTLException::TFixture_UnexpectedRTLException()
    : inherited("Fixture_UnexpectedRTLException")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_UnexpectedRTLException::Test_ThrowsUnexpectedly, "ThrowsUnexpectedly");
}
//---------------------------------------------------------------------------
void TFixture_UnexpectedRTLException::Test_ThrowsUnexpectedly()
{
    throw System::Sysutils::EConvertError(L"unexpected boom");
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_RTLExceptions
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_RTLExceptions::TTest_ASWUnitTests_RTLExceptions()
    : inherited("ASWUnitTests_RTLExceptions_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_DescribeRTLException_IncludesClassNameAndMessage,
        "DescribeRTLException_IncludesClassNameAndMessage");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_RTLExceptionMessage_ConvertsToUTF8,
        "RTLExceptionMessage_ConvertsToUTF8");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_Run_PropagatesUnexpectedRTLException,
        "Run_PropagatesUnexpectedRTLException");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_Run_WrapsUnexpectedRTLExceptionFromWorkerThread,
        "Run_WrapsUnexpectedRTLExceptionFromWorkerThread");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessage,
        "SetExceptionExpected_MatchesRTLTypeAndMessage");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessageOnWorkerThread,
        "SetExceptionExpected_MatchesRTLTypeAndMessageOnWorkerThread");
    RegisterTest(
        &TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessageUnderCatchCrashes,
        "SetExceptionExpected_MatchesRTLTypeAndMessageUnderCatchCrashes");
    RegisterTest(
        &TTest_ASWUnitTests_RTLExceptions::Test_TExceptRTLException_KeepsClassAndDescriptionAfterOriginalIsFreed,
        "TExceptRTLException_KeepsClassAndDescriptionAfterOriginalIsFreed");
    RegisterTest(&TTest_ASWUnitTests_RTLExceptions::Test_Throws_MatchesRTLTypeAndMessage,
        "Throws_MatchesRTLTypeAndMessage");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_RTLExceptions::~TTest_ASWUnitTests_RTLExceptions()
{
}
//---------------------------------------------------------------------------
/*
    TTest_ASWUnitTests_RTLExceptions::CheckFixtureOutcomes

    Runs TFixture_RTLExceptionExpectations with the given 'testTimeoutSeconds'/'catchCrashes', and checks
    that each of its tests passed or failed as its "_Passes"/"_Fails" name says. 'method'/'line' identify
    the calling test in any failure message.
*/
void TTest_ASWUnitTests_RTLExceptions::CheckFixtureOutcomes(std::optional<unsigned int> testTimeoutSeconds,
    bool catchCrashes, std::string const& method, int line)
{
    TFixture_RTLExceptionExpectations fixture;

    fixture.Run(TestFilter(), std::nullopt, testTimeoutSeconds, catchCrashes);

    TTestResults const& results = fixture.Results();
#if defined(ASWUNITTESTS_SOURCE_LOCATION_ENABLED)
    size_t const expectedCount = 16;
#else
    size_t const expectedCount = 14;
#endif
    CheckEquals(expectedCount, results.CaseRecords.size(), method, line, "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, method, line, record.TestName + " should pass: " + record.Message);
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, method, line, record.TestName + " should fail");
        else
            Fail(method, line, record.TestName + " name must end with _Passes or _Fails");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_DescribeRTLException_IncludesClassNameAndMessage()
{
    // Arrange
    // RTL (Delphi-style) classes can only live on the heap.
    std::unique_ptr<System::Sysutils::EConvertError> const ex(new System::Sysutils::EConvertError(L"boom"));

    // Act
    std::string const description = DescribeRTLException(*ex);

    // Assert
    CheckEquals(std::string("EConvertError: boom"), description, __func__, __LINE__, "\"ClassName: Message\"");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_RTLExceptionMessage_ConvertsToUTF8()
{
    // Arrange
    std::unique_ptr<System::Sysutils::EConvertError> const ex(new System::Sysutils::EConvertError(L"caf\x00E9"));

    // Act
    std::string const message = RTLExceptionMessage(*ex);

    // Assert
    CheckEquals(std::string("caf\xC3\xA9"), message, __func__, __LINE__, "U+00E9 becomes its two UTF-8 bytes");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_Run_PropagatesUnexpectedRTLException()
{
    // Arrange
    TFixture_UnexpectedRTLException fixture;
    std::string caughtMessage;
    bool caught = false;

    // Act
    try
    {
        fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);
    }
    catch (System::Sysutils::EConvertError& ex)
    {
        caught = true;
        caughtMessage = RTLExceptionMessage(ex);
    }

    // Assert
    CheckTrue(caught, __func__, __LINE__, "the unexpected EConvertError escapes Run() as an EConvertError");
    CheckEquals(std::string("unexpected boom"), caughtMessage, __func__, __LINE__, "its message is intact");
    CheckEquals(1u, fixture.Results().FailedCount, __func__, __LINE__, "the throwing test counts as failed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_Run_WrapsUnexpectedRTLExceptionFromWorkerThread()
{
    // Arrange
    // A timeout runs the test on a worker thread. std::exception_ptr can't carry an RTL exception back from
    // it (see TTestGroupBase::RunWithTimeout()), so it arrives wrapped in a TExceptRTLException instead.
    TFixture_UnexpectedRTLException fixture;
    std::string caughtWhat;
    System::TClass caughtClass = nullptr;
    bool caught = false;

    // Act
    try
    {
        fixture.Run(TestFilter(), std::nullopt, 30u, false);
    }
    catch (TExceptRTLException const& ex)
    {
        caught = true;
        caughtWhat = ex.what();
        caughtClass = ex.RTLClass();
    }

    // Assert
    AssertTrue(caught, __func__, __LINE__, "the unexpected EConvertError escapes Run() as a TExceptRTLException");
    CheckEquals(std::string("EConvertError: unexpected boom"), caughtWhat, __func__, __LINE__,
        "what() is the DescribeRTLException() text");
    CheckTrue(caughtClass == __classid(System::Sysutils::EConvertError), __func__, __LINE__,
        "RTLClass() is the original exception's exact class");
    CheckEquals(1u, fixture.Results().FailedCount, __func__, __LINE__, "the throwing test counts as failed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessage()
{
    CheckFixtureOutcomes(std::nullopt, false, __func__, __LINE__);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessageOnWorkerThread()
{
    CheckFixtureOutcomes(30u, false, __func__, __LINE__);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_SetExceptionExpected_MatchesRTLTypeAndMessageUnderCatchCrashes()
{
    // The crash guard's handler must let Delphi's own raise mechanism (see RaisedFromDelphiCode) pass through.
    CheckFixtureOutcomes(std::nullopt, true, __func__, __LINE__);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_TExceptRTLException_KeepsClassAndDescriptionAfterOriginalIsFreed()
{
    // So main()'s std::exception handler reports it like any other unhandled exception.
    static_assert(std::is_base_of_v<std::runtime_error, TExceptRTLException>,
        "TExceptRTLException must be a std::runtime_error");

    // Arrange
    // RTL (Delphi-style) classes can only live on the heap.
    std::unique_ptr<System::Sysutils::EConvertError> original(new System::Sysutils::EConvertError(L"boom"));

    // Act
    TExceptRTLException const wrapped(*original);
    original.reset(); // The wrapper must not depend on the original object staying alive.

    // Assert
    CheckEquals(std::string("EConvertError: boom"), std::string(wrapped.what()), __func__, __LINE__,
        "what() is the DescribeRTLException() text");
    CheckTrue(wrapped.RTLClass() == __classid(System::Sysutils::EConvertError), __func__, __LINE__,
        "RTLClass() is the original exception's exact class");
    CheckTrue(wrapped.RTLClass()->InheritsFrom(__classid(System::Sysutils::Exception)), __func__, __LINE__,
        "RTLClass() supports a polymorphic base-class check");
    CheckFalse(wrapped.RTLClass()->InheritsFrom(__classid(System::Sysutils::EArgumentException)), __func__, __LINE__,
        "RTLClass() doesn't match an unrelated sibling class");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_RTLExceptions::Test_Throws_MatchesRTLTypeAndMessage()
{
    // Arrange
    TFixture_RTLThrowsChecks fixture;
    auto const findMessage = [&fixture](std::string const& testName)
        {
            for (TTestCaseRecord const& record : fixture.Results().CaseRecords)
            {
                if (record.TestName == testName)
                    return record.Message;
            }

            return std::string("(no record for " + testName + ")");
        };

    // Act
    fixture.Run(TestFilter(), std::nullopt, std::nullopt, false);

    // Assert
    TTestResults const& results = fixture.Results();
    CheckEquals(static_cast<size_t>(8), results.CaseRecords.size(), __func__, __LINE__,
        "one record per registered test");

    for (TTestCaseRecord const& record : results.CaseRecords)
    {
        if (NameEndsWith(record.TestName, "_Passes"))
            CheckEquals(TTestOutcome::Pass, record.Outcome, __func__, __LINE__, record.TestName + " should pass: " + record.Message);
        else if (NameEndsWith(record.TestName, "_Fails"))
            CheckEquals(TTestOutcome::Fail, record.Outcome, __func__, __LINE__, record.TestName + " should fail");
        else
            Fail(__func__, __LINE__, record.TestName + " name must end with _Passes or _Fails");
    }

    // An RTL exception is described as "ClassName: Message", like DescribeRTLException().
    CheckEndsWith(findMessage("CheckNoThrow_RTLExceptionThrown_Fails"),
        "): Expected no exception but caught: EConvertError: boom. must not throw", __func__, __LINE__,
        "CheckNoThrow shows the RTL exception's class and message");
    CheckEndsWith(findMessage("CheckThrows_MessageSubstringAbsent_Fails"),
        "): Expected the exception message to contain \"needle\" but caught: EConvertError: boom. "
        "message must contain 'needle'", __func__, __LINE__, "a message mismatch shows the RTL exception");
    CheckEndsWith(findMessage("CheckThrows_StdTypeExpected_RTLExceptionThrown_Fails"),
        "): Expected a different exception type but caught: EConvertError: boom. std type expected, RTL type thrown",
        __func__, __LINE__, "an RTL exception is a wrong type for a std type");
    CheckEndsWith(findMessage("CheckThrows_WrongSiblingType_Fails"),
        "): Expected a different exception type but caught: EArgumentException: boom. wrong sibling type thrown",
        __func__, __LINE__, "a wrong sibling type shows its class");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_RTLExceptions)

#endif // #if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)
