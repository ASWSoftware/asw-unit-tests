/* **************************************************************************
Test_ASWUnitTests_CLI.h
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
#ifndef Test_ASWUnitTests_CLIH
#define Test_ASWUnitTests_CLIH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_CLI
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_CLI : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    // Builds a real argv-shaped array from 'args' (owned locally, so ParseArguments's
    // char* argv[] never points at temporary or literal storage) and parses it.
    std::optional<int> ParseArgs(std::vector<std::string> args, TCLIOptions& options);

private: // Test methods
    void Test_BuildTestFilter_Filter();
    void Test_BuildTestFilter_Partition();
    void Test_ParseArguments_Color();
    void Test_ParseArguments_Filter();
    void Test_ParseArguments_HelpVersionList();
    void Test_ParseArguments_InvalidOption();
    void Test_ParseArguments_PartitionValidation();
    void Test_ParseArguments_Pause();
    void Test_ParseArguments_ReportAndProjectName();
    void Test_ParseArguments_Shuffle();
    void Test_ParseColorMode();
    void Test_ParseUnsignedInt_Invalid();
    void Test_ParseUnsignedInt_Valid();

public:
    TTest_ASWUnitTests_CLI();
    ~TTest_ASWUnitTests_CLI() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_CLIH
