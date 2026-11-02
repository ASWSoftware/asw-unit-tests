/* **************************************************************************
ASWUnitTests_WebGUI_HTTP.cpp
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
#include "ASWUnitTests_WebGUI_HTTP.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::optional<std::string> DecodeFormPart(std::string const& part);
bool EqualsIgnoringCase(std::string const& a, std::string const& b);
int HexDigitValue(char c);
bool IsToken(std::string const& text);

//---------------------------------------------------------------------------
/*
    DecodeFormPart

    A form field's name or value with '+' and each %XX decoded; std::nullopt if a '%' isn't followed by two
    hex digits.
*/
std::optional<std::string> DecodeFormPart(std::string const& part)
{
    std::string decoded;
    decoded.reserve(part.size());

    for (std::size_t i = 0; i < part.size(); ++i)
    {
        if (part[i] == '+')
            decoded += ' ';
        else if (part[i] != '%')
            decoded += part[i];
        else
        {
            if (i + 2 >= part.size())
                return std::nullopt;

            int const high = HexDigitValue(part[i + 1]);
            int const low = HexDigitValue(part[i + 2]);

            if (high < 0 || low < 0)
                return std::nullopt;

            decoded += static_cast<char>(high * 16 + low);
            i += 2;
        }
    }

    return decoded;
}
//---------------------------------------------------------------------------
/*
    EqualsIgnoringCase

    Compares ASCII letters ignoring case, as HTTP compares header names; other bytes must be equal.
*/
bool EqualsIgnoringCase(std::string const& a, std::string const& b)
{
    if (a.size() != b.size())
        return false;

    for (std::size_t i = 0; i < a.size(); ++i)
    {
        char x = a[i];
        char y = b[i];

        if (x >= 'A' && x <= 'Z')
            x = static_cast<char>(x - 'A' + 'a');

        if (y >= 'A' && y <= 'Z')
            y = static_cast<char>(y - 'A' + 'a');

        if (x != y)
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
/*
    HexDigitValue

    The value of a hex digit, either case; -1 for any other character.
*/
int HexDigitValue(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;

    return -1;
}
//---------------------------------------------------------------------------
/*
    IsToken

    Whether 'text' is an HTTP token, as a method or a header name must be: one or more letters, digits or
    any of !#$%&'*+-.^_`|~.
*/
bool IsToken(std::string const& text)
{
    static std::string const symbols = "!#$%&'*+-.^_`|~";

    if (text.empty())
        return false;

    for (char const c : text)
    {
        bool const alphanumeric = (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');

        if (!alphanumeric && symbols.find(c) == std::string::npos)
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
std::optional<std::vector<TWebGUIField> > DecodeWebGUIForm(std::string const& text)
{
    std::vector<TWebGUIField> fields;
    std::size_t start = 0;

    while (start <= text.size())
    {
        std::size_t end = text.find('&', start);

        if (end == std::string::npos)
            end = text.size();

        if (end > start)
        {
            std::string const pair = text.substr(start, end - start);
            std::size_t const equals = pair.find('=');
            std::optional<std::string> name = DecodeFormPart(pair.substr(0, equals));
            std::optional<std::string> value = equals == std::string::npos ? std::string() :
                    DecodeFormPart(pair.substr(equals + 1));

            if (!name.has_value() || !value.has_value())
                return std::nullopt;

            fields.push_back(TWebGUIField { std::move(*name), std::move(*value) });
        }

        start = end + 1;
    }

    return fields;
}
//---------------------------------------------------------------------------
TWebGUIResponse WebGUIErrorResponse(int status)
{
    TWebGUIResponse response;
    response.Status = status;
    response.ContentType = "text/plain; charset=utf-8";
    response.Body = WebGUIStatusReason(status) + "\n";

    if (status == 405)
        response.Headers.push_back(TWebGUIHeader { "Allow", "GET, POST" });

    return response;
}
//---------------------------------------------------------------------------
/*
    WebGUIResponseBytes

    The checks keep a header from being split into two by a line break in a value, should one ever come
    from outside the server's own code.
*/
std::string WebGUIResponseBytes(TWebGUIResponse const& response)
{
    if (response.Status < 100 || response.Status > 999)
        throw std::logic_error("WebGUIResponseBytes: status code " + std::to_string(response.Status) +
            " isn't three digits");

    std::vector<TWebGUIHeader> headers;

    if (!response.ContentType.empty())
        headers.push_back(TWebGUIHeader { "Content-Type", response.ContentType });

    headers.push_back(TWebGUIHeader { "Content-Length", std::to_string(response.Body.size()) });
    headers.push_back(TWebGUIHeader { "Cache-Control", "no-store" });
    headers.push_back(TWebGUIHeader { "Connection", "close" });
    headers.insert(headers.end(), response.Headers.begin(), response.Headers.end());

    std::string bytes = "HTTP/1.1 " + std::to_string(response.Status) + " " + WebGUIStatusReason(response.Status) +
        "\r\n";

    for (TWebGUIHeader const& header : headers)
    {
        if (!IsToken(header.Name) || header.Value.find_first_of("\r\n") != std::string::npos)
            throw std::logic_error("WebGUIResponseBytes: header \"" + header.Name +
                "\" has a malformed name or a line break in its value");

        bytes += header.Name + ": " + header.Value + "\r\n";
    }

    bytes += "\r\n";
    bytes += response.Body;
    return bytes;
}
//---------------------------------------------------------------------------
std::string WebGUIStatusReason(int status)
{
    switch (status)
    {
        case 200:
            return "OK";

        case 204:
            return "No Content";

        case 303:
            return "See Other";

        case 400:
            return "Bad Request";

        case 403:
            return "Forbidden";

        case 404:
            return "Not Found";

        case 405:
            return "Method Not Allowed";

        case 413:
            return "Content Too Large";

        case 431:
            return "Request Header Fields Too Large";

        case 500:
            return "Internal Server Error";

        case 501:
            return "Not Implemented";

        case 505:
            return "HTTP Version Not Supported";

        default:
            return std::string();
    }
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWebGUIRequest
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
std::string const* TWebGUIRequest::FindHeader(std::string const& name) const
{
    for (TWebGUIHeader const& header : Headers)
    {
        if (EqualsIgnoringCase(header.Name, name))
            return &header.Value;
    }

    return nullptr;
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWebGUIRequestParser
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TWebGUIRequestParser::TWebGUIRequestParser()
    : m_ContentLength(0),
      m_ErrorStatus(0),
      m_HasContentLength(false),
      m_HasCookie(false),
      m_HasHost(false),
      m_HeadSize(0),
      m_InBody(false),
      m_Status(EWebGUIParseStatus::NeedMore)
{
}
//---------------------------------------------------------------------------
int TWebGUIRequestParser::ErrorStatus() const
{
    return m_ErrorStatus;
}
//---------------------------------------------------------------------------
void TWebGUIRequestParser::Fail(int status)
{
    m_ErrorStatus = status;
    m_Status = EWebGUIParseStatus::Failed;
}
//---------------------------------------------------------------------------
/*
    TWebGUIRequestParser::Feed

    The head is read a byte at a time, so its limit holds however the bytes are split, and each line is
    checked as soon as it's complete; the body is taken in pieces.
*/
EWebGUIParseStatus TWebGUIRequestParser::Feed(char const* data, std::size_t size)
{
    std::size_t i = 0;

    while (m_Status == EWebGUIParseStatus::NeedMore && i < size)
    {
        if (m_InBody)
        {
            std::size_t const taken = (std::min)(size - i, m_ContentLength - m_Request.Body.size());
            m_Request.Body.append(data + i, taken);
            i += taken;

            if (m_Request.Body.size() == m_ContentLength)
                m_Status = EWebGUIParseStatus::Done;

            continue;
        }

        char const c = data[i];
        ++i;
        ++m_HeadSize;

        if (m_HeadSize > WebGUIMaxHeadSize)
        {
            Fail(431);
            break;
        }

        if (c != '\n')
        {
            m_Line += c;
            continue;
        }

        if (!m_Line.empty() && m_Line.back() == '\r')
            m_Line.pop_back();

        ParseLine();
        m_Line.clear();
    }

    return m_Status;
}
//---------------------------------------------------------------------------
/*
    TWebGUIRequestParser::ParseHeader

    A header line is 'name: value'. A space before the colon, or at the start of the line (the obsolete
    folding of a long value onto several lines), leaves a name that isn't a token, which HTTP says to refuse
    with 400. Content-Length must be digits only, so a list such as "5, 5" is refused too.
*/
void TWebGUIRequestParser::ParseHeader(std::string const& line)
{
    std::size_t const colon = line.find(':');

    if (colon == std::string::npos || !IsToken(line.substr(0, colon)))
    {
        Fail(400);
        return;
    }

    TWebGUIHeader header;
    header.Name = line.substr(0, colon);
    std::size_t const first = line.find_first_not_of(" \t", colon + 1);

    if (first != std::string::npos)
        header.Value = line.substr(first, line.find_last_not_of(" \t") + 1 - first);

    for (char const c : header.Value)
    {
        unsigned char const byte = static_cast<unsigned char>(c);

        if ((byte < 0x20 && c != '\t') || byte == 0x7F)
        {
            Fail(400);
            return;
        }
    }

    if (EqualsIgnoringCase(header.Name, "Transfer-Encoding"))
    {
        Fail(501);
        return;
    }

    if (EqualsIgnoringCase(header.Name, "Host") || EqualsIgnoringCase(header.Name, "Cookie"))
    {
        bool& seen = EqualsIgnoringCase(header.Name, "Host") ? m_HasHost : m_HasCookie;

        if (seen)
        {
            Fail(400);
            return;
        }

        seen = true;
    }

    if (EqualsIgnoringCase(header.Name, "Content-Length"))
    {
        if (m_HasContentLength || header.Value.empty())
        {
            Fail(400);
            return;
        }

        m_HasContentLength = true;
        std::size_t length = 0;

        for (char const c : header.Value)
        {
            if (c < '0' || c > '9')
            {
                Fail(400);
                return;
            }

            // Stops growing once past the limit, so a long number can't overflow.
            if (length <= WebGUIMaxBodySize)
                length = length * 10 + static_cast<std::size_t>(c - '0');
        }

        if (length > WebGUIMaxBodySize)
        {
            Fail(413);
            return;
        }

        m_ContentLength = length;
    }

    m_Request.Headers.push_back(std::move(header));
}
//---------------------------------------------------------------------------
/*
    TWebGUIRequestParser::ParseLine

    The empty line after the headers ends the head. A CR left in a line, which isn't part of a line break,
    is refused with 400 by the checks of the part it's in, as HTTP says to: no part may hold one.
*/
void TWebGUIRequestParser::ParseLine()
{
    if (m_Request.Method.empty())
    {
        if (!m_Line.empty())
            ParseRequestLine(m_Line);

        return;
    }

    if (!m_Line.empty())
    {
        ParseHeader(m_Line);
        return;
    }

    if (m_ContentLength == 0)
        m_Status = EWebGUIParseStatus::Done;
    else
    {
        m_Request.Body.reserve(m_ContentLength);
        m_InBody = true;
    }
}
//---------------------------------------------------------------------------
/*
    TWebGUIRequestParser::ParseRequestLine

    The request line is 'method target version', with one space between each; a further space ends up in
    the version, which then isn't one. A malformed line gets 400 before an unsupported version gets 505,
    and that before an unsupported method gets 405. The target's characters are the visible ASCII ones, as
    browsers send it, percent-encoding the rest.
*/
void TWebGUIRequestParser::ParseRequestLine(std::string const& line)
{
    std::size_t const firstSpace = line.find(' ');
    std::size_t const secondSpace = firstSpace == std::string::npos ? std::string::npos :
            line.find(' ', firstSpace + 1);

    if (secondSpace == std::string::npos)
    {
        Fail(400);
        return;
    }

    std::string const method = line.substr(0, firstSpace);
    std::string const target = line.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    std::string const version = line.substr(secondSpace + 1);
    bool validTarget = !target.empty() && target[0] == '/';

    for (char const c : target)
    {
        unsigned char const byte = static_cast<unsigned char>(c);
        validTarget = validTarget && byte >= 0x21 && byte <= 0x7E;
    }

    bool const versionShape = version.size() == 8 && version.compare(0, 5, "HTTP/") == 0 && version[5] >= '0' &&
        version[5] <= '9' && version[6] == '.' && version[7] >= '0' && version[7] <= '9';

    if (!IsToken(method) || !validTarget || !versionShape)
        Fail(400);
    else if (version != "HTTP/1.0" && version != "HTTP/1.1")
        Fail(505);
    else if (method != "GET" && method != "POST")
        Fail(405);
    else
    {
        std::size_t const question = target.find('?');
        m_Request.Method = method;
        m_Request.Path = target.substr(0, question);
        m_Request.Query = question == std::string::npos ? std::string() : target.substr(question + 1);
        m_Request.Version = version;
    }
}
//---------------------------------------------------------------------------
TWebGUIRequest const& TWebGUIRequestParser::Request() const
{
    return m_Request;
}
//---------------------------------------------------------------------------
EWebGUIParseStatus TWebGUIRequestParser::Status() const
{
    return m_Status;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
