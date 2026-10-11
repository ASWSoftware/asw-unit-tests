/* **************************************************************************
ASWUnitTests_Registry.h
Author: Anthony S. West - ASW Software

Self-registration support for test groups and their tests.

Use the ASW_REGISTER_TEST_GROUP macro in a test module's .cpp file to
register the test group with TTestHandler, and optionally the ASW_TEST
macro to define a TAutoTestGroup's test methods so they register
themselves with their group.

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

#ifndef ASWUnitTests_RegistryH
#define ASWUnitTests_RegistryH
//---------------------------------------------------------------------------
#include <functional>
#include <memory>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TRegisteredTestGroupFactory
//
// A self-registered test group's factory function, plus its optional
// ordering hint (see ASW_REGISTER_TEST_GROUP_ORDERED). Groups left at the
// default order of 0 are run in alphabetical order by group name; an
// explicit lower/higher order runs a group before/after that block. Order
// is compared before name, so ties fall back to alphabetical order.
/////////////////////////////////////////////////////////////////////////////
struct TRegisteredTestGroupFactory
{
    std::function<std::unique_ptr<ITestGroup> ()> Factory;
    int Order;
};


/////////////////////////////////////////////////////////////////////////////
// TTestGroupRegistry
//
// Holds the factory functions for self-registered test groups. Test
// modules register themselves via the ASW_REGISTER_TEST_GROUP or
// ASW_REGISTER_TEST_GROUP_ORDERED macro rather than being listed in
// TTestHandler::RegisterTestGroups(), so no framework source file needs to
// change when a test module is added or removed.
/////////////////////////////////////////////////////////////////////////////
class TTestGroupRegistry
{
public:
    typedef std::function<std::unique_ptr<ITestGroup> ()> TestGroupFactory;
    typedef std::vector<TRegisteredTestGroupFactory> TestGroupFactoryList;

public:
    static TestGroupFactoryList& Factories();
    static int Register(TestGroupFactory factory, int order = 0);
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ASW_REGISTER_TEST_GROUP / ASW_REGISTER_TEST_GROUP_ORDERED
//
// Place at namespace or file scope in a test module's .cpp file to
// self-register that test group's class with TTestHandler.
// ASW_REGISTER_TEST_GROUP runs the group in alphabetical order by group
// name, alongside every other group that doesn't specify an order.
// ASW_REGISTER_TEST_GROUP_ORDERED overrides that with an explicit order:
// groups run in ascending order, with ties broken alphabetically by name.
//
// Examples:
//     ASW_REGISTER_TEST_GROUP(TTest_TMyClassToTest)
//     ASW_REGISTER_TEST_GROUP_ORDERED(TTest_TMyClassToTest, -1) // runs first
//---------------------------------------------------------------------------
#define ASWUnitTests_CONCAT_INNER(a, b) a ## b
#define ASWUnitTests_CONCAT(a, b) ASWUnitTests_CONCAT_INNER(a, b)

#if defined(__COUNTER__)
#  define ASWUnitTests_UNIQUE_NAME(base) ASWUnitTests_CONCAT(base, __COUNTER__)
#else
#  define ASWUnitTests_UNIQUE_NAME(base) ASWUnitTests_CONCAT(base, __LINE__)
#endif

// __COUNTER__ isn't standard C++ (C2y adopts it), so newer Clang versions
// report each use under -Wpedantic as -Wc2y-extensions. The registration
// macro silences that warning around its own declaration only. The
// __has_warning check keeps Clang versions without that warning group from
// reporting an unknown warning option.
#if defined(__clang__) && defined(__has_warning)
#  if __has_warning("-Wc2y-extensions")
#    define ASWUnitTests_SUPPRESS_COUNTER_WARNING_BEGIN \
    _Pragma("clang diagnostic push") \
    _Pragma("clang diagnostic ignored \"-Wc2y-extensions\"")
#    define ASWUnitTests_SUPPRESS_COUNTER_WARNING_END \
    _Pragma("clang diagnostic pop")
#  endif
#endif

#if !defined(ASWUnitTests_SUPPRESS_COUNTER_WARNING_BEGIN)
#  define ASWUnitTests_SUPPRESS_COUNTER_WARNING_BEGIN
#  define ASWUnitTests_SUPPRESS_COUNTER_WARNING_END
#endif

#define ASW_REGISTER_TEST_GROUP_ORDERED(ClassName, order) \
    ASWUnitTests_SUPPRESS_COUNTER_WARNING_BEGIN \
    static int const ASWUnitTests_UNIQUE_NAME(g_ASWUnitTests_Registered_) = \
        ::ASWUnitTests::TTestGroupRegistry::Register( \
            []() -> std::unique_ptr<::ASWUnitTests::ITestGroup> \
            { \
                return std::make_unique<ClassName>(); \
            }, (order)); \
    ASWUnitTests_SUPPRESS_COUNTER_WARNING_END

#define ASW_REGISTER_TEST_GROUP(ClassName) \
    ASW_REGISTER_TEST_GROUP_ORDERED(ClassName, 0)

//---------------------------------------------------------------------------
// ASW_TEST
//
// Defines a TAutoTestGroup test method and registers it with its group, so
// the group's constructor doesn't call RegisterTest() for it. Use it in
// place of the method's usual definition line, in the group's .cpp file:
//
//     ASW_TEST(TTest_TMyClassToTest, SetAndGetSeed)
//     {
//         // ... test body ...
//     }
//
// This defines TTest_TMyClassToTest::Test_SetAndGetSeed() and registers it
// as the test "SetAndGetSeed". The class must derive from
// TAutoTestGroup<TTest_TMyClassToTest> and declare
// 'void Test_SetAndGetSeed();' as a public member. The group runs its
// ASW_TEST tests in alphabetical order by name, before any it registers in
// its constructor.
//
// Keep a group's ASW_TEST tests in the same .cpp file as its
// ASW_REGISTER_TEST_GROUP. Nothing else refers to a .cpp file that only
// holds ASW_TEST tests, so a linker could leave it out when the test code is
// built into a static library, just as it could the group's own file.
//---------------------------------------------------------------------------
#define ASW_TEST(ClassName, TestName) \
    ASWUnitTests_SUPPRESS_COUNTER_WARNING_BEGIN \
    static int const ASWUnitTests_UNIQUE_NAME(g_ASWUnitTests_AutoTest_) = \
        ::ASWUnitTests::TAutoTestGroup<ClassName>::AddAutoTest(&ClassName::Test_ ## TestName, #TestName); \
    ASWUnitTests_SUPPRESS_COUNTER_WARNING_END \
    void ClassName::Test_ ## TestName()

//---------------------------------------------------------------------------

#endif // #ifndef ASWUnitTests_RegistryH
