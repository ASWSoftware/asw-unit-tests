# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Versions before 0.26.1 are not itemized here;
see [0.26.1](#0261---2026-09-12) for the initial versioned baseline.

## [Unreleased]

### Added

- `CheckContains`/`AssertContains` and `CheckNotContains`/`AssertNotContains`,
  for `std::string` and `std::wstring`, taking the text first and then the
  substring. Unlike `CheckTrue(text.find(substring) != std::string::npos, ...)`,
  a failure shows both, with wide text converted to UTF-8. Case-insensitive
  `IC` variants (`CheckContainsIC`, etc.) ignore the case of the ASCII letters
  `A`-`Z` only, the same on every platform and locale. They use the new
  `src/ASWUnitTests_Utils.cpp`, which a project that lists the framework's
  source files by hand, rather than through `ASWUnitTests_Sources.cmake`, must
  add.
- VCL GUI runner: the checked tests are saved when the window closes and
  restored when it next opens, in `%APPDATA%\ASWUnitTests\<exe name>.selection`,
  unless the command line chooses the tests itself (`--run`, `--filter`, or
  partition options). New tests are checked if their group was entirely
  checked, removed tests are skipped, and the log notes what was restored.
- VCL GUI runner: **Select Failed** (Tests menu and toolbar), which checks only
  the shown tests that failed in the latest run, so they can be rerun with Run
  Selected.
- Version macros in `ASWUnitTests_Version.h`: `ASWUNITTESTS_VERSION_MAJOR`,
  `ASWUNITTESTS_VERSION_MINOR`, `ASWUNITTESTS_VERSION_PATCH`,
  `ASWUNITTESTS_VERSION_PRERELEASE` (empty on a release, e.g. `dev.1` between
  releases) and `ASWUNITTESTS_VERSION_STRING` (e.g. `1.1.0-dev.1`), usable in
  `#if`, plus a matching `ASWUnitTests::VersionPreRelease` constant. The
  existing `VersionMajor`, `VersionMinor`, `VersionPatch` and `Version`
  constants now come from the macros.
- `std::source_location` overloads of every `Check*`/`Assert*` method,
  `Skip()`, and `SetExceptionExpected()`, taking the caller's location by
  default instead of a method and line, so a call no longer needs
  `__func__, __LINE__`. Requires C++20, so they're unavailable with RAD
  Studio's 32-bit compilers; `ASWUNITTESTS_SOURCE_LOCATION_ENABLED` is defined
  when they're available. The method and line overloads are unchanged.
- `CheckEquals`/`AssertEquals`/`CheckNotEquals`/`AssertNotEquals` overloads
  for any two integer types other than `bool`, compared by value. A call
  mixing integer types (e.g. `int` and `int64_t`), or using one that matches
  none of the fixed-width overloads (e.g. `long` on Windows, `long long` on
  Linux), used to be an ambiguous-overload compile error. A negative value
  never equals an unsigned one, unlike with the built-in `==`.

### Changed

- Development builds between releases are versioned with a SemVer pre-release
  (e.g. `1.1.0-dev.1`), which sorts before the release it leads up to.

### Fixed

- `CheckEquals`/`AssertEquals`/`CheckNotEquals`/`AssertNotEquals` comparing two
  C strings (e.g. two string literals) as `bool` instead of by content, so an
  `Equals` check always passed and a `NotEquals` check always failed, whatever
  the text. New `char const*` and `wchar_t const*` overloads compare by
  content. A test that used to pass this way may now correctly fail.
- A failed `CheckEquals` of two `bool` values showing them as `1`/`0`. It now
  shows `true`/`false`, the same as `AssertEquals` and the `NotEquals` methods.
- CI test-results reporting failing for pull requests from forks, whose
  read-only token can't create check runs. Each CI job now shows its JUnit
  report on the run's summary page instead, which needs no write permission,
  and also uploads it as an artifact.

## [1.0.0] - 2026-09-27

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
- Opt-in support for RAD Studio RTL exceptions (`System::Sysutils::Exception`
  and its subclasses, shared by VCL and FMX), enabled by defining
  `ASWUNITTESTS_RTL_EXCEPTIONS` in a C++Builder project built with `bcc32c` or
  `bcc64x` (any other compiler is a compile error). `SetExceptionExpected<T>`
  accepts RTL exception classes, matched polymorphically, with the optional
  message substring checked against `Message`. Failure details and unhandled
  RTL exceptions are reported as `ClassName: Message`, and an unhandled one
  exits with code `2` instead of `3` ("Unknown"). Under
  `--test-timeout-seconds`, an unexpected RTL exception crosses back from the
  worker thread as a `TExceptRTLException` (a `std::runtime_error` that keeps
  the original class in `RTLClass()`), since `std::exception_ptr` can't carry
  an RTL exception past its handler; verified on `bcc64x`, where it
  terminated the process.
- `vcl/console/rad370/`, a VCL console RAD Studio project with its own build
  scripts, defining `ASWUNITTESTS_RTL_EXCEPTIONS` and running the full
  self-test suite plus the RTL-specific tests in `vcl/tests/`.
- `ITestRunObserver`, set with `TTestHandler::SetRunObserver()`, for
  following a run as it happens (e.g. from a GUI runner): it receives the log
  output that would otherwise go to `std::cout`, a start and finish event for
  each test, and can stop the run between tests. Its events arrive on the
  thread that called `Run()`, even under `--test-timeout-seconds`. Without
  one, nothing changes.
- `TTestHandler::GetTests()`, listing every registered test as a `TTestId`
  (its group and test names, kept separate since a group name may contain
  `.`), e.g. for a runner to show the tests before running them.
- `ExitCodeForResults()` and `ToJUnitTestCases()` in `ASWUnitTests_CLI`,
  moved out of `main.cpp` so the console and GUI runners share them. A run
  stopped early through `ITestRunObserver` exits with `1`.
- `vcl/gui/rad370/`, a VCL GUI runner (with its own build scripts, sources in
  `vcl/gui/src/`, and a project group opening it with the VCL console
  project), running the same tests as the console runner. It shows every test
  in a check box tree with a colored status dot, a detail pane for the
  selected test or group, a Failures & Skips list, and the run's colored log.
  Its toolbar runs the checked tests (F9), reruns failures, stops a run after
  the current test, checks or unchecks every shown test, copies the details,
  and filters the tree with `--filter`-style wildcards; its main menu has the
  same commands, plus File > Export JUnit Report, saving the latest run's
  results as the report `--report-junit` writes, and the command line help
  and version. It remembers its window size, position, and panel sizes
  between sessions (per executable, in `%APPDATA%\ASWUnitTests`), with
  View > Reset Layout to restore the defaults, and GUI-only
  `--layout-ignore` and `--layout-reset` options to skip or delete the saved
  layout. It takes the console runner's command line options (`--filter`
  and partitions choose the initially checked tests, and `--filter` also
  fills in the filter box), plus GUI-only `--run`, running the checked tests
  on startup, and `--exit`, closing afterward with the console runner's exit
  code. Tests run on the main thread, so they can create VCL forms and
  controls.
