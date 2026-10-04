/* **************************************************************************
ASWUnitTests_GUI_Selection.h
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
#ifndef ASWUnitTests_GUI_SelectionH
#define ASWUnitTests_GUI_SelectionH
//---------------------------------------------------------------------------
#include <cstddef>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_TestList.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUISavedTest
//
// A test's check state, as the VCL GUI runner saves it between sessions.
/////////////////////////////////////////////////////////////////////////////
struct TGUISavedTest
{
    bool Checked = false;
    std::string GroupName;
    std::string TestName;
};


/////////////////////////////////////////////////////////////////////////////
// TGUISelectionRestore
//
// What RestoreGUISelection() did, for the log.
/////////////////////////////////////////////////////////////////////////////
struct TGUISelectionRestore
{
    // Set when none of the saved checked tests exist any more, so every test was checked instead.
    bool CheckedAll = false;
    size_t CheckedCount = 0; // Tests checked once restored.
    size_t MissingCount = 0; // Saved tests that no longer exist.
    size_t NewCheckedCount = 0; // Of NewCount, those checked.
    size_t NewCount = 0; // Tests that weren't saved: new, or renamed, which can't be told apart.
};

//---------------------------------------------------------------------------

// The VCL GUI runner's test selection, saved when the window closes and restored when it next opens, so the
// tests checked in one session stay checked in the next, even as tests are added, removed, or renamed in
// between. Contains no VCL UI code, so it's unit tested from the VCL console runner (see
// vcl/tests/Test_ASWUnitTests_GUI_Selection.cpp).

// The selection to save: every test, checked only if it's both checked and shown, i.e. if Run Selected would
// run it.
std::vector<TGUISavedTest> CaptureGUISelection(TGUITestList const& testList);
// A note for the log about what RestoreGUISelection() did.
std::string DescribeGUISelectionRestore(TGUISelectionRestore const& restore);
// The selection file's text: a comment line, then one line per test, "+" if it's checked or "-" if not, its
// group name, a tab, and its test name. A test whose name has a tab or line break is left out, since its line
// couldn't be read back; it's then treated as new when restored.
std::string FormatGUISelection(std::vector<TGUISavedTest> const& tests);
// This executable's selection file, beside its layout file (see GUILayoutFilePath()): its name with a
// ".selection" extension.
System::UnicodeString GUISelectionFilePath();
// Reads the tests FormatGUISelection() wrote, skipping any line it can't read.
std::vector<TGUISavedTest> ParseGUISelection(std::string const& text);
// Checks 'testList's tests to match 'saved', best-guessing for tests the saved selection doesn't have. A saved
// test keeps its saved check state. A test that wasn't saved is checked if its group was saved fully checked,
// or, in a group that wasn't saved at all, if every saved test was checked. If 'saved' has checked tests but
// none of them exist any more, every test is checked instead. Does nothing if 'saved' is empty.
TGUISelectionRestore RestoreGUISelection(TGUITestList& testList, std::vector<TGUISavedTest> const& saved);

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_SelectionH
