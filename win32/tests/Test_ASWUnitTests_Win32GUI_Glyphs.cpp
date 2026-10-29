/* **************************************************************************
Test_ASWUnitTests_Win32GUI_Glyphs.cpp
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
// Module header
#include "Test_ASWUnitTests_Win32GUI_Glyphs.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Win32GUI_Glyphs.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

//---------------------------------------------------------------------------

int At(std::vector<std::uint8_t> const& coverage, int size, int x, int y);
bool IsMirrored(std::vector<std::uint8_t> const& coverage, int size, bool leftToRight);
int Total(std::vector<std::uint8_t> const& coverage);

//---------------------------------------------------------------------------
int At(std::vector<std::uint8_t> const& coverage, int size, int x, int y)
{
    return coverage[static_cast<std::size_t>(y) * static_cast<std::size_t>(size) + static_cast<std::size_t>(x)];
}
//---------------------------------------------------------------------------
/*
    IsMirrored

    Whether the glyph is the same mirrored left to right (or else top to bottom).
*/
bool IsMirrored(std::vector<std::uint8_t> const& coverage, int size, bool leftToRight)
{
    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            int const mirrored = leftToRight ? At(coverage, size, size - 1 - x, y) : At(coverage, size, x, size - 1 - y);

            if (At(coverage, size, x, y) != mirrored)
                return false;
        }
    }

    return true;
}
//---------------------------------------------------------------------------
int Total(std::vector<std::uint8_t> const& coverage)
{
    int total = 0;

    for (std::uint8_t const value : coverage)
        total += value;

    return total;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Win32GUI_Glyphs
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_Glyphs::TTest_ASWUnitTests_Win32GUI_Glyphs()
    : inherited("ASWUnitTests_Win32GUI_Glyphs_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_ArrowPointsRight,
        "RenderWin32GUIGlyph_ArrowPointsRight");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CirclesAreRoundAndNested,
        "RenderWin32GUIGlyph_CirclesAreRoundAndNested");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CoversOnePixelPerValue,
        "RenderWin32GUIGlyph_CoversOnePixelPerValue");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CrossIsCarbonsCheckMark,
        "RenderWin32GUIGlyph_CrossIsCarbonsCheckMark");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_IsEmptyWithoutASize,
        "RenderWin32GUIGlyph_IsEmptyWithoutASize");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_ScalesWithTheSize,
        "RenderWin32GUIGlyph_ScalesWithTheSize");
    RegisterTest(&TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_SquareAndDotAreInTheMiddle,
        "RenderWin32GUIGlyph_SquareAndDotAreInTheMiddle");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Win32GUI_Glyphs::~TTest_ASWUnitTests_Win32GUI_Glyphs()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_ArrowPointsRight()
{
    // Act: the menus' 16-pixel cell, the triangle's flat side at pixel 6 and its point at 10.
    std::vector<std::uint8_t> const arrow = RenderWin32GUIGlyph(EWin32GUIGlyph::ArrowRight, 16);

    std::vector<std::uint8_t> const left = RenderWin32GUIGlyph(EWin32GUIGlyph::ArrowLeft, 16);

    bool mirrored = true;

    for (int y = 0; y < 16; ++y)
    {
        for (int x = 0; x < 16; ++x)
            mirrored = mirrored && At(left, 16, x, y) == At(arrow, 16, 15 - x, y);
    }

    // Assert
    CheckTrue(IsMirrored(arrow, 16, false), __func__, __LINE__, "the same above and below its middle");
    CheckEquals(255, At(arrow, 16, 6, 8), __func__, __LINE__, "full at its flat side, in the middle");
    CheckEquals(0, At(arrow, 16, 5, 8), __func__, __LINE__, "nothing left of it");
    CheckEquals(0, At(arrow, 16, 11, 8), __func__, __LINE__, "nothing right of its point");
    CheckTrue(At(arrow, 16, 6, 5) > At(arrow, 16, 9, 5), __func__, __LINE__, "narrowing to the right");
    CheckTrue(mirrored, __func__, __LINE__, "the left arrow is the right one mirrored");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CirclesAreRoundAndNested()
{
    // Act: a radio button's circle and its inside, at Carbon's 13 pixels.
    std::vector<std::uint8_t> const circle = RenderWin32GUIGlyph(EWin32GUIGlyph::Circle, 13);
    std::vector<std::uint8_t> const inner = RenderWin32GUIGlyph(EWin32GUIGlyph::InnerCircle, 13);

    bool nested = true;

    for (std::size_t i = 0; i < inner.size(); ++i)
        nested = nested && inner[i] <= circle[i];

    // Assert
    CheckTrue(IsMirrored(circle, 13, true) && IsMirrored(circle, 13, false), __func__, __LINE__,
        "the circle is symmetric");
    CheckEquals(255, At(circle, 13, 6, 6), __func__, __LINE__, "the circle's middle");
    CheckEquals(255, At(circle, 13, 0, 6), __func__, __LINE__, "the circle reaches the cell's edge");
    CheckEquals(0, At(circle, 13, 0, 0), __func__, __LINE__, "the circle leaves the corners");
    CheckEquals(0, At(inner, 13, 0, 6), __func__, __LINE__, "the inside is a pixel in from the edge");
    CheckEquals(255, At(inner, 13, 1, 6), __func__, __LINE__, "the inside starts a pixel in");
    CheckTrue(nested, __func__, __LINE__, "the inside is within the circle");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CoversOnePixelPerValue()
{
    // Arrange
    EWin32GUIGlyph const glyphs[] = {
        EWin32GUIGlyph::ArrowLeft, EWin32GUIGlyph::ArrowRight, EWin32GUIGlyph::Circle, EWin32GUIGlyph::Cross, EWin32GUIGlyph::Dot,
        EWin32GUIGlyph::InnerCircle, EWin32GUIGlyph::Square, EWin32GUIGlyph::Tick,
    };

    for (EWin32GUIGlyph const glyph : glyphs)
    {
        // Act
        std::vector<std::uint8_t> const coverage = RenderWin32GUIGlyph(glyph, 13);

        // Assert
        std::string const name = "glyph " + std::to_string(static_cast<int>(glyph));
        CheckEquals(static_cast<std::size_t>(13 * 13), coverage.size(), __func__, __LINE__, name + ": 13 x 13 values");
        CheckTrue(Total(coverage) > 0, __func__, __LINE__, name + ": covers something");
        CheckTrue(Total(coverage) < 13 * 13 * 255, __func__, __LINE__, name + ": doesn't cover everything");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_CrossIsCarbonsCheckMark()
{
    // Act: Carbon's 13-pixel check box's cross, whose arms end at pixels 3 and 9.
    std::vector<std::uint8_t> const cross = RenderWin32GUIGlyph(EWin32GUIGlyph::Cross, 13);

    // Assert
    CheckTrue(IsMirrored(cross, 13, true) && IsMirrored(cross, 13, false), __func__, __LINE__, "symmetric");
    CheckEquals(255, At(cross, 13, 6, 6), __func__, __LINE__, "full where its arms cross");
    CheckTrue(At(cross, 13, 3, 3) >= 200, __func__, __LINE__, "nearly full at an arm's end");
    CheckEquals(0, At(cross, 13, 6, 3), __func__, __LINE__, "nothing between the upper arms");
    CheckEquals(0, At(cross, 13, 1, 1), __func__, __LINE__, "nothing past an arm's end");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_IsEmptyWithoutASize()
{
    // Act, Assert
    CheckTrue(RenderWin32GUIGlyph(EWin32GUIGlyph::Cross, 0).empty(), __func__, __LINE__, "size 0");
    CheckTrue(RenderWin32GUIGlyph(EWin32GUIGlyph::Cross, -1).empty(), __func__, __LINE__, "a negative size");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_ScalesWithTheSize()
{
    // Act: 96 and 192 DPI.
    int const at13 = Total(RenderWin32GUIGlyph(EWin32GUIGlyph::Cross, 13));
    int const at26 = Total(RenderWin32GUIGlyph(EWin32GUIGlyph::Cross, 26));

    // Assert: twice the size, four times the area, within what the pixel grid changes.
    CheckTrue(at26 > at13 * 7 / 2 && at26 < at13 * 9 / 2, __func__, __LINE__, "covers about four times as much");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Win32GUI_Glyphs::Test_RenderWin32GUIGlyph_SquareAndDotAreInTheMiddle()
{
    // Act: the partial check box's square and the radio button's dot, at 13 pixels.
    std::vector<std::uint8_t> const square = RenderWin32GUIGlyph(EWin32GUIGlyph::Square, 13);
    std::vector<std::uint8_t> const dot = RenderWin32GUIGlyph(EWin32GUIGlyph::Dot, 13);

    // Assert
    for (std::vector<std::uint8_t> const* const glyph : { &square, &dot })
    {
        std::string const name = glyph == &square ? "the square" : "the dot";
        CheckTrue(IsMirrored(*glyph, 13, true) && IsMirrored(*glyph, 13, false), __func__, __LINE__,
            name + " is symmetric");
        CheckEquals(255, At(*glyph, 13, 6, 6), __func__, __LINE__, name + " is full in the middle");
        CheckEquals(0, At(*glyph, 13, 6, 2), __func__, __LINE__, name + " leaves the edges");
    }

    CheckTrue(At(dot, 13, 4, 4) < At(square, 13, 4, 4), __func__, __LINE__, "the dot is round where the square isn't");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Win32GUI_Glyphs)