- `TJUnitReportWriter::BuildXML()`, returning the report's XML as a string
  for a caller that writes the file itself (as the GUI runner does, since
  `Write()`'s `std::ofstream` can't open a path with characters outside the
  Windows code page from a `std::string`).

### Changed

- `TTestGroupBase::Run()` starts from empty `Results()`, so running the same
  group again (e.g. repeated runs in the VCL GUI runner) reports only that
  run's outcomes instead of adding them to the previous run's.
- MSVC warnings in the framework sources cleaned up, or suppressed where
  intentional, so a project building them at a high warning level stays
  clean.

### Fixed

- `CheckTrue()` and `CheckFalse()` failure messages had their expectations
  swapped (a failed `CheckTrue()` reported "Expected false but was true", and
  vice versa).
- A test that failed only through `Check*` calls recorded an empty failure
  detail, so its `--report-junit` entry was `<failure message="">` with no
  explanation. Its record now carries each of its `Check*` failures (followed
  by any `Assert*` failure after them). Console output is unchanged, since
  those failures are still logged as they happen.
- A `Check*` failure was ignored if the test then threw the exception it had
  set up with `SetExceptionExpected()`, so the test was reported as passed.
  It now fails, with the `Check*` failure in its detail.
- A failed `Assert*` counted as the expected exception while one was expected
  with `SetExceptionExpected()`, so the test was reported as passed. With the
  templated `SetExceptionExpected<T>()`, the requested type wasn't even
  checked. A failed `Assert*` now always fails the test.
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

[Unreleased]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.0.0...HEAD
[1.0.0]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.5...v1.0.0
[0.26.5]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.3...v0.26.5
[0.26.3]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.1...v0.26.3
[0.26.1]: https://github.com/ASWSoftware/asw-unit-tests/releases/tag/v0.26.1
