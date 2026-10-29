# AGENTS.md

## Scope

These instructions apply to the entire repository unless a more specific `AGENTS.md` is added in a subdirectory. Keep changes focused on the requested behavior and preserve existing public APIs unless a change is necessary.

## Repository Overview

ASWUnitTests is a lightweight C++ console unit-testing framework.

- `src/` contains the framework implementation.
- `gui/src/` contains the GUI runners' shared units, which contain no UI or VCL code, and `gui/tests/` their unit tests, which every project (CMake and RAD Studio) compiles, so they also run on Linux.
- `tests/` contains unit tests for the example code.
- `toTest/` contains example code under test.
- `rad370/` contains the RAD Studio 13.1 project and Windows build scripts.
- `vcl/` contains RAD Studio projects that link the VCL and define `ASWUNITTESTS_RTL_EXCEPTIONS`, exercising the framework's opt-in RTL support (RTL exceptions and the `System::String` overloads): `vcl/console/rad370/` is a VCL console project (with its own build scripts) that runs the full self-test suite plus the RTL-specific and GUI unit tests in `vcl/tests/`. `vcl/gui/rad370/` is the VCL GUI runner project (with its own build scripts, and a project group that also opens the console project), built from the sources in `vcl/gui/src/`. The console project also compiles the GUI's non-form units, so their tests run without the GUI.
- `win32/` contains the Win32 GUI runner, which uses only the Windows API (no VCL) and has the VCL GUI runner's features: `win32/gui/src/` holds its units, its `WinMain()`, its manifest and `ASWUnitTests_Win32GUI_Sources.cmake`, from which `cmake/CMakeLists.txt` builds the `ASWUnitTests_Win32_GUI` target on Windows. `win32/gui/rad370/` is its RAD Studio project (Win32 and Win64x, with its own build scripts). `win32/tests/` has the tests of its units that need no window, which the CMake console target and the `rad370` console project also compile on Windows.
- `README.md` contains usage and integration examples.
- `.uncrustify.cfg` and `.githooks/` define the repository formatting workflow.

## Accuracy-First Engineering

Contributors and AI coding agents are expected to be expert, skeptical, and careful. Verify assumptions by inspecting nearby code, call sites, tests, project files, and relevant history or documentation when needed. Neither the human contributor nor the AI is infallible; treat requirements and proposed fixes as hypotheses to check, and prefer evidence from a focused build or test over intuition.

Before changing behavior:

1. Identify the code path that owns the behavior.
2. State or test a concrete hypothesis about the cause or intended result.
3. Make the smallest change that addresses the cause.
4. Run the narrowest relevant validation, then broader checks when practical.

Do not hide unrelated failures by changing tests or weakening diagnostics. Do not revert changes that were already present unless explicitly asked.

## Language and Toolchain Compatibility

New and modified code must compile as C++17, the minimum the framework supports; RAD Studio's 32-bit compiler (bcc32c) only supports C++17. C++20 features are allowed only as optional additions behind a feature-test check, with the C++17 path still complete, as `ASWUnitTests_TestBase.h` does for `std::source_location` by defining `ASWUNITTESTS_SOURCE_LOCATION_ENABLED`. Test the library feature-test macro (e.g. `__cpp_lib_source_location`) rather than `__has_include`, since a header can be present but unusable. The CMake and RAD Studio Win64x builds compile at C++20, so validate code with a C++20 path on both a C++17 build (such as RAD Studio Win32) and a C++20 build.

New and modified code must also be compatible with C++ console projects and with:

- RAD Studio 13.1 and later
- CMake
- JetBrains CLion
- Visual Studio
- Clang
- MinGW
- Windows and Linux

Prefer standard C++ and portable library facilities. Avoid compiler-, IDE-, or operating-system-specific extensions unless they are isolated behind a clear portability boundary. For OS-specific code, use valid preprocessor checks such as `#if defined(_WIN32)` and provide the corresponding Linux or portable path where appropriate. Keep headers self-contained and avoid relying on transitive includes.

