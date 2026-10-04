/* **************************************************************************
ASWUnitTests_GUI_RunObserver.h
Author: Anthony S. West - ASW Software

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
#ifndef ASWUnitTests_GUI_RunObserverH
#define ASWUnitTests_GUI_RunObserverH
//---------------------------------------------------------------------------
#include <atomic>
#include <functional>
#include <mutex>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIRunObserver
//
// The VCL GUI runner's ITestRunObserver. It contains no VCL code itself: it
// hands each event to the std::function callbacks below, which the main form
// sets, so its behavior can be unit tested without a window (see
// vcl/tests/Test_ASWUnitTests_GUI_RunObserver.cpp).
//
// Log text can arrive from a test's worker thread (see ITestRunObserver), so
// OnLog() only appends it to a buffer under a lock. The buffer is handed to
// LogTextCallback when a test starts or finishes, and by Flush(), so every
// callback runs on the thread that called TTestHandler::Run(), which for the
// GUI is its main (VCL) thread.
//
// Text logged between a test's start and finish is also collected as that
// test's own log and passed to TestFinishedCallback, for the detail pane.
/////////////////////////////////////////////////////////////////////////////
class TGUIRunObserver : public ITestRunObserver
{
private:
    std::string m_CurrentTestLog;
    std::string m_PendingLog;
    std::mutex m_PendingLogMutex;
    std::atomic<bool> m_StopRequested;

private:
    std::string TakePendingLog();

public:
    // Receives log text, in the order it was logged.
    std::function<void (std::string const& text)> LogTextCallback;
    // Receives each test's record, plus the text it logged between starting and finishing.
    std::function<void (TTestCaseRecord const& record, std::string const& testLog)> TestFinishedCallback;
    std::function<void (std::string const& groupName, std::string const& testName)> TestStartedCallback;
    // Called after each test starts and after it finishes, so the GUI can process pending messages (repaint,
    // a click on Stop) while tests run on its main thread.
    std::function<void()> YieldCallback;

public:
    TGUIRunObserver();

    // Hands any buffered log text to LogTextCallback. Call after TTestHandler::Initialize() or Run() returns,
    // for the text logged after the last test event.
    void Flush();
    void OnLog(std::string const& text) override;
    void OnTestFinished(TTestCaseRecord const& record) override;
    void OnTestStarted(std::string const& groupName, std::string const& testName) override;
    // Asks the run to stop before its next test. Safe to call from any thread.
    void RequestStop();
    // Clears an earlier RequestStop(), ready for the next run.
    void ResetStop();
    bool StopRequested() override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_RunObserverH
