/* **************************************************************************
Test_ASWUnitTests_JUnitReport.h
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
#ifndef Test_ASWUnitTests_JUnitReportH
#define Test_ASWUnitTests_JUnitReportH
//---------------------------------------------------------------------------
#include <filesystem>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_JUnitReport
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_JUnitReport : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private:
    std::filesystem::path const m_GroupBaseTempDir;
    std::filesystem::path m_TestTempDir; // Set per-test, in SetUp_Test().

private: // Test methods
    void Test_EscapeXml_PlainTextUnchanged();
    void Test_EscapeXml_SpecialCharacters();
    void Test_Write_ProducesReadableReport();

public:
    TTest_ASWUnitTests_JUnitReport();
    ~TTest_ASWUnitTests_JUnitReport() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_JUnitReportH
