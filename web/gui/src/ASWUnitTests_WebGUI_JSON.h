/* **************************************************************************
ASWUnitTests_WebGUI_JSON.h
Author: Anthony S. West - ASW Software

JSON text, as the Web GUI runner's server sends it to its page: a writer
for objects, arrays, strings, integers, booleans and null, and the UTF-8
repair its strings go through. Contains no socket code, so it's unit tested
from the console runners (see web/tests/Test_ASWUnitTests_WebGUI_JSON.cpp).

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
#ifndef ASWUnitTests_WebGUI_JSONH
#define ASWUnitTests_WebGUI_JSONH
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TWebGUIJSONWriter
//
// Writes one JSON value, in order: Begin and End calls for the objects and
// arrays, Key() before each value in an object, and a value call for each
// value. Commas come by themselves. Strings go through WebGUIJSONString().
// It writes numbers only as integers: the page is sent display texts for
// anything else, since a test can change the C locale's decimal point.
// JavaScript keeps integers exact up to 2^53, far beyond the counts it's
// sent.
//
// Misuse, such as a value in an object without a key, a key in an array, or
// an End call that doesn't match its Begin, throws std::logic_error, since
// it's a bug in the caller, never something the page or a test causes.
/////////////////////////////////////////////////////////////////////////////
class TWebGUIJSONWriter
{
private:
    // An object or array that's been begun and not yet ended.
    struct TContainer
    {
        // Whether a key was written and its value is due.
        bool AfterKey;
        // Whether a member or element was written, so the next one needs a comma first.
        bool HasValue;
        bool IsObject;
    };

private:
    // Whether the outermost value is complete, after which nothing more may be written.
    bool m_Done;
    std::vector<TContainer> m_Open;
    std::string m_Text;

private:
    // Writes what goes before a value (a comma, as needed), checking that a value may come here.
    void BeforeValue();
    // Ends the container on top, checking that it's an object ('object') or an array.
    void End(bool object);

public:
    TWebGUIJSONWriter();

    void BeginArray();
    void BeginObject();
    void Bool(bool value);
    void EndArray();
    void EndObject();
    // Writes an integer.
    void Int(long long value);
    // Whether the outermost value is complete.
    bool IsComplete() const;
    // Writes the name of the object member whose value comes next.
    void Key(std::string const& name);
    void Null();
    // Writes 'value' as a JSON string (see WebGUIJSONString()).
    void String(std::string const& value);
    // The JSON written so far.
    std::string const& Text() const;
    // Writes an unsigned integer, e.g. a count or an index.
    void UInt(unsigned long long value);
};

//---------------------------------------------------------------------------

// 'text' as UTF-8 a browser decodes the same way every time: each invalid sequence in it (a byte that can't
// start a character, a sequence cut short, an overlong form, a surrogate, a code point above U+10FFFF)
// replaced with U+FFFD, one for each maximal subpart, as the WHATWG Encoding Standard's decoder does; valid
// text unchanged. Tests can log any bytes, and the page shows them.
std::string RepairWebGUIUTF8(std::string const& text);
// 'text' as a JSON string literal, quotes included: RepairWebGUIUTF8()'s text, with '"', '\' and the control
// characters escaped (\b, \f, \n, \r and \t, the others as \u00XX).
std::string WebGUIJSONString(std::string const& text);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_WebGUI_JSONH
