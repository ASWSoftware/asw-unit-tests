/* **************************************************************************
Test_ASWUnitTests_Utils.cpp
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
#include "Test_ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

#if defined(_WIN32)
namespace
{

//---------------------------------------------------------------------------

std::string Joined(std::vector<std::string> const& items);

//---------------------------------------------------------------------------
/*
    Joined

    The items separated by '|', to compare a list with one CheckEquals().
*/
std::string Joined(std::vector<std::string> const& items)
{
    std::string joined;

    for (std::size_t i = 0; i < items.size(); ++i)
        joined += (i == 0 ? "" : "|") + items[i];

    return joined;
}
//---------------------------------------------------------------------------

} // namespace
#endif

//---------------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Utils
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Utils::TTest_ASWUnitTests_Utils()
    : inherited("ASWUnitTests_Utils_Tests")
{
#if defined(_WIN32)
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_CommandLineArguments_ConvertsNonASCIIToUTF8,
        "CommandLineArguments_ConvertsNonASCIIToUTF8");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_CommandLineArguments_FollowsTheCRuntimeQuotingRules,
        "CommandLineArguments_FollowsTheCRuntimeQuotingRules");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_CommandLineArguments_SkipsTheProgramName,
        "CommandLineArguments_SkipsTheProgramName");
#endif
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_FNV1aHash_MatchesReferenceValues, "FNV1aHash_MatchesReferenceValues");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_ShuffledIndices_GivesSameOrderEverywhere,
        "ShuffledIndices_GivesSameOrderEverywhere");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_ShuffledIndices_ReturnsEachIndexOnce,
        "ShuffledIndices_ReturnsEachIndexOnce");
#if defined(_WIN32)
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_SystemErrorText_DescribesAKnownCode,
        "SystemErrorText_DescribesAKnownCode");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_SystemErrorText_GivesTheNumberForAnUnknownCode,
        "SystemErrorText_GivesTheNumberForAnUnknownCode");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_UTF8ToWide_ConvertsEveryPlane, "UTF8ToWide_ConvertsEveryPlane");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_UTF8ToWide_ReplacesInvalidUTF8, "UTF8ToWide_ReplacesInvalidUTF8");
#endif
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_WideToUTF8_EncodesEachSequenceLength,
        "WideToUTF8_EncodesEachSequenceLength");
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_WideToUTF8_KeepsASCIIUnchanged, "WideToUTF8_KeepsASCIIUnchanged");
#if WCHAR_MAX > 0xFFFF
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_WideToUTF8_ReplacesOutOfRangeValues,
        "WideToUTF8_ReplacesOutOfRangeValues");
#endif
    RegisterTest(&TTest_ASWUnitTests_Utils::Test_WideToUTF8_ReplacesUnpairedSurrogates,
        "WideToUTF8_ReplacesUnpairedSurrogates");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Utils::~TTest_ASWUnitTests_Utils()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
