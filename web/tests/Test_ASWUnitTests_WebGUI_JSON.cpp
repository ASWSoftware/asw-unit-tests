/* **************************************************************************
Test_ASWUnitTests_WebGUI_JSON.cpp
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
#include "Test_ASWUnitTests_WebGUI_JSON.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_JSON.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

// U+FFFD, as RepairWebGUIUTF8() puts it in place of an invalid sequence.
std::string const FFFD = "\xEF\xBF\xBD";

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_JSON
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_JSON::TTest_ASWUnitTests_WebGUI_JSON()
    : inherited("ASWUnitTests_WebGUI_JSON_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_RepairWebGUIUTF8_KeepsValidText,
        "RepairWebGUIUTF8_KeepsValidText");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_RepairWebGUIUTF8_ReplacesEachMaximalSubpart,
        "RepairWebGUIUTF8_ReplacesEachMaximalSubpart");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_WebGUIJSONString_EscapesQuotesBackslashesAndControls,
        "WebGUIJSONString_EscapesQuotesBackslashesAndControls");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_NestsValuesWithCommas, "Writer_NestsValuesWithCommas");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_RejectsMisuse, "Writer_RejectsMisuse");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_WritesIntegerLimits, "Writer_WritesIntegerLimits");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_JSON::~TTest_ASWUnitTests_WebGUI_JSON()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_RepairWebGUIUTF8_KeepsValidText()
{
    // Arrange: each length of character, and the edges of the ranges the lead bytes allow.
    char const* const valid[] = {
        "",
        "plain ASCII, with \x01 and \x7F",
        "\xC2\x80",           // U+0080, the first 2-byte character
        "\xDF\xBF",           // U+07FF, the last
        "caf" "\xC3\xA9",     // e acute
        "\xE0\xA0\x80",       // U+0800, the first 3-byte character
        "\xE2\x82\xAC" "5",   // the euro sign
        "\xED\x9F\xBF",       // U+D7FF, just below the surrogates
        "\xEE\x80\x80",       // U+E000, just above them
        "\xEF\xBF\xBD",       // U+FFFD itself
        "\xF0\x90\x80\x80",   // U+10000, the first 4-byte character
        "\xF0\x9F\x98\x80",   // an emoji
        "\xF4\x8F\xBF\xBF",   // U+10FFFF, the last code point
    };

    // Act, Assert
    for (char const* const text : valid)
        CheckEquals(std::string(text), RepairWebGUIUTF8(text), __func__, __LINE__, "unchanged");

    std::string const withNul("a\0b", 3);
    CheckEquals(withNul, RepairWebGUIUTF8(withNul), __func__, __LINE__, "a NUL byte is valid UTF-8, and kept");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_RepairWebGUIUTF8_ReplacesEachMaximalSubpart()
{
    // Arrange: each input, and what the WHATWG decoder makes of it.
    struct TCase
    {
        char const* Input;
        std::string Expected;
        char const* Name;
    };

    TCase const cases[] = {
        { "\x80", FFFD, "a lone continuation byte" },
        { "a\xBF" "b", "a" + FFFD + "b", "one among ASCII, which is kept" },
        { "\xC0\x80", FFFD + FFFD, "C0, never a lead byte, then a lone continuation byte" },
        { "\xC1\xBF", FFFD + FFFD, "C1 likewise" },
        { "\xE0\x80\x80", FFFD + FFFD + FFFD, "an overlong 3-byte form: E0 needs A0 to BF next" },
        { "\xF0\x80\x80\x80", FFFD + FFFD + FFFD + FFFD, "an overlong 4-byte form: F0 needs 90 to BF next" },
        { "\xED\xA0\x80", FFFD + FFFD + FFFD, "a surrogate, U+D800: ED needs 80 to 9F next" },
        { "\xF4\x90\x80\x80", FFFD + FFFD + FFFD + FFFD, "above U+10FFFF: F4 needs 80 to 8F next" },
        { "\xF5\x80", FFFD + FFFD, "F5, never a lead byte" },
        { "\xFF", FFFD, "FF" },
        { "\xE2\x82", FFFD, "a 3-byte character cut short at the end: one replacement" },
        { "\xE2\x82" "A", FFFD + "A", "cut short by ASCII, which is kept" },
        { "\xF0\x9F\x98", FFFD, "a 4-byte character cut short at the end" },
        { "\xF0\x9F" "\xE2\x82\xAC", FFFD + "\xE2\x82\xAC", "cut short by another character, which is kept" },
        { "\xC3", FFFD, "a 2-byte lead byte at the end" },
        { "x" "\xC3\xA9\xA9" "y", "x" "\xC3\xA9" + FFFD + "y", "a continuation byte too many" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
        CheckEquals(testCase.Expected, RepairWebGUIUTF8(testCase.Input), __func__, __LINE__, testCase.Name);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_WebGUIJSONString_EscapesQuotesBackslashesAndControls()
{
    // Act, Assert
    CheckEquals(std::string("\"\""), WebGUIJSONString(""), __func__, __LINE__, "empty");
    CheckEquals(std::string("\"plain text\""), WebGUIJSONString("plain text"), __func__, __LINE__, "plain");
    CheckEquals(std::string("\"say \\\"hi\\\"\""), WebGUIJSONString("say \"hi\""), __func__, __LINE__, "quotes");
    CheckEquals(std::string("\"C:\\\\dir\\\\file\""), WebGUIJSONString("C:\\dir\\file"), __func__, __LINE__,
        "backslashes");
    CheckEquals(std::string("\"a\\nb\\r\\nc\\td\\be\\ff\""), WebGUIJSONString("a\nb\r\nc\td\be\ff"), __func__,
        __LINE__, "the short escapes");
    CheckEquals(std::string("\"\\u0000\\u0001\\u001F\""), WebGUIJSONString(std::string("\0\x01\x1F", 3)), __func__,
        __LINE__, "other control characters, NUL included");
    CheckEquals(std::string("\"\x7F/<\""), WebGUIJSONString("\x7F/<"), __func__, __LINE__,
        "DEL, '/' and '<' as they are");
    CheckEquals(std::string("\"caf" "\xC3\xA9" "\""), WebGUIJSONString("caf" "\xC3\xA9"), __func__, __LINE__,
        "UTF-8 as it is");
    CheckEquals("\"a" + FFFD + "\\n\"", WebGUIJSONString("a\xFF\n"), __func__, __LINE__,
        "invalid UTF-8 repaired, then escaped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_NestsValuesWithCommas()
{
    // Arrange
    TWebGUIJSONWriter writer;

    // Act
    writer.BeginObject();
    writer.Key("name");
    writer.String("Group.Test");
    writer.Key("checked");
    writer.Bool(true);
    writer.Key("failed");
    writer.Bool(false);
    writer.Key("result");
    writer.Null();
    writer.Key("items");
    writer.BeginArray();
    writer.Int(-1);
    writer.UInt(2);
    writer.BeginObject();
    writer.EndObject();
    writer.BeginArray();
    writer.EndArray();
    writer.BeginArray();
    writer.String("x");
    writer.EndArray();
    writer.EndArray();
    writer.Key("a \"key\"");
    writer.BeginObject();
    writer.Key("inner");
    writer.UInt(0);
    writer.EndObject();
    bool const completeBeforeEnd = writer.IsComplete();
    writer.EndObject();

    // Assert
    CheckEquals(std::string("{\"name\":\"Group.Test\",\"checked\":true,\"failed\":false,\"result\":null,"
        "\"items\":[-1,2,{},[],[\"x\"]],\"a \\\"key\\\"\":{\"inner\":0}}"), writer.Text(), __func__, __LINE__,
        "members and elements separated by commas, keys escaped");
    CheckFalse(completeBeforeEnd, __func__, __LINE__, "incomplete while the object is open");
    CheckTrue(writer.IsComplete(), __func__, __LINE__, "complete once it's closed");

    TWebGUIJSONWriter single;
    single.String("only");
    CheckEquals(std::string("\"only\""), single.Text(), __func__, __LINE__, "a single value");
    CheckTrue(single.IsComplete(), __func__, __LINE__, "complete at once");
    CheckFalse(TWebGUIJSONWriter().IsComplete(), __func__, __LINE__, "nothing written: incomplete");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_RejectsMisuse()
{
    // Act, Assert
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.BeginObject();
            writer.String("no key");
        }, __func__, __LINE__, "a value in an object without a key", "without a key");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.BeginArray();
            writer.Key("k");
        }, __func__, __LINE__, "a key in an array", "outside an object");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.Key("k");
        }, __func__, __LINE__, "a key with nothing open", "outside an object");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.BeginObject();
            writer.Key("a");
            writer.Key("b");
        }, __func__, __LINE__, "two keys in a row", "after a key");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.BeginObject();
            writer.Key("a");
            writer.EndObject();
        }, __func__, __LINE__, "an object ended between a key and its value", "without its value");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.BeginArray();
            writer.EndObject();
        }, __func__, __LINE__, "an array ended as an object", "EndObject()");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.EndArray();
        }, __func__, __LINE__, "an end with nothing open", "EndArray()");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIJSONWriter writer;
            writer.UInt(1);
            writer.UInt(2);
        }, __func__, __LINE__, "a second value after a complete one", "after the outermost");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_JSON::Test_Writer_WritesIntegerLimits()
{
    // Arrange
    TWebGUIJSONWriter writer;

    // Act
    writer.BeginArray();
    writer.Int((std::numeric_limits<long long>::min)());
    writer.Int((std::numeric_limits<long long>::max)());
    writer.Int(0);
    writer.UInt((std::numeric_limits<unsigned long long>::max)());
    writer.UInt(static_cast<unsigned long long>((std::numeric_limits<std::size_t>::max)()));
    writer.EndArray();

    // Assert: 64-bit everywhere the framework builds, std::size_t's being 32-bit or 64-bit.
    std::string const sizeMax = std::to_string((std::numeric_limits<std::size_t>::max)());
    CheckEquals("[-9223372036854775808,9223372036854775807,0,18446744073709551615," + sizeMax + "]",
        writer.Text(), __func__, __LINE__, "every digit, no exponent");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_WebGUI_JSON)
