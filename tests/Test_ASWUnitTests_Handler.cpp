/* **************************************************************************
Test_ASWUnitTests_Handler.cpp
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
#include "Test_ASWUnitTests_Handler.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Handler.h"
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

TestFilter WildcardMatchTestsFilter(bool alsoStdOutRedirectTests);

//---------------------------------------------------------------------------

// Matches only this group's own WildcardMatch_* tests (pure static-function checks, so safe to run from a
// nested TTestHandler inside this group, and never including the test doing the nesting), plus, when asked,
// every ASWUnitTests_StdOutRedirect_Tests test, a second group that runs after this one.
TestFilter WildcardMatchTestsFilter(bool alsoStdOutRedirectTests)
{
    return [alsoStdOutRedirectTests](std::string const& fullName)
        {
            return fullName.rfind("ASWUnitTests_Handler_Tests.WildcardMatch_", 0) == 0 ||
                (alsoStdOutRedirectTests && fullName.rfind("ASWUnitTests_StdOutRedirect_Tests.", 0) == 0);
        };
}

//---------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////////
// TRecordingObserver
//
// An ITestRunObserver that records what it's told, for the SetRunObserver_* tests below. No test here
// uses a timeout, so every call arrives on the calling thread and no locking is needed.
/////////////////////////////////////////////////////////////////////////////
class TRecordingObserver : public ITestRunObserver
{
public:
    std::vector<std::string> FinishedGroupNames;
    std::string LogText;
    size_t StopAfterFinishedCount = static_cast<size_t>(-1);

public:
    void OnLog(std::string const& text) override
    {
        LogText += text;
    }

    void OnTestFinished(TTestCaseRecord const& record) override
    {
        FinishedGroupNames.push_back(record.GroupName);
    }

    void OnTestStarted(std::string const& /*groupName*/, std::string const& /*testName*/) override
    {
    }

    bool StopRequested() override
    {
        return FinishedGroupNames.size() >= StopAfterFinishedCount;
    }
};

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Handler
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_Handler::TTest_ASWUnitTests_Handler()
    : inherited("ASWUnitTests_Handler_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_GetTests_MatchesGetAllTestFullNames,
        "GetTests_MatchesGetAllTestFullNames");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesInitializeAndRunOutput,
        "SetRunObserver_ReceivesInitializeAndRunOutput");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_SetRunObserver_StopsBetweenGroups, "SetRunObserver_StopsBetweenGroups");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_CaseSensitivity, "WildcardMatch_CaseSensitivity");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_ExactAndStar, "WildcardMatch_ExactAndStar");
    RegisterTest(&TTest_ASWUnitTests_Handler::Test_WildcardMatch_QuestionMark, "WildcardMatch_QuestionMark");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_Handler::~TTest_ASWUnitTests_Handler()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_GetTests_MatchesGetAllTestFullNames()
{
    // Arrange
    TTestHandler handler;
    {
        TStdOutRedirect const suppressOutput; // Initialize() logs its own version/registration banner.
        handler.Initialize("Test_GetTests_MatchesGetAllTestFullNames");
    }

    // Act
    std::vector<TTestId> const tests = handler.GetTests();
    std::vector<std::string> const fullNames = handler.GetAllTestFullNames();

    // Assert
    AssertEquals(fullNames.size(), tests.size(), __func__, __LINE__, "one entry per registered test in each");
    CheckFalse(tests.empty(), __func__, __LINE__, "the real, self-registered suite is listed");

    bool foundThisTest = false;

    for (size_t i = 0; i < tests.size(); ++i)
    {
        CheckEquals(fullNames[i], tests[i].GroupName + "." + tests[i].TestName, __func__, __LINE__,
            "same tests, in the same canonical order, just with group and test names kept separate");

        if (tests[i].GroupName == "ASWUnitTests_Handler_Tests" &&
            tests[i].TestName == "GetTests_MatchesGetAllTestFullNames")
            foundThisTest = true;
    }

    CheckTrue(foundThisTest, __func__, __LINE__, "this very test is listed, under its own group");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_SetRunObserver_ReceivesInitializeAndRunOutput()
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    handler.SetRunObserver(&observer); // Before Initialize(), so its output and the groups it creates are covered.
    std::string consoleOutput;
    TTestResults results;

    // Act
    {
        TStdOutRedirect redirect;
        handler.Initialize("ObserverTestProject");
        results = handler.Run(WildcardMatchTestsFilter(false), "WildcardMatch_* tests only");
        consoleOutput = redirect.Str();
    }

    // Assert
    CheckEquals(3u, results.SuccessCount, __func__, __LINE__, "the three WildcardMatch_* tests ran and passed");
    CheckEquals(static_cast<size_t>(3), observer.FinishedGroupNames.size(), __func__, __LINE__,
        "each was reported to the observer, through the groups Initialize() created after it was set");
    CheckTrue(observer.LogText.find("registering test groups for ObserverTestProject") != std::string::npos,
        __func__, __LINE__, "Initialize()'s output went to the observer");
    CheckTrue(observer.LogText.find("Tests done") != std::string::npos, __func__, __LINE__,
        "and so did Run()'s own summary");
    CheckTrue(consoleOutput.empty(), __func__, __LINE__, "none of it went to std::cout");
    CheckFalse(results.Stopped, __func__, __LINE__, "a run the observer never asked to stop isn't marked stopped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_SetRunObserver_StopsBetweenGroups()
{
    // Arrange
    TTestHandler handler;
    TRecordingObserver observer;
    observer.StopAfterFinishedCount = 3; // Exactly this group's WildcardMatch_* tests, then stop.
    handler.SetRunObserver(&observer);
    TTestResults results;

    // Act
    handler.Initialize("ObserverTestProject");
    results = handler.Run(WildcardMatchTestsFilter(true), "WildcardMatch_* and StdOutRedirect tests");

    // Assert
    CheckTrue(results.Stopped, __func__, __LINE__, "the results say the run was stopped");
    CheckEquals(static_cast<size_t>(3), results.CaseRecords.size(), __func__, __LINE__,
        "only the first group's tests ran");
    CheckTrue(observer.LogText.find("\"ASWUnitTests_StdOutRedirect_Tests\"") == std::string::npos, __func__,
        __LINE__, "the second group was never even set up");
    CheckTrue(observer.LogText.find("Run stopped on request") != std::string::npos, __func__, __LINE__,
        "the log says the run was stopped");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_CaseSensitivity()
{
    // Act & Assert
    CheckFalse(TTestHandler::WildcardMatch("ABC", "abc"), __func__, __LINE__, "case-sensitive by default");
    CheckTrue(TTestHandler::WildcardMatch("ABC", "abc", true), __func__, __LINE__, "case-insensitive when requested");
    CheckTrue(TTestHandler::WildcardMatch("*STRING*", "Test_ASWTools_String.cpp", true),
        __func__, __LINE__, "case-insensitive substring search");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_ExactAndStar()
{
    // Act & Assert
    CheckTrue(TTestHandler::WildcardMatch("abc", "abc"), __func__, __LINE__, "exact match");
    CheckFalse(TTestHandler::WildcardMatch("abc", "abd"), __func__, __LINE__, "exact mismatch");
    CheckTrue(TTestHandler::WildcardMatch("*", "anything"), __func__, __LINE__, "bare star matches everything");
    CheckTrue(TTestHandler::WildcardMatch("*", ""), __func__, __LINE__, "bare star matches empty text");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "abc"), __func__, __LINE__, "star matches one char");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "abbbbc"), __func__, __LINE__, "star matches many chars");
    CheckTrue(TTestHandler::WildcardMatch("a*c", "ac"), __func__, __LINE__, "star matches zero chars");
    CheckTrue(TTestHandler::WildcardMatch("*.cpp", "Test_ASWTools_String.cpp"), __func__, __LINE__, "leading star");
    CheckTrue(TTestHandler::WildcardMatch("Test_*", "Test_ASWTools_String.cpp"), __func__, __LINE__, "trailing star");
    CheckFalse(TTestHandler::WildcardMatch("String", "Test_ASWTools_String.cpp"),
        __func__, __LINE__, "a substring alone does not match - the whole name must match");
    CheckTrue(TTestHandler::WildcardMatch("*String*", "Test_ASWTools_String.cpp"),
        __func__, __LINE__, "explicit substring search via surrounding stars");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_Handler::Test_WildcardMatch_QuestionMark()
{
    // Act & Assert
    CheckTrue(TTestHandler::WildcardMatch("a?c", "abc"), __func__, __LINE__, "question mark matches one char");
    CheckFalse(TTestHandler::WildcardMatch("a?c", "ac"), __func__, __LINE__, "question mark requires a char, not zero");
    CheckFalse(TTestHandler::WildcardMatch("a?c", "abbc"), __func__, __LINE__, "question mark matches one char, not two");
    CheckTrue(TTestHandler::WildcardMatch("???", "abc"), __func__, __LINE__, "three question marks match three chars");
    CheckFalse(TTestHandler::WildcardMatch("??", "abc"), __func__, __LINE__, "too few question marks for the text length");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_Handler)
