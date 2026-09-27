/* **************************************************************************
ASWUnitTests_GUI_RunObserver.cpp
Author: Anthony S. West - ASW Software

See header for info.

Copyright 2026 Anthony S. West

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
#include "ASWUnitTests_GUI_RunObserver.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIRunObserver
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TGUIRunObserver::TGUIRunObserver()
    : m_StopRequested(false)
{
}
//---------------------------------------------------------------------------
void TGUIRunObserver::Flush()
{
    std::string const text = TakePendingLog();

    if (!text.empty() && LogTextCallback != nullptr)
        LogTextCallback(text);
}
//---------------------------------------------------------------------------
void TGUIRunObserver::OnLog(std::string const& text)
{
    std::lock_guard<std::mutex> lock(m_PendingLogMutex);
    m_PendingLog += text;
}
//---------------------------------------------------------------------------
void TGUIRunObserver::OnTestFinished(TTestCaseRecord const& record)
{
    // Everything logged since the test started belongs to it, including its own "Finished test" line.
    std::string const text = TakePendingLog();
    m_CurrentTestLog += text;

    if (!text.empty() && LogTextCallback != nullptr)
        LogTextCallback(text);

    if (TestFinishedCallback != nullptr)
        TestFinishedCallback(record, m_CurrentTestLog);

    m_CurrentTestLog.clear();

    if (YieldCallback != nullptr)
        YieldCallback();
}
//---------------------------------------------------------------------------
void TGUIRunObserver::OnTestStarted(std::string const& groupName, std::string const& testName)
{
    // Anything logged before the test started (e.g. its group's setup) doesn't belong to it.
    Flush();
    m_CurrentTestLog.clear();

    if (TestStartedCallback != nullptr)
        TestStartedCallback(groupName, testName);

    if (YieldCallback != nullptr)
        YieldCallback();
}
//---------------------------------------------------------------------------
void TGUIRunObserver::RequestStop()
{
    m_StopRequested = true;
}
//---------------------------------------------------------------------------
void TGUIRunObserver::ResetStop()
{
    m_StopRequested = false;
}
//---------------------------------------------------------------------------
bool TGUIRunObserver::StopRequested()
{
    return m_StopRequested;
}
//---------------------------------------------------------------------------
std::string TGUIRunObserver::TakePendingLog()
{
    std::lock_guard<std::mutex> lock(m_PendingLogMutex);

    std::string text;
    text.swap(m_PendingLog);
    return text;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
