/* **************************************************************************
ASWUnitTests_Utils.cpp
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
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>
//---------------------------------------------------------------------------

namespace
{

void AppendUTF8(std::string& text, uint32_t codePoint);
bool IsHighSurrogate(uint32_t codeUnit);
bool IsLowSurrogate(uint32_t codeUnit);

//---------------------------------------------------------------------------
// Appends 'codePoint', which must be a valid code point, to 'text' as UTF-8.
void AppendUTF8(std::string& text, uint32_t codePoint)
{
    if (codePoint < 0x80)
    {
        text += static_cast<char>(codePoint);
    }
    else if (codePoint < 0x800)
    {
        text += static_cast<char>(0xC0 | (codePoint >> 6));
        text += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else if (codePoint < 0x10000)
    {
        text += static_cast<char>(0xE0 | (codePoint >> 12));
        text += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        text += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
    else
    {
        text += static_cast<char>(0xF0 | (codePoint >> 18));
        text += static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F));
        text += static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F));
        text += static_cast<char>(0x80 | (codePoint & 0x3F));
    }
}

//---------------------------------------------------------------------------
bool IsHighSurrogate(uint32_t codeUnit)
{
    return codeUnit >= 0xD800 && codeUnit <= 0xDBFF;
}

//---------------------------------------------------------------------------
bool IsLowSurrogate(uint32_t codeUnit)
{
    return codeUnit >= 0xDC00 && codeUnit <= 0xDFFF;
}

//---------------------------------------------------------------------------

} // namespace

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
uint32_t FNV1aHash(std::string const& text)
{
    uint32_t hash = 0x811C9DC5; // FNV offset basis

    for (char c : text)
    {
        hash ^= static_cast<unsigned char>(c);
        hash *= 0x01000193; // FNV prime
    }

    return hash;
}
//---------------------------------------------------------------------------
/*
    ShuffledIndices

    A Fisher-Yates shuffle driven by std::mt19937, whose output the C++ standard fully specifies. std::shuffle and
    std::uniform_int_distribution aren't used, since how they turn that output into positions is left to each
    standard library. Each position is instead drawn by rejection sampling, which also keeps it unbiased.
*/
std::vector<std::size_t> ShuffledIndices(std::size_t count, unsigned int seed)
{
    std::vector<std::size_t> indices(count);

    for (std::size_t i = 0; i < count; ++i)
        indices[i] = i;

    std::mt19937 rng(seed);

    for (std::size_t i = count; i > 1; --i)
    {
        // Draws a position in [0, i). A draw below 2^32 mod i is rejected, so each position is equally likely.
        uint32_t const bound = static_cast<uint32_t>(i);
        uint32_t const rejectBelow = static_cast<uint32_t>(0u - bound) % bound;
        uint32_t draw = static_cast<uint32_t>(rng());

        while (draw < rejectBelow)
            draw = static_cast<uint32_t>(rng());

        std::swap(indices[i - 1], indices[draw % bound]);
    }

    return indices;
}
//---------------------------------------------------------------------------
std::string WideToUTF8(std::wstring const& text)
{
    uint32_t const replacementCharacter = 0xFFFD;
    std::string utf8;
    utf8.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); ++i)
    {
        // Through uint32_t, so a negative value (wchar_t is signed on Linux) becomes one above U+10FFFF.
        uint32_t codePoint = static_cast<uint32_t>(text[i]);

        if (IsHighSurrogate(codePoint) && (i + 1 < text.size()) && IsLowSurrogate(static_cast<uint32_t>(text[i + 1])))
        {
            uint32_t const lowSurrogate = static_cast<uint32_t>(text[i + 1]);
            codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (lowSurrogate - 0xDC00);
            ++i;
        }
        else if (IsHighSurrogate(codePoint) || IsLowSurrogate(codePoint) || (codePoint > 0x10FFFF))
        {
            codePoint = replacementCharacter;
        }

        AppendUTF8(utf8, codePoint);
    }

    return utf8;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
