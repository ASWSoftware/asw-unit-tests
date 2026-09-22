/* **************************************************************************
ASWUnitTests_Registry.h
Author: Anthony S. West - ASW Software

Self-registration support for test groups.

Use the ASW_REGISTER_TEST_GROUP macro in a test module's .cpp file to
register the test group with TTestHandler.

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
// TTestGroupRegistry
//
// Holds the factory functions for self-registered test groups. Test
// modules register themselves via the ASW_REGISTER_TEST_GROUP macro rather
// than being listed in TTestHandler::RegisterTestGroups(), so no framework
// source file needs to change when a test module is added or removed.
/////////////////////////////////////////////////////////////////////////////
class TTestGroupRegistry
{
public:
    typedef std::function<std::unique_ptr<ITestGroup> ()> TestGroupFactory;
    typedef std::vector<TestGroupFactory> TestGroupFactoryList;

public:
    static TestGroupFactoryList& Factories();
    static int Register(TestGroupFactory factory);
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// ASW_REGISTER_TEST_GROUP
//
// Place at namespace or file scope in a test module's .cpp file to
// self-register that test group's class with TTestHandler.
//
// Example:
//     ASW_REGISTER_TEST_GROUP(TTest_TMyClassToTest)
//---------------------------------------------------------------------------
#define ASWUnitTests_CONCAT_INNER(a, b) a ## b
#define ASWUnitTests_CONCAT(a, b) ASWUnitTests_CONCAT_INNER(a, b)

#if defined(__COUNTER__)
#  define ASWUnitTests_UNIQUE_NAME(base) ASWUnitTests_CONCAT(base, __COUNTER__)
#else
#  define ASWUnitTests_UNIQUE_NAME(base) ASWUnitTests_CONCAT(base, __LINE__)
#endif

#define ASW_REGISTER_TEST_GROUP(ClassName) \
    static int const ASWUnitTests_UNIQUE_NAME(g_ASWUnitTests_Registered_) = \
        ::ASWUnitTests::TTestGroupRegistry::Register( \
            []() -> std::unique_ptr<::ASWUnitTests::ITestGroup> \
            { \
                return std::make_unique<ClassName>(); \
            });

//---------------------------------------------------------------------------

#endif // #ifndef ASWUnitTests_RegistryH
