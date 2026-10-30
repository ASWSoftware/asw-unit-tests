/* **************************************************************************
ASWUnitTests_Handler.h
Author: Anthony S. West - ASW Software

A simple unit testing framework.

To register a test module, create a class that inherits 'TTestGroupBase'
and self-register it with the ASW_REGISTER_TEST_GROUP macro
(see ASWUnitTests_Registry.h). No framework source file needs to change.

Copyright 2025-2026 ASW Software

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
#include <cstddef>
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

struct TRegisteredTestGroupFactory; // See ASWUnitTests_Registry.h.

/////////////////////////////////////////////////////////////////////////////
// TTestId
//
// Identifies one registered test by its group and test names, kept separate
// rather than joined into a "GroupName.TestName" full name, since a group
// name may itself contain '.'. GroupName is the group's qualified name, and
// SuitePath its suite part on its own, as in TTestCaseRecord. Returned by
// TTestHandler::GetTests().
/////////////////////////////////////////////////////////////////////////////
struct TTestId
{
    std::string GroupName;
    std::string TestName;
    // Empty when the group isn't in a suite. Defaulted, so a brace initializer written before this member existed
    // still compiles without a -Wmissing-field-initializers warning.
    std::string SuitePath = std::string();
};


/////////////////////////////////////////////////////////////////////////////
// TTestGroupOrderKey
//
// What decides a registered test group's place in the canonical run order:
// its suite path, its own name (not its qualified name), and its registered
// order (see ASW_REGISTER_TEST_GROUP_ORDERED). Passed to
// TTestHandler::CanonicalGroupOrder().
/////////////////////////////////////////////////////////////////////////////
struct TTestGroupOrderKey
{
    std::string SuitePath;
    std::string Name;
    int Order;
};


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
    ITestRunObserver* m_RunObserver; // Not owned; nullptr when none is set.
    ITestGroups m_TestGroups;

private:
    std::unique_ptr<ITestGroup> CreateTestGroup(TRegisteredTestGroupFactory const& registered);
    TTestResults FailGroupTests(ITestGroup& group, TestFilter const& filter, std::string const& detail);
    void RegisterTestGroups();

public:
    // Puts 'group' in suite 'registeredPath', the suite its registration names (see
    // ASW_REGISTER_TEST_GROUP_IN_SUITE); does nothing when that's empty, or already the group's own suite path. Only
    // a TTestGroupBase without a suite of its own can be put in one. Throws TExceptInvalidSuitePath for an invalid
    // 'registeredPath' (see TSuitePath), or TExceptInvalidSuitePath::Conflicting() when the group's own suite path
    // differs: one its constructor gives, or, for an ITestGroup that doesn't derive from TTestGroupBase, any its
    // GetSuitePath() gives, including none.
    static void ApplyRegisteredSuitePath(ITestGroup& group, std::string const& registeredPath);
    // Returns the indexes of 'groups' in their canonical (unshuffled) run order. Within each suite, and at the
    // top level, the groups and suites in it run in ascending order of their registered order, then by name,
    // with a suite treated as order 0 and named by its own suite name. A suite's groups, and the suites inside
    // it, always run together. Groups that tie on both keep their order in 'groups'. Throws
    // TExceptInvalidSuitePath for an invalid suite path (see TSuitePath).
    static std::vector<size_t> CanonicalGroupOrder(std::vector<TTestGroupOrderKey> const& groups);
    // Returns "64-bit" or "32-bit": the pointer size this framework was built for.
    static std::string GetAppBitsStr();
    // Returns the operating system this framework was built for: "Windows" or "Linux" ("Unknown" for any other).
    static std::string GetBuildOS();
    static std::string GetUTCTimeISO8601();
    static std::string GetVersionFullStr();
    static std::string GetVersionStr();
    // Returns the indexes of the groups whose suite paths are 'suitePaths', given in canonical order, in a
    // shuffled order determined entirely by 'seed'. The groups and suites at the top level are shuffled with
    // 'seed' itself, so without suites the order is ShuffledIndices(suitePaths.size(), seed). Those in each
    // suite are shuffled with 'seed' plus the FNV-1a hash of the suite's path, and a suite's groups still run
    // together. Throws TExceptInvalidSuitePath for an invalid suite path (see TSuitePath).
    static std::vector<size_t> ShuffledGroupOrder(std::vector<std::string> const& suitePaths, unsigned int seed);
    static bool WildcardMatch(std::string const& pattern, std::string const& text, bool ignoreCase = false);

public:
    TTestHandler();
    ~TTestHandler();

    std::vector<std::string> GetAllTestFullNames();
    std::string const& GetProjectName() const;
    std::vector<TTestId> GetTests();
    void Initialize(std::string const& projectName = "ASWUnitTests");
    void ListTests(TestFilter const& filter = TestFilter(), std::string const& filterDescription = std::string());
    void Log(std::string const& msg);
    void LogAppend(std::string const& msg);
    TTestResults Run(TestFilter const& filter = TestFilter(), std::string const& filterDescription = std::string(),
        bool shuffle = false, std::optional<unsigned int> shuffleSeed = std::nullopt,
        std::optional<unsigned int> testTimeoutSeconds = std::nullopt, bool catchCrashes = false);
    // Sets (or, with nullptr, clears) the observer that receives this handler's and every test group's log
    // output and per-test events; see ITestRunObserver. Not owned: it must outlive its use here. Can be called
    // before Initialize(), so Initialize()'s own output reaches the observer too.
    void SetRunObserver(ITestRunObserver* observer);
};

} // namespace ASWUnitTests

#endif // #ifndef TestHandlerH
