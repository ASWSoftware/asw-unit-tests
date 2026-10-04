/* **************************************************************************
Test_ASWUnitTests_Utils.h
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
#ifndef Test_ASWUnitTests_UtilsH
#define Test_ASWUnitTests_UtilsH
//---------------------------------------------------------------------------
#include <cwchar>
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_Utils
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_Utils : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_WideToUTF8_EncodesEachSequenceLength();
    void Test_WideToUTF8_KeepsASCIIUnchanged();
#if WCHAR_MAX > 0xFFFF
    void Test_WideToUTF8_ReplacesOutOfRangeValues();
#endif
    void Test_WideToUTF8_ReplacesUnpairedSurrogates();

public:
    TTest_ASWUnitTests_Utils();
    ~TTest_ASWUnitTests_Utils() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_UtilsH
