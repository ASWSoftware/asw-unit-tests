/* **************************************************************************
ASWUnitTests_WebGUI_JSON.cpp
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
#include "ASWUnitTests_WebGUI_JSON.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

char const ReplacementCharacter[] = "\xEF\xBF\xBD";

//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
/*
    RepairWebGUIUTF8

    The WHATWG Encoding Standard's UTF-8 decoder: a lead byte sets how many continuation bytes follow and,
    for E0, ED, F0 and F4, a narrower range for the first of them, which rules out overlong forms, surrogates
    and code points above U+10FFFF. A byte outside the expected range ends the sequence as one U+FFFD and is
    then read again on its own, so an ASCII byte after a cut-short sequence is never lost.
*/
std::string RepairWebGUIUTF8(std::string const& text)
{
    std::string repaired;
    repaired.reserve(text.size());
    std::size_t i = 0;

    while (i < text.size())
    {
        unsigned char const lead = static_cast<unsigned char>(text[i]);
        std::size_t needed = 0;
        unsigned char lower = 0x80;
        unsigned char upper = 0xBF;

        if (lead <= 0x7F)
        {
            repaired += text[i];
            ++i;
            continue;
        }

        if (lead >= 0xC2 && lead <= 0xDF)
            needed = 1;
        else if (lead >= 0xE0 && lead <= 0xEF)
        {
            needed = 2;
            lower = lead == 0xE0 ? 0xA0 : 0x80;
            upper = lead == 0xED ? 0x9F : 0xBF;
        }
        else if (lead >= 0xF0 && lead <= 0xF4)
        {
            needed = 3;
            lower = lead == 0xF0 ? 0x90 : 0x80;
            upper = lead == 0xF4 ? 0x8F : 0xBF;
        }
        else
        {
            // A continuation byte with no lead, C0, C1 or F5 to FF: never part of valid UTF-8.
            repaired += ReplacementCharacter;
            ++i;
            continue;
        }

        std::size_t length = 1;

        while (length <= needed && i + length < text.size())
        {
            unsigned char const next = static_cast<unsigned char>(text[i + length]);

            if (next < lower || next > upper)
                break;

            ++length;
            lower = 0x80;
            upper = 0xBF;
        }

        if (length == needed + 1)
            repaired.append(text, i, length);
        else
            repaired += ReplacementCharacter;

        i += length;
    }

    return repaired;
}
//---------------------------------------------------------------------------
std::string WebGUIJSONString(std::string const& text)
{
    static char const hexDigits[] = "0123456789ABCDEF";
    std::string const repaired = RepairWebGUIUTF8(text);
    std::string quoted;
    quoted.reserve(repaired.size() + 2);
    quoted += '"';

    for (char const c : repaired)
    {
        unsigned char const byte = static_cast<unsigned char>(c);

        switch (c)
        {
            case '"':
                quoted += "\\\"";
                break;

            case '\\':
                quoted += "\\\\";
                break;

            case '\b':
                quoted += "\\b";
                break;

            case '\f':
                quoted += "\\f";
                break;

            case '\n':
                quoted += "\\n";
                break;

            case '\r':
                quoted += "\\r";
                break;

            case '\t':
                quoted += "\\t";
                break;

            default:
                if (byte < 0x20)
                {
                    quoted += "\\u00";
                    quoted += hexDigits[byte >> 4];
                    quoted += hexDigits[byte & 0x0F];
                }
                else
                {
                    quoted += c;
                }

                break;
        }
    }

    quoted += '"';
    return quoted;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWebGUIJSONWriter
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWebGUIJSONWriter::TWebGUIJSONWriter()
    : m_Done(false)
{
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::BeforeValue()
{
    if (m_Done)
        throw std::logic_error("TWebGUIJSONWriter: a value after the outermost one was complete");

    if (m_Open.empty())
        return;

    TContainer& container = m_Open.back();

    if (container.IsObject)
    {
        if (!container.AfterKey)
            throw std::logic_error("TWebGUIJSONWriter: a value in an object without a key");

        container.AfterKey = false;
    }
    else
    {
        if (container.HasValue)
            m_Text += ',';

        container.HasValue = true;
    }
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::BeginArray()
{
    BeforeValue();
    m_Text += '[';
    m_Open.push_back(TContainer { false, false, false });
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::BeginObject()
{
    BeforeValue();
    m_Text += '{';
    m_Open.push_back(TContainer { false, false, true });
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::Bool(bool value)
{
    BeforeValue();
    m_Text += value ? "true" : "false";
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------
/*
    TWebGUIJSONWriter::End

    An object can't end between a key and its value.
*/
void TWebGUIJSONWriter::End(bool object)
{
    if (m_Open.empty() || m_Open.back().IsObject != object)
        throw std::logic_error(object ? "TWebGUIJSONWriter: EndObject() without its BeginObject()" :
                "TWebGUIJSONWriter: EndArray() without its BeginArray()");

    if (m_Open.back().AfterKey)
        throw std::logic_error("TWebGUIJSONWriter: an object ended after a key, without its value");

    m_Open.pop_back();
    m_Text += object ? '}' : ']';
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::EndArray()
{
    End(false);
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::EndObject()
{
    End(true);
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::Int(long long value)
{
    BeforeValue();
    m_Text += std::to_string(value);
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------
bool TWebGUIJSONWriter::IsComplete() const
{
    return m_Done;
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::Key(std::string const& name)
{
    if (m_Open.empty() || !m_Open.back().IsObject)
        throw std::logic_error("TWebGUIJSONWriter: a key outside an object");

    TContainer& container = m_Open.back();

    if (container.AfterKey)
        throw std::logic_error("TWebGUIJSONWriter: a key after a key, without its value");

    if (container.HasValue)
        m_Text += ',';

    container.HasValue = true;
    container.AfterKey = true;
    m_Text += WebGUIJSONString(name);
    m_Text += ':';
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::Null()
{
    BeforeValue();
    m_Text += "null";
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::String(std::string const& value)
{
    BeforeValue();
    m_Text += WebGUIJSONString(value);
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------
std::string const& TWebGUIJSONWriter::Text() const
{
    return m_Text;
}
//---------------------------------------------------------------------------
void TWebGUIJSONWriter::UInt(unsigned long long value)
{
    BeforeValue();
    m_Text += std::to_string(value);
    m_Done = m_Open.empty();
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
