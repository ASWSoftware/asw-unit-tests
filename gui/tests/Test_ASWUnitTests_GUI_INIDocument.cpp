/* **************************************************************************
Test_ASWUnitTests_GUI_INIDocument.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "Test_ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_INIDocument.h"
//---------------------------------------------------------------------------

namespace
{

//---------------------------------------------------------------------------

std::string JoinNames(std::vector<std::string> const& names);

//---------------------------------------------------------------------------
/*
    JoinNames

    The names separated by commas, compared whole, so a failure shows the whole list.
*/
std::string JoinNames(std::vector<std::string> const& names)
{
    std::string text;

    for (std::string const& name : names)
        text += (text.empty() ? "" : ",") + name;

    return text;
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_INIDocument
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_INIDocument::TTest_ASWUnitTests_GUI_INIDocument()
    : inherited("ASWUnitTests_GUI_INIDocument_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_IgnoresARepeatedSection,
        "Constructor_IgnoresARepeatedSection");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_ReadsTheFirstOfARepeatedKey,
        "Constructor_ReadsTheFirstOfARepeatedKey");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_ReadsTrimmedNamesAndValues,
        "Constructor_ReadsTrimmedNamesAndValues");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_SkipsLinesTMemIniFileSkips,
        "Constructor_SkipsLinesTMemIniFileSkips");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_DeleteKey_RemovesOnlyThatKey, "DeleteKey_RemovesOnlyThatKey");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_KeyNames_ListsEachKeyAsWritten,
        "KeyNames_ListsEachKeyAsWritten");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_ReadString_IgnoresCase, "ReadString_IgnoresCase");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_ReadString_MissingIsEmpty, "ReadString_MissingIsEmpty");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_SectionNames_ListsEachSectionOnce,
        "SectionNames_ListsEachSectionOnce");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Text_EmptyDocumentIsEmpty, "Text_EmptyDocumentIsEmpty");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_Text_RoundTripsWhatItRead, "Text_RoundTripsWhatItRead");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_WriteString_AddsNewKeysAndSectionsAtTheEnd,
        "WriteString_AddsNewKeysAndSectionsAtTheEnd");
    RegisterTest(&TTest_ASWUnitTests_GUI_INIDocument::Test_WriteString_ReplacesAValueInPlace,
        "WriteString_ReplacesAValueInPlace");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_INIDocument::~TTest_ASWUnitTests_GUI_INIDocument()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_IgnoresARepeatedSection()
{
    // Act
    TGUIINIDocument const ini("[A]\r\nOne=1\r\n[B]\r\nTwo=2\r\n[a]\r\nOne=changed\r\nThree=3\r\n");

    // Assert
    CheckEquals(std::string("1"), ini.ReadString("A", "One").value_or("missing"), __func__, __LINE__,
        "the first occurrence's value is kept");
    CheckFalse(ini.ReadString("A", "Three").has_value(), __func__, __LINE__,
        "and the repeated section's own keys are skipped too, as TMemIniFile skips them");
    CheckEquals(std::string("[A]\r\nOne=1\r\n\r\n[B]\r\nTwo=2\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "nor is it written back");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_ReadsTheFirstOfARepeatedKey()
{
    // Act
    TGUIINIDocument const ini("[A]\r\nKey=first\r\nKEY=second\r\n");

    // Assert
    CheckEquals(std::string("first"), ini.ReadString("A", "Key").value_or("missing"), __func__, __LINE__,
        "the first one is read, as with TMemIniFile");
    CheckEquals(std::string("[A]\r\nKey=first\r\nKEY=second\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "and both are written back");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_ReadsTrimmedNamesAndValues()
{
    // Act
    TGUIINIDocument const ini("  [ Spaced Section ]  \r\n\tName  =  some value \t\r\nEquation=a=b\r\nEmpty=\r\n");

    // Assert
    CheckEquals(std::string("some value"), ini.ReadString("Spaced Section", "Name").value_or("missing"), __func__,
        __LINE__, "the section and key names and the value are trimmed, but not spaces inside them");
    CheckEquals(std::string("a=b"), ini.ReadString("Spaced Section", "Equation").value_or("missing"), __func__,
        __LINE__, "a value is everything after the first '='");
    CheckEquals(std::string(), ini.ReadString("Spaced Section", "Empty").value_or("missing"), __func__, __LINE__,
        "an empty value is a value");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Constructor_SkipsLinesTMemIniFileSkips()
{
    // Arrange: a byte order mark, a key before any section, a comment, blank lines, a line with no '=', and LF
    // line breaks mixed with CRLF ones.
    std::string const text = "\xEF\xBB\xBF" "Before=section\r\n"
        "[A]\n"
        "; Key=commented\r\n"
        "\r\n"
        "   \n"
        "NoEquals\r\n"
        "Key=value";

    // Act
    TGUIINIDocument const ini(text);

    // Assert
    CheckEquals(std::string("value"), ini.ReadString("A", "Key").value_or("missing"), __func__, __LINE__,
        "the one key, whose line has no line break after it");
    CheckFalse(ini.ReadString("A", "; Key").has_value(), __func__, __LINE__, "a comment isn't a key");
    CheckFalse(ini.ReadString("A", "NoEquals").has_value(), __func__, __LINE__, "nor a line with no '='");
    CheckEquals(std::string("[A]\r\nKey=value\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "only the section and its key are written back, without the byte order mark");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_DeleteKey_RemovesOnlyThatKey()
{
    // Arrange
    TGUIINIDocument ini("[A]\r\nOne=1\r\nTwo=2\r\n[B]\r\nOne=1\r\n");

    // Act
    ini.DeleteKey("a", "ONE");
    ini.DeleteKey("B", "One");
    ini.DeleteKey("A", "Missing");
    ini.DeleteKey("Missing", "One");

    // Assert
    CheckEquals(std::string("[A]\r\nTwo=2\r\n\r\n[B]\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "a key is found ignoring case, and its emptied section stays, as with TMemIniFile");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_KeyNames_ListsEachKeyAsWritten()
{
    // Arrange
    TGUIINIDocument ini("[A]\r\n One = 1\r\nNoEquals\r\nTwo=2\r\nONE=again\r\n[B]\r\n[C]\r\nThree=3\r\n");

    // Act and Assert
    CheckEquals(std::string("One,Two,ONE"), JoinNames(ini.KeyNames("a")), __func__, __LINE__,
        "trimmed, in order, a repeated key each time, found ignoring the section's case");
    CheckEmpty(JoinNames(ini.KeyNames("B")), __func__, __LINE__, "an empty section");
    CheckEmpty(JoinNames(ini.KeyNames("Missing")), __func__, __LINE__, "a missing section");

    ini.WriteString("C", "Four", "4");
    CheckEquals(std::string("Three,Four"), JoinNames(ini.KeyNames("C")), __func__, __LINE__,
        "with a key written since");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_ReadString_IgnoresCase()
{
    // Arrange
    TGUIINIDocument const ini("[Window]\r\nHeight=600\r\n");

    // Act and Assert
    CheckEquals(std::string("600"), ini.ReadString("WINDOW", "height").value_or("missing"), __func__, __LINE__,
        "section and key names match ignoring case");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_ReadString_MissingIsEmpty()
{
    // Arrange
    TGUIINIDocument const ini("[Window]\r\nHeight=600\r\n");

    // Act and Assert
    CheckFalse(ini.ReadString("Window", "Width").has_value(), __func__, __LINE__, "a missing key");
    CheckFalse(ini.ReadString("Panels", "Height").has_value(), __func__, __LINE__, "a missing section");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_SectionNames_ListsEachSectionOnce()
{
    // Arrange
    TGUIINIDocument ini("Before=section\r\n[ Colors ]\r\nA=1\r\n[Theme]\r\n[colors]\r\nB=2\r\n");

    // Act and Assert
    CheckEquals(std::string("Colors,Theme"), JoinNames(ini.SectionNames()), __func__, __LINE__,
        "trimmed, in order, a repeated one once, as it's skipped");
    CheckEmpty(JoinNames(TGUIINIDocument().SectionNames()), __func__, __LINE__, "an empty document");

    ini.WriteString("View", "Theme", "Dark");
    CheckEquals(std::string("Colors,Theme,View"), JoinNames(ini.SectionNames()), __func__, __LINE__,
        "with a section written since");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Text_EmptyDocumentIsEmpty()
{
    // Act and Assert
    CheckEmpty(TGUIINIDocument().Text(), __func__, __LINE__, "a new document");
    CheckEmpty(TGUIINIDocument("; only a comment\r\n").Text(), __func__, __LINE__, "a file with no sections");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_Text_RoundTripsWhatItRead()
{
    // Arrange
    std::string const text = "[Panels]\r\nDetailHeight=288\r\n\r\n[Window]\r\nMaximized=1\r\n\r\n";

    // Act and Assert
    CheckEquals(text, TGUIINIDocument(text).Text(), __func__, __LINE__, "text in Text()'s own form is unchanged");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_WriteString_AddsNewKeysAndSectionsAtTheEnd()
{
    // Arrange
    TGUIINIDocument ini("[A]\r\nOne=1\r\n[B]\r\nTwo=2\r\n");

    // Act
    ini.WriteString("A", "Three", "3");
    ini.WriteString("C", "Four", "4");

    // Assert
    CheckEquals(std::string("[A]\r\nOne=1\r\nThree=3\r\n\r\n[B]\r\nTwo=2\r\n\r\n[C]\r\nFour=4\r\n\r\n"), ini.Text(),
        __func__, __LINE__, "a new key at the end of its section, and a new section at the end");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_INIDocument::Test_WriteString_ReplacesAValueInPlace()
{
    // Arrange
    TGUIINIDocument ini("[A]\r\nOne=1\r\nTwo=2\r\n");

    // Act
    ini.WriteString("a", "ONE", "changed");

    // Assert
    CheckEquals(std::string("[A]\r\nOne=changed\r\nTwo=2\r\n\r\n"), ini.Text(), __func__, __LINE__,
        "the key keeps its place and its name as written, as with TMemIniFile");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_INIDocument)
