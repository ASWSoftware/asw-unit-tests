/* **************************************************************************
UT_Helper_ExceptionTypes.h
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

#ifndef UT_Helper_ExceptionTypesH
#define UT_Helper_ExceptionTypesH
#include "ASWUnitTests_TestBase.h"

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TFixture_AnyTypeExceptionExpectations
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group expecting, with SetExceptionExpected<TException>(),
// types that aren't std::exception classes: int, a C string pointer, classes of its own (polymorphic or not), and a
// mixin base of a std::exception subclass. Each is thrown, not thrown, crossed with another kind, or expected with a
// message, so Test_SetExceptionExpected_MatchesAnyType (Test_ASWUnitTests_TestBase.cpp) can check the outcomes and
// messages. Test names end with "_Passes" or "_Fails", the expected outcome.
/////////////////////////////////////////////////////////////////////////////
class TFixture_AnyTypeExceptionExpectations : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_CheckFailedThenIntThrown_Fails();
    void Test_ClassExpected_MessageExpected_Fails();
    void Test_ClassExpected_SubclassThrown_Passes();
    void Test_ClassExpected_SubclassThrownMessageExpected_Fails();
    void Test_IntExpected_AssertFails_Fails();
    void Test_IntExpected_DoubleThrown_Fails();
    void Test_IntExpected_IntThrown_Passes();
    void Test_IntExpected_MessageExpected_Fails();
    void Test_IntExpected_NoneThrown_Fails();
    void Test_IntExpected_StdExceptionThrown_Fails();
    void Test_IntThenGenericExpected_DoubleThrown_Passes();
    void Test_IntThenStdTypeExpected_IntThrown_Fails();
    void Test_MixinExpected_StdExceptionWithoutMixinThrown_Fails();
    void Test_MixinExpected_TaggedErrorMessageAbsent_Fails();
    void Test_MixinExpected_TaggedErrorThrown_Passes();
    void Test_PointerExpected_StringLiteralThrown_Passes();
    void Test_StdTypeExpected_IntThrown_Fails();

public:
    TFixture_AnyTypeExceptionExpectations();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_AnyTypeThrowsChecks
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group calling AssertThrows()/CheckThrows() with the same
// kinds of types as TFixture_AnyTypeExceptionExpectations, so Test_Throws_MatchesAnyType
// (Test_ASWUnitTests_TestBase.cpp) can check the outcomes and messages. Test names end with "_Passes" or "_Fails",
// the expected outcome.
/////////////////////////////////////////////////////////////////////////////
class TFixture_AnyTypeThrowsChecks : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_AssertThrows_IntMessageExpected_Fails();
    void Test_AssertThrows_IntThrown_Passes();
    void Test_CheckThrows_ClassSubclassMessageExpected_Fails();
    void Test_CheckThrows_ClassSubclassThrown_Passes();
    void Test_CheckThrows_IntExpectedAssertFailsInside_Fails();
    void Test_CheckThrows_IntExpectedDoubleThrown_Fails();
    void Test_CheckThrows_IntExpectedStdExceptionThrown_Fails();
    void Test_CheckThrows_MixinMessageAbsent_Fails();
    void Test_CheckThrows_MixinMessagePresent_Passes();
    void Test_CheckThrows_PlainClassMessageExpected_Fails();
    void Test_CheckThrows_PointerStringLiteralThrown_Passes();
    void Test_CheckThrows_StdStringMessageExpected_Fails();

public:
    TFixture_AnyTypeThrowsChecks();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef UT_Helper_ExceptionTypesH
