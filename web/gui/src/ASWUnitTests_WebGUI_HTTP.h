/* **************************************************************************
ASWUnitTests_WebGUI_HTTP.h
Author: Anthony S. West - ASW Software

HTTP/1.1, as the Web GUI runner's server speaks it with its page: an
incremental request parser with limits, the bytes of a response, and the
decoding of form fields and query strings. Only the subset a local page
needs: GET and POST, bodies sized by Content-Length, and one request per
connection. Contains no socket code, so it's unit tested from the console
runners (see web/tests/Test_ASWUnitTests_WebGUI_HTTP.cpp).

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
#ifndef ASWUnitTests_WebGUI_HTTPH
#define ASWUnitTests_WebGUI_HTTPH
//---------------------------------------------------------------------------
#include <cstddef>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------

// The most bytes a request's head may have, from its first byte to the end of the empty line after its
// headers; a longer one gets 431.
constexpr std::size_t WebGUIMaxHeadSize = 16384;
// The largest Content-Length a request may have; a larger one gets 413.
constexpr std::size_t WebGUIMaxBodySize = 65536;

//---------------------------------------------------------------------------

enum class EWebGUIParseStatus
{
    // The request is complete.
    Done,
    // The request is malformed or not supported: TWebGUIRequestParser::ErrorStatus() says how to answer it.
    Failed,
    // More bytes are needed.
    NeedMore,
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIField
//
// A form field or query string parameter, decoded.
/////////////////////////////////////////////////////////////////////////////
struct TWebGUIField
{
    std::string Name;
    std::string Value;
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIHeader
//
// A header field of a request or a response.
/////////////////////////////////////////////////////////////////////////////
struct TWebGUIHeader
{
    // The name, as it was sent.
    std::string Name;
    // The value, without the spaces and tabs around it.
    std::string Value;
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIRequest
//
// A request, as TWebGUIRequestParser read it.
/////////////////////////////////////////////////////////////////////////////
struct TWebGUIRequest
{
    // The body: Content-Length bytes, none without that header.
    std::string Body;
    // The header fields, in the order they were sent.
    std::vector<TWebGUIHeader> Headers;
    // "GET" or "POST".
    std::string Method;
    // The request target up to its '?', as it was sent (not percent-decoded); always starts with '/'.
    std::string Path;
    // The request target after its '?', as it was sent; empty without one. DecodeWebGUIForm() decodes it.
    std::string Query;
    // "HTTP/1.0" or "HTTP/1.1".
    std::string Version;

    // The value of the header field named 'name', compared ignoring case; nullptr if there's none. Host,
    // Content-Length and Cookie can't be repeated (the parser refuses that), so they have only one value.
    std::string const* FindHeader(std::string const& name) const;
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIResponse
//
// A response, whose bytes WebGUIResponseBytes() writes.
/////////////////////////////////////////////////////////////////////////////
struct TWebGUIResponse
{
    std::string Body;
    // The Content-Type header's value; none is sent when it's empty.
    std::string ContentType;
    // Header fields sent after the ones every response has, e.g. Location or Set-Cookie.
    std::vector<TWebGUIHeader> Headers;
    int Status = 200;
};


/////////////////////////////////////////////////////////////////////////////
// TWebGUIRequestParser
//
// Reads one request from the bytes a connection receives, fed to it in
// pieces of any size as they arrive. Lines may end with CRLF or LF alone,
// and empty lines before the request line are skipped. It answers
// malformed requests with 400, methods other than GET and POST with 405,
// a Transfer-Encoding header with 501, HTTP versions other than 1.0 and
// 1.1 with 505, a head longer than WebGUIMaxHeadSize with 431 and a body
// larger than WebGUIMaxBodySize with 413, as soon as it sees the problem.
// A repeated Host, Content-Length or Cookie header is malformed, since
// two servers could each take a different one. The request target must
// start with '/', as browsers send it.
//
// Bytes after the request are ignored, since the server answers one
// request per connection and then closes it.
/////////////////////////////////////////////////////////////////////////////
class TWebGUIRequestParser
{
private:
    std::size_t m_ContentLength;
    int m_ErrorStatus;
    // Whether a Content-Length, Cookie or Host header was read, so another one is refused.
    bool m_HasContentLength;
    bool m_HasCookie;
    bool m_HasHost;
    std::size_t m_HeadSize;
    bool m_InBody;
    // The line being read, without its line break.
    std::string m_Line;
    TWebGUIRequest m_Request;
    EWebGUIParseStatus m_Status;

private:
    // Ends parsing with 'status' as the answer.
    void Fail(int status);
    // Reads a header field's line.
    void ParseHeader(std::string const& line);
    // Reads a complete line of the head.
    void ParseLine();
    // Reads the request line.
    void ParseRequestLine(std::string const& line);

public:
    TWebGUIRequestParser();

    // The status code to answer a failed request with (400, 405, 413, 431, 501 or 505); 0 otherwise.
    int ErrorStatus() const;
    // Reads the next 'size' bytes of the connection; once the status is Done or Failed, ignores them.
    EWebGUIParseStatus Feed(char const* data, std::size_t size);
    // The request read so far; complete once the status is Done.
    TWebGUIRequest const& Request() const;
    EWebGUIParseStatus Status() const;
};

//---------------------------------------------------------------------------

// The fields of a form body (application/x-www-form-urlencoded, as fetch() sends a URLSearchParams) or a
// query string, in order: 'name=value' pairs separated by '&', '+' standing for a space and %XX for any
// byte. A pair without '=' has an empty value, and empty pairs are skipped. A '%' not followed by two hex
// digits gives std::nullopt, since the page never sends one. The decoded bytes aren't checked as UTF-8.
std::optional<std::vector<TWebGUIField> > DecodeWebGUIForm(std::string const& text);
// A plain text response with status code 'status' and its reason phrase as the body, with the Allow header
// a 405 needs; for answering a request TWebGUIRequestParser failed.
TWebGUIResponse WebGUIErrorResponse(int status);
// The bytes of 'response': the status line, Content-Type (if any), Content-Length, Cache-Control:
// no-store, Connection: close, the response's own header fields, an empty line and the body. Throws
// std::logic_error if a header name or value holds a line break, which would be a bug in the caller.
std::string WebGUIResponseBytes(TWebGUIResponse const& response);
// The reason phrase of each status code the server sends ("OK", "See Other", "Not Found", ...); empty
// for any other, which HTTP allows.
std::string WebGUIStatusReason(int status);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_WebGUI_HTTPH
