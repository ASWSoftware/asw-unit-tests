/* **************************************************************************
ASWUnitTests_WebGUI_SSE.h
Author: Anthony S. West - ASW Software

Server-Sent Events, as the Web GUI runner's server pushes changes to its
page: the event stream's response head, its messages and its keep-alive
comment, and the queue that collects events between flushes. Contains no
socket code, so it's unit tested from the console runners (see
web/tests/Test_ASWUnitTests_WebGUI_SSE.cpp).

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
#ifndef ASWUnitTests_WebGUI_SSEH
#define ASWUnitTests_WebGUI_SSEH
//---------------------------------------------------------------------------
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------

// How often an idle event stream is sent WebGUIEventPing(), so the server notices a stream whose tab is gone.
constexpr int WebGUIEventPingSeconds = 15;


/////////////////////////////////////////////////////////////////////////////
// TWebGUIEventQueue
//
// The events that happened since the last flush, each a JSON value (an
// object with a "type"), as the server collects them between yields. A
// flush takes them all as one message holding a JSON array, which the
// server sends to every event stream: one message per flush, however many
// events, keeps a fast run from flooding the page.
/////////////////////////////////////////////////////////////////////////////
class TWebGUIEventQueue
{
private:
    // The events' JSON, separated by commas.
    std::string m_Events;

public:
    // Adds an event: 'eventJSON' must be one complete JSON value, such as TWebGUIJSONWriter::Text() once
    // it's complete. Throws std::logic_error if it's empty, which would be a bug in the caller.
    void Append(std::string const& eventJSON);
    // Whether no event was appended since the last Take().
    bool IsEmpty() const;
    // The events appended since the last Take(), as one WebGUIEventMessage() holding a JSON array of them,
    // and empties the queue; an empty string when there are none, so nothing is sent.
    std::string Take();
};

//---------------------------------------------------------------------------

// An event stream's message carrying 'data': a "data: " line for each of its lines, then an empty line,
// which makes the page's EventSource fire one message event with 'data' as it is. A line break in 'data'
// (CRLF, CR or LF, all of which end a line in an event stream) starts a new "data: " line, and the page
// gets it back as LF.
std::string WebGUIEventMessage(std::string const& data);
// A comment line, which the page's EventSource ignores, sent on an idle stream every WebGUIEventPingSeconds:
// writing it to a stream whose tab is gone fails, so the server notices.
std::string WebGUIEventPing();
// The head of the event stream's response: 200, Content-Type: text/event-stream and Cache-Control:
// no-store, and no Content-Length, since the stream's messages go on until the server or the page closes
// it. It's the one response that doesn't say Connection: close, as it doesn't end after a body.
std::string WebGUIEventStreamHead();

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_WebGUI_SSEH
