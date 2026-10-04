/* **************************************************************************
Test_ASWUnitTests_GUI_Selection.cpp
Author: Anthony S. West - ASW Software

See header for info.

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
// Module header
#include "Test_ASWUnitTests_GUI_Selection.h"
//---------------------------------------------------------------------------
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.SysUtils.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_Registry.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Layout.h"
#include "ASWUnitTests_GUI_Selection.h"
#include "ASWUnitTests_GUI_Strings.h"
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

std::vector<TTestId> SampleTests();

//---------------------------------------------------------------------------

// Four tests in three groups: indexes 0 and 1 in "Alpha", 2 in "Beta", 3 in "Gamma".
std::vector<TTestId> SampleTests()
{
    return { { "Alpha", "One" }, { "Alpha", "Two" }, { "Beta", "Three" }, { "Gamma", "Four" } };
}

//---------------------------------------------------------------------------

} // namespace


namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TTest_ASWUnitTests_GUI_Selection
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Selection::TTest_ASWUnitTests_GUI_Selection()
    : inherited("ASWUnitTests_GUI_Selection_Tests")
{
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_CaptureGUISelection_SavesWhatRunSelectedWouldRun,
        "CaptureGUISelection_SavesWhatRunSelectedWouldRun");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_DescribeGUISelectionRestore_NotesMissingAndNewTests,
        "DescribeGUISelectionRestore_NotesMissingAndNewTests");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_DescribeGUISelectionRestore_NotesWhenEveryTestWasChecked,
        "DescribeGUISelectionRestore_NotesWhenEveryTestWasChecked");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_FormatGUISelection_LeavesOutNamesItCannotStore,
        "FormatGUISelection_LeavesOutNamesItCannotStore");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_FormatGUISelection_RoundTripsThroughParse,
        "FormatGUISelection_RoundTripsThroughParse");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_GUISelectionFilePath_IsBesideTheLayoutFile,
        "GUISelectionFilePath_IsBesideTheLayoutFile");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_ParseGUISelection_SkipsLinesItCannotRead,
        "ParseGUISelection_SkipsLinesItCannotRead");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksEveryTestIfNoSavedCheckedTestExists,
        "RestoreGUISelection_ChecksEveryTestIfNoSavedCheckedTestExists");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksNewTestsLikeTheirGroup,
        "RestoreGUISelection_ChecksNewTestsLikeTheirGroup");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksNewGroupsOnlyIfEveryTestWasChecked,
        "RestoreGUISelection_ChecksNewGroupsOnlyIfEveryTestWasChecked");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_DoesNothingWithoutASavedSelection,
        "RestoreGUISelection_DoesNothingWithoutASavedSelection");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_KeepsASavedSelectionWithNothingChecked,
        "RestoreGUISelection_KeepsASavedSelectionWithNothingChecked");
    RegisterTest(&TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_RestoresSavedCheckStates,
        "RestoreGUISelection_RestoresSavedCheckStates");
}
//---------------------------------------------------------------------------
TTest_ASWUnitTests_GUI_Selection::~TTest_ASWUnitTests_GUI_Selection()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::SetUp_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::SetUp_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::TearDown_Group()
{
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::TearDown_Test(ITestCase& /*testCase*/)
{
}
//---------------------------------------------------------------------------

// /////// Begin tests after this line ///////////////////////

