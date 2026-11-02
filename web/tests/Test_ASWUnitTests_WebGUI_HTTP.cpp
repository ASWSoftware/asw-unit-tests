/* **************************************************************************
Test_ASWUnitTests_WebGUI_HTTP.cpp
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
#include "Test_ASWUnitTests_WebGUI_HTTP.h"
//---------------------------------------------------------------------------
#include <algorithm>
#include <cstddef>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_HTTP.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::string FieldsText(std::optional<std::vector<ASWUnitTests::TWebGUIField> > const& fields);
std::string HeaderText(ASWUnitTests::TWebGUIRequest const& request, std::string const& name);
std::string Outcome(ASWUnitTests::TWebGUIRequestParser const& parser);
ASWUnitTests::TWebGUIRequestParser Parse(std::string const& bytes);
std::string RequestText(ASWUnitTests::TWebGUIRequest const& request);

//---------------------------------------------------------------------------
/*
    FieldsText

    Decoded fields as "{name|value}" each, or "nullopt", for comparing in one check.
*/
std::string FieldsText(std::optional<std::vector<ASWUnitTests::TWebGUIField> > const& fields)
{
    if (!fields.has_value())
        return "nullopt";

    std::string text;

    for (ASWUnitTests::TWebGUIField const& field : *fields)
        text += "{" + field.Name + "|" + field.Value + "}";

    return text;
}
//---------------------------------------------------------------------------
/*
    HeaderText

    The value FindHeader() gives for 'name', or "<none>".
*/
std::string HeaderText(ASWUnitTests::TWebGUIRequest const& request, std::string const& name)
{
    std::string const* const value = request.FindHeader(name);
    return value != nullptr ? *value : "<none>";
}
//---------------------------------------------------------------------------
/*
    Outcome

    The parser's status, with the status code to answer when it failed: "Done", "NeedMore" or "Failed 400".
*/
std::string Outcome(ASWUnitTests::TWebGUIRequestParser const& parser)
{
    switch (parser.Status())
    {
        case ASWUnitTests::EWebGUIParseStatus::Done:
            return "Done";

        case ASWUnitTests::EWebGUIParseStatus::NeedMore:
            return "NeedMore";

        default:
            return "Failed " + std::to_string(parser.ErrorStatus());
    }
}
//---------------------------------------------------------------------------
/*
    Parse

    A parser fed 'bytes' all at once.
*/
ASWUnitTests::TWebGUIRequestParser Parse(std::string const& bytes)
{
    ASWUnitTests::TWebGUIRequestParser parser;
    parser.Feed(bytes.data(), bytes.size());
    return parser;
}
//---------------------------------------------------------------------------
/*
    RequestText

    A request's parts, joined by '|' for comparing in one check: method, path, query, version, then each
    header as "name=value", then the body.
*/
std::string RequestText(ASWUnitTests::TWebGUIRequest const& request)
{
    std::string text = request.Method + "|" + request.Path + "|" + request.Query + "|" + request.Version;

    for (ASWUnitTests::TWebGUIHeader const& header : request.Headers)
        text += "|" + header.Name + "=" + header.Value;

    return text + "|" + request.Body;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_HTTP
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_HTTP::TTest_ASWUnitTests_WebGUI_HTTP()
    : inherited("ASWUnitTests_WebGUI_HTTP_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_DecodeWebGUIForm_DecodesFields,
        "DecodeWebGUIForm_DecodesFields");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_DecodeWebGUIForm_RefusesMalformedEscapes,
        "DecodeWebGUIForm_RefusesMalformedEscapes");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_EnforcesLimits, "Parser_EnforcesLimits");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_ReadsAnySplit, "Parser_ReadsAnySplit");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_ReadsRequests, "Parser_ReadsRequests");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_RefusesMalformedRequests,
        "Parser_RefusesMalformedRequests");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_HTTP::Test_Response_WritesBytes, "Response_WritesBytes");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_HTTP::~TTest_ASWUnitTests_WebGUI_HTTP()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_DecodeWebGUIForm_DecodesFields()
{
    // Arrange: each input, and its fields.
    struct TCase
    {
        char const* Input;
        std::string Expected;
        char const* Name;
    };

    TCase const cases[] = {
        { "", "", "empty: no fields" },
        { "command=SetChecked&node=t12&checked=1", "{command|SetChecked}{node|t12}{checked|1}", "a command" },
        { "filter=a+b%2Bc", "{filter|a b+c}", "'+' is a space, %2B a '+'" },
        { "x=%26%3D%25%2b", "{x|&=%+}", "escaped '&', '=', '%' and '+' (lowercase hex)" },
        { "e=caf%C3%a9", "{e|caf" "\xC3\xA9" "}", "UTF-8 bytes, either case of hex" },
        { "%41%42=%43", "{AB|C}", "the name decoded too" },
        { "a&&b=&=c&", "{a|}{b|}{|c}", "no '=', an empty value, an empty name; empty pairs skipped" },
        { "a=1&a=2", "{a|1}{a|2}", "a repeated name kept, in order" },
        { "k=v=w", "{k|v=w}", "only the first '=' splits" },
        { "n=%FF", "{n|\xFF}", "any byte, not checked as UTF-8" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
        CheckEquals(testCase.Expected, FieldsText(DecodeWebGUIForm(testCase.Input)), __func__, __LINE__,
            testCase.Name);

    CheckEquals(std::string("{n|") + '\0' + "}", FieldsText(DecodeWebGUIForm("n=%00")), __func__, __LINE__,
        "a NUL byte");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_DecodeWebGUIForm_RefusesMalformedEscapes()
{
    // Arrange
    char const* const malformed[] = {
        "%",
        "a=%",
        "a=%4",
        "a=%G0",
        "a=%4G",
        "a=%%41",
        "a=%-1",
        "%zz=1",
        "ok=1&bad=%zz",
        "bad=50%&ok=1",
    };

    // Act, Assert
    for (char const* const text : malformed)
        CheckEquals(std::string("nullopt"), FieldsText(DecodeWebGUIForm(text)), __func__, __LINE__, text);
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_EnforcesLimits()
{
    // Arrange: a request whose head is 'size' bytes, padded in a header's value.
    std::string const start = "GET / HTTP/1.1\r\nX: ";
    std::string const end = "\r\n\r\n";
    auto const headOfSize = [&](std::size_t size)
        {
            return start + std::string(size - start.size() - end.size(), 'a') + end;
        };

    // Act, Assert: the head.
    CheckEquals(std::string("Done"), Outcome(Parse(headOfSize(WebGUIMaxHeadSize))), __func__, __LINE__,
        "a head of the most bytes allowed");
    CheckEquals(std::string("Failed 431"), Outcome(Parse(headOfSize(WebGUIMaxHeadSize + 1))), __func__, __LINE__,
        "one byte more");
    CheckEquals(std::string("Failed 431"), Outcome(Parse(start + std::string(WebGUIMaxHeadSize, 'a'))), __func__,
        __LINE__, "refused before the line ends");
    CheckEquals(std::string("Failed 431"), Outcome(Parse(std::string(WebGUIMaxHeadSize + 1, '\n'))), __func__,
        __LINE__, "the empty lines before the request line count");

    // Act, Assert: the body.
    std::string const post = "POST /command HTTP/1.1\r\nContent-Length: ";
    TWebGUIRequestParser largest = Parse(post + std::to_string(WebGUIMaxBodySize) + "\r\n\r\n");
    CheckEquals(std::string("NeedMore"), Outcome(largest), __func__, __LINE__, "the largest body allowed, awaited");
    std::string const body(WebGUIMaxBodySize, 'b');
    largest.Feed(body.data(), body.size());
    CheckEquals(std::string("Done"), Outcome(largest), __func__, __LINE__, "the largest body allowed, read");
    CheckEquals(body.size(), largest.Request().Body.size(), __func__, __LINE__, "all of it");

    CheckEquals(std::string("Failed 413"), Outcome(Parse(post + std::to_string(WebGUIMaxBodySize + 1) + "\r\n")),
        __func__, __LINE__, "a byte too large, refused as soon as the header is read");
    CheckEquals(std::string("Failed 413"), Outcome(Parse(post + "99999999999999999999999999\r\n")), __func__,
        __LINE__, "a number too large for any integer");
    CheckEquals(std::string("NeedMore"), Outcome(Parse(post + "000000000000000000000065536\r\n\r\n")), __func__,
        __LINE__, "leading zeros");
    CheckEquals(std::string("Failed 400"), Outcome(Parse(post + "99999999999999999999999999x\r\n")), __func__,
        __LINE__, "malformed before too large");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_ReadsAnySplit()
{
    // Arrange
    std::string const bytes = "POST /command?x=1 HTTP/1.1\r\nHost: localhost:1\r\nContent-Length: 5\r\n\r\nhello";
    std::string const expected = "POST|/command|x=1|HTTP/1.1|Host=localhost:1|Content-Length=5|hello";

    // Act, Assert: pieces of every size, the last one possibly shorter.
    for (std::size_t pieceSize = 1; pieceSize <= bytes.size(); ++pieceSize)
    {
        TWebGUIRequestParser parser;
        std::size_t fed = 0;
        bool doneEarly = false;

        while (fed < bytes.size())
        {
            doneEarly = doneEarly || parser.Status() != EWebGUIParseStatus::NeedMore;
            std::size_t const size = (std::min)(pieceSize, bytes.size() - fed);
            parser.Feed(bytes.data() + fed, size);
            fed += size;
        }

        std::string const name = "pieces of " + std::to_string(pieceSize);
        CheckFalse(doneEarly, __func__, __LINE__, name + ": needs more until the last byte");
        CheckEquals(std::string("Done"), Outcome(parser), __func__, __LINE__, name);
        CheckEquals(expected, RequestText(parser.Request()), __func__, __LINE__, name);
    }

    // Act, Assert: two pieces, split at each position.
    for (std::size_t split = 0; split <= bytes.size(); ++split)
    {
        TWebGUIRequestParser parser;
        parser.Feed(bytes.data(), split);
        parser.Feed(bytes.data() + split, bytes.size() - split);
        CheckEquals(expected, RequestText(parser.Request()), __func__, __LINE__, "split at " +
            std::to_string(split));
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_ReadsRequests()
{
    // Arrange, Act: a GET with a query and headers to trim, Host and Cookie once each.
    TWebGUIRequestParser const get = Parse("GET /detail?node=g3 HTTP/1.1\r\nHost: 127.0.0.1:8080\r\n"
        "Cookie: a=1\r\nAccept:  text/html \t\r\nX-Empty:\r\nX-Tab: a\tb\r\nX-Text: caf" "\xC3\xA9" "\r\n\r\n");

    // Assert
    CheckEquals(std::string("Done"), Outcome(get), __func__, __LINE__, "GET");
    CheckEquals(0, get.ErrorStatus(), __func__, __LINE__, "no error status");
    CheckEquals(std::string("GET|/detail|node=g3|HTTP/1.1|Host=127.0.0.1:8080|Cookie=a=1|Accept=text/html|"
        "X-Empty=|X-Tab=a\tb|X-Text=caf" "\xC3\xA9" "|"), RequestText(get.Request()), __func__, __LINE__,
        "values trimmed, inner tabs and UTF-8 kept");
    CheckEquals(std::string("127.0.0.1:8080"), HeaderText(get.Request(), "host"), __func__, __LINE__,
        "found in lowercase");
    CheckEquals(std::string("text/html"), HeaderText(get.Request(), "ACCEPT"), __func__, __LINE__,
        "found in uppercase");
    CheckEquals(std::string(), HeaderText(get.Request(), "x-empty"), __func__, __LINE__, "an empty value found");
    CheckEquals(std::string("<none>"), HeaderText(get.Request(), "Origin"), __func__, __LINE__, "none");
    CheckEquals(std::string("<none>"), HeaderText(get.Request(), "Hos"), __func__, __LINE__,
        "a prefix of a name isn't it");

    // Arrange, Act: HTTP/1.0, LF line breaks, empty lines first, a body, and bytes after it.
    TWebGUIRequestParser post = Parse("\r\n\nPOST /command HTTP/1.0\nContent-Length:  11 \n"
        "content-type: application/x-www-form-urlencoded\n\ncommand=Run" "GET / HTTP/1.1\r\n\r\n");

    // Assert
    CheckEquals(std::string("Done"), Outcome(post), __func__, __LINE__, "POST");
    CheckEquals(std::string("POST|/command||HTTP/1.0|Content-Length=11|"
        "content-type=application/x-www-form-urlencoded|command=Run"), RequestText(post.Request()), __func__,
        __LINE__, "the body, and nothing after it");
    std::string const more = "more";
    CheckTrue(post.Feed(more.data(), more.size()) == EWebGUIParseStatus::Done, __func__, __LINE__,
        "still done when fed more");
    CheckEquals(std::string("command=Run"), post.Request().Body, __func__, __LINE__, "the more ignored");

    // Act, Assert: the target's parts, and empty bodies.
    CheckEquals(std::string("GET|/||HTTP/1.1|"), RequestText(Parse("GET /? HTTP/1.1\r\n\r\n").Request()), __func__,
        __LINE__, "an empty query");
    CheckEquals(std::string("GET|/a|b?c=%20|HTTP/1.1|"),
        RequestText(Parse("GET /a?b?c=%20 HTTP/1.1\r\n\r\n").Request()), __func__, __LINE__,
        "split at the first '?', nothing decoded");
    CheckEquals(std::string("Done"), Outcome(Parse("POST /x HTTP/1.1\r\nContent-Length: 0\r\n\r\n")), __func__,
        __LINE__, "Content-Length 0: done at the empty line");
    CheckEquals(std::string("Done"), Outcome(Parse("POST /x HTTP/1.1\r\n\r\n")), __func__, __LINE__,
        "no Content-Length: no body");
    CheckEquals(std::string("NeedMore"), Outcome(Parse("GET / HTTP/1.1\r\n")), __func__, __LINE__,
        "no empty line yet");
    CheckEquals(std::string("NeedMore"), Outcome(Parse("\r\n\r\n\r\n")), __func__, __LINE__,
        "only empty lines: still waiting for the request line");

    TWebGUIRequestParser fresh;
    CheckTrue(fresh.Feed(nullptr, 0) == EWebGUIParseStatus::NeedMore, __func__, __LINE__, "fed nothing");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_Parser_RefusesMalformedRequests()
{
    // Arrange: request lines, and header lines sent after "GET / HTTP/1.1".
    struct TCase
    {
        std::string Input;
        char const* Expected;
        char const* Name;
    };

    TCase const requestLines[] = {
        { "get / HTTP/1.1", "Failed 405", "methods are case-sensitive" },
        { "HEAD / HTTP/1.1", "Failed 405", "HEAD" },
        { "PUT / HTTP/1.1", "Failed 405", "PUT" },
        { "GET / HTTP/2.0", "Failed 505", "HTTP/2.0" },
        { "GET / HTTP/1.2", "Failed 505", "HTTP/1.2" },
        { "PUT / HTTP/0.9", "Failed 505", "an unsupported version before an unsupported method" },
        { "GET / http/1.1", "Failed 400", "the version in lowercase" },
        { "GET / HTTP/1.10", "Failed 400", "a version with two digits" },
        { "GET / HTTP/1", "Failed 400", "a version without its minor digit" },
        { "GET /", "Failed 400", "no version" },
        { "GET", "Failed 400", "no target" },
        { "GET /  HTTP/1.1", "Failed 400", "two spaces" },
        { "GET / HTTP/1.1 ", "Failed 400", "a trailing space" },
        { " GET / HTTP/1.1", "Failed 400", "a leading space" },
        { "GET\t/ HTTP/1.1", "Failed 400", "a tab" },
        { "G(T / HTTP/1.1", "Failed 400", "a method that isn't a token" },
        { "PUT x HTTP/2.0", "Failed 400", "malformed before an unsupported version or method" },
        { "GET http://127.0.0.1/ HTTP/1.1", "Failed 400", "an absolute target" },
        { "GET * HTTP/1.1", "Failed 400", "the asterisk target" },
        { "GET /caf" "\xC3\xA9" " HTTP/1.1", "Failed 400", "a target that isn't ASCII" },
        { "GET /\r HTTP/1.1", "Failed 400", "a CR that doesn't end the line" },
        { std::string("GET /\0 HTTP/1.1", 15), "Failed 400", "a NUL in the target" },
    };

    TCase const headerLines[] = {
        { "Host: a\r\nhost: a", "Failed 400", "Host twice, in either case" },
        { "Cookie: a=1\r\nCOOKIE: b=2", "Failed 400", "Cookie twice" },
        { "Content-Length: 0\r\ncontent-length: 0", "Failed 400", "Content-Length twice, even equal" },
        { "Content-Length: 5, 5", "Failed 400", "Content-Length as a list" },
        { "Content-Length: -1", "Failed 400", "a negative Content-Length" },
        { "Content-Length: +5", "Failed 400", "a sign" },
        { "Content-Length: 0x10", "Failed 400", "hex" },
        { "Content-Length:", "Failed 400", "an empty Content-Length" },
        { "Transfer-Encoding: chunked", "Failed 501", "Transfer-Encoding" },
        { "transfer-encoding: identity", "Failed 501", "any Transfer-Encoding, in any case" },
        { "Host : a", "Failed 400", "a space before the colon" },
        { "X-A: 1\r\n folded", "Failed 400", "a folded line, with a space" },
        { "X-A: 1\r\n\tfolded", "Failed 400", "a folded line, with a tab" },
        { "NoColon", "Failed 400", "no colon" },
        { ": value", "Failed 400", "an empty name" },
        { "X(A): 1", "Failed 400", "a name that isn't a token" },
        { "X-A: a\x01" "b", "Failed 400", "a control character in a value" },
        { "X-A: a\x7F", "Failed 400", "DEL in a value" },
        { "X-A: a\rb", "Failed 400", "a bare CR in a value" },
        { std::string("X-A: a\0b", 8), "Failed 400", "a NUL in a value" },
    };

    // Act, Assert
    for (TCase const& testCase : requestLines)
        CheckEquals(std::string(testCase.Expected), Outcome(Parse(testCase.Input + "\r\n")), __func__, __LINE__,
            testCase.Name);

    for (TCase const& testCase : headerLines)
        CheckEquals(std::string(testCase.Expected), Outcome(Parse("GET / HTTP/1.1\r\n" + testCase.Input +
            "\r\n\r\n")), __func__, __LINE__, testCase.Name);

    TWebGUIRequestParser parser = Parse("PUT / HTTP/1.1\r\n");
    std::string const rest = "\r\n";
    CheckTrue(parser.Feed(rest.data(), rest.size()) == EWebGUIParseStatus::Failed, __func__, __LINE__,
        "still failed when fed more");
    CheckEquals(405, parser.ErrorStatus(), __func__, __LINE__, "the first error kept");
    CheckEquals(std::string(), parser.Request().Method, __func__, __LINE__, "no method taken from a refused line");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_HTTP::Test_Response_WritesBytes()
{
    // Arrange
    TWebGUIResponse json;
    json.ContentType = "application/json";
    json.Body = "{}";

    TWebGUIResponse redirect;
    redirect.Status = 303;
    redirect.Headers.push_back(TWebGUIHeader { "Location", "/" });
    redirect.Headers.push_back(TWebGUIHeader { "Set-Cookie", "a=b; Path=/" });

    TWebGUIResponse unknown;
    unknown.Status = 299;

    // Act, Assert
    CheckEquals(std::string("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: 2\r\n"
        "Cache-Control: no-store\r\nConnection: close\r\n\r\n{}"), WebGUIResponseBytes(json), __func__, __LINE__,
        "a 200 with a body");
    CheckEquals(std::string("HTTP/1.1 303 See Other\r\nContent-Length: 0\r\nCache-Control: no-store\r\n"
        "Connection: close\r\nLocation: /\r\nSet-Cookie: a=b; Path=/\r\n\r\n"), WebGUIResponseBytes(redirect),
        __func__, __LINE__, "no Content-Type, the response's own headers last, an empty body");
    CheckEquals(std::string("HTTP/1.1 299 \r\nContent-Length: 0\r\nCache-Control: no-store\r\nConnection: close\r\n"
        "\r\n"), WebGUIResponseBytes(unknown), __func__, __LINE__, "an unknown status code: an empty reason");
    CheckEquals(std::string("HTTP/1.1 405 Method Not Allowed\r\nContent-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: 19\r\nCache-Control: no-store\r\nConnection: close\r\nAllow: GET, POST\r\n\r\n"
        "Method Not Allowed\n"), WebGUIResponseBytes(WebGUIErrorResponse(405)), __func__, __LINE__,
        "a 405, with Allow");
    CheckEquals(std::string("HTTP/1.1 413 Content Too Large\r\nContent-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: 18\r\nCache-Control: no-store\r\nConnection: close\r\n\r\nContent Too Large\n"),
        WebGUIResponseBytes(WebGUIErrorResponse(413)), __func__, __LINE__, "a 413, without Allow");

    // Act, Assert: the reasons.
    struct TReason
    {
        int Status;
        char const* Reason;
    };

    TReason const reasons[] = {
        { 200, "OK" },
        { 204, "No Content" },
        { 303, "See Other" },
        { 400, "Bad Request" },
        { 403, "Forbidden" },
        { 404, "Not Found" },
        { 405, "Method Not Allowed" },
        { 413, "Content Too Large" },
        { 431, "Request Header Fields Too Large" },
        { 500, "Internal Server Error" },
        { 501, "Not Implemented" },
        { 505, "HTTP Version Not Supported" },
        { 201, "" },
    };

    for (TReason const& reason : reasons)
        CheckEquals(std::string(reason.Reason), WebGUIStatusReason(reason.Status), __func__, __LINE__,
            std::to_string(reason.Status));

    // Act, Assert: misuse.
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.Headers.push_back(TWebGUIHeader { "Location", "/\r\nSet-Cookie: x=1" });
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "CRLF in a value", "line break");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.ContentType = "text/plain\n";
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "LF in the content type", "line break");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.Headers.push_back(TWebGUIHeader { "Bad Name", "x" });
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "a space in a name", "malformed name");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.Headers.push_back(TWebGUIHeader { "", "x" });
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "an empty name", "malformed name");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.Status = 99;
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "a two-digit status code", "three digits");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIResponse response;
            response.Status = 1000;
            WebGUIResponseBytes(response);
        }, __func__, __LINE__, "a four-digit status code", "three digits");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_WebGUI_HTTP)
