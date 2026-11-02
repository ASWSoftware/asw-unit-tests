/* **************************************************************************
Test_ASWUnitTests_WebGUI_Security.cpp
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
#include "Test_ASWUnitTests_WebGUI_Security.h"
//---------------------------------------------------------------------------
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_HTTP.h"
#include "ASWUnitTests_WebGUI_Security.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

// The token the tests' runner has, and another one.
std::string const OtherToken = "fedcba9876543210fedcba9876543210";
std::string const RunnerToken = "0123456789abcdef0123456789abcdef";

//---------------------------------------------------------------------------

std::string Decide(std::string const& head, bool requirePostOrigin);
std::string JoinValues(std::vector<std::string> const& values);

//---------------------------------------------------------------------------
/*
    Decide

    What DecideWebGUIAccess() says about the request 'head' (its lines, each ending with CRLF, without the
    empty line that ends it) for the runner on port 8080 with RunnerToken: "Allowed", "Refused" or
    "TokenAccepted", or "not parsed" if TWebGUIRequestParser didn't read it.
*/
std::string Decide(std::string const& head, bool requirePostOrigin)
{
    std::string const bytes = head + "\r\n";
    ASWUnitTests::TWebGUIRequestParser parser;

    if (parser.Feed(bytes.data(), bytes.size()) != ASWUnitTests::EWebGUIParseStatus::Done)
        return "not parsed";

    switch (ASWUnitTests::DecideWebGUIAccess(parser.Request(), 8080, RunnerToken, requirePostOrigin))
    {
        case ASWUnitTests::EWebGUIAccess::Allowed:
            return "Allowed";

        case ASWUnitTests::EWebGUIAccess::Refused:
            return "Refused";

        default:
            return "TokenAccepted";
    }
}
//---------------------------------------------------------------------------
/*
    JoinValues

    Cookie values as "{value}" each, for comparing in one check.
*/
std::string JoinValues(std::vector<std::string> const& values)
{
    std::string text;

    for (std::string const& value : values)
        text += "{" + value + "}";

    return text;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_Security
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_Security::TTest_ASWUnitTests_WebGUI_Security()
    : inherited("ASWUnitTests_WebGUI_Security_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_WebGUI_Security::Test_CookieValues_FindsEveryCopyOfACookie,
        "CookieValues_FindsEveryCopyOfACookie");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_Security::Test_DecideAccess_ServesOnlyThisRunnersPage,
        "DecideAccess_ServesOnlyThisRunnersPage");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_Security::Test_HostAndOrigin_NameThisRunner,
        "HostAndOrigin_NameThisRunner");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_Security::Test_Responses_RedirectAndRefuse,
        "Responses_RedirectAndRefuse");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_Security::Test_Token_FormatAndConstantTimeCompare,
        "Token_FormatAndConstantTimeCompare");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_Security::~TTest_ASWUnitTests_WebGUI_Security()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::Test_CookieValues_FindsEveryCopyOfACookie()
{
    // Arrange: each Cookie header, and the values of ASWUnitTests_8080 in it.
    struct TCase
    {
        char const* Header;
        char const* Values;
        char const* Name;
    };

    TCase const cases[] = {
        { "ASWUnitTests_8080=abc", "{abc}", "the only cookie" },
        { "a=1; ASWUnitTests_8080=abc; b=2", "{abc}", "among others, as browsers send them" },
        { "a=1;ASWUnitTests_8080=abc;b=2", "{abc}", "no spaces" },
        { "\t ASWUnitTests_8080 \t=\t abc\t ; b=2", "{abc}", "spaces and tabs around the name and the value" },
        { "ASWUnitTests_8080=x; ASWUnitTests_8080=abc", "{x}{abc}", "two copies, in order" },
        { "ASWUnitTests_8081=abc", "", "another port's cookie" },
        { "ASWUnitTests_80800=abc; ASWUnitTests_808=abc", "", "a port that starts or ends the same" },
        { "aswunittests_8080=abc", "", "names are case-sensitive" },
        { "ASWUnitTests_8080", "", "no '=': skipped" },
        { "ASWUnitTests_8080=", "{}", "an empty value" },
        { "ASWUnitTests_8080=a=b", "{a=b}", "'=' in the value" },
        { "", "", "an empty header" },
        { ";;  ; ASWUnitTests_8080=abc;", "{abc}", "empty parts" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
    {
        CheckEquals(std::string(testCase.Values), JoinValues(WebGUICookieValues(testCase.Header,
            WebGUICookieName(8080))), __func__, __LINE__, testCase.Name);
    }

    CheckEquals(std::string("ASWUnitTests_8080"), WebGUICookieName(8080), __func__, __LINE__, "the name");
    CheckEquals(std::string("ASWUnitTests_1"), WebGUICookieName(1), __func__, __LINE__, "the name, a short port");
    CheckEquals(std::string("ASWUnitTests_65535"), WebGUICookieName(65535), __func__, __LINE__,
        "the name, the largest port");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::Test_DecideAccess_ServesOnlyThisRunnersPage()
{
    // Arrange: each request to the runner on port 8080, whether a POST must have an Origin, and the answer.
    std::string const get = "GET /state HTTP/1.1\r\n";
    std::string const post = "POST /command HTTP/1.1\r\n";
    std::string const host = "Host: 127.0.0.1:8080\r\n";
    std::string const cookie = "Cookie: ASWUnitTests_8080=" + RunnerToken + "\r\n";
    std::string const origin = "Origin: http://127.0.0.1:8080\r\n";

    struct TCase
    {
        std::string Head;
        bool RequirePostOrigin;
        char const* Expected;
        char const* Name;
    };

    TCase const cases[] = {
        { get + host + cookie, false, "Allowed", "the cookie" },
        { get + "Host: localhost:8080\r\n" + cookie, false, "Allowed", "Host localhost" },
        { get + "Host: LocalHost:8080\r\n" + cookie, false, "Allowed", "Host ignoring case" },
        { get + "Host: 127.0.0.1:8081\r\n" + cookie, false, "Refused", "Host: another port" },
        { get + "Host: rebound.example:8080\r\n" + cookie, false, "Refused", "Host: another name (DNS rebinding)" },
        { "GET /state HTTP/1.0\r\n" + cookie, false, "Refused", "no Host" },
        { get + host, false, "Refused", "no cookie" },
        { get + host + "Cookie: ASWUnitTests_8080=" + OtherToken + "\r\n", false, "Refused", "another token" },
        { get + host + "Cookie: ASWUnitTests_8081=" + RunnerToken + "\r\n", false, "Refused",
          "another port's cookie" },
        { get + host + "Cookie: ASWUnitTests_8080=" + RunnerToken.substr(1) + "\r\n", false, "Refused",
          "a token one digit short" },
        { get + host + "Cookie: a=1; ASWUnitTests_8080=" + OtherToken + "; ASWUnitTests_8080=" + RunnerToken +
          "\r\n", false, "Allowed", "a stale copy of the cookie first" },
        { get + host + "Cookie: ASWUnitTests_8080=" + RunnerToken + "; ASWUnitTests_8080=" + OtherToken + "\r\n",
          false, "Allowed", "a stale copy of the cookie last" },
        { "GET /?token=" + RunnerToken + " HTTP/1.1\r\n" + host, false, "TokenAccepted", "the token" },
        { "GET /?token=" + RunnerToken + " HTTP/1.1\r\n" + host + cookie, false, "TokenAccepted",
          "the token, with the cookie already set" },
        { "GET /?lang=en&token=" + RunnerToken + " HTTP/1.1\r\n" + host, false, "TokenAccepted",
          "the token among other fields" },
        { "GET /?token=" + OtherToken + " HTTP/1.1\r\n" + host + cookie, false, "Refused",
          "another token, even with the cookie" },
        { "GET /?token=" + RunnerToken + "&token=" + RunnerToken + " HTTP/1.1\r\n" + host, false, "Refused",
          "two tokens" },
        { "GET /?token= HTTP/1.1\r\n" + host + cookie, false, "Refused", "an empty token" },
        { "GET /?token=" + RunnerToken + " HTTP/1.1\r\nHost: 127.0.0.1:8081\r\n", false, "Refused",
          "the token, Host another port" },
        { "GET /index.html?token=" + RunnerToken + " HTTP/1.1\r\n" + host, false, "Refused",
          "the token on another path" },
        { "POST /?token=" + RunnerToken + " HTTP/1.1\r\n" + host + origin, false, "Refused", "the token in a POST" },
        { "GET /?Token=" + RunnerToken + " HTTP/1.1\r\n" + host, false, "Refused", "the field's name is \"token\"" },
        { "GET /?lang=en HTTP/1.1\r\n" + host + cookie, false, "Allowed", "/ with a query but no token" },
        { "GET /?token=%zz HTTP/1.1\r\n" + host + cookie, false, "Allowed",
          "a malformed query: not a token request" },
        { post + host + cookie + origin, false, "Allowed", "a POST from the page" },
        { post + host + cookie + "Origin: http://localhost:8080\r\n", false, "Allowed",
          "a POST from the page at localhost" },
        { post + host + cookie + "Origin: http://evil.example\r\n", false, "Refused", "a POST from another site" },
        { post + host + cookie + "Origin: http://127.0.0.1:8081\r\n", false, "Refused",
          "a POST from another port" },
        { post + host + cookie + "Origin: null\r\n", false, "Refused", "a POST from an opaque origin" },
        { post + host + cookie, false, "Allowed", "a POST without Origin, allowed" },
        { post + host + cookie, true, "Refused", "a POST without Origin, required" },
        { post + host + cookie + origin, true, "Allowed", "a POST with Origin, required" },
        { get + host + cookie, true, "Allowed", "a GET without Origin, required on POSTs only" },
        { get + host + cookie + "Origin: http://evil.example\r\n", false, "Refused", "a GET from another site" },
        { post + host + cookie + origin + "Origin: http://evil.example\r\n", false, "Refused",
          "two Origins, the second another site's" },
        { post + host + cookie + "origin: http://evil.example\r\n", false, "Refused",
          "the Origin header's name ignoring case" },
        { post + host + origin, false, "Refused", "a POST from the page without the cookie" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
    {
        CheckEquals(std::string(testCase.Expected), Decide(testCase.Head, testCase.RequirePostOrigin), __func__,
            __LINE__, testCase.Name);
    }

    std::string const notTokens[] = { std::string(), RunnerToken.substr(1), "0123456789ABCDEF0123456789abcdef" };

    for (std::string const& token : notTokens)
    {
        CheckThrows<std::logic_error>([&token]
            {
                TWebGUIRequest request;
                DecideWebGUIAccess(request, 8080, token, false);
            }, __func__, __LINE__, "the runner's token isn't a token: \"" + token + "\"",
            "isn't 32 lowercase hex digits");
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::Test_HostAndOrigin_NameThisRunner()
{
    // Arrange: each Host header's value, the port, and whether it names the runner; Origin is "http://" and
    // the same value.
    struct TCase
    {
        char const* Host;
        std::uint16_t Port;
        bool Allowed;
        char const* Name;
    };

    TCase const cases[] = {
        { "127.0.0.1:8080", 8080, true, "127.0.0.1" },
        { "localhost:8080", 8080, true, "localhost" },
        { "LOCALHOST:8080", 8080, true, "ignoring case" },
        { "127.0.0.1:8081", 8080, false, "another port" },
        { "127.0.0.1:808", 8080, false, "a port's prefix" },
        { "127.0.0.1:80800", 8080, false, "a longer port" },
        { "127.0.0.1:08080", 8080, false, "a leading zero" },
        { "127.0.0.1", 8080, false, "no port" },
        { "127.0.0.1:", 8080, false, "an empty port" },
        { "", 8080, false, "empty" },
        { "127.0.0.2:8080", 8080, false, "another loopback address" },
        { "[::1]:8080", 8080, false, "IPv6 loopback, not listened on" },
        { "localhost.:8080", 8080, false, "a trailing dot" },
        { "evil.example:8080", 8080, false, "another name (DNS rebinding)" },
        { "localhost.evil.example:8080", 8080, false, "a name that starts with localhost" },
        { " 127.0.0.1:8080", 8080, false, "a space" },
        { "127.0.0.1", 80, true, "port 80 left out" },
        { "localhost", 80, true, "port 80 left out, localhost" },
        { "127.0.0.1:80", 80, true, "port 80 given" },
        { "127.0.0.1", 8000, false, "port 8000 left out" },
        { "127.0.0.1:65535", 65535, true, "the largest port" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
    {
        std::string const origin = std::string("http://") + testCase.Host;
        CheckEquals(testCase.Allowed, IsWebGUIHostAllowed(testCase.Host, testCase.Port), __func__, __LINE__,
            std::string("Host: ") + testCase.Name);
        CheckEquals(testCase.Allowed, IsWebGUIOriginAllowed(origin, testCase.Port), __func__, __LINE__,
            std::string("Origin: ") + testCase.Name);
    }

    CheckTrue(IsWebGUIOriginAllowed("HTTP://127.0.0.1:8080", 8080), __func__, __LINE__, "Origin: scheme ignoring case");
    CheckFalse(IsWebGUIOriginAllowed("https://127.0.0.1:8080", 8080), __func__, __LINE__, "Origin: https");
    CheckFalse(IsWebGUIOriginAllowed("file://127.0.0.1:8080", 8080), __func__, __LINE__,
        "Origin: another scheme as long as http's");
    CheckFalse(IsWebGUIOriginAllowed("http://127.0.0.1:8080/", 8080), __func__, __LINE__, "Origin: a path");
    CheckFalse(IsWebGUIOriginAllowed("127.0.0.1:8080", 8080), __func__, __LINE__, "Origin: no scheme");
    CheckFalse(IsWebGUIOriginAllowed("null", 8080), __func__, __LINE__, "Origin: opaque");
    CheckFalse(IsWebGUIOriginAllowed("", 8080), __func__, __LINE__, "Origin: empty");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::Test_Responses_RedirectAndRefuse()
{
    // Act
    std::string const redirect = WebGUIResponseBytes(WebGUITokenResponse(8080, RunnerToken));
    std::string const refused = WebGUIResponseBytes(WebGUIRefusedResponse());

    // Assert
    CheckEquals(std::string("HTTP/1.1 303 See Other\r\nContent-Length: 0\r\nCache-Control: no-store\r\n"
        "Connection: close\r\nLocation: /\r\nSet-Cookie: ASWUnitTests_8080=") + RunnerToken +
        "; Path=/; HttpOnly; SameSite=Strict\r\n\r\n", redirect, __func__, __LINE__,
        "303 to /, setting the port's cookie");
    CheckEquals(std::string("HTTP/1.1 403 Forbidden\r\nContent-Type: text/plain; charset=utf-8\r\n"
        "Content-Length: 53\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n"
        "Forbidden: open the address the test runner printed.\n"), refused, __func__, __LINE__,
        "403, one line, nothing about the check");
    CheckThrows<std::logic_error>([]
        {
            WebGUITokenResponse(8080, "token");
        }, __func__, __LINE__, "a cookie with a token that isn't one", "isn't 32 lowercase hex digits");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_Security::Test_Token_FormatAndConstantTimeCompare()
{
    // Arrange: each text, and whether it's a token.
    struct TFormatCase
    {
        std::string Text;
        bool IsToken;
        char const* Name;
    };

    TFormatCase const formatCases[] = {
        { RunnerToken, true, "digits and a-f" },
        { std::string(32, '0'), true, "all zeros" },
        { std::string(32, 'f'), true, "all f" },
        { RunnerToken.substr(1), false, "31 digits" },
        { RunnerToken + "0", false, "33 digits" },
        { std::string(), false, "empty" },
        { "0123456789ABCDEF0123456789abcdef", false, "uppercase" },
        { "0123456789abcdeg0123456789abcdef", false, "a 'g'" },
        { "0123456789abcdef0123456789abcde/", false, "'/', before '0'" },
        { "0123456789abcdef0123456789abcde:", false, "':', after '9'" },
        { "0123456789abcdef0123456789abcde`", false, "'`', before 'a'" },
        { "0123456789abcdef 123456789abcdef", false, "a space" },
        { std::string("0123456789abcdef0123456789abcde") + '\0', false, "a NUL" },
    };

    // Act, Assert
    for (TFormatCase const& testCase : formatCases)
        CheckEquals(testCase.IsToken, IsWebGUIToken(testCase.Text), __func__, __LINE__, testCase.Name);

    CheckTrue(WebGUIEqualsInConstantTime(RunnerToken, RunnerToken), __func__, __LINE__, "equal");
    CheckTrue(WebGUIEqualsInConstantTime("", ""), __func__, __LINE__, "both empty");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken, OtherToken), __func__, __LINE__, "unequal");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken, "1" + RunnerToken.substr(1)), __func__, __LINE__,
        "the first byte differs");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken, RunnerToken.substr(0, 31) + "e"), __func__, __LINE__,
        "the last byte differs");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken, RunnerToken.substr(0, 31)), __func__, __LINE__,
        "a prefix");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken.substr(0, 31), RunnerToken), __func__, __LINE__,
        "longer");
    CheckFalse(WebGUIEqualsInConstantTime(RunnerToken, ""), __func__, __LINE__, "empty");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_WebGUI_Security)
