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
#include <cwchar>
#include <random>
#include <utility>
//---------------------------------------------------------------------------
#if defined(_WIN32)
#  include <memory>
#  include <stdexcept>
#  if !defined(NOMINMAX)
#    define NOMINMAX
#  endif
#  if !defined(WIN32_LEAN_AND_MEAN)
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <windows.h>
#  include <shellapi.h>
#endif
//---------------------------------------------------------------------------

namespace
{

#if defined(_WIN32)
// A buffer from CommandLineToArgvW() or FormatMessageW(), freed with LocalFree().
template <typename T>
using TLocalPtr = std::unique_ptr<T, HLOCAL(WINAPI*)(HLOCAL)>;
#endif

//---------------------------------------------------------------------------

void AppendUTF8(std::string& text, uint32_t codePoint);
void AppendWide(std::wstring& text, uint32_t codePoint);
std::size_t DecodeUTF8Sequence(std::string const& text, std::size_t start, uint32_t& codePoint);
bool IsHighSurrogate(uint32_t codeUnit);
bool IsLowSurrogate(uint32_t codeUnit);

//---------------------------------------------------------------------------
/*
    AppendUTF8

    Appends 'codePoint', which must be a valid code point, to 'text' as UTF-8.
*/
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
/*
    AppendWide

    Appends 'codePoint', which must be a valid code point, to 'text': as is where wchar_t is 32 bits, and as a
    surrogate pair, above U+FFFF, where it's 16.
*/
void AppendWide(std::wstring& text, uint32_t codePoint)
{
#if WCHAR_MAX > 0xFFFF
    text += static_cast<wchar_t>(codePoint);
#else
    if (codePoint < 0x10000)
    {
        text += static_cast<wchar_t>(codePoint);
        return;
    }

    text += static_cast<wchar_t>(0xD800 + ((codePoint - 0x10000) >> 10));
    text += static_cast<wchar_t>(0xDC00 + ((codePoint - 0x10000) & 0x3FF));
#endif
}
//---------------------------------------------------------------------------
/*
    DecodeUTF8Sequence

    Decodes the UTF-8 sequence at 'start' in 'text', which must be within it, into 'codePoint', and returns how many
    bytes it took. A maximal ill-formed sequence (a byte that can't start a sequence, or a sequence's valid start
    before it breaks off or 'text' ends) gives U+FFFD and takes just those bytes, so the next sequence starts at the
    byte that broke it off. The second byte's range depends on the first, which rules out overlong sequences,
    surrogates and values above U+10FFFF (the Unicode standard's table 3-7, "Well-Formed UTF-8 Byte Sequences").
*/
std::size_t DecodeUTF8Sequence(std::string const& text, std::size_t start, uint32_t& codePoint)
{
    uint32_t const replacementCharacter = 0xFFFD;
    unsigned char const lead = static_cast<unsigned char>(text[start]);

    if (lead < 0x80)
    {
        codePoint = lead;
        return 1;
    }

    std::size_t length = 0;
    unsigned char secondLow = 0x80;
    unsigned char secondHigh = 0xBF;

    if (lead >= 0xC2 && lead <= 0xDF)
    {
        length = 2;
    }
    else if (lead >= 0xE0 && lead <= 0xEF)
    {
        length = 3;
        secondLow = (lead == 0xE0) ? 0xA0 : 0x80;
        secondHigh = (lead == 0xED) ? 0x9F : 0xBF;
    }
    else if (lead >= 0xF0 && lead <= 0xF4)
    {
        length = 4;
        secondLow = (lead == 0xF0) ? 0x90 : 0x80;
        secondHigh = (lead == 0xF4) ? 0x8F : 0xBF;
    }
    else
    {
        codePoint = replacementCharacter; // A continuation byte, or one never used in UTF-8.
        return 1;
    }

    codePoint = lead & (0xFFu >> (length + 1));

    for (std::size_t i = 1; i < length; ++i)
    {
        if (start + i >= text.size())
        {
            codePoint = replacementCharacter;
            return i;
        }

        unsigned char const next = static_cast<unsigned char>(text[start + i]);
        unsigned char const low = (i == 1) ? secondLow : 0x80;
        unsigned char const high = (i == 1) ? secondHigh : 0xBF;

        if (next < low || next > high)
        {
            codePoint = replacementCharacter;
            return i;
        }

        codePoint = (codePoint << 6) | (next & 0x3Fu);
    }

    return length;
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

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
#if defined(_WIN32)
std::vector<std::string> CommandLineArguments(std::wstring const& commandLine)
{
    // CommandLineToArgvW() splits an empty command line as if it were this executable's path, unquoted, so a path
    // with a space in it would become a program name and an argument.
    if (commandLine.empty())
        return std::vector<std::string>();

    int count = 0;
    TLocalPtr<wchar_t*> const argv(CommandLineToArgvW(commandLine.c_str(), &count), &LocalFree);

    if (argv == nullptr)
        throw std::runtime_error("Couldn't split the command line: " + SystemErrorText(GetLastError()));

    std::vector<std::string> args;

    for (int i = 1; i < count; ++i)
        args.push_back(WideToUTF8(argv.get()[i]));

    return args;
}
#endif
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
#if defined(_WIN32)
std::string SystemErrorText(unsigned long error)
{
    wchar_t* buffer = nullptr;
    DWORD const length = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS, nullptr, error, 0, reinterpret_cast<wchar_t*>(&buffer), 0, nullptr);

    if (length == 0 || buffer == nullptr)
        return "error " + std::to_string(error);

    TLocalPtr<wchar_t> const owner(buffer, &LocalFree);
    std::wstring text(buffer, length);

    // FormatMessageW() ends the text with a line break.
    while (!text.empty() && (text.back() == L'\r' || text.back() == L'\n' || text.back() == L' '))
        text.pop_back();

    return WideToUTF8(text);
}
#endif
//---------------------------------------------------------------------------
std::wstring UTF8ToWide(std::string const& text)
{
    std::wstring wide;
    wide.reserve(text.size());

    for (std::size_t i = 0; i < text.size(); )
    {
        uint32_t codePoint = 0;
        i += DecodeUTF8Sequence(text, i, codePoint);
        AppendWide(wide, codePoint);
    }

    return wide;
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
