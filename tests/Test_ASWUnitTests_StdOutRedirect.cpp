/* **************************************************************************
Test_ASWUnitTests_StdOutRedirect.cpp
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
#include "Test_ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <iostream>
//---------------------------------------------------------------------------
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_StdOutRedirect
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_StdOutRedirect::TTest_ASWUnitTests_StdOutRedirect()
    : inherited("ASWUnitTests_StdOutRedirect_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_StdOutRedirect::Test_Capture_ResumesCapturingWithFreshBuffer,
        "Capture_ResumesCapturingWithFreshBuffer");
    RegisterTest(&TTest_ASWUnitTests_StdOutRedirect::Test_Restore_StopsCapturingAndIsIdempotent,
        "Restore_StopsCapturingAndIsIdempotent");
    RegisterTest(&TTest_ASWUnitTests_StdOutRedirect::Test_Str_CapturesWrittenOutput, "Str_CapturesWrittenOutput");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_StdOutRedirect::~TTest_ASWUnitTests_StdOutRedirect()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::Test_Capture_ResumesCapturingWithFreshBuffer()
{
    // Arrange
    // 'outer' catches whatever escapes while 'inner' is restored below.
    TStdOutRedirect outer;
    TStdOutRedirect inner;

    // Act
    std::cout << "first-capture";
    std::string const firstCaptured = inner.Str();

    inner.Restore();
    std::cout << "escapes-to-outer";

    inner.Capture();
    std::cout << "second-capture";
    std::string const secondCaptured = inner.Str();

    inner.Capture(); // Already capturing: must be a harmless no-op, not clear "second-capture" again.
    std::string const afterRedundantCapture = inner.Str();

    // Assert
    CheckEquals(std::string("first-capture"), firstCaptured, __func__, __LINE__, "first capture session's content");
    CheckEquals(std::string("second-capture"), secondCaptured, __func__, __LINE__,
        "Capture() after Restore() starts a fresh buffer, discarding the first session's content");
    CheckEquals(std::string("second-capture"), afterRedundantCapture, __func__, __LINE__,
        "calling Capture() while already capturing does not clear the buffer");
    CheckTrue(outer.Str().find("escapes-to-outer") != std::string::npos, __func__, __LINE__,
        "text written between Restore() and Capture() reached the real stream");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::Test_Restore_StopsCapturingAndIsIdempotent()
{
    // Arrange
    // 'outer' catches whatever escapes once 'inner' restores below, so this test never prints
    // anything to the real console while proving Restore() actually hands std::cout back.
    TStdOutRedirect outer;
    std::string innerCaptured;

    // Act
    {
        TStdOutRedirect inner;
        std::cout << "captured-by-inner";
        inner.Restore();
        std::cout << "not-captured-by-inner";
        inner.Restore(); // Idempotent: a second call must not misbehave (e.g. restore something else).
        innerCaptured = inner.Str();
    } // inner's destructor calls Restore() a third time (already restored) - must also be harmless.

    std::cout << "after-inner-destroyed";
    std::string const outerCaptured = outer.Str();

    // Assert
    CheckEquals(std::string("captured-by-inner"), innerCaptured, __func__, __LINE__,
        "only text written before Restore() was captured");
    CheckTrue(outerCaptured.find("not-captured-by-inner") != std::string::npos, __func__, __LINE__,
        "text written after Restore() reached the real stream (here, outer's capture) instead");
    CheckTrue(outerCaptured.find("after-inner-destroyed") != std::string::npos, __func__, __LINE__,
        "text written after inner's destructor still reaches the real stream (its Restore() was harmless)");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_StdOutRedirect::Test_Str_CapturesWrittenOutput()
{
    // Arrange
    TStdOutRedirect redirect;

    // Act
    std::cout << "hello " << 42;

    // Assert
    CheckEquals(std::string("hello 42"), redirect.Str(), __func__, __LINE__, "captures exactly what was written");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_StdOutRedirect)