Keep all source files (`.cpp`/`.h`) plain ASCII, including comments. MSVC reads a BOM-less file using the system code page by default, unlike GCC/Clang/RAD Studio's Clang-based compiler, which default to UTF-8. A literal non-ASCII character can therefore silently compile to the wrong value on MSVC while working correctly everywhere else, with no error or warning. If a string needs a specific non-ASCII character, use a numeric escape instead of typing the character literally (e.g. `"\xC3\xA9"` for UTF-8 bytes, or `L"\x00E9"` for a wide-string code point), splitting adjacent escapes into separate concatenated string literals if needed so one escape doesn't consume a following hex digit.

## Code Organization

- Insert new class methods in alphabetical order within their existing section. If the class has named or visibly separated sections, preserve those sections and alphabetize only within the relevant section.
- Insert new free functions in alphabetical order within their existing section. Apply the same section rule when functions are grouped.
- Lay out a new class's members by kind, each kind in a section of its own with its own access label, even when the label repeats: nested types first, then static data, then data members, then the methods, grouped by access (private, then protected, then public), with each group's static methods in a section before its other methods. See `TWin32GUIMainWindow` (`win32/gui/src/ASWUnitTests_Win32GUI_MainWindow.h`) and `TStringCheckProbe` (`tests/UT_Helper_StringChecks.h`).
- Follow the surrounding naming, indentation, brace, include, and comment style. Avoid unrelated formatting changes.
- Keep declarations and definitions consistent, and update all relevant project or build files when adding, removing or renaming source files, including `Deploy.bat`, which copies an explicit list of files, and the `*_Sources.cmake` files (`src/`, `gui/src/`, `win32/gui/src/`), which list each folder's sources for projects that use this repository as a submodule.
- Add or update focused tests for behavior changes, including error and platform-specific cases where applicable.
- Each unit under test has exactly one test module: `ASWUnitTests_X` is tested by `Test_ASWUnitTests_X.cpp`/`.h` (class `TTest_ASWUnitTests_X`, group name `"ASWUnitTests_X_Tests"`), in the matching tests folder, and its tests go there, however large it grows. Supporting code that isn't a test itself, such as a fixture group or a probe whose templates are instantiated for the tests, may go in a helper module named `UT_Helper_<what>.cpp`/`.h` in the same folder (e.g. `tests/UT_Helper_StringChecks`), listed in the projects and build files like any other source file.
- RAD Studio's Win32 linker (ilink32) limits the line-number debug information of each module (its "Line number cache"), and `tests/Test_ASWUnitTests_TestBase.cpp` is close to that limit. A Win32 Debug build then fails with "Fatal: Exceeded memory limit for block Line number cache in module ...". Every distinct lambda registered as a test costs a `std::function` instantiation, so prefer a few tests that each check many cases (or parameterized tests, which share one callback) to hundreds of registered lambdas, and move heavy template instantiations into a `UT_Helper_` module, defined and explicitly instantiated in its `.cpp`. Check with the VCL console's Win32 Debug build, which links the most modules.

## Naming and Style

