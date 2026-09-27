/* **************************************************************************
ASWUnitTests_GUI_CommandLine.cpp
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
#include "ASWUnitTests_GUI_CommandLine.h"
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
#include "ASWUnitTests_StdOutRedirect.h"
//---------------------------------------------------------------------------
#include "ASWUnitTests_GUI_Strings.h"
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TGUICommandLine
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
std::string TGUICommandLine::GUIOptionsHelp()
{
    return
        "\n"
        "GUI-only options:\n"
        "  --run               Run the checked tests once the window opens.\n"
        "  --exit              With --run, close the window when that run finishes,\n"
        "                      returning the same exit code the console runner would.\n"
        "                      Requires --run.\n"
        "  --layout-ignore     Neither load nor save the window layout (its size,\n"
        "                      position, and panel sizes); open with the defaults.\n"
        "  --layout-reset      Delete the saved window layout and open with the\n"
        "                      defaults; the layout is saved again on closing.\n"
        "\n"
        "Ignored by the GUI: --list, --pause, and the color options.\n";
}
//---------------------------------------------------------------------------
std::string TGUICommandLine::HelpText()
{
    TGUIOptions options;
    return Parse({ "--help" }, options).Message;
}
//---------------------------------------------------------------------------
TGUICommandLineResult TGUICommandLine::Parse(std::vector<std::string> const& args, TGUIOptions& options)
{
    TGUICommandLineResult result;

    // TCLIParser rejects options it doesn't know, so the GUI-only ones are taken out first. Its argv[0] is
    // the program name, which it skips.
    std::vector<std::string> cliArgs{ "ASWUnitTests_VCL_GUI" };

    for (std::string const& arg : args)
    {
        if (arg == "--run")
            options.RunOnStart = true;
        else if (arg == "--exit")
            options.ExitAfterRun = true;
        else if (arg == "--layout-ignore")
            options.LayoutIgnore = true;
        else if (arg == "--layout-reset")
            options.LayoutReset = true;
        else
            cliArgs.push_back(arg);
    }

    std::vector<char*> argv;
    for (std::string& arg : cliArgs)
        argv.push_back(&arg[0]);

    {
        TStdOutRedirect const capture;
        result.EarlyExitCode = TCLIParser::ParseArguments(static_cast<int>(argv.size()), argv.data(), options.CLI);
        result.Message = capture.Str();
    }

    // The usage text is printed for --help, and after an unrecognized option; either way, it should
    // mention the GUI-only options too.
    if (result.Message.find("Usage: ASWUnitTests [options]") != std::string::npos)
        result.Message += GUIOptionsHelp();

    if (result.EarlyExitCode.has_value())
        return result;

    if (options.ExitAfterRun && !options.RunOnStart)
    {
        result.EarlyExitCode = ExitCode_InvalidArguments;
        result.Message = "Error: --exit requires --run.\n";
        return result;
    }

    if (options.CLI.ListOnly)
        options.IgnoredOptions.push_back("--list");

    if (options.CLI.PauseOnExit)
        options.IgnoredOptions.push_back("--pause");

    if (options.CLI.ColorMode != TColorMode::Auto)
        options.IgnoredOptions.push_back("--color/--no-color");

    if (options.CLI.ColorPass.has_value() || options.CLI.ColorFail.has_value() || options.CLI.ColorSkip.has_value())
        options.IgnoredOptions.push_back("--color-pass/--color-fail/--color-skip");

    return result;
}
//---------------------------------------------------------------------------
std::vector<std::string> TGUICommandLine::ProcessArguments()
{
    std::vector<std::string> args;

    for (int i = 1; i <= System::ParamCount(); ++i)
        args.push_back(ToUTF8(System::ParamStr(i)));

    return args;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
