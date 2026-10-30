/* **************************************************************************
Test_ASWUnitTests_GUI_INIDocument.h
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
#ifndef Test_ASWUnitTests_GUI_INIDocumentH
#define Test_ASWUnitTests_GUI_INIDocumentH
//---------------------------------------------------------------------------
#include "ASWUnitTests_TestBase.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_INIDocument
//
// Exercises the GUI runners' INI file document
// (gui/src/ASWUnitTests_GUI_INIDocument.h), which reads and writes files the
// way the VCL's TMemIniFile does. Listed in every project, so it also runs
// from the console runners.
/////////////////////////////////////////////////////////////////////////////
class TTest_ASWUnitTests_GUI_INIDocument : public TTestGroupBase
{
private:
    typedef TTestGroupBase inherited;

private: // Test methods
    void Test_Constructor_IgnoresARepeatedSection();
    void Test_Constructor_ReadsTheFirstOfARepeatedKey();
    void Test_Constructor_ReadsTrimmedNamesAndValues();
    void Test_Constructor_SkipsLinesTMemIniFileSkips();
    void Test_DeleteKey_RemovesOnlyThatKey();
    void Test_KeyNames_ListsEachKeyAsWritten();
    void Test_ReadString_IgnoresCase();
    void Test_ReadString_MissingIsEmpty();
    void Test_SectionNames_ListsEachSectionOnce();
    void Test_Text_EmptyDocumentIsEmpty();
    void Test_Text_RoundTripsWhatItRead();
    void Test_WriteString_AddsNewKeysAndSectionsAtTheEnd();
    void Test_WriteString_ReplacesAValueInPlace();

public:
    TTest_ASWUnitTests_GUI_INIDocument();
    ~TTest_ASWUnitTests_GUI_INIDocument() override;

    void SetUp_Group() override;
    void SetUp_Test(ITestCase& testCase) override;
    void TearDown_Group() override;
    void TearDown_Test(ITestCase& testCase) override;
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef Test_ASWUnitTests_GUI_INIDocumentH
