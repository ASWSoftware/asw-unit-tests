/* **************************************************************************
Test_ASWUnitTests_GUI_Palette.h
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
#ifndef Test_ASWUnitTests_GUI_PaletteH
#define Test_ASWUnitTests_GUI_PaletteH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Palette
//
// Exercises the GUI runners' dark theme colors
// (gui/src/ASWUnitTests_GUI_Palette.h). Compiled by every console runner,
// so it runs without a GUI, on Linux too.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_Palette : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_BlendGUIColors_MixesEachChannelInProportion();
    void Test_CarbonGUIPalette_HasCarbonsColors();
    void Test_CarbonGUIThemeFileText_ReadsBackAsCarbon();
    void Test_FindGUIBuiltInDarkTheme_IgnoresCase();
    void Test_GUIBuiltInDarkThemes_AreCarbonThenItsTints();
    void Test_GUIBuiltInDarkThemes_KeepTextReadable();
    void Test_GUIFailureAndSkipTextColorOn_AreLighterOnADarkBackground();
    void Test_GUIPaletteRoles_ListEveryMemberOnce();
    void Test_GUIPaletteWithThemeColors_SetsOnlyTheirRoles();
    void Test_GUIRGB_PacksTheChannelsAsRRGGBB();
    void Test_IsDarkGUIColor_IsByTheLuma();
    void Test_TintedCarbonGUIPalette_TintsOnlyTheGraysAndTheAccent();

public:
    TTest_ASWUnitTests_GUI_Palette();
    ~TTest_ASWUnitTests_GUI_Palette() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_PaletteH
