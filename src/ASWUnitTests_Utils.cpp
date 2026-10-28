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
#if defined(_WIN32)
#  include <limits>
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
bool IsHighSurrogate(uint32_t codeUnit);
bool IsLowSurrogate(uint32_t codeUnit);
#if defined(_WIN32)
int ToConversionLength(std::size_t length);
#endif

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
#if defined(_WIN32)
/*
    ToConversionLength

    A length for MultiByteToWideChar() or WideCharToMultiByte(), which take an int.
*/
int ToConversionLength(std::size_t length)
{
    if (length > static_cast<std::size_t>((std::numeric_limits<int>::max)()))
        throw std::length_error("text too long to convert");

    return static_cast<int>(length);
}
#endif
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
//---------------------------------------------------------------------------
std::wstring UTF8ToWide(std::string const& text)
{
    if (text.empty())
        return std::wstring();

    int const length = ToConversionLength(text.size());
    int const wideLength = MultiByteToWideChar(CP_UTF8, 0, text.data(), length, nullptr, 0);

    if (wideLength <= 0)
        throw std::runtime_error("Couldn't convert UTF-8 text: " + SystemErrorText(GetLastError()));

    std::wstring wide(static_cast<std::size_t>(wideLength), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.data(), length, &wide[0], wideLength);
    return wide;
}
#endif
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
