/* **************************************************************************
Test_ASWUnitTests_WebGUI_Security.h
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
#ifndef Test_ASWUnitTests_WebGUI_SecurityH
#define Test_ASWUnitTests_WebGUI_SecurityH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_Security
//
// Exercises the Web GUI runner's security checks
// (web/gui/src/ASWUnitTests_WebGUI_Security.h). Listed in every project, so
// it also runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_WebGUI_Security : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_CookieValues_FindsEveryCopyOfACookie();
    void Test_DecideAccess_ServesOnlyThisRunnersPage();
    void Test_HostAndOrigin_NameThisRunner();
    void Test_Responses_RedirectAndRefuse();
    void Test_Token_FormatAndConstantTimeCompare();

public:
    TTest_ASWUnitTests_WebGUI_Security();
    ~TTest_ASWUnitTests_WebGUI_Security() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_WebGUI_SecurityH
