/* **************************************************************************
Test_ASWUnitTests_GUI_RunObserver.cpp
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
#include "Test_ASWUnitTests_GUI_RunObserver.h"
//---------------------------------------------------------------------------
#include <string>
#include <thread>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_RunObserver.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

/////////////////////////////////////////////////////////////////////////////
// TCallbackRecorder
//
// Wires a TGUIRunObserver's callbacks up to record, in order, everything
// they're called with, as "log:<text>", "started:<test>", "finished:<test>",
// and "yield" strings; each finished test's own log is kept separately. Also
// tracks whether every callback ran on the thread that constructed it.
/////////////////////////////////////////////////////////////////////////////
class TCallbackRecorder
{
private:
    std::thread::id const m_ConstructingThread;

public:
    bool CallbacksOnConstructingThread = true;
    std::vector<std::string> Calls;
    std::vector<std::string> TestLogs;

public:
    explicit TCallbackRecorder(TGUIRunObserver& observer)
        : m_ConstructingThread(std::this_thread::get_id())
    {
        observer.LogTextCallback = [this](std::string const& text)
            {
                Record("log:" + text);
            };
        observer.TestStartedCallback = [this](std::string const& /*groupName*/, std::string const& testName)
            {
                Record("started:" + testName);
            };
        observer.TestFinishedCallback = [this](TTestCaseRecord const& record, std::string const& testLog)
            {
                Record("finished:" + record.TestName);
                TestLogs.push_back(testLog);
            };
        observer.YieldCallback = [this]()
            {
                Record("yield");
            };
    }

    void Record(std::string const& call)
    {
        CallbacksOnConstructingThread = CallbacksOnConstructingThread &&
            (std::this_thread::get_id() == m_ConstructingThread);
        Calls.push_back(call);
    }
};


/////////////////////////////////////////////////////////////////////////////
// TFixture_GUIObserved
//
// A never-registered (no ASW_REGISTER_TEST_GROUP) fixture group with logging
// left on, so its real log output flows through a TGUIRunObserver: one test
// that passes and one that fails a Check*, so each has distinct log text.
/////////////////////////////////////////////////////////////////////////////
class TFixture_GUIObserved : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    void Test_FailsCheck();
    void Test_Passes();

public:
    TFixture_GUIObserved();

    void SetUp_Group() override {}
    void TearDown_Group() override {}
};

