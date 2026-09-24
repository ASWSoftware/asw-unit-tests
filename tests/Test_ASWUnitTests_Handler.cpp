/* **************************************************************************
Test_ASWUnitTests_Handler.cpp
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
#include "Test_ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Handler
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Handler::TTest_ASWUnitTests_Handler()
    : inherited("ASWUnitTests_Handler_Tests")
{
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