- New enums take an `E` prefix (`EWin32GUICommand`); classes and structs take `T`. Existing enums that use `T` (e.g. `TLogKind`, `TTestOutcome`) keep it.
- Write acronyms in identifiers and file names in uppercase: `GUI`, `CLI`, `RTL`, `UTF8`, `XML` (e.g. `TCLIParser`, `ToUTF8`, `ASWUnitTests_GUI_TestList`). `JUnit` is a name, not an acronym. Check an existing name against this rule before copying its casing.
- Compare a pointer, `std::function` or other nullable value with `nullptr` explicitly (`if (filter != nullptr)`), rather than relying on its conversion to `bool` (`if (filter)`).
- In a `.cpp` file, describe a function in a block comment between its `//----` separator line and its definition (`/*`, the function's name, a blank line, the description, `*/`), not in `//` lines above it; a function that needs no explanation gets no comment. Declarations in a header keep `//` comments. See `win32/gui/src/ASWUnitTests_Win32GUI_System.cpp`.
- Suppress an intentional MSVC warning with `#pragma warning(push)`, `#pragma warning(disable: N)` and `#pragma warning(pop)` around the whole function or block, guarded by `#if defined(_MSC_VER)`, never with `#pragma warning(suppress: N)`: MSVC checks some warnings (e.g. C4702) against the state at the end of the function, after a `suppress` has lapsed.
- Don't name an identifier `assert`, even a parameter: `assert()` is a macro wherever `<cassert>` is included, and the VCL headers include it.

## Building and Testing

The RAD Studio debug build can be run from `rad370/Build_Win64x_Debug.bat` when the `Rad370` environment variable points to the RAD Studio installation. The release script is `rad370/Build_Win64x_Release.bat`. Neither pauses before exiting by default; pass `--pause` to have the console prompt "press enter to continue" first, e.g. when running from an IDE that would otherwise close the console immediately.

For other environments, use the repository's CMake configuration when present or the IDE's native project configuration. Tests should be run outside the debugger unless debugging an expected exception is intentional. Before considering a change complete:

- Build the affected configuration.
- Run the unit-test executable.
- Check compiler warnings and errors.
- Validate both Windows-specific and portable paths when the change touches platform code.
- When the change touches code guarded by `ASWUNITTESTS_RTL_EXCEPTIONS_ENABLED`, or the exception handling around it, also build and run the VCL console project (`vcl/console/rad370/Build_Win64x_Debug.bat` and the Win32 script), since no other build compiles that code.
- When the change touches `vcl/gui/src/` or `gui/src/`, or framework code the GUI runners use (such as `ITestRunObserver` or `ASWUnitTests_CLI`), also build the VCL console project (which runs the GUI's unit tests) and the VCL GUI project (`vcl/gui/rad370/Build_Win64x_Debug.bat` and the Win32 script), then run the GUI and check the affected behavior by hand.
- When the change touches `win32/gui/src/` or `gui/src/`, or framework code the GUI runners use, also build the CMake `ASWUnitTests_Win32_GUI` target with MinGW, with Clang (llvm-mingw) and, when available, with MSVC, and the RAD Studio Win32 GUI project (`win32/gui/rad370/Build_Win64x_Debug.bat` and the Win32 script), then run the GUI and check the affected behavior by hand. A full `--run --exit` of each build (as CI does) checks that it still runs every test.

## Changelog

`CHANGELOG.md` follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/) and Semantic Versioning.

- Record each notable change under `## [Unreleased]`, in the matching `### Added`, `### Changed`, `### Deprecated`, `### Removed`, `### Fixed`, or `### Security` section.
- Describe the effect on someone using the framework, not the commit. For a breaking change, say what existing code must change.
- The version lives only in `src/ASWUnitTests_Version.h` (the `ASWUNITTESTS_VERSION_*` macros; the constants and `TTestHandler::GetVersionStr()` come from them). `main` carries only release versions, with an empty `ASWUNITTESTS_VERSION_PRERELEASE`. Between releases, `develop` carries the next planned version with a pre-release such as `dev.1` (e.g. `1.1.0-dev.1` after `1.0.0`), which sorts before that release; bump its number only to tell dev builds apart, and never bump the patch number on `develop`. A unit test checks that `ASWUNITTESTS_VERSION_STRING` matches the parts.
- When preparing a release, set the release version in `src/ASWUnitTests_Version.h` (empty pre-release; the number develop forecast may change, e.g. to a patch release or the next major), rename `[Unreleased]` to `[x.y.z] - YYYY-MM-DD`, start a new empty `[Unreleased]`, and update the compare links at the bottom of the file. After merging the release into `develop`, move `develop` to the next minor version with pre-release `dev.1`.

## Formatting and Review

Use the repository's `.uncrustify.cfg` configuration. The README documents how to enable the local Git hook with:

```text
git config --local core.hooksPath .githooks/
```

Wrap Markdown prose at 100 columns in `README.md`, `CONTRIBUTING.md` and `CHANGELOG.md`, re-wrapping the edited paragraph when a change makes a line too long. Don't split inline code or a link's text across lines. Code blocks and tables are exempt, and so is a line that only overflows because a single link or URL can't be split.

Don't use em dashes in anything written for this repository (documentation, comments, messages): use a hyphen with a space on each side (" - "), or restructure the sentence.

Review the final diff for accidental file changes, ordering regressions, missing includes, platform assumptions, and tests that do not actually exercise the changed behavior. Do not commit changes unless the user explicitly requests a commit.
