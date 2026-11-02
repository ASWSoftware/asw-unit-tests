/* **************************************************************************
Test_ASWUnitTests_WebGUI_API.h
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
#ifndef Test_ASWUnitTests_WebGUI_APIH
#define Test_ASWUnitTests_WebGUI_APIH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_API
//
// Exercises the Web GUI runner's API (web/gui/src/ASWUnitTests_WebGUI_API.h).
// Listed in every project, so it also runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_WebGUI_API : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_CommandStates_AsIfANodeWereSelected();
    void Test_DetailAndNodeIds_NameEachNode();
    void Test_Log_AppendKeepsWholeLines();
    void Test_Log_EventSendsLinesFromAnIndex();
    void Test_SnapshotEvent_HoldsEverything();
    void Test_StateEvent_CommandsPanelsProgressShuffleTheme();
    void Test_TestEvents_StatusOfEachNodeTheTestChanges();
    void Test_TreeAndChecksEvents_FollowTheTestList();

public:
    TTest_ASWUnitTests_WebGUI_API();
    ~TTest_ASWUnitTests_WebGUI_API() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_WebGUI_APIH
