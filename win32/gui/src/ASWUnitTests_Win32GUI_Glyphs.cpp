/* **************************************************************************
ASWUnitTests_Win32GUI_Glyphs.cpp
Author: Anthony S. West - ASW Software

The small shapes the Win32 GUI runner's dark theme draws.

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
#include "ASWUnitTests_Win32GUI_Glyphs.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

namespace
{

// Each pixel is sampled at SamplesPerSide x SamplesPerSide points.
int const SamplesPerSide = 4;

//---------------------------------------------------------------------------

bool Covers(EWin32GUIGlyph glyph, double x, double y);
bool InTriangle(double x, double y, double ax, double ay, double bx, double by, double cx, double cy);
bool NearSegment(double x, double y, double ax, double ay, double bx, double by, double halfWidth);

//---------------------------------------------------------------------------
/*
    Covers

    Whether the point (x, y) of a cell one unit wide, from its top left, is inside 'glyph'. The shapes'
    proportions come from Carbon's glyphs: its 13-pixel check box's cross spans pixels 3 to 9 in strokes about
    1.7 pixels wide, its partial square pixels 4 to 8, its radio dot about 4.6 pixels across, and its 16-pixel
    submenu arrow is 4 pixels wide and 7 tall.
*/
bool Covers(EWin32GUIGlyph glyph, double x, double y)
{
    double const dx = x - 0.5;
    double const dy = y - 0.5;
    double const squaredDistance = dx * dx + dy * dy;

    switch (glyph)
    {
        case EWin32GUIGlyph::ArrowRight:
            return InTriangle(x, y, 0.375, 0.28, 0.375, 0.72, 0.625, 0.5);

        case EWin32GUIGlyph::Circle:
            return squaredDistance <= 0.5 * 0.5;

        case EWin32GUIGlyph::Cross:
            return NearSegment(x, y, 0.285, 0.285, 0.715, 0.715, 0.066) || NearSegment(x, y, 0.715, 0.285, 0.285,
                0.715, 0.066);

        case EWin32GUIGlyph::Dot:
            return squaredDistance <= 0.18 * 0.18;

        case EWin32GUIGlyph::InnerCircle:
            return squaredDistance <= (5.5 / 13) * (5.5 / 13);

        case EWin32GUIGlyph::Square:
            return x >= 0.34 && x <= 0.66 && y >= 0.34 && y <= 0.66;

        case EWin32GUIGlyph::Tick:
            return NearSegment(x, y, 0.25, 0.52, 0.42, 0.69, 0.07) || NearSegment(x, y, 0.42, 0.69, 0.76, 0.33,
                0.07);
    }

    return false;
}
//---------------------------------------------------------------------------
/*
    InTriangle

    Whether (x, y) is inside the triangle a, b, c (in either winding), or on its edge.
*/
bool InTriangle(double x, double y, double ax, double ay, double bx, double by, double cx, double cy)
{
    auto const side = [x, y](double x1, double y1, double x2, double y2)
        {
            return (x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
        };

    double const ab = side(ax, ay, bx, by);
    double const bc = side(bx, by, cx, cy);
    double const ca = side(cx, cy, ax, ay);

    return (ab >= 0 && bc >= 0 && ca >= 0) || (ab <= 0 && bc <= 0 && ca <= 0);
}
//---------------------------------------------------------------------------
/*
    NearSegment

    Whether (x, y) is within 'halfWidth' of the segment from a to b: inside a stroke along it with round ends.
*/
bool NearSegment(double x, double y, double ax, double ay, double bx, double by, double halfWidth)
{
    double const lengthX = bx - ax;
    double const lengthY = by - ay;
    double const squaredLength = lengthX * lengthX + lengthY * lengthY;
    double const along = squaredLength > 0 ? (std::max)(0.0, (std::min)(1.0, ((x - ax) * lengthX + (y - ay) *
        lengthY) / squaredLength)) : 0.0;
    double const offsetX = x - (ax + along * lengthX);
    double const offsetY = y - (ay + along * lengthY);

    return offsetX * offsetX + offsetY * offsetY <= halfWidth * halfWidth;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
std::vector<std::uint8_t> RenderWin32GUIGlyph(EWin32GUIGlyph glyph, int size)
{
    if (size <= 0)
        return std::vector<std::uint8_t>();

    std::vector<std::uint8_t> coverage(static_cast<std::size_t>(size) * static_cast<std::size_t>(size), 0);
    int const samples = SamplesPerSide * SamplesPerSide;

    for (int row = 0; row < size; ++row)
    {
        for (int column = 0; column < size; ++column)
        {
            int inside = 0;

            for (int sampleY = 0; sampleY < SamplesPerSide; ++sampleY)
            {
                for (int sampleX = 0; sampleX < SamplesPerSide; ++sampleX)
                {
                    // Each sample at the middle of its part of the pixel.
                    double const x = (column + (sampleX + 0.5) / SamplesPerSide) / size;
                    double const y = (row + (sampleY + 0.5) / SamplesPerSide) / size;

                    if (Covers(glyph, x, y))
                        ++inside;
                }
            }

            coverage[static_cast<std::size_t>(row) * static_cast<std::size_t>(size) + static_cast<std::size_t>(column)] =
                static_cast<std::uint8_t>((inside * 255 + samples / 2) / samples);
        }
    }

    return coverage;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
