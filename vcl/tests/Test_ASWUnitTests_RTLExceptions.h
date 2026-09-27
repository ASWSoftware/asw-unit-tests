/* **************************************************************************
Test_ASWUnitTests_RTLExceptions.h
Author: Anthony S. West - ASW Software

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
#ifndef Test_ASWUnitTests_RTLExceptionsH
#define Test_ASWUnitTests_RTLExceptionsH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_RTLExceptions
//
// Exercises the framework's opt-in RTL exception support (see
// ASWUnitTests_Exception.h): SetExceptionExpected<TException>() with RTL
// exception types, the RTL catch in TTestGroupBase::Test(), and the
// DescribeRTLException()/RTLExceptionMessage() helpers. Like
// TTest_ASWUnitTests_TestBase, it runs small unregistered fixture groups
// (see the .cpp) directly and inspects their Results(), so a deliberately
// failing fixture test never affects this run's own counts or exit code.
//
// Only compiled when ASWUNITTESTS_RTL_EXCEPTIONS is defined, so this module
// is only listed in the vcl/ projects.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_RTLExceptions : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Helpers
    void CheckFixtureOutcomes(std::optional<unsigned int> testTimeoutSeconds, bool catchCrashes,
        std::string const& method, int line);

private: // Test methods
    void Test_DescribeRTLException_IncludesClassNameAndMessage();
    void Test_RTLExceptionMessage_ConvertsToUTF8();
    void Test_Run_PropagatesUnexpectedRTLException();
    void Test_Run_WrapsUnexpectedRTLExceptionFromWorkerThread();
    void Test_SetExceptionExpected_MatchesRTLTypeAndMessage();
    void Test_SetExceptionExpected_MatchesRTLTypeAndMessageOnWorkerThread();
    void Test_SetExceptionExpected_MatchesRTLTypeAndMessageUnderCatchCrashes();
    void Test_TExceptRTLException_KeepsClassAndDescriptionAfterOriginalIsFreed();

public:
    TTest_ASWUnitTests_RTLExceptions();
    ~TTest_ASWUnitTests_RTLExceptions() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

#endif // #if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_RTLExceptionsH
