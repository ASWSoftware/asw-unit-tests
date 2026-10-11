/* **************************************************************************
Test_ASWUnitTests_AutoTest.h
Author: Anthony S. West - ASW Software

Unit tests for TAutoTestGroup and the ASW_TEST macro: self-registering
test methods, their order, and duplicate test name detection.

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
#ifndef Test_ASWUnitTests_AutoTestH
#define Test_ASWUnitTests_AutoTestH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_AutoTest
//
// Itself a TAutoTestGroup, so its own tests are registered with ASW_TEST.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_AutoTest : public TAutoTestGroup<TTest_ASWUnitTests_AutoTest>
{
private:
    typedef TAutoTestGroup<TTest_ASWUnitTests_AutoTest> inherited;

public: // Test methods, each defined with ASW_TEST
    void Test_AutoTests_AreRegisteredAlphabeticallyBeforeExplicitTests();
    void Test_AutoTests_AreRegisteredForEveryInstance();
    void Test_AutoTests_RunOnTheirOwnInstance();
    void Test_AutoTests_RunShuffled();
    void Test_AutoTests_RunWithFilter();
    void Test_DuplicateName_AutoAndExplicit_Throws();
    void Test_DuplicateName_ExplicitTwice_Throws();
    void Test_DuplicateName_RegisterTestCases_Throws();

public:
    TTest_ASWUnitTests_AutoTest();
    ~TTest_ASWUnitTests_AutoTest() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_AutoTestH
