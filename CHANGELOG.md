# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Versions before 0.26.1 are not itemized here;
see [0.26.1](#0261---2026-09-12) for the initial versioned baseline.

## [Unreleased]

### Added

- CONTRIBUTING.md, documenting the contribution workflow, branch and commit
  conventions, and bug report format.
- This CHANGELOG.md file.
- CI workflow (`.github/workflows/ci.yml`), building and running the self-test
  suite on Windows (MSVC, MinGW) and Linux (GCC, Clang), plus a JUnit-based
  native test-results summary and a partitioning smoke test.
- `--test-timeout-seconds`, aborting the run if a single test doesn't finish
  in time (e.g. an infinite loop), instead of hanging forever. The offending
  test is recorded as failed and the process exits with a dedicated exit code
  (`5`); if `--report-junit` was also given, the report still gets written,
  covering everything that completed before the timeout.
- `rad370/Build_Win32_Debug.bat` and `Build_Win32_Release.bat`, for building
  and testing the RAD Studio 32-bit target the same way the existing Win64x
  scripts already covered the 64-bit one.
- `--catch-crashes`, catching a native crash (e.g. an access violation or
  segmentation fault) in a test and recording it as failed instead of letting
  it take down the whole process. Most crash types let the run continue with
  the next test; a stack overflow on Windows, or any segmentation fault on
  POSIX (which can't be cheaply told apart from a stack overflow there),
  aborts the run afterward instead, with its own dedicated exit code (`6`).
  Never attempts to catch `SIGABRT`. Implemented with
  `AddVectoredExceptionHandler` on Windows, verified directly (including
  recovering from a genuine stack overflow) across MSVC, MinGW, and RAD
  Studio's `bcc32c`/`bcc64`, after `__try`/`__except` turned out to compile
  but not actually work on RAD Studio's compilers. On POSIX, catching a
  genuine stack overflow relies on an alternate signal stack
  (`sigaltstack()`/`SA_ONSTACK`), since the default handler would otherwise
  run on the same, already-exhausted stack that just overflowed and have
  nowhere to run; verified on Linux via CLion/SSH.

### Fixed

- `IsStdoutTTY()` compile error on RAD Studio's 32-bit compiler (`bcc32c`),
  which declares the POSIX-style `isatty()` in `<io.h>` rather than the
  underscore-prefixed `_isatty()` MSVC, MinGW, and RAD Studio's own 64-bit
  compiler use. Windows64/MSVC/MinGW are unaffected.

## [0.26.5] - 2026-09-24

### Added

- Self-registration for test groups via `ASW_REGISTER_TEST_GROUP` /
  `ASW_REGISTER_TEST_GROUP_ORDERED`, so adding, removing, or renaming a test
  module never requires editing a framework source file.
- Parameterized/data-driven test support (`RegisterTestCases`), registering one
  independently named, filterable test case per row of data.
- `Skip()`, aborting a test and reporting it as skipped (separately from
  passed/failed, with no effect on the process exit code) without removing its
  registration.
- Tolerance-based floating-point comparison: `CheckNear`/`AssertNear` and
  `CheckNotNear`/`AssertNotNear` for `float`/`double`.
- Exception type and message matching for `SetExceptionExpected<TException>`,
  matched polymorphically against the thrown exception.
- `--filter`/`--filter-ignore-case` and `--list`, for selecting and previewing
  which registered tests will run.
- `--shuffle`/`--shuffle-seed`, to randomize test/group run order as a check
  against hidden inter-test coupling, with a logged seed for reproducing a
  shuffle-induced failure.
- `--partition-index`/`--partition-count`, splitting the suite across separate
  process invocations (e.g. a CI job matrix) for parallel execution with no
  in-process threading and no merge step.
- Colorized console output for pass/fail/skip status, with `--color`,
  `--no-color`, and `--color-pass`/`--color-fail`/`--color-skip` options, and
  automatic detection of terminal support and the `NO_COLOR` environment
  variable.
- `--report-junit`, writing a JUnit-style XML test report with no third-party
  dependency, recognized natively by most CI systems.
- `--pause`, prompting "press enter to continue" before exit, replacing the
  previous RAD Studio/C++Builder-only, debug-only pause behavior.
- Per-test timing in console output, and the test's full name included in its
  skip/fail message.
- `ASWUnitTests_StdOutRedirect`, a small utility for capturing console output
  written during a test.
- Unit tests covering the framework's own components
  (`ASWUnitTests_TestBase`, `ASWUnitTests_CLI`, `ASWUnitTests_Console`,
  `ASWUnitTests_Handler`, `ASWUnitTests_JUnitReport`,
  `ASWUnitTests_StdOutRedirect`), plus example tests for UTF-8 string handling.
- `ASWUnitTests_Sources.cmake`, `Deploy.bat`, `.gitattributes`, and submodule
  integration documentation, to support dropping this framework into another
  repository as a git submodule.
- ASCII-only source enforcement across framework and example files.

### Changed

- `main()` split into smaller functions, with command-line parsing extracted
  into its own `ASWUnitTests_CLI` unit.
- Minimum required C++ standard raised to C++17.
- Example `ASWTools_String` tests refactored to remove compiler warnings.

### Fixed

- Integer overflow bug in `TCLIParser::ParseUnsignedInt`.
- MinGW linker issue.
- Non-Windows compiler errors in framework and example code.
- Line endings in `ASWUnitTests.cbproj`.

## [0.26.3] - 2026-09-12

### Fixed

- Linux compatibility issues in the example `ASWTools_String` code.

### Changed

- README cleanup and a typo fix.

## [0.26.1] - 2026-09-12

Initial versioned release, switching the project to semantic versioning and
adding `ASWUnitTests_Version.h`/`--version`. Everything already present in the
framework at this point (test group registration, `Check`/`Assert` methods,
RAD Studio and CMake build support, etc.) is treated as the baseline and is not
itemized commit-by-commit.

[Unreleased]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.5...HEAD
[0.26.5]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.3...v0.26.5
[0.26.3]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.1...v0.26.3
[0.26.1]: https://github.com/ASWSoftware/asw-unit-tests/releases/tag/v0.26.1
