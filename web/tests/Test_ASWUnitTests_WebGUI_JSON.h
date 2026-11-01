/* **************************************************************************
Test_ASWUnitTests_WebGUI_JSON.h
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
#ifndef Test_ASWUnitTests_WebGUI_JSONH
#define Test_ASWUnitTests_WebGUI_JSONH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_JSON
//
// Exercises the Web GUI runner's JSON writer
// (web/gui/src/ASWUnitTests_WebGUI_JSON.h). Listed in every project, so it
// also runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_WebGUI_JSON : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_RepairWebGUIUTF8_KeepsValidText();
    void Test_RepairWebGUIUTF8_ReplacesEachMaximalSubpart();
    void Test_WebGUIJSONString_EscapesQuotesBackslashesAndControls();
    void Test_Writer_NestsValuesWithCommas();
    void Test_Writer_RejectsMisuse();
    void Test_Writer_WritesIntegerLimits();

public:
    TTest_ASWUnitTests_WebGUI_JSON();
    ~TTest_ASWUnitTests_WebGUI_JSON() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_WebGUI_JSONH
