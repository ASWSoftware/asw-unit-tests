/* **************************************************************************
Test_ASWUnitTests_WebGUI_SSE.cpp
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
#include "Test_ASWUnitTests_WebGUI_SSE.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_WebGUI_SSE.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------
/*
    DecodeEventStream

    The data of each message event a browser's EventSource fires for 'stream', each followed by '|', as the
    HTML Standard's event stream interpretation gives them: lines end with CRLF, CR or LF; a line starting
    with ':' is a comment; a "data" field's value, less one leading space, is added to the data with an LF;
    an empty line fires an event with the data less its last LF, unless no data came, and starts anew. An
    unfinished line at the end isn't read yet.
*/
std::string DecodeEventStream(std::string const& stream)
{
    std::string events;
    std::string data;
    std::size_t i = 0;

    while (i < stream.size())
    {
        std::size_t const end = stream.find_first_of("\r\n", i);

        if (end == std::string::npos)
            break;

        std::string const line = stream.substr(i, end - i);
        i = end + (stream[end] == '\r' && end + 1 < stream.size() && stream[end + 1] == '\n' ? 2 : 1);

        if (line.empty())
        {
            if (!data.empty())
            {
                data.erase(data.size() - 1);
                events += data + "|";
                data.clear();
            }
        }
        else if (line[0] != ':')
        {
            std::size_t const colon = line.find(':');
            std::string value = colon == std::string::npos ? std::string() : line.substr(colon + 1);

            if (!value.empty() && value[0] == ' ')
                value.erase(0, 1);

            if (line.substr(0, colon) == "data")
                data += value + "\n";
        }
    }

    return events;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_WebGUI_SSE
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_SSE::TTest_ASWUnitTests_WebGUI_SSE()
    : inherited("ASWUnitTests_WebGUI_SSE_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_WebGUI_SSE::Test_EventMessage_FramesEachLine, "EventMessage_FramesEachLine");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_SSE::Test_EventPing_IsAComment, "EventPing_IsAComment");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_SSE::Test_EventQueue_TakesAllEventsAsOneMessage,
        "EventQueue_TakesAllEventsAsOneMessage");
    RegisterTest(&TTest_ASWUnitTests_WebGUI_SSE::Test_EventStreamHead_HasNoLength, "EventStreamHead_HasNoLength");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_WebGUI_SSE::~TTest_ASWUnitTests_WebGUI_SSE()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::Test_EventMessage_FramesEachLine()
{
    // Arrange: each data, its message, and the data the page's EventSource gets from it.
    struct TCase
    {
        char const* Data;
        char const* Message;
        char const* Received;
        char const* Name;
    };

    TCase const cases[] = {
        { "[{\"type\":\"log\"}]", "data: [{\"type\":\"log\"}]\n\n", "[{\"type\":\"log\"}]|", "one line" },
        { "", "data: \n\n", "|", "empty: still an event" },
        { "a\nb", "data: a\ndata: b\n\n", "a\nb|", "LF" },
        { "a\r\nb", "data: a\ndata: b\n\n", "a\nb|", "CRLF as one line break" },
        { "a\rb", "data: a\ndata: b\n\n", "a\nb|", "CR alone" },
        { "a\n\nb", "data: a\ndata: \ndata: b\n\n", "a\n\nb|", "an empty line in the data" },
        { "a\n", "data: a\ndata: \n\n", "a\n|", "a trailing line break kept" },
        { "\r\n\r", "data: \ndata: \ndata: \n\n", "\n\n|", "only line breaks" },
        { "\n\r", "data: \ndata: \ndata: \n\n", "\n\n|", "LF then CR: two line breaks" },
        { " lead", "data:  lead\n\n", " lead|", "a leading space kept" },
        { ": not a comment", "data: : not a comment\n\n", ": not a comment|", "a colon at the start" },
        { "caf" "\xC3\xA9", "data: caf" "\xC3\xA9" "\n\n", "caf" "\xC3\xA9" "|", "UTF-8 as it is" },
    };

    // Act, Assert
    for (TCase const& testCase : cases)
    {
        std::string const message = WebGUIEventMessage(testCase.Data);
        CheckEquals(std::string(testCase.Message), message, __func__, __LINE__, testCase.Name);
        CheckEquals(std::string(testCase.Received), DecodeEventStream(message), __func__, __LINE__,
            std::string(testCase.Name) + ": received");
    }

    CheckEquals(std::string("a|b|"), DecodeEventStream(WebGUIEventMessage("a") + WebGUIEventMessage("b")),
        __func__, __LINE__, "two messages in a row: two events");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::Test_EventPing_IsAComment()
{
    // Act, Assert
    CheckEquals(std::string(": ping\n"), WebGUIEventPing(), __func__, __LINE__, "a comment line");
    CheckEquals(std::string(), DecodeEventStream(WebGUIEventPing() + WebGUIEventPing()), __func__, __LINE__,
        "no event");
    CheckEquals(std::string("a|b|"), DecodeEventStream(WebGUIEventMessage("a") + WebGUIEventPing() +
        WebGUIEventMessage("b")), __func__, __LINE__, "between messages, changing neither");
    CheckEquals(15, WebGUIEventPingSeconds, __func__, __LINE__, "every 15 seconds (P2-D6)");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::Test_EventQueue_TakesAllEventsAsOneMessage()
{
    // Arrange
    TWebGUIEventQueue queue;
    bool const emptyAtFirst = queue.IsEmpty();
    std::string const nothing = queue.Take();

    // Act
    queue.Append("{\"type\":\"started\",\"test\":\"t1\"}");
    queue.Append("{\"type\":\"log\",\"text\":\"a\\nb\"}");
    bool const emptyAfterAppend = queue.IsEmpty();
    std::string const first = queue.Take();
    bool const emptyAfterTake = queue.IsEmpty();
    std::string const again = queue.Take();
    queue.Append("{\"type\":\"finished\"}");
    std::string const second = queue.Take();

    // Assert
    CheckTrue(emptyAtFirst, __func__, __LINE__, "empty at first");
    CheckEquals(std::string(), nothing, __func__, __LINE__, "nothing happened: nothing to send");
    CheckFalse(emptyAfterAppend, __func__, __LINE__, "not empty after Append()");
    CheckEquals(std::string("data: [{\"type\":\"started\",\"test\":\"t1\"},{\"type\":\"log\",\"text\":\"a\\nb\"}]\n\n"),
        first, __func__, __LINE__, "both events in one array, in one message, on one line");
    CheckTrue(emptyAfterTake, __func__, __LINE__, "empty after Take()");
    CheckEquals(std::string(), again, __func__, __LINE__, "taken once only");
    CheckEquals(std::string("data: [{\"type\":\"finished\"}]\n\n"), second, __func__, __LINE__,
        "a single event, after a Take(), with no comma");
    CheckThrows<std::logic_error>([]
        {
            TWebGUIEventQueue empty;
            empty.Append("");
        }, __func__, __LINE__, "an empty event", "empty event");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_WebGUI_SSE::Test_EventStreamHead_HasNoLength()
{
    // Act, Assert
    CheckEquals(std::string("HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-store\r\n\r\n"),
        WebGUIEventStreamHead(), __func__, __LINE__, "no Content-Length and no Connection: close");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_WebGUI_SSE)
