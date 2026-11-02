/* **************************************************************************
ASWUnitTests_WebGUI_SSE.cpp
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
#include "ASWUnitTests_WebGUI_SSE.h"
//---------------------------------------------------------------------------
#include <cstddef>
#include <stdexcept>
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

//---------------------------------------------------------------------------
/*
    WebGUIEventMessage

    The page's EventSource strips one space after "data:", so a line that starts with a space keeps it.
*/
std::string WebGUIEventMessage(std::string const& data)
{
    std::string message = "data: ";
    message.reserve(data.size() + 8);

    for (std::size_t i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\r' || data[i] == '\n')
        {
            if (data[i] == '\r' && i + 1 < data.size() && data[i + 1] == '\n')
                ++i;

            message += "\ndata: ";
        }
        else
        {
            message += data[i];
        }
    }

    message += "\n\n";
    return message;
}
//---------------------------------------------------------------------------
std::string WebGUIEventPing()
{
    return ": ping\n";
}
//---------------------------------------------------------------------------
std::string WebGUIEventStreamHead()
{
    return "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream\r\nCache-Control: no-store\r\n\r\n";
}
//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TWebGUIEventQueue
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
void TWebGUIEventQueue::Append(std::string const& eventJSON)
{
    if (eventJSON.empty())
        throw std::logic_error("TWebGUIEventQueue: an empty event");

    if (!m_Events.empty())
        m_Events += ',';

    m_Events += eventJSON;
}
//---------------------------------------------------------------------------
bool TWebGUIEventQueue::IsEmpty() const
{
    return m_Events.empty();
}
//---------------------------------------------------------------------------
std::string TWebGUIEventQueue::Take()
{
    if (m_Events.empty())
        return std::string();

    std::string const message = WebGUIEventMessage("[" + m_Events + "]");
    m_Events.clear();
    return message;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
