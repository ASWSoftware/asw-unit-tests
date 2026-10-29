/* **************************************************************************
Test_ASWUnitTests_GUI_Theme.h
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
#ifndef Test_ASWUnitTests_GUI_ThemeH
#define Test_ASWUnitTests_GUI_ThemeH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Theme
//
// Exercises the GUI runners' theme choice
// (gui/src/ASWUnitTests_GUI_Theme.h). Listed in every project, so it also
// runs from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_Theme : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_GUIThemeIsDark_FollowsTheChoiceOrWindows();
    void Test_LoadGUITheme_DefaultsToWindows();
    void Test_LoadGUITheme_ReadsEachThemeIgnoringCase();
    void Test_SaveGUITheme_KeepsTheLayout();
    void Test_SaveGUITheme_RoundTrips();

public:
    TTest_ASWUnitTests_GUI_Theme();
    ~TTest_ASWUnitTests_GUI_Theme() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_ThemeH
