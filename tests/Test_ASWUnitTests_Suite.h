/* **************************************************************************
Test_ASWUnitTests_Suite.h
Author: Anthony S. West - ASW Software

Unit tests for test suites: TSuitePath, a group's suite path and qualified
name, and how they reach records, filters, observers and the handler.

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
#ifndef Test_ASWUnitTests_SuiteH
#define Test_ASWUnitTests_SuiteH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Suite
//
// Itself in a nested suite, "Suites.Nested", so the real self-registered
// run includes a group in a suite, and the Handler_* tests can find it.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_Suite : public TAutoTestGroup<TTest_ASWUnitTests_Suite>
{
private:
    typedef TAutoTestGroup<TTest_ASWUnitTests_Suite> inherited;

public: // Test methods, each defined with ASW_TEST
    void Test_AutoTestGroup_InSuite();
    void Test_DuplicateName_ReportsQualifiedName();
    void Test_GroupInSuite_FilterMatchesQualifiedName();
    void Test_GroupInSuite_ReportsQualifiedName();
    void Test_GroupWithoutSuite_KeepsItsOwnName();
    void Test_Handler_ListsGroupUnderItsSuite();
    void Test_Handler_RunsSuiteByFilter();
    void Test_ITestGroup_DefaultsToNoSuite();
    void Test_TSuitePath_RejectsEmptyNames();
    void Test_TSuitePath_SplitsNames();

public:
    TTest_ASWUnitTests_Suite();
    ~TTest_ASWUnitTests_Suite() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_SuiteH
