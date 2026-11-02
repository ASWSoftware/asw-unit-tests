/* **************************************************************************
ASWUnitTests_WebGUI_Security.h
Author: Anthony S. West - ASW Software

The Web GUI runner's security checks, which keep anything but the page the
runner opened from driving it: the Host and Origin checks, the session
cookie and the request that sets it, the token's format and its
constant-time comparison, and the one function that decides whether a
request is served. Contains no socket code, so it's unit tested from the
console runners (see web/tests/Test_ASWUnitTests_WebGUI_Security.cpp).

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
#ifndef ASWUnitTests_WebGUI_SecurityH
#define ASWUnitTests_WebGUI_SecurityH
//---------------------------------------------------------------------------
#include <cstdint>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_HTTP.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------

enum class EWebGUIAccess
{
    // The request carries the session cookie: serve it.
    Allowed,
    // Answer it with WebGUIRefusedResponse().
    Refused,
    // The request opens the page with the right token: answer it with WebGUITokenResponse().
    TokenAccepted,
};

//---------------------------------------------------------------------------

// Whether the runner listening on 127.0.0.1:'port' with 'token' serves 'request'. In this order:
// - Every request needs a Host header naming this runner (IsWebGUIHostAllowed()), which stops DNS
//   rebinding: a site whose name resolves to 127.0.0.1 still sends its own name.
// - Every Origin header must name this runner's page (IsWebGUIOriginAllowed()); a POST without one is
//   refused when 'requirePostOrigin' is true.
// - A GET of "/" whose query has a "token" field is the request the runner opens: TokenAccepted if it has
//   one such field, equal to 'token', and Refused otherwise.
// - Any other request needs a cookie named WebGUICookieName('port') whose value is 'token'.
// Throws std::logic_error if 'token' isn't a token (IsWebGUIToken()), which would be a bug in the caller.
EWebGUIAccess DecideWebGUIAccess(TWebGUIRequest const& request, std::uint16_t port, std::string const& token,
    bool requirePostOrigin);
// Whether a Host header's value names the runner on 127.0.0.1:'port': "127.0.0.1:<port>" or
// "localhost:<port>", ignoring case, or either name alone when 'port' is 80, which browsers leave out.
bool IsWebGUIHostAllowed(std::string const& host, std::uint16_t port);
// Whether an Origin header's value names the page of the runner on 127.0.0.1:'port': "http://" and a value
// IsWebGUIHostAllowed() accepts.
bool IsWebGUIOriginAllowed(std::string const& origin, std::uint16_t port);
// Whether 'text' has a token's format: 32 lowercase hex digits, for 128 random bits.
bool IsWebGUIToken(std::string const& text);
// The session cookie's name for the runner on 'port', "ASWUnitTests_<port>". Browsers keep one cookie jar
// per host whatever the port, so two runners on 127.0.0.1 would otherwise overwrite each other's cookie.
std::string WebGUICookieName(std::uint16_t port);
// The values of every cookie named 'name' in a Cookie header's value ("a=1; b=2"), in order. A name can
// come more than once, e.g. when another server on 127.0.0.1 set a cookie of that name for another path.
std::vector<std::string> WebGUICookieValues(std::string const& cookieHeader, std::string const& name);
// Whether 'a' and 'b' are equal, taking the same time whichever bytes differ, so a client can't find a
// token byte by byte from the response times. Their lengths aren't secret, so a different length returns
// at once.
bool WebGUIEqualsInConstantTime(std::string const& a, std::string const& b);
// The response to a refused request: 403, with a line telling the user to open the address the runner
// printed, and nothing about which check failed.
TWebGUIResponse WebGUIRefusedResponse();
// The response to the request the runner opens ("/?token=<token>"): 303 to "/", setting the session
// cookie (HttpOnly, SameSite=Strict, Path=/), so the token leaves the address bar and the history. Throws
// std::logic_error if 'token' isn't a token (IsWebGUIToken()).
TWebGUIResponse WebGUITokenResponse(std::uint16_t port, std::string const& token);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_WebGUI_SecurityH