//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_CaptureGUISelection_SavesWhatRunSelectedWouldRun()
{
    // Arrange: "Beta.Three" is checked but hidden, so Run Selected wouldn't run it.
    TGUITestList list;
    list.Load(SampleTests());
    list.SetChecked(1, false);
    list.SetVisibleFilter([](std::string const& fullTestName)
        {
            return fullTestName != "Beta.Three";
        });

    // Act
    std::vector<TGUISavedTest> const saved = CaptureGUISelection(list);

    // Assert
    AssertEquals(static_cast<size_t>(4), saved.size(), __func__, __LINE__, "every test, checked or not");
    CheckEquals(std::string("Alpha"), saved[0].GroupName, __func__, __LINE__, "group name");
    CheckEquals(std::string("One"), saved[0].TestName, __func__, __LINE__, "test name");
    CheckTrue(saved[0].Checked, __func__, __LINE__, "a checked, shown test is saved checked");
    CheckFalse(saved[1].Checked, __func__, __LINE__, "an unchecked test is saved unchecked");
    CheckFalse(saved[2].Checked, __func__, __LINE__, "a checked but hidden test is saved unchecked");
    CheckTrue(saved[3].Checked, __func__, __LINE__, "another checked, shown test");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_DescribeGUISelectionRestore_NotesMissingAndNewTests()
{
    // Arrange
    TGUISelectionRestore plain;
    plain.CheckedCount = 1;

    TGUISelectionRestore changed;
    changed.CheckedCount = 5;
    changed.MissingCount = 2;
    changed.NewCount = 3;
    changed.NewCheckedCount = 1;

    // Act
    std::string const plainText = DescribeGUISelectionRestore(plain);
    std::string const changedText = DescribeGUISelectionRestore(changed);

    // Assert
    CheckEquals(std::string("Restored the saved selection: 1 test checked."), plainText, __func__, __LINE__,
        "nothing missing or new");
    CheckEquals(std::string("Restored the saved selection: 5 tests checked; 2 saved tests no longer exist; "
        "3 new or renamed tests (1 checked)."), changedText, __func__, __LINE__, "missing and new tests noted");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_DescribeGUISelectionRestore_NotesWhenEveryTestWasChecked()
{
    // Arrange
    TGUISelectionRestore restore;
    restore.CheckedAll = true;
    restore.CheckedCount = 4;

    // Act
    std::string const text = DescribeGUISelectionRestore(restore);

    // Assert
    CheckEquals(std::string("None of the saved selection's checked tests exist any more, so every test is checked."),
        text, __func__, __LINE__, "the fallback is explained");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_FormatGUISelection_LeavesOutNamesItCannotStore()
{
    // Arrange
    std::vector<TGUISavedTest> const tests = { { true, "Alpha", "One" }, { true, "Alpha", "Tab\tName" },
        { false, "Line\nBreak", "Two" } };

    // Act
    std::vector<TGUISavedTest> const parsed = ParseGUISelection(FormatGUISelection(tests));

    // Assert
    AssertEquals(static_cast<size_t>(1), parsed.size(), __func__, __LINE__, "only the test it can store");
    CheckEquals(std::string("One"), parsed[0].TestName, __func__, __LINE__, "the storable test");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_FormatGUISelection_RoundTripsThroughParse()
{
    // Arrange: names with spaces, dots, brackets, and a UTF-8 'e' with acute accent.
    std::vector<TGUISavedTest> const tests = { { true, "Alpha.Sub", "One[row 1]" }, { false, "Beta", "Two = 2" },
        { true, "Caf\xC3\xA9", "Three" } };

    // Act
    std::string const text = FormatGUISelection(tests);
    std::vector<TGUISavedTest> const parsed = ParseGUISelection(text);

    // Assert
    CheckEquals(0, text.find("; "), __func__, __LINE__, "starts with a comment describing the file");
    AssertEquals(tests.size(), parsed.size(), __func__, __LINE__, "every test read back");

    for (size_t i = 0; i < tests.size(); ++i)
    {
        CheckEquals(tests[i].GroupName, parsed[i].GroupName, __func__, __LINE__, "group name " + std::to_string(i));
        CheckEquals(tests[i].TestName, parsed[i].TestName, __func__, __LINE__, "test name " + std::to_string(i));
        CheckEquals(tests[i].Checked, parsed[i].Checked, __func__, __LINE__, "check state " + std::to_string(i));
    }
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_GUISelectionFilePath_IsBesideTheLayoutFile()
{
    // Act
    System::UnicodeString const selectionPath = GUISelectionFilePath();
    System::UnicodeString const layoutPath = GUILayoutFilePath();

    // Assert
    CheckEquals(ToUTF8(System::Sysutils::ChangeFileExt(layoutPath, ".selection")), ToUTF8(selectionPath), __func__,
        __LINE__, "the layout file's path with a .selection extension");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_ParseGUISelection_SkipsLinesItCannotRead()
{
    // Arrange: a byte order mark, CRLF and LF line ends, and lines that aren't tests.
    std::string const text = "\xEF\xBB\xBF; comment\r\n"
        "+Alpha\tOne\r\n"
        "\r\n"
        "Alpha\tNo sign\n"
        "+No tab\n"
        "-Beta\tTwo";

    // Act
    std::vector<TGUISavedTest> const parsed = ParseGUISelection(text);

    // Assert
    AssertEquals(static_cast<size_t>(2), parsed.size(), __func__, __LINE__, "only the two test lines");
    CheckEquals(std::string("Alpha"), parsed[0].GroupName, __func__, __LINE__, "first group, past the BOM");
    CheckEquals(std::string("One"), parsed[0].TestName, __func__, __LINE__, "first test, without the CR");
    CheckTrue(parsed[0].Checked, __func__, __LINE__, "'+' is checked");
    CheckEquals(std::string("Two"), parsed[1].TestName, __func__, __LINE__, "the last line needs no line end");
    CheckFalse(parsed[1].Checked, __func__, __LINE__, "'-' is unchecked");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksEveryTestIfNoSavedCheckedTestExists()
{
    // Arrange: the only checked test was renamed, and its group was only partly checked.
    TGUITestList list;
    list.Load(SampleTests());
    std::vector<TGUISavedTest> const saved = { { true, "Alpha", "Renamed" }, { false, "Alpha", "One" },
        { false, "Beta", "Three" } };

    // Act
    TGUISelectionRestore const restore = RestoreGUISelection(list, saved);

    // Assert
    CheckTrue(restore.CheckedAll, __func__, __LINE__, "falls back to every test");
    CheckEquals(static_cast<size_t>(4), list.CheckedCount(), __func__, __LINE__, "every test is checked");
    CheckEquals(static_cast<size_t>(4), restore.CheckedCount, __func__, __LINE__, "and counted");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksNewTestsLikeTheirGroup()
{
    // Arrange: "Alpha.Two" and "Beta.Three" are new; Alpha was saved fully checked, Beta only partly.
    TGUITestList list;
    list.Load(SampleTests());
    std::vector<TGUISavedTest> const saved = { { true, "Alpha", "One" }, { true, "Beta", "Failed" },
        { false, "Beta", "Passed" }, { false, "Gamma", "Four" } };

    // Act
    TGUISelectionRestore const restore = RestoreGUISelection(list, saved);

    // Assert
    CheckTrue(list.IsChecked(1), __func__, __LINE__, "a new test in a fully checked group is checked");
    CheckFalse(list.IsChecked(2), __func__, __LINE__, "a new test in a partly checked group isn't");
    CheckEquals(static_cast<size_t>(2), restore.NewCount, __func__, __LINE__, "two new tests");
    CheckEquals(static_cast<size_t>(1), restore.NewCheckedCount, __func__, __LINE__, "one of them checked");
    CheckEquals(static_cast<size_t>(2), restore.MissingCount, __func__, __LINE__, "Beta's two saved tests are gone");
    CheckEquals(static_cast<size_t>(2), restore.CheckedCount, __func__, __LINE__, "Alpha's two tests checked");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_ChecksNewGroupsOnlyIfEveryTestWasChecked()
{
    // Arrange: Gamma is a new group in both cases.
    std::vector<TGUISavedTest> const allChecked = { { true, "Alpha", "One" }, { true, "Alpha", "Two" },
        { true, "Beta", "Three" } };
    std::vector<TGUISavedTest> const someChecked = { { true, "Alpha", "One" }, { true, "Alpha", "Two" },
        { false, "Beta", "Three" } };

    TGUITestList afterAll;
    afterAll.Load(SampleTests());
    TGUITestList afterSome;
    afterSome.Load(SampleTests());

    // Act
    RestoreGUISelection(afterAll, allChecked);
    RestoreGUISelection(afterSome, someChecked);

    // Assert
    CheckTrue(afterAll.IsChecked(3), __func__, __LINE__, "a new group is checked when every test was");
    CheckFalse(afterSome.IsChecked(3), __func__, __LINE__, "but not when only some were");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_DoesNothingWithoutASavedSelection()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    list.SetChecked(0, false);

    // Act
    TGUISelectionRestore const restore = RestoreGUISelection(list, std::vector<TGUISavedTest>());

    // Assert
    CheckFalse(list.IsChecked(0), __func__, __LINE__, "the check states are left as they were");
    CheckEquals(static_cast<size_t>(3), list.CheckedCount(), __func__, __LINE__, "the others too");
    CheckEquals(static_cast<size_t>(0), restore.CheckedCount + restore.NewCount + restore.MissingCount, __func__,
        __LINE__, "nothing to report");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_KeepsASavedSelectionWithNothingChecked()
{
    // Arrange
    TGUITestList list;
    list.Load(SampleTests());
    std::vector<TGUISavedTest> const saved = { { false, "Alpha", "One" }, { false, "Beta", "Three" } };

    // Act
    TGUISelectionRestore const restore = RestoreGUISelection(list, saved);

    // Assert
    CheckFalse(restore.CheckedAll, __func__, __LINE__, "no fallback, since nothing was checked when saved");
    CheckEquals(static_cast<size_t>(0), list.CheckedCount(), __func__, __LINE__, "nothing is checked");
}
//---------------------------------------------------------------------------
void TTest_ASWUnitTests_GUI_Selection::Test_RestoreGUISelection_RestoresSavedCheckStates()
{
    // Arrange: saved out of order, with "Alpha.Two" saved twice; the last one counts.
    TGUITestList list;
    list.Load(SampleTests());
    std::vector<TGUISavedTest> const saved = { { false, "Gamma", "Four" }, { true, "Alpha", "Two" },
        { false, "Alpha", "One" }, { true, "Beta", "Three" }, { false, "Alpha", "Two" } };

    // Act
    TGUISelectionRestore const restore = RestoreGUISelection(list, saved);

    // Assert
    CheckFalse(list.IsChecked(0), __func__, __LINE__, "Alpha.One unchecked");
    CheckFalse(list.IsChecked(1), __func__, __LINE__, "Alpha.Two unchecked, its last saved state");
    CheckTrue(list.IsChecked(2), __func__, __LINE__, "Beta.Three checked");
    CheckFalse(list.IsChecked(3), __func__, __LINE__, "Gamma.Four unchecked");
    CheckEquals(static_cast<size_t>(1), restore.CheckedCount, __func__, __LINE__, "one checked");
    CheckEquals(static_cast<size_t>(0), restore.NewCount + restore.MissingCount, __func__, __LINE__,
        "nothing new or missing");
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_ASWUnitTests_GUI_Selection)