//---------------------------------------------------------------------------
TFixture_GUIObserved::TFixture_GUIObserved()
    : inherited("Fixture_GUIObserved")
{
    RegisterTest(&TFixture_GUIObserved::Test_FailsCheck, "FailsCheck");
    RegisterTest(&TFixture_GUIObserved::Test_Passes, "Passes");
}
//---------------------------------------------------------------------------
void TFixture_GUIObserved::Test_FailsCheck()
{
    CheckTrue(false, __func__, __LINE__, "deliberate Check failure, fixture test");
}
//---------------------------------------------------------------------------
void TFixture_GUIObserved::Test_Passes()
{
    CheckTrue(true, __func__, __LINE__, "trivially passes");
}
//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_RunObserver
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_RunObserver::TTest_ASWUnitTests_GUI_RunObserver()
    : inherited("ASWUnitTests_GUI_RunObserver_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_RunObserver::Test_Events_ArriveInOrderWithEachTestsOwnLog,
        "Events_ArriveInOrderWithEachTestsOwnLog");
    RegisterTest(&TTest_ASWUnitTests_GUI_RunObserver::Test_Flush_HandsOverLogAfterLastTest, "Flush_HandsOverLogAfterLastTest");
    RegisterTest(&TTest_ASWUnitTests_GUI_RunObserver::Test_OnLog_FromWorkerThreadWaitsForNextEvent,
        "OnLog_FromWorkerThreadWaitsForNextEvent");
    RegisterTest(&TTest_ASWUnitTests_GUI_RunObserver::Test_Run_ReportsEachTestWithItsOwnLogUnderTimeout,
        "Run_ReportsEachTestWithItsOwnLogUnderTimeout");
    RegisterTest(&TTest_ASWUnitTests_GUI_RunObserver::Test_StopRequest_CanBeRequestedAndReset, "StopRequest_CanBeRequestedAndReset");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_RunObserver::~TTest_ASWUnitTests_GUI_RunObserver()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::Test_Events_ArriveInOrderWithEachTestsOwnLog()
{
    // Arrange
    TGUIRunObserver observer;
    TCallbackRecorder recorder(observer);
    TTestCaseRecord record{ "Group", "Test", 0.0, TTestOutcome::Pass, std::string() };

    // Act
    observer.OnLog("before\n");
    observer.OnTestStarted("Group", "Test");
    observer.OnLog("during\n");
    observer.OnTestFinished(record);

    // Assert
    std::vector<std::string> const expected{
        "log:before\n", "started:Test", "yield", "log:during\n", "finished:Test", "yield" };
    CheckTrue(recorder.Calls == expected, __func__, __LINE__,
        "log text is handed over at each event, in order, with a yield after each start and finish");
    AssertEquals(static_cast<size_t>(1), recorder.TestLogs.size(), __func__, __LINE__, "one finished test");
    CheckEquals(std::string("during\n"), recorder.TestLogs.front(), __func__, __LINE__,
        "the test's own log is only what was logged between its start and finish");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::Test_Flush_HandsOverLogAfterLastTest()
{
    // Arrange
    TGUIRunObserver observer;
    TCallbackRecorder recorder(observer);

    // Act
    observer.OnLog("Tests done\n");
    size_t const callsBeforeFlush = recorder.Calls.size();
    observer.Flush();
    observer.Flush(); // Nothing left, so no second call.

    // Assert
    CheckEquals(static_cast<size_t>(0), callsBeforeFlush, __func__, __LINE__, "logged text waits to be handed over");
    CheckTrue(recorder.Calls == std::vector<std::string>{ "log:Tests done\n" }, __func__, __LINE__,
        "Flush() hands it over exactly once");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::Test_OnLog_FromWorkerThreadWaitsForNextEvent()
{
    // Arrange
    TGUIRunObserver observer;
    TCallbackRecorder recorder(observer);

    // Act
    std::thread worker([&observer]()
            {
        observer.OnLog("from a worker thread\n");
            });
    worker.join();
    size_t const callsAfterWorker = recorder.Calls.size();
    observer.Flush();

    // Assert
    CheckEquals(static_cast<size_t>(0), callsAfterWorker, __func__, __LINE__,
        "the worker's log call doesn't call back on the worker thread");
    CheckTrue(recorder.Calls == std::vector<std::string>{ "log:from a worker thread\n" }, __func__, __LINE__,
        "the text arrives at the next hand-over");
    CheckTrue(recorder.CallbacksOnConstructingThread, __func__, __LINE__, "on the thread that made the call");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::Test_Run_ReportsEachTestWithItsOwnLogUnderTimeout()
{
    // Arrange
    // A timeout runs each test on a worker thread, where its log text is written, so this covers the path the
    // GUI relies on most: log text from another thread, callbacks on this one.
    TFixture_GUIObserved fixture;
    TGUIRunObserver observer;
    TCallbackRecorder recorder(observer);
    fixture.SetRunObserver(&observer);

    // Act
    fixture.Run(TestFilter(), std::nullopt, 30u, false); // Only long enough never to fire.
    observer.Flush();

    // Assert
    AssertEquals(static_cast<size_t>(2), recorder.TestLogs.size(), __func__, __LINE__, "both tests were reported");
    CheckContains(recorder.TestLogs[0], "Running test: Fixture_GUIObserved.FailsCheck", __func__, __LINE__,
        "the first test's log is its own");
    CheckContains(recorder.TestLogs[0], "deliberate Check failure", __func__, __LINE__, "including its Check failure");
    CheckContains(recorder.TestLogs[1], "Running test: Fixture_GUIObserved.Passes", __func__, __LINE__,
        "the second test's log is its own");
    CheckNotContains(recorder.TestLogs[1], "deliberate Check failure", __func__, __LINE__,
        "with nothing carried over from the first");
    CheckTrue(recorder.CallbacksOnConstructingThread, __func__, __LINE__,
        "every callback ran on the calling thread, though the tests ran on worker threads");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_RunObserver::Test_StopRequest_CanBeRequestedAndReset()
{
    // Arrange
    TGUIRunObserver observer;
    bool const initially = observer.StopRequested();

    // Act
    std::thread requester([&observer]()
            {
        observer.RequestStop();
            });
    requester.join();
    bool const afterRequest = observer.StopRequested();
    observer.ResetStop();
    bool const afterReset = observer.StopRequested();

    // Assert
    CheckFalse(initially, __func__, __LINE__, "no stop is requested initially");
    CheckTrue(afterRequest, __func__, __LINE__, "a stop requested from another thread is seen");
    CheckFalse(afterReset, __func__, __LINE__, "ResetStop() clears it for the next run");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_RunObserver)
