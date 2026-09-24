/* **************************************************************************
ASWUnitTests_Handler.h
Author: Anthony S. West - ASW Software

A simple unit testing framework.

To register a test module, create a class that inherits 'TTestGroupBase'
and self-register it with the ASW_REGISTER_TEST_GROUP macro
(see ASWUnitTests_Registry.h). No framework source file needs to change.

Copyright 2025 Anthony S. West

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

#ifndef ASWUnitTests_HandlerH
#define ASWUnitTests_HandlerH
//---------------------------------------------------------------------------
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

//#if __cplusplus < 201103L
//
//#if !defined(nullptr)
//#  define nullptr NULL
//#endif
//
//#if !defined(override)
//#  define override
//#endif
//
//#endif // #if __cplusplus < 201103L

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTestHandler
//
// Handles the registration running of tests
/////////////////////////////////////////////////////////////////////////////
class TTestHandler
{
private:
    typedef std::vector<std::unique_ptr<ITestGroup> > ITestGroups;

private:
    std::string m_ProjectName;
    ITestGroups m_TestGroups;

private:
    void RegisterTestGroups();

public:
    static std::string GetUTCTimeISO8601();
    static std::string GetVersionFullStr();
    static std::string GetVersionStr();
    static bool WildcardMatch(std::string const& pattern, std::string const& text, bool ignoreCase = false);

public:
    TTestHandler();
    ~TTestHandler();

    std::vector<std::string> GetAllTestFullNames();
    std::string const& GetProjectName() const;
    void Initialize(std::string const& projectName = "ASWUnitTests");
    void ListTests(TestFilter const& filter = TestFilter(), std::string const& filterDescription = std::string());
    void Log(std::string const& msg);
    void LogAppend(std::string const& msg);
    TTestResults Run(TestFilter const& filter = TestFilter(), std::string const& filterDescription = std::string(),
        bool shuffle = false, std::optional<unsigned int> shuffleSeed = std::nullopt);
};

} // namespace ASWUnitTests

#endif // #ifndef TestHandlerH
