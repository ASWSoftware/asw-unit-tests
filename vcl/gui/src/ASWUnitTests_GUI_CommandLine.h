/* **************************************************************************
ASWUnitTests_GUI_CommandLine.h
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
#ifndef ASWUnitTests_GUI_CommandLineH
#define ASWUnitTests_GUI_CommandLineH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
#include <vector>
//---------------------------------------------------------------------------
#include "ASWUnitTests_CLI.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUIOptions
//
// The VCL GUI runner's command-line options: everything the console runner
// accepts (in CLI), plus the GUI-only options.
/////////////////////////////////////////////////////////////////////////////
struct TGUIOptions
{
    TCLIOptions CLI;
    bool ExitAfterRun = false; // --exit: close when the --run run finishes, returning the console's exit code.
    // Console-only options that were given but have no effect in the GUI (e.g. "--list"), for the GUI to note.
    std::vector<std::string> IgnoredOptions;
    bool LayoutIgnore = false; // --layout-ignore: neither load nor save the window layout.
    bool LayoutReset = false; // --layout-reset: delete the saved window layout, and start from the default one.
    bool RunOnStart = false; // --run: run the checked tests once the window opens.
};


/////////////////////////////////////////////////////////////////////////////
// TGUICommandLineResult
//
// What TGUICommandLine::Parse() found, beyond the options themselves.
/////////////////////////////////////////////////////////////////////////////
struct TGUICommandLineResult
{
    // Set when the GUI shouldn't open normally: 0 (ExitCode_Success) after --help or --version, or
    // ExitCode_InvalidArguments. The caller shows Message, then exits with this code.
    std::optional<int> EarlyExitCode;
    // Text for the user: the --help/--version output, or an argument error. Empty otherwise.
    std::string Message;
};


/////////////////////////////////////////////////////////////////////////////
// TGUICommandLine
//
// Parses the VCL GUI runner's command line. The GUI-only options are handled
// here; everything else goes through the console runner's own TCLIParser, so
// both runners accept the same options with the same validation and help
// text. TCLIParser writes its messages to std::cout, which a GUI app has no
// console for, so Parse() captures them into the result's Message instead.
// Contains no VCL UI code, so it's unit tested from the VCL console runner
// (see vcl/tests/Test_ASWUnitTests_GUI_CommandLine.cpp).
/////////////////////////////////////////////////////////////////////////////
class TGUICommandLine
{
public:
    // Help text for the GUI-only options, appended to the console runner's usage text.
    static std::string GUIOptionsHelp();
    // The full help text, exactly as --help shows it: the console runner's usage text plus the GUI-only
    // options.
    static std::string HelpText();
    // Parses 'args' (the arguments without the program name, as ProcessArguments() returns them) into
    // 'options'.
    static TGUICommandLineResult Parse(std::vector<std::string> const& args, TGUIOptions& options);
    // This process's command-line arguments, without the program name, converted to UTF-8.
    static std::vector<std::string> ProcessArguments();
};

} // namespace ASWUnitTests

//---------------------------------------------------------------------------
#endif // #ifndef ASWUnitTests_GUI_CommandLineH
