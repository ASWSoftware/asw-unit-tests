/* **************************************************************************
ASWUnitTests_Console.cpp
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
#include "ASWUnitTests_Console.h"
//---------------------------------------------------------------------------
#include <cstdio>
#include <cstdlib>
#include <unordered_map>
//---------------------------------------------------------------------------
#if defined(_WIN32)
#  define WIN32_LEAN_AND_MEAN
#  include <io.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif
//---------------------------------------------------------------------------

namespace
{

using namespace ASWUnitTests;

TColorMode g_ColorMode = TColorMode::Auto;
TConsoleColor g_PassColor = TConsoleColor::Green;
TConsoleColor g_FailColor = TConsoleColor::Red;
TConsoleColor g_SkipColor = TConsoleColor::Yellow;

//---------------------------------------------------------------------------

std::string const& AnsiCode(TConsoleColor color);
std::string const& AnsiReset();
bool DetectColorSupport();
#if defined(_WIN32)
bool EnableWindowsAnsiSupport();
#endif
bool EnvVarIsSet(char const* name);
TConsoleColor GetColorFor(TLogKind kind);
bool IsStdoutTTY();

//---------------------------------------------------------------------------
std::string const& AnsiCode(TConsoleColor color)
{
    static std::string const codes[] =
    {
        "\x1b[39m", // Default
        "\x1b[30m", // Black
        "\x1b[31m", // Red
        "\x1b[32m", // Green
        "\x1b[33m", // Yellow
        "\x1b[34m", // Blue
        "\x1b[35m", // Magenta
        "\x1b[36m", // Cyan
        "\x1b[37m", // White
        "\x1b[90m", // BrightBlack
        "\x1b[91m", // BrightRed
        "\x1b[92m", // BrightGreen
        "\x1b[93m", // BrightYellow
        "\x1b[94m", // BrightBlue
        "\x1b[95m", // BrightMagenta
        "\x1b[96m", // BrightCyan
        "\x1b[97m", // BrightWhite
    };

    return codes[static_cast<size_t>(color)];
}
//---------------------------------------------------------------------------
std::string const& AnsiReset()
{
    static std::string const reset = "\x1b[0m";
    return reset;
}
//---------------------------------------------------------------------------
bool DetectColorSupport()
{
    if (!IsStdoutTTY())
        return false;

#if defined(_WIN32)
    return EnableWindowsAnsiSupport();
#else
    return true;
#endif
}
//---------------------------------------------------------------------------
#if defined(_WIN32)
bool EnableWindowsAnsiSupport()
{
    HANDLE const hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE)
        return false;

    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode))
        return false;

    return SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING) != 0;
}
#endif
//---------------------------------------------------------------------------
bool EnvVarIsSet(char const* name)
{
    char const* value = std::getenv(name);
    return value != nullptr && value[0] != '\0';
}
//---------------------------------------------------------------------------
TConsoleColor GetColorFor(TLogKind kind)
{
    switch (kind)
    {
        case TLogKind::Pass:
            return g_PassColor;
        case TLogKind::Fail:
            return g_FailColor;
        case TLogKind::Skip:
            return g_SkipColor;
        case TLogKind::Info:
        default:
            return TConsoleColor::Default;
    }
}
//---------------------------------------------------------------------------
bool IsStdoutTTY()
{
#if defined(_WIN32)
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}
//---------------------------------------------------------------------------

} // namespace

//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TConsole
/////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------
std::string TConsole::Colorize(std::string const& text, TLogKind kind)
{
    if (kind == TLogKind::Info || !IsColorEnabled())
        return text;

    return AnsiCode(GetColorFor(kind)) + text + AnsiReset();
}
//---------------------------------------------------------------------------
bool TConsole::IsColorEnabled()
{
    if (g_ColorMode == TColorMode::Always)
        return true;

    if (g_ColorMode == TColorMode::Never)
        return false;

    // Auto: respect https://no-color.org, then fall back to TTY/VT support detection.
    if (EnvVarIsSet("NO_COLOR"))
        return false;

    static bool const supported = DetectColorSupport();
    return supported;
}
//---------------------------------------------------------------------------
std::optional<TConsoleColor> TConsole::ParseColorName(std::string const& name)
{
    static std::unordered_map<std::string, TConsoleColor> const colors =
    {
        { "default", TConsoleColor::Default },
        { "black", TConsoleColor::Black },
        { "red", TConsoleColor::Red },
        { "green", TConsoleColor::Green },
        { "yellow", TConsoleColor::Yellow },
        { "blue", TConsoleColor::Blue },
        { "magenta", TConsoleColor::Magenta },
        { "cyan", TConsoleColor::Cyan },
        { "white", TConsoleColor::White },
        { "bright-black", TConsoleColor::BrightBlack },
        { "bright-red", TConsoleColor::BrightRed },
        { "bright-green", TConsoleColor::BrightGreen },
        { "bright-yellow", TConsoleColor::BrightYellow },
        { "bright-blue", TConsoleColor::BrightBlue },
        { "bright-magenta", TConsoleColor::BrightMagenta },
        { "bright-cyan", TConsoleColor::BrightCyan },
        { "bright-white", TConsoleColor::BrightWhite },
    };

    std::unordered_map<std::string, TConsoleColor>::const_iterator const it = colors.find(name);
    if (it == colors.end())
        return std::nullopt;

    return it->second;
}
//---------------------------------------------------------------------------
void TConsole::SetColor(TLogKind kind, TConsoleColor color)
{
    switch (kind)
    {
        case TLogKind::Pass:
            g_PassColor = color;
            break;
        case TLogKind::Fail:
            g_FailColor = color;
            break;
        case TLogKind::Skip:
            g_SkipColor = color;
            break;
        case TLogKind::Info:
        default:
            break; // Info is never colored
    }
}
//---------------------------------------------------------------------------
void TConsole::SetColorMode(TColorMode mode)
{
    g_ColorMode = mode;
}
//---------------------------------------------------------------------------

} // namespace ASWUnitTests
