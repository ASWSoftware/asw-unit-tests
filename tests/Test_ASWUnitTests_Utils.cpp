/* **************************************************************************
Test_ASWUnitTests_Utils.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

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
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Utils.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Utils
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Utils::TTest_ASWUnitTests_Utils()
    : inherited("ASWUnitTests_Utils_Tests")
{
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
