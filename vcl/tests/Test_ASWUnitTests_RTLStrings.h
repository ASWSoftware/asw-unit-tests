/* **************************************************************************
Test_ASWUnitTests_RTLStrings.h
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
#ifndef Test_ASWUnitTests_RTLStringsH
#define Test_ASWUnitTests_RTLStringsH
//---------------------------------------------------------------------------
#include <string>
#include <type_traits>
#include <utility>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

#if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_RTLStrings
//
// Exercises the System::String overloads of the Check*/Assert* string methods
// (see ASWUnitTests_TestBase.h). Like TTest_ASWUnitTests_TestBase, it runs a
// small unregistered fixture group (see the .cpp) directly and inspects its
// Results(), so a deliberately failing fixture test never affects this run's
// own counts or exit code.
//
// Only compiled when ASWUNITTESTS_RTL_EXCEPTIONS is defined, so this module
// is only listed in the vcl/ projects.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_RTLStrings : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Helpers
    // Whether CheckContains()/CheckEquals() accept the two argument types, as a test in this class would call them.
    // TGroup is always this class; as a template parameter, it defers the member access until the class is complete.
    template <typename TFirst, typename TSecond, typename TGroup = TTest_ASWUnitTests_RTLStrings, typename = void>
    struct TCanCheckContains : std::false_type
    {
    };
    template <typename TFirst, typename TSecond, typename TGroup>
    struct TCanCheckContains<TFirst, TSecond, TGroup, std::void_t<decltype(std::declval<TGroup&>().CheckContains(
        std::declval<TFirst>(), std::declval<TSecond>(), std::string(), 0, std::string()))> > : std::true_type
    {
    };
    template <typename TText, typename TGroup = TTest_ASWUnitTests_RTLStrings, typename = void>
    struct TCanCheckEmpty : std::false_type
    {
    };
    template <typename TText, typename TGroup>
    struct TCanCheckEmpty<TText, TGroup, std::void_t<decltype(std::declval<TGroup&>().CheckEmpty(
        std::declval<TText>(), std::string(), 0, std::string()))> > : std::true_type
    {
    };
    template <typename TFirst, typename TSecond, typename TGroup = TTest_ASWUnitTests_RTLStrings, typename = void>
    struct TCanCheckEquals : std::false_type
    {
    };
    template <typename TFirst, typename TSecond, typename TGroup>
    struct TCanCheckEquals<TFirst, TSecond, TGroup, std::void_t<decltype(std::declval<TGroup&>().CheckEquals(
        std::declval<TFirst>(), std::declval<TSecond>(), std::string(), 0, std::string()))> > : std::true_type
    {
    };

private: // Test methods
    void Test_Overloads_AcceptEveryTextKind();
    void Test_Overloads_ForwardAndShowTheUsualMessages();
    void Test_Overloads_RejectNonText();

public:
    TTest_ASWUnitTests_RTLStrings();
    ~TTest_ASWUnitTests_RTLStrings() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

#endif // #if defined(ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED)

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_RTLStringsH