#if defined(_WIN32)
void TTest_ASWUnitTests_Utils::Test_CommandLineArguments_ConvertsNonASCIIToUTF8()
{
    // Arrange: U+00E9 (two UTF-8 bytes), U+4E2D (three) and U+1F600 (four; a surrogate pair in UTF-16).
    std::wstring const commandLine = std::wstring(L"prog.exe \x00E9t\x00E9 \x4E2D ") + L"\xD83D\xDE00";

    // Act
    std::vector<std::string> const args = CommandLineArguments(commandLine);

    // Assert
    CheckEquals(std::string("\xC3\xA9t\xC3\xA9|\xE4\xB8\xAD|\xF0\x9F\x98\x80"), Joined(args), __func__, __LINE__,
        "each argument in UTF-8");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_CommandLineArguments_FollowsTheCRuntimeQuotingRules()
{
    // Arrange: a quoted program name with a space, a quoted argument with a space, an empty quoted argument, an
    // escaped quote, an escaped backslash before a closing quote, and backslashes that precede no quote.
    std::wstring const commandLine =
        L"\"C:\\Program Files\\prog.exe\" --filter \"a b\" \"\" a\\\"b \"c\\\\\" d\\\\e";

    // Act
    std::vector<std::string> const args = CommandLineArguments(commandLine);

    // Assert
    CheckEquals(std::string("--filter|a b||a\"b|c\\|d\\\\e"), Joined(args), __func__, __LINE__,
        "split and unquoted as the C runtime splits argv");
    CheckEquals(6u, args.size(), __func__, __LINE__, "the empty argument is kept");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_CommandLineArguments_SkipsTheProgramName()
{
    // Act
    std::vector<std::string> const programOnly = CommandLineArguments(L"prog.exe");
    std::vector<std::string> const spaced = CommandLineArguments(L"prog.exe   --run   --exit  ");
    // CommandLineToArgvW() gives an empty command line the executable's path, unquoted, as its program name, which
    // it then splits at any space in the path; so this checks the fix only when the path has one.
    std::vector<std::string> const empty = CommandLineArguments(L"");
    std::vector<std::string> const blank = CommandLineArguments(L"   ");

    // Assert
    CheckEmpty(programOnly, __func__, __LINE__, "no arguments after the program name");
    CheckEquals(std::string("--run|--exit"), Joined(spaced), __func__, __LINE__, "extra spaces separate only");
    CheckEmpty(empty, __func__, __LINE__, "an empty command line has no arguments");
    CheckEmpty(blank, __func__, __LINE__, "nor has a blank one");
}
#endif
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_FNV1aHash_MatchesReferenceValues()
{
    // The published 32-bit FNV-1a test vectors.
    CheckEquals(static_cast<uint32_t>(0x811C9DC5), FNV1aHash(""), __func__, __LINE__, "empty text");
    CheckEquals(static_cast<uint32_t>(0xE40C292C), FNV1aHash("a"), __func__, __LINE__, "\"a\"");
    CheckEquals(static_cast<uint32_t>(0xBF9CF968), FNV1aHash("foobar"), __func__, __LINE__, "\"foobar\"");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_ShuffledIndices_GivesSameOrderEverywhere()
{
    // These orders must not depend on the compiler or standard library, so a --shuffle-seed reproduces a run's
    // order on any of them. A change here also changes the order every existing seed gives.
    std::vector<std::size_t> const expectedSeed42 = { 1, 3, 9, 7, 6, 0, 8, 4, 5, 2 };
    std::vector<std::size_t> const expectedSeedMax = { 6, 9, 7, 2, 5, 0, 8, 4, 3, 1 };

    CheckTrue(ShuffledIndices(10, 42u) == expectedSeed42, __func__, __LINE__, "seed 42");
    CheckTrue(ShuffledIndices(10, 4294967295u) == expectedSeedMax, __func__, __LINE__, "the largest seed");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_ShuffledIndices_ReturnsEachIndexOnce()
{
    CheckEmpty(ShuffledIndices(0, 1u), __func__, __LINE__, "no indices");
    CheckTrue(ShuffledIndices(1, 1u) == std::vector<std::size_t>{ 0 }, __func__, __LINE__, "a single index");

    std::vector<std::size_t> const shuffled = ShuffledIndices(100, 7u);
    std::vector<std::size_t> sorted = shuffled;
    std::sort(sorted.begin(), sorted.end());
    std::vector<std::size_t> identity(100);

    for (std::size_t i = 0; i < identity.size(); ++i)
        identity[i] = i;

    CheckTrue(sorted == identity, __func__, __LINE__, "each of 0 to 99 appears exactly once");
    CheckTrue(shuffled != identity, __func__, __LINE__, "and the order is actually shuffled");
}
//---------------------------------------------------------------------------
#if defined(_WIN32)
void TTest_ASWUnitTests_Utils::Test_SystemErrorText_DescribesAKnownCode()
{
    // Act: ERROR_FILE_NOT_FOUND, described in the system's language.
    std::string const text = SystemErrorText(2);

    // Assert
    CheckNotEmpty(text, __func__, __LINE__, "described");
    CheckNotEquals(std::string("error 2"), text, __func__, __LINE__, "by Windows, not by its number");
    CheckFalse(text.back() == '\n' || text.back() == '\r' || text.back() == ' ', __func__, __LINE__,
        "without FormatMessageW()'s line break at the end");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_SystemErrorText_GivesTheNumberForAnUnknownCode()
{
    // Act: a code with the customer bit (bit 29) set, which Windows never describes.
    std::string const text = SystemErrorText(0x2FFFFFFFul);

    // Assert
    CheckEquals(std::string("error 805306367"), text, __func__, __LINE__, "the code, in decimal");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_UTF8ToWide_ConvertsEveryPlane()
{
    // Arrange: ASCII, U+00E9, U+4E2D and U+1F600 (a surrogate pair in UTF-16), and a NUL, which ends neither.
    std::string const utf8 = std::string("a\xC3\xA9\xE4\xB8\xAD\xF0\x9F\x98\x80") + std::string("\0z", 2);
    std::wstring const expected = std::wstring(L"a\x00E9\x4E2D\xD83D\xDE00") + std::wstring(L"\0z", 2);

    // Act
    std::wstring const wide = UTF8ToWide(utf8);

    // Assert
    CheckEquals(expected, wide, __func__, __LINE__, "every character converted");
    CheckEquals(utf8, WideToUTF8(wide), __func__, __LINE__, "and back again");
    CheckEmpty(UTF8ToWide(std::string()), __func__, __LINE__, "empty stays empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_UTF8ToWide_ReplacesInvalidUTF8()
{
    // Act: a byte that never appears in UTF-8, and a continuation byte with no lead byte.
    std::wstring const invalid = UTF8ToWide("a\xFF" "b");
    std::wstring const continuation = UTF8ToWide("\x80" "c");

    // Assert
    CheckEquals(std::wstring(L"a\xFFFD" L"b"), invalid, __func__, __LINE__, "0xFF becomes U+FFFD");
    CheckEquals(std::wstring(L"\xFFFD" L"c"), continuation, __func__, __LINE__, "a lone 0x80 becomes U+FFFD");
}
#endif
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_WideToUTF8_EncodesEachSequenceLength()
{
    // The first and last code point of each UTF-8 sequence length. The four-byte ones are a surrogate pair where
    // wchar_t is 16 bits.
    CheckEquals(std::string("\x7F"), WideToUTF8(L"\x007F"), __func__, __LINE__, "U+007F, one byte");
    CheckEquals(std::string("\xC2\x80"), WideToUTF8(L"\x0080"), __func__, __LINE__, "U+0080, two bytes");
    CheckEquals(std::string("\xDF\xBF"), WideToUTF8(L"\x07FF"), __func__, __LINE__, "U+07FF, two bytes");
    CheckEquals(std::string("\xE0\xA0\x80"), WideToUTF8(L"\x0800"), __func__, __LINE__, "U+0800, three bytes");
    CheckEquals(std::string("\xEF\xBF\xBF"), WideToUTF8(L"\xFFFF"), __func__, __LINE__, "U+FFFF, three bytes");
    CheckEquals(std::string("\xF0\x90\x80\x80"), WideToUTF8(L"\U00010000"), __func__, __LINE__,
        "U+10000, four bytes");
    CheckEquals(std::string("\xF4\x8F\xBF\xBF"), WideToUTF8(L"\U0010FFFF"), __func__, __LINE__,
        "U+10FFFF, four bytes");

    CheckEquals(std::string("caf" "\xC3\xA9" " \xE2\x82\xAC" "5 \xF0\x9F\x98\x80"),
        WideToUTF8(L"caf" L"\x00E9" L" \x20AC" L"5 \U0001F600"), __func__, __LINE__, "mixed lengths in one string");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_WideToUTF8_KeepsASCIIUnchanged()
{
    CheckEquals(std::string(), WideToUTF8(L""), __func__, __LINE__, "empty text");
    CheckEquals(std::string("Hello, World! 123"), WideToUTF8(L"Hello, World! 123"), __func__, __LINE__,
        "ASCII text");
    CheckEquals(std::string("a\0b", 3), WideToUTF8(std::wstring(L"a\0b", 3)), __func__, __LINE__,
        "an embedded null is kept");
}
//---------------------------------------------------------------------------
#if WCHAR_MAX > 0xFFFF
void TTest_ASWUnitTests_Utils::Test_WideToUTF8_ReplacesOutOfRangeValues()
{
    // Only possible where wchar_t is 32 bits (UTF-32), where it can hold a value above U+10FFFF or a negative one.
    std::wstring text = L"a";
    text += static_cast<wchar_t>(0x110000);
    text += L'b';
    text += static_cast<wchar_t>(-1);

    CheckEquals(std::string("a\xEF\xBF\xBD" "b\xEF\xBF\xBD"), WideToUTF8(text), __func__, __LINE__,
        "a value above U+10FFFF, or a negative one, becomes U+FFFD");
}
#endif
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Utils::Test_WideToUTF8_ReplacesUnpairedSurrogates()
{
    std::string const replacement = "\xEF\xBF\xBD"; // U+FFFD

    CheckEquals(replacement, WideToUTF8(L"\xD800"), __func__, __LINE__, "a lone high surrogate");
    CheckEquals(replacement, WideToUTF8(L"\xDFFF"), __func__, __LINE__, "a lone low surrogate");
    CheckEquals("a" + replacement + "b", WideToUTF8(L"a" L"\xDBFF" L"b"), __func__, __LINE__,
        "a high surrogate followed by a non-surrogate");
    CheckEquals(replacement + replacement, WideToUTF8(L"\xDC00" L"\xD800"), __func__, __LINE__,
        "a low surrogate before a high one is not a pair");
    CheckEquals(replacement + replacement, WideToUTF8(L"\xD800" L"\xD800"), __func__, __LINE__,
        "two high surrogates are not a pair");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Utils)
