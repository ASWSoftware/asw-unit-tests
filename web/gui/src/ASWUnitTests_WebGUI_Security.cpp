/* **************************************************************************
ASWUnitTests_WebGUI_Security.cpp
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
#include "ASWUnitTests_WebGUI_Security.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::string LowerASCII(std::string text);
std::string TrimSpaces(std::string const& text);

//---------------------------------------------------------------------------
/*
    LowerASCII

    'text' with its ASCII letters in lowercase, for comparing host names and header names ignoring case.
*/
std::string LowerASCII(std::string text)
{
    for (char& c : text)
    {
        if (c >= 'A' && c <= 'Z')
            c = static_cast<char>(c - 'A' + 'a');
    }

    return text;
}
//---------------------------------------------------------------------------
/*
    TrimSpaces

    'text' without the spaces and tabs at its ends.
*/
std::string TrimSpaces(std::string const& text)
{
    std::size_t const first = text.find_first_not_of(" \t");

    if (first == std::string::npos)
        return std::string();

    return text.substr(first, text.find_last_not_of(" \t") - first + 1);
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
/*
    DecideWebGUIAccess

    Each refusal gets the same answer, so a client learns nothing about which check failed.
*/
EWebGUIAccess DecideWebGUIAccess(TWebGUIRequest const& request, std::uint16_t port, std::string const& token,
    bool requirePostOrigin)
{
    if (!IsWebGUIToken(token))
        throw std::logic_error("DecideWebGUIAccess: the token isn't 32 lowercase hex digits");

    std::string const* const host = request.FindHeader("Host");

    if (host == nullptr || !IsWebGUIHostAllowed(*host, port))
        return EWebGUIAccess::Refused;

    bool hasOrigin = false;

    for (TWebGUIHeader const& header : request.Headers)
    {
        if (LowerASCII(header.Name) == "origin")
        {
            if (!IsWebGUIOriginAllowed(header.Value, port))
                return EWebGUIAccess::Refused;

            hasOrigin = true;
        }
    }

    if (!hasOrigin && requirePostOrigin && request.Method == "POST")
        return EWebGUIAccess::Refused;

    if (request.Method == "GET" && request.Path == "/")
    {
        std::optional<std::vector<TWebGUIField> > const fields = DecodeWebGUIForm(request.Query);
        std::vector<std::string> tokens;

        if (fields.has_value())
        {
            for (TWebGUIField const& field : *fields)
            {
                if (field.Name == "token")
                    tokens.push_back(field.Value);
            }
        }

        if (!tokens.empty())
        {
            if (tokens.size() == 1 && WebGUIEqualsInConstantTime(tokens[0], token))
                return EWebGUIAccess::TokenAccepted;

            return EWebGUIAccess::Refused;
        }
    }

    std::string const* const cookie = request.FindHeader("Cookie");

    if (cookie != nullptr)
    {
        for (std::string const& value : WebGUICookieValues(*cookie, WebGUICookieName(port)))
        {
            if (WebGUIEqualsInConstantTime(value, token))
                return EWebGUIAccess::Allowed;
        }
    }

    return EWebGUIAccess::Refused;
}
//---------------------------------------------------------------------------
bool IsWebGUIHostAllowed(std::string const& host, std::uint16_t port)
{
    std::string const name = LowerASCII(host);
    std::string const portSuffix = ":" + std::to_string(port);

    for (char const* const local : { "127.0.0.1", "localhost" })
    {
        if (name == local + portSuffix || (port == 80 && name == local))
            return true;
    }

    return false;
}
//---------------------------------------------------------------------------
bool IsWebGUIOriginAllowed(std::string const& origin, std::uint16_t port)
{
    return LowerASCII(origin.substr(0, 7)) == "http://" && IsWebGUIHostAllowed(origin.substr(7), port);
}
//---------------------------------------------------------------------------
bool IsWebGUIToken(std::string const& text)
{
    if (text.size() != 32)
        return false;

    for (char const c : text)
    {
        if ((c < '0' || c > '9') && (c < 'a' || c > 'f'))
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
std::string WebGUICookieName(std::uint16_t port)
{
    return "ASWUnitTests_" + std::to_string(port);
}
//---------------------------------------------------------------------------
/*
    WebGUICookieValues

    Cookies are separated by ';', and the spaces around each name and value are ignored, as browsers do. A
    part without '=' is skipped. Names are compared exactly, as cookies' names are case-sensitive.
*/
std::vector<std::string> WebGUICookieValues(std::string const& cookieHeader, std::string const& name)
{
    std::vector<std::string> values;
    std::size_t start = 0;

    while (start < cookieHeader.size())
    {
        std::size_t end = cookieHeader.find(';', start);

        if (end == std::string::npos)
            end = cookieHeader.size();

        std::string const part = cookieHeader.substr(start, end - start);
        std::size_t const equals = part.find('=');

        if (equals != std::string::npos && TrimSpaces(part.substr(0, equals)) == name)
            values.push_back(TrimSpaces(part.substr(equals + 1)));

        start = end + 1;
    }

    return values;
}
//---------------------------------------------------------------------------
/*
    WebGUIEqualsInConstantTime

    Every byte is compared, and the differences are only combined, never tested, until the end.
*/
bool WebGUIEqualsInConstantTime(std::string const& a, std::string const& b)
{
    if (a.size() != b.size())
        return false;

    unsigned char difference = 0;

    for (std::size_t i = 0; i < a.size(); ++i)
        difference |= static_cast<unsigned char>(a[i] ^ b[i]);

    return difference == 0;
}
//---------------------------------------------------------------------------
TWebGUIResponse WebGUIRefusedResponse()
{
    TWebGUIResponse response;
    response.Status = 403;
    response.ContentType = "text/plain; charset=utf-8";
    response.Body = "Forbidden: open the address the test runner printed.\n";
    return response;
}
//---------------------------------------------------------------------------
TWebGUIResponse WebGUITokenResponse(std::uint16_t port, std::string const& token)
{
    if (!IsWebGUIToken(token))
        throw std::logic_error("WebGUITokenResponse: the token isn't 32 lowercase hex digits");

    std::string const cookie = WebGUICookieName(port) + "=" + token + "; Path=/; HttpOnly; SameSite=Strict";
    TWebGUIResponse response;
    response.Status = 303;
    response.Headers.push_back(TWebGUIHeader { "Location", "/" });
    response.Headers.push_back(TWebGUIHeader { "Set-Cookie", cookie });
    return response;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
