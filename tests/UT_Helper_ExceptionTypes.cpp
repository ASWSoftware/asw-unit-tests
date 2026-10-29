/* **************************************************************************
UT_Helper_ExceptionTypes.cpp
Author: Anthony S. West - ASW Software

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
#include "UT_Helper_ExceptionTypes.h"
//---------------------------------------------------------------------------
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------

namespace
{

/////////////////////////////////////////////////////////////////////////////
// TFixtureErrorTag
//
// A polymorphic class not derived from std::exception, used as a mixin base of a std::exception subclass
// (TFixtureTaggedError), so expecting it matches that subclass and finds its what().
/////////////////////////////////////////////////////////////////////////////
class TFixtureErrorTag
{
public:
    virtual ~TFixtureErrorTag() = default;
};


/////////////////////////////////////////////////////////////////////////////
// TFixtureNonStdError
//
// A polymorphic exception class not derived from std::exception, with a subclass, TFixtureNonStdSubError, so a
// failure shows the dynamic type of what was caught.
/////////////////////////////////////////////////////////////////////////////
class TFixtureNonStdError
{
public:
    virtual ~TFixtureNonStdError() = default;
};


/////////////////////////////////////////////////////////////////////////////
// TFixtureNonStdSubError
/////////////////////////////////////////////////////////////////////////////
class TFixtureNonStdSubError : public TFixtureNonStdError
{
};


/////////////////////////////////////////////////////////////////////////////
// TFixturePlainError
//
// An exception class with no virtual functions, so nothing but its type is known about a caught one.
/////////////////////////////////////////////////////////////////////////////
struct TFixturePlainError
{
    int Code;
};


/////////////////////////////////////////////////////////////////////////////
// TFixtureTaggedError
/////////////////////////////////////////////////////////////////////////////
class TFixtureTaggedError : public std::runtime_error, public TFixtureErrorTag
{
public:
    explicit TFixtureTaggedError(std::string const& msg)
        : std::runtime_error(msg)
    {
    }
};

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TFixture_AnyTypeExceptionExpectations
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_AnyTypeExceptionExpectations::TFixture_AnyTypeExceptionExpectations()
    : inherited("Fixture_AnyTypeExceptionExpectations")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_CheckFailedThenIntThrown_Fails,
        "CheckFailedThenIntThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_MessageExpected_Fails,
        "ClassExpected_MessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_SubclassThrown_Passes,
        "ClassExpected_SubclassThrown_Passes");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_SubclassThrownMessageExpected_Fails,
        "ClassExpected_SubclassThrownMessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_AssertFails_Fails,
        "IntExpected_AssertFails_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_DoubleThrown_Fails,
        "IntExpected_DoubleThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_IntThrown_Passes,
        "IntExpected_IntThrown_Passes");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_MessageExpected_Fails,
        "IntExpected_MessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_NoneThrown_Fails,
        "IntExpected_NoneThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntExpected_StdExceptionThrown_Fails,
        "IntExpected_StdExceptionThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntThenGenericExpected_DoubleThrown_Passes,
        "IntThenGenericExpected_DoubleThrown_Passes");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_IntThenStdTypeExpected_IntThrown_Fails,
        "IntThenStdTypeExpected_IntThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_StdExceptionWithoutMixinThrown_Fails,
        "MixinExpected_StdExceptionWithoutMixinThrown_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_TaggedErrorMessageAbsent_Fails,
        "MixinExpected_TaggedErrorMessageAbsent_Fails");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_TaggedErrorThrown_Passes,
        "MixinExpected_TaggedErrorThrown_Passes");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_PointerExpected_StringLiteralThrown_Passes,
        "PointerExpected_StringLiteralThrown_Passes");
    RegisterTest(&TFixture_AnyTypeExceptionExpectations::Test_StdTypeExpected_IntThrown_Fails,
        "StdTypeExpected_IntThrown_Fails");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_CheckFailedThenIntThrown_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected, then a failed Check");
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure before the expected exception");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_MessageExpected_Fails()
{
    SetExceptionExpected<TFixturePlainError>(__func__, __LINE__, "a class with no message", "needle");
    throw TFixturePlainError{ 1 };
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_SubclassThrown_Passes()
{
    SetExceptionExpected<TFixtureNonStdError>(__func__, __LINE__, "base class expected, subclass thrown");
    throw TFixtureNonStdSubError();
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_ClassExpected_SubclassThrownMessageExpected_Fails()
{
    SetExceptionExpected<TFixtureNonStdError>(__func__, __LINE__, "a subclass with no message", "needle");
    throw TFixtureNonStdSubError();
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_AssertFails_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected, then a failed Assert");
    AssertTrue(false, __func__, __LINE__, "deliberate Assert failure while an int is expected");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_DoubleThrown_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected, double thrown");
    throw 4.2;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_IntThrown_Passes()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected and thrown");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_MessageExpected_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "an int has no message", "needle");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_NoneThrown_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "expected an int that never comes");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntExpected_StdExceptionThrown_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected, std::exception thrown");
    throw std::runtime_error("boom");
}
//---------------------------------------------------------------------------
/*
    TFixture_AnyTypeExceptionExpectations::Test_IntThenGenericExpected_DoubleThrown_Passes

    This and the next replace an expectation of int with another, which must replace its type check too.
*/
void TFixture_AnyTypeExceptionExpectations::Test_IntThenGenericExpected_DoubleThrown_Passes()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected first");
    SetExceptionExpected(true, __func__, __LINE__, "then any exception");
    throw 4.2;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_IntThenStdTypeExpected_IntThrown_Fails()
{
    SetExceptionExpected<int>(__func__, __LINE__, "int expected first");
    SetExceptionExpected<std::runtime_error>(__func__, __LINE__, "then a std::runtime_error");
    throw 42;
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_StdExceptionWithoutMixinThrown_Fails()
{
    SetExceptionExpected<TFixtureErrorTag>(__func__, __LINE__, "mixin expected, a std::exception without it thrown");
    throw std::runtime_error("boom");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_TaggedErrorMessageAbsent_Fails()
{
    SetExceptionExpected<TFixtureErrorTag>(__func__, __LINE__, "the subclass's what() is checked", "needle");
    throw TFixtureTaggedError("boom");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_MixinExpected_TaggedErrorThrown_Passes()
{
    SetExceptionExpected<TFixtureErrorTag>(__func__, __LINE__, "the subclass's what() is checked", "needle");
    throw TFixtureTaggedError("hay needle stack");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_PointerExpected_StringLiteralThrown_Passes()
{
    SetExceptionExpected<char const*>(__func__, __LINE__, "a string literal is thrown as a char const*");
    throw "boom";
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeExceptionExpectations::Test_StdTypeExpected_IntThrown_Fails()
{
    SetExceptionExpected<std::runtime_error>(__func__, __LINE__, "std::runtime_error expected, int thrown");
    throw 42;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TFixture_AnyTypeThrowsChecks
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TFixture_AnyTypeThrowsChecks::TFixture_AnyTypeThrowsChecks()
    : inherited("Fixture_AnyTypeThrowsChecks")
{
    SetLogSuppressed(true);

    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_AssertThrows_IntMessageExpected_Fails,
        "AssertThrows_IntMessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_AssertThrows_IntThrown_Passes, "AssertThrows_IntThrown_Passes");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_ClassSubclassMessageExpected_Fails,
        "CheckThrows_ClassSubclassMessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_ClassSubclassThrown_Passes,
        "CheckThrows_ClassSubclassThrown_Passes");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedAssertFailsInside_Fails,
        "CheckThrows_IntExpectedAssertFailsInside_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedDoubleThrown_Fails,
        "CheckThrows_IntExpectedDoubleThrown_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedStdExceptionThrown_Fails,
        "CheckThrows_IntExpectedStdExceptionThrown_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_MixinMessageAbsent_Fails,
        "CheckThrows_MixinMessageAbsent_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_MixinMessagePresent_Passes,
        "CheckThrows_MixinMessagePresent_Passes");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_PlainClassMessageExpected_Fails,
        "CheckThrows_PlainClassMessageExpected_Fails");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_PointerStringLiteralThrown_Passes,
        "CheckThrows_PointerStringLiteralThrown_Passes");
    RegisterTest(&TFixture_AnyTypeThrowsChecks::Test_CheckThrows_StdStringMessageExpected_Fails,
        "CheckThrows_StdStringMessageExpected_Fails");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_AssertThrows_IntMessageExpected_Fails()
{
    AssertThrows<int>([] {
            throw 42;
        }, __func__, __LINE__, "an int has no message", "needle");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_AssertThrows_IntThrown_Passes()
{
    AssertThrows<int>([] {
            throw 42;
        }, __func__, __LINE__, "int expected and thrown");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_ClassSubclassMessageExpected_Fails()
{
    CheckThrows<TFixtureNonStdError>([] {
            throw TFixtureNonStdSubError();
        }, __func__, __LINE__, "a subclass with no message", "needle");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_ClassSubclassThrown_Passes()
{
    CheckThrows<TFixtureNonStdError>([] {
            throw TFixtureNonStdSubError();
        }, __func__, __LINE__, "base class expected, subclass thrown");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedAssertFailsInside_Fails()
{
    CheckThrows<int>([this] {
            AssertTrue(false, __func__, __LINE__, "inner assert");
        }, __func__, __LINE__, "an Assert failure isn't the int being checked for");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedDoubleThrown_Fails()
{
    CheckThrows<int>([] {
            throw 4.2;
        }, __func__, __LINE__, "int expected, double thrown");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_IntExpectedStdExceptionThrown_Fails()
{
    CheckThrows<int>([] {
            throw std::runtime_error("boom");
        }, __func__, __LINE__, "int expected, std::exception thrown");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_MixinMessageAbsent_Fails()
{
    CheckThrows<TFixtureErrorTag>([] {
            throw TFixtureTaggedError("boom");
        }, __func__, __LINE__, "the subclass's what() is checked", "needle");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_MixinMessagePresent_Passes()
{
    CheckThrows<TFixtureErrorTag>([] {
            throw TFixtureTaggedError("hay needle stack");
        }, __func__, __LINE__, "the subclass's what() is checked", "needle");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_PlainClassMessageExpected_Fails()
{
    CheckThrows<TFixturePlainError>([] {
            throw TFixturePlainError{ 1 };
        }, __func__, __LINE__, "a class with no message", "needle");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_PointerStringLiteralThrown_Passes()
{
    CheckThrows<char const*>([] {
            throw "boom";
        }, __func__, __LINE__, "a string literal is thrown as a char const*");
}
//---------------------------------------------------------------------------
void TFixture_AnyTypeThrowsChecks::Test_CheckThrows_StdStringMessageExpected_Fails()
{
    // A std::string isn't a std::exception, so even one holding the text has no message.
    CheckThrows<std::string>([] {
            throw std::string("needle");
        }, __func__, __LINE__, "a std::string has no message", "needle");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
