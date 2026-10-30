/* **************************************************************************
ASWUnitTests_Console.h
Author: Anthony S. West - ASW Software

Portable ANSI console coloring, used to colorize pass/fail/skip output.

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

#ifndef ASWUnitTests_ConsoleH
#define ASWUnitTests_ConsoleH
//---------------------------------------------------------------------------
#include <optional>
#include <string>
//---------------------------------------------------------------------------

namespace ASWUnitTests
{

/////////////////////////////////////////////////////////////////////////////
// TConsoleColor
/////////////////////////////////////////////////////////////////////////////
enum class TConsoleColor
{
    Default,
    Black,
    Red,
    Green,
    Yellow,
    Blue,
    Magenta,
    Cyan,
    White,
    BrightBlack,
    BrightRed,
    BrightGreen,
    BrightYellow,
    BrightBlue,
    BrightMagenta,
    BrightCyan,
    BrightWhite,
};


/////////////////////////////////////////////////////////////////////////////
// TColorMode
/////////////////////////////////////////////////////////////////////////////
enum class TColorMode
{
    Auto,   // color only if stdout is an interactive terminal that supports it
    Always,
    Never,
};


/////////////////////////////////////////////////////////////////////////////
// TLogKind
//
// What a piece of output represents, for TConsole::Colorize(). Info is
// never colored; it exists so callers can pass a kind unconditionally.
/////////////////////////////////////////////////////////////////////////////
enum class TLogKind
{
    Info,
    Pass,
    Fail,
    Skip,
};


/////////////////////////////////////////////////////////////////////////////
// TConsole
//
// Centralizes ANSI color output so it degrades cleanly: colors are emitted
// only when explicitly requested (TColorMode::Always) or, in the default
// TColorMode::Auto, only when stdout is an interactive terminal that
// supports it. Detection is portable for Windows and Linux/POSIX; on
// Windows this also enables Virtual Terminal Processing so cmd.exe/
// PowerShell consoles predating native ANSI support still work when
// possible. Auto mode also respects the https://no-color.org convention
// (the NO_COLOR environment variable). The color used for each TLogKind is
// independently configurable via SetColor().
/////////////////////////////////////////////////////////////////////////////
class TConsole
{
public:
    static TColorMode GetColorMode();
    static void SetColorMode(TColorMode mode);
    static void SetColor(TLogKind kind, TConsoleColor color);
    static bool IsColorEnabled();

    // Wraps 'text' in the ANSI code configured for 'kind', or returns it unchanged if color is
    // disabled or kind is TLogKind::Info.
    static std::string Colorize(std::string const& text, TLogKind kind);

    // Parses a color name (e.g. "red", "bright-green") as accepted by --color-pass/--color-fail/
    // --color-skip; returns std::nullopt if 'name' isn't recognized.
    static std::optional<TConsoleColor> ParseColorName(std::string const& name);

    // Returns 'text' without the ANSI color sequences Colorize() adds (ESC '[', digits and ';', then 'm'), as a
    // run observer gets the log. Any other text, an incomplete sequence included, is left as it is.
    static std::string StripColor(std::string const& text);
};


/////////////////////////////////////////////////////////////////////////////
// TConsoleUTF8Output
//
// Sets the Windows console's output code page to UTF-8 for as long as it
// exists, so the UTF-8 text the framework writes (such as a non-ASCII test
// name or message) shows correctly in a console that uses another code
// page, then restores the console's own. Output redirected to a file or a
// pipe gets the same bytes either way. Does nothing without a console, or
// on Linux, where terminals already expect UTF-8.
/////////////////////////////////////////////////////////////////////////////
class TConsoleUTF8Output
{
private:
    unsigned int m_OriginalCodePage; // The code page to restore, or 0 when there's nothing to restore.

public:
    TConsoleUTF8Output();
    TConsoleUTF8Output(TConsoleUTF8Output const&) = delete;
    TConsoleUTF8Output& operator=(TConsoleUTF8Output const&) = delete;
    ~TConsoleUTF8Output();

    // Restores the console's own code page now, e.g. before std::_Exit(), which skips destructors. Does nothing
    // once it's restored.
    void Restore();
};

} // namespace ASWUnitTests

#endif // #ifndef ASWUnitTests_ConsoleH
