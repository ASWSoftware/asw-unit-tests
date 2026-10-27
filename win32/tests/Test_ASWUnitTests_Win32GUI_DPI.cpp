/* **************************************************************************
Test_ASWUnitTests_Win32GUI_DPI.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
#include "ASWUnitTests_Win32GUI_Windows.h"
//---------------------------------------------------------------------------
// Module header
#include "Test_ASWUnitTests_Win32GUI_DPI.h"
//---------------------------------------------------------------------------
#include <string>
#include <utility>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_DPI.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

LOGFONTW FontDescription(HFONT font);
bool IsFontHandle(HFONT font);
LOGFONTW MessageFont(UINT dpi);

//---------------------------------------------------------------------------
LOGFONTW FontDescription(HFONT font)
{
    LOGFONTW description = {};
    GetObjectW(font, sizeof(description), &description);
    return description;
}
//---------------------------------------------------------------------------
/*
    IsFontHandle

    Whether 'font' is a font that hasn't been deleted.
*/
bool IsFontHandle(HFONT font)
{
    return GetObjectType(font) == OBJ_FONT;
}
//---------------------------------------------------------------------------
LOGFONTW MessageFont(UINT dpi)
{
    NONCLIENTMETRICSW metrics = {};
    metrics.cbSize = sizeof(metrics);
    SystemParametersInfoForDpi(SPI_GETNONCLIENTMETRICS, sizeof(metrics), &metrics, 0, dpi);
    return metrics.lfMessageFont;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_DPI
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_DPI::TTest_ASWUnitTests_Win32GUI_DPI()
    : inherited("ASWUnitTests_Win32GUI_DPI_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DPI::Test_CreateMonospaceFont_IsConsolas10PointForTheDPI,
        "CreateMonospaceFont_IsConsolas10PointForTheDPI");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DPI::Test_CreateUIFont_IsTheMessageFontForTheDPI,
        "CreateUIFont_IsTheMessageFontForTheDPI");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DPI::Test_ScaleForDPI_RoundsToTheNearestPixel,
        "ScaleForDPI_RoundsToTheNearestPixel");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DPI::Test_TWin32GUIFont_MoveTransfersOwnership,
        "TWin32GUIFont_MoveTransfersOwnership");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_DPI::Test_WindowDPI_IsTheDefaultWithoutAWindow,
        "WindowDPI_IsTheDefaultWithoutAWindow");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_DPI::~TTest_ASWUnitTests_Win32GUI_DPI()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::Test_CreateMonospaceFont_IsConsolas10PointForTheDPI()
{
    // Act
    TWin32GUIFont const at96 = CreateMonospaceFont(96);
    TWin32GUIFont const at144 = CreateMonospaceFont(144);
    TWin32GUIFont const at192 = CreateMonospaceFont(192);

    // Assert: 10 pt is 13.3 pixels at 96 DPI, 20 at 144 and 26.7 at 192; a negative height is the character
    // height, without the internal leading.
    CheckEquals(std::wstring(L"Consolas"), std::wstring(FontDescription(at96.Get()).lfFaceName), __func__, __LINE__,
        "Consolas");
    CheckEquals(-13, FontDescription(at96.Get()).lfHeight, __func__, __LINE__, "at 96 DPI");
    CheckEquals(-20, FontDescription(at144.Get()).lfHeight, __func__, __LINE__, "at 144 DPI");
    CheckEquals(-27, FontDescription(at192.Get()).lfHeight, __func__, __LINE__, "at 192 DPI");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::Test_CreateUIFont_IsTheMessageFontForTheDPI()
{
    // Arrange
    LOGFONTW const expected96 = MessageFont(96);
    LOGFONTW const expected192 = MessageFont(192);

    // Act
    TWin32GUIFont const at96 = CreateUIFont(96);
    TWin32GUIFont const at192 = CreateUIFont(192);

    // Assert
    CheckEquals(std::wstring(expected96.lfFaceName), std::wstring(FontDescription(at96.Get()).lfFaceName), __func__,
        __LINE__, "the message font's face");
    CheckEquals(expected96.lfHeight, FontDescription(at96.Get()).lfHeight, __func__, __LINE__, "its height at 96 DPI");
    CheckEquals(expected192.lfHeight, FontDescription(at192.Get()).lfHeight, __func__, __LINE__,
        "its height at 192 DPI");
    CheckNotEquals(expected96.lfHeight, expected192.lfHeight, __func__, __LINE__, "which differ");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::Test_ScaleForDPI_RoundsToTheNearestPixel()
{
    // Act, Assert
    CheckEquals(100, ScaleForDPI(100, 96), __func__, __LINE__, "unchanged at 96 DPI");
    CheckEquals(150, ScaleForDPI(100, 144), __func__, __LINE__, "150% at 144 DPI");
    CheckEquals(9, ScaleForDPI(5, 168), __func__, __LINE__, "8.75 rounds up");
    CheckEquals(6, ScaleForDPI(5, 120), __func__, __LINE__, "6.25 rounds down");
    CheckEquals(3, ScaleForDPI(2, 120), __func__, __LINE__, "2.5 rounds away from zero");
    CheckEquals(-20, ScaleForDPI(-13, 144), __func__, __LINE__, "-19.5 rounds away from zero");
    CheckEquals(0, ScaleForDPI(0, 192), __func__, __LINE__, "zero stays zero");
}
//---------------------------------------------------------------------------
/*
    Test_TWin32GUIFont_MoveTransfersOwnership

    Whether a font the owner deleted is gone can't be checked: GDI keeps recently deleted fonts' handles in a
    cache, where GetObjectType() can still find them.
*/
void TTest_ASWUnitTests_Win32GUI_DPI::Test_TWin32GUIFont_MoveTransfersOwnership()
{
    // Arrange
    TWin32GUIFont source = CreateMonospaceFont(96);
    HFONT const font = source.Get();
    TWin32GUIFont assigned;

    // Act
    TWin32GUIFont moved(std::move(source));
    assigned = std::move(moved);

    // Assert
    CheckTrue(assigned.Get() == font, __func__, __LINE__, "the last owner has the font");
    CheckNull(source.Get(), __func__, __LINE__, "the first owner has none");
    CheckNull(moved.Get(), __func__, __LINE__, "nor has the one it was moved through");
    CheckTrue(IsFontHandle(font), __func__, __LINE__, "and the font still exists");
    CheckNull(TWin32GUIFont().Get(), __func__, __LINE__, "an empty owner has no font");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_DPI::Test_WindowDPI_IsTheDefaultWithoutAWindow()
{
    // Act, Assert
    CheckEquals(96u, WindowDPI(nullptr), __func__, __LINE__, "96 DPI when there's no window to ask");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_DPI)
