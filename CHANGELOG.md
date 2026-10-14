# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and this project
adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

Versions before 0.26.1 are not itemized here; see [0.26.1](#0261---2026-09-12) for the initial
versioned baseline.

## [Unreleased]

### Added

- Test suites, for organizing test groups into a hierarchy of any depth. Pass a `TSuitePath` to
  the new `TTestGroupBase(TSuitePath const& suitePath, std::string const& name)` constructor, or
  the matching `TAutoTestGroup` one, e.g.
  `inherited(TSuitePath("Framework.Console"), "TTest_TConsole")` for suite `Console` inside suite
  `Framework`. A suite's groups run together, and the suite path starts its tests' full names, so
  `--filter "Framework.*"` runs every test in suite `Framework`, and `--list`, the console log and
  the `--report-junit` report's `classname` show `Framework.Console.TTest_TConsole`. Suites only
  organize groups: they have no set-up or tear-down of their own. The order given to
  `ASW_REGISTER_TEST_GROUP_ORDERED` places a group among its own suite's groups and suites, where a
  suite sorts as order `0` by its own name, and `--shuffle` shuffles each suite's contents
  separately, so they still run together. A suite path with an empty suite name, such as
  `"Framework..Console"`, throws the new `TExceptInvalidSuitePath`, stopping the run at startup with
  exit code `2`. Groups that aren't in a suite run in the same order as before, including the order
  a given `--shuffle-seed` produces.
- `ASW_REGISTER_TEST_GROUP_IN_SUITE(ClassName, suitePath)` and
  `ASW_REGISTER_TEST_GROUP_IN_SUITE_ORDERED(ClassName, suitePath, order)` put a group in a suite
  from its registration, e.g.
  `ASW_REGISTER_TEST_GROUP_IN_SUITE(TTest_TConsole, "Framework.Console")`, so its constructor
  needn't name the suite. A group whose constructor names a different suite, or an `ITestGroup`
  implemented without `TTestGroupBase` whose own `GetSuitePath()` doesn't give that suite, stops the
  run at startup with `TExceptInvalidSuitePath`. `TTestGroupRegistry::Register()`
  takes the suite path as a new last parameter, and `TRegisteredTestGroupFactory` gains a
  `SuitePath` member, which defaults to empty as `TTestCaseRecord`'s does.
- `ITestGroup::GetSuitePath()` and `ITestGroup::GetQualifiedTestGroupName()`, and a `SuitePath`
  member in `TTestCaseRecord` and `TTestId`, for code that reports or displays tests by suite.
  `GetSuitePath()` isn't pure virtual, so an `ITestGroup` implemented without `TTestGroupBase`
  still compiles unchanged, outside any suite. `SuitePath` is the last member and defaults to
  empty, so existing brace initializers of either struct still compile, without a
  `-Wmissing-field-initializers` warning.
- The VCL GUI runner's test tree shows each suite as a node holding the groups and suites in it,
  with group and suite nodes showing just their own names. A suite node's check box checks or
  unchecks every test in it, including those in the suites inside it, its status image and detail
  pane sum up those tests like a group's, and it's shown only while the filter box leaves some of
  its tests visible.

### Changed

- `TTestCaseRecord::GroupName`, `TTestId::GroupName` and the group name passed to
  `ITestRunObserver::OnTestStarted()` are now the group's qualified name, its suite path then its
  own name (e.g. `Framework.Console.TTest_TConsole`), so groups with the same name in different
  suites can be told apart. For a group that isn't in a suite, that's its own name, as before.
  `GetTestGroupName()` is still just the group's own name. Moving a group into a suite changes its
  tests' full names, as renaming the group would, so `--filter` patterns and saved VCL GUI runner
  selections that named its tests no longer match them.

### Fixed

- `CheckNear`/`AssertNear` passed when either value was a NaN, whatever the tolerance, and so did
  `CheckNotNear`/`AssertNotNear`. A NaN is now near nothing, not even another NaN, so `CheckNear`
  fails and `CheckNotNear` passes. Equal infinities are near each other, as before, and
  `CheckNotNear` now fails for them. A test that passed only because the value it checked was a NaN
  now fails.
- A `*` in a `--filter` pattern, or in the VCL GUI runner's filter box, failed to match where the
  test's full name had a `*` of its own, which a group name or a parameterized test's label can
  have: `--filter "*"` didn't match a group named `*Group`. A `*` in the pattern is now always a
  wildcard.
- `--shuffle-seed`, `--partition-index`, `--partition-count` and `--test-timeout-seconds` accepted
  a number with leading white space or a `+` sign, and on Windows also one with a `-` sign after
  white space, which wrapped around to a large number: `--shuffle-seed " -5"` used seed
  `4294967291`. They now take decimal digits only, and so does the VCL GUI runner's shuffle seed
  box, which still ignores spaces around the number. `TCLIParser::ParseUnsignedInt()` changes the
  same way.
- On Windows, the console runner's `--report-junit` report, and any written with
  `TJUnitReportWriter::Write()`, had CRLF line ends, unlike the VCL GUI runner's and those written
  on Linux. Every report is now `TJUnitReportWriter::BuildXML()`'s text byte for byte, with LF line
  ends.

## [1.3.0] - 2026-10-11

### Added

- Self-registering test methods. A group derived from `TAutoTestGroup<itself>` (instead of
  `TTestGroupBase`) can define each test method with `ASW_TEST(ClassName, TestName) { ... }`, from
  `ASWUnitTests_Registry.h`, which defines `ClassName::Test_TestName()` and registers it as the test
  `TestName`, so a test can no longer be written but left out of the group's constructor. The
  method's declaration in the class must be `public`. A group runs its `ASW_TEST` tests first, in
  alphabetical order by name, then any tests its constructor registers with `RegisterTest()` or
  `RegisterTestCases()`, in the order registered, so the two styles can be mixed. Keep a group's
  `ASW_TEST` tests in the same `.cpp` file as its `ASW_REGISTER_TEST_GROUP`. Existing groups don't
  need to change. `tests/Test_ASWTools_Random.cpp` now uses it.

### Changed

- A group that registers the same test name twice, whether with `RegisterTest()`,
  `RegisterTestCases()` (e.g. a name generator that gives two rows the same label) or `ASW_TEST`,
  now throws the new `TExceptDuplicateTestName` from its constructor, so the run stops at startup
  with `Test group "Name" registers more than one test named "Test"` and exit code `2`. Both tests
  used to be registered and run, but `--filter`, `--list`, the JUnit report and the VCL GUI runner
  couldn't tell them apart. A group that did this must give each test its own name.
- `Deploy.bat` now also copies `.gitattributes`, so a deployed copy committed into another
  repository keeps this repository's line-ending rules for its own files, such as storing the RAD
  Studio `.cbproj` files exactly as they are.

### Fixed

- `ASW_REGISTER_TEST_GROUP` and `ASW_REGISTER_TEST_GROUP_ORDERED` no longer cause a
  `'__COUNTER__' is a C2y extension [-Wc2y-extensions]` warning in every test module when building
  with Clang 22 or later and `-Wpedantic` (an error with `-Werror`). Test modules don't need to
  change.

## [1.2.1] - 2026-10-08

### Changed

- An exception a test throws without expecting it (outside a `Check*` or `Assert*` call, with no
  `SetExceptionExpected()`) now fails only that test, and the run carries on. The failure detail is
  `Unexpected exception: ` followed by the exception's message (`ClassName: Message` for an RTL
  exception). `TearDown_Test()`, the remaining tests and groups, the totals and the `--report-junit`
  report all follow as usual, with the test shown as a `<failure>`, and the VCL GUI runner shows it
  like any other failure, including with `--shuffle`, `--filter`, `--test-timeout-seconds` and
  `--catch-crashes`. Such an exception used to end the whole run, with no totals or JUnit report,
  and exit with code `2` (`3` if it wasn't a `std::exception`). It's now an ordinary test failure,
  with exit code `1`, so a script that checked for `2` or `3` to detect it must check for `1`. Codes
  `2` and `3` now only mean that an exception from `SetUp_Test()`, `TearDown_Test()`,
  `SetUp_Group()` or `TearDown_Group()` ended the run, which is unchanged. Code that calls a group's
  `Run()` directly no longer sees a test's unexpected exception escape; it's in `Results()` instead.

### Fixed

- An exception from `TearDown_Test()` after the test threw the exception it expected was taken for
  the expected exception, so the test passed. It now ends the run with `!!FATAL ERROR!!`, like any
  other `TearDown_Test()` exception, including one after a test's unexpected exception, which
  previously wasn't logged as `!!FATAL ERROR!!`.
- With a MinGW build of the console runner, a `--filter` pattern was replaced by the names of any
  files in the current directory it matched, however it was quoted, because MinGW-w64's C runtime
  can expand wildcards in the command line (WinLibs' builds do by default). Matching one file
  selected no tests and still exited with code `0`; matching several failed with "unrecognized
  option". The runner (`src/main.cpp`) now turns that expansion off, so the pattern arrives as
  typed. A project with its own `main()` can do the same by defining
  `extern "C" { int _dowildcard = 0; }` for MinGW.
- With RAD Studio's 32-bit compiler (`bcc32c`), almost every test was timed as `0.000 ms`, with
  `time="0.000000"` in the `--report-junit` report, in both the console and VCL GUI runners.
  Durations were measured with `std::chrono::high_resolution_clock`, which that compiler's library
  implements as the system clock, advancing only about every 10 ms. Test durations and the total
  elapsed time now use `std::chrono::steady_clock`. It's also monotonic, so with GCC's standard
  library (MinGW, Linux), where `high_resolution_clock` is the system clock too, a change to the
  system clock during a run no longer distorts them. Nothing changes with MSVC or RAD Studio's
  Win64x compiler, where `high_resolution_clock` already was `steady_clock`.

## [1.2.0] - 2026-10-08

### Added

- `CheckNull`/`AssertNull` and `CheckNotNull`/`AssertNotNull`, which compare a value with `nullptr`.
  They accept anything that can be compared with `nullptr`, such as a raw pointer,
  `std::unique_ptr`, `std::shared_ptr` or `std::function`. A C string is checked as a pointer, so an
  empty string such as `""` is not null. With C++20, they also have `std::source_location`
  overloads, like the other `Check*`/`Assert*` methods.
- `CheckSame`/`AssertSame` and `CheckNotSame`/`AssertNotSame`, which check that two values do or
  don't point to the same object. They take raw pointers, arrays, and smart pointers
  (`std::unique_ptr`, `std::shared_ptr`) in any mix, and compare C strings by address, not content.
  A failure shows the addresses.
- `Fail()`, which fails the current test unconditionally, with a message, in place of
  `AssertTrue(false, ...)`. Like an `Assert*` method, it aborts the test, and it fails the test even
  while an exception is expected. It throws the new `TExceptFail`.
- `CheckThrows<TException>`/`AssertThrows<TException>` and `CheckNoThrow`/`AssertNoThrow`, which run
  a callable (e.g. a lambda) and check that it does or doesn't throw. Unlike
  `SetExceptionExpected()`, the test carries on afterwards, so it can check several calls and the
  state after each. The Throws methods take an optional substring the exception's message must
  contain, and match the type like `SetExceptionExpected()`, RTL exception types included. A failed
  `Assert*`, `Fail()` or `Skip()` inside the callable still ends the test as usual.
- `CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` accept two values of the same
  scoped enum (`enum class`) type, which was a compile error, and a failure shows their underlying
  values. Comparing two different enum types, or an `enum class` with an integer, is still a compile
  error.
- `CheckEmpty`/`AssertEmpty` and `CheckNotEmpty`/`AssertNotEmpty`, for anything with an `empty()`
  member (strings, containers) and, with `ASWUNITTESTS_RTL_EXCEPTIONS`, a `System::String`. Unlike
  `CheckTrue(x.empty(), ...)`, a failure shows the text, or a container's element count.
- `CheckMatches`/`AssertMatches` and `CheckNotMatches`/`AssertNotMatches`, which check whether the
  whole of a `std::string` or `std::wstring` matches an ECMAScript regular expression
  (`std::regex_match`). A failure shows the text and the pattern, and an invalid pattern fails the
  check rather than throwing `std::regex_error`.
- `CheckIsType<TType>`/`AssertIsType<TType>` and `CheckIsNotType<TType>`/`AssertIsNotType<TType>`,
  which check with `dynamic_cast` whether an object, given as a raw or smart pointer or by
  reference, is a `TType` (a subclass counting as one). A failure shows the expected type and the
  object's actual type. RTL classes work too.
- `CheckEqualsMem`/`AssertEqualsMem` and `CheckNotEqualsMem`/ `AssertNotEqualsMem`, which compare a
  number of bytes at two addresses. A failure shows the offset of the first differing byte and up to
  16 bytes from there, in hex.
- The VCL GUI runner has an Options menu. **Run in Shuffled Order** turns shuffling on and off, like
  `--shuffle`, and **Shuffle Seed...** chooses between a new random seed for each run and one seed
  for every run, like `--shuffle-seed`. Both start from the command line. The status bar shows the
  seed, and Shuffle Seed... offers the latest run's, so tests can be rerun in the same order to
  track down a failure that depends on the order.

### Changed

- A `--shuffle-seed` now gives the same run order with every compiler and standard library, so a
  seed logged by one build (e.g. MSVC on Windows) reproduces the order with another (e.g. GCC on
  Linux). As a result, a seed from an earlier version gives a different order than it did; to
  reproduce an order from an earlier version, rerun that version.
- With `--shuffle`, a filtered run (`--filter`, or a partition) keeps the order the same seed gives
  the full suite, so rerunning just the tests involved in an order-dependent failure, with the
  logged seed, runs them in the same order. Before, the groups that matched were reshuffled among
  themselves.

### Fixed

- `CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` given two pointers compared
  them as `bool`, so any two non-null pointers were equal: `CheckEquals(&a, &b, ...)` passed. They
  now compare the addresses, and a failure shows both (e.g. `0x7ffd5a2c`, or `(null)`). Two C
  strings are still compared by content. A test that passed only because of this now fails, and
  comparing two pointers to unrelated types (e.g. `int*` and `long*`) is now a compile error.

## [1.1.0] - 2026-10-04

### Added

- With `ASWUNITTESTS_RTL_EXCEPTIONS` defined, the string `Check`/`Assert` methods (`Equals`,
  `Contains`, `StartsWith`, `EndsWith`, and their `Not` and `IC` forms) also take a
  `System::String`, compared with another `System::String`, a `std::string`, a `std::wstring` or a C
  string. Both are compared as UTF-8, and a failure shows them as for `std::string`. Comparing a
  `System::String` with a number remains a compile error.
- `CheckStartsWith`/`AssertStartsWith` and `CheckEndsWith`/`AssertEndsWith`, and their `Not` forms
  (`CheckNotStartsWith`, etc.), for `std::string` and `std::wstring`, taking the text first and then
  the prefix or suffix. Unlike `CheckTrue(text.rfind(prefix, 0) == 0, ...)`, a failure shows both,
  with wide text converted to UTF-8. Case-insensitive `IC` variants (`CheckStartsWithIC`, etc.)
  ignore the case of the ASCII letters `A`-`Z` only, like the `ContainsIC` methods.
- `CheckGreaterThan`, `CheckGreaterThanOrEqual`, `CheckLessThan` and `CheckLessThanOrEqual`, with
  matching `Assert` methods, taking the value first and then the bound (e.g.
  `CheckGreaterThanOrEqual(count, 1, ...)` checks `count >= 1`). Unlike
  `CheckTrue(count >= 1, ...)`, a failure shows both, e.g. `Expected 0 to be >= 1`. They accept any
  two integer or floating-point types except `bool`. Integers are compared by value, so `-1` is less
  than any unsigned value, unlike with the built-in `<`, and a NaN fails every check.
- `CheckEqualsIC`/`AssertEqualsIC` and `CheckNotEqualsIC`/`AssertNotEqualsIC`, which compare two
  `std::string` or `std::wstring` values ignoring the case of the ASCII letters `A`-`Z` only, like
  the `ContainsIC` methods. A failure shows both values, with wide text converted to UTF-8.
- `CheckContains`/`AssertContains` and `CheckNotContains`/`AssertNotContains`, for `std::string` and
  `std::wstring`, taking the text first and then the substring. Unlike
  `CheckTrue(text.find(substring) != std::string::npos, ...)`, a failure shows both, with wide text
  converted to UTF-8. Case-insensitive `IC` variants (`CheckContainsIC`, etc.) ignore the case of
  the ASCII letters `A`-`Z` only, the same on every platform and locale. They use the new
  `src/ASWUnitTests_Utils.cpp`, which a project that lists the framework's source files by hand,
  rather than through `ASWUnitTests_Sources.cmake`, must add.
- VCL GUI runner: the checked tests are saved when the window closes and restored when it next
  opens, in `%APPDATA%\ASWUnitTests\<exe name>.selection`, unless the command line chooses the tests
  itself (`--run`, `--filter`, or partition options). New tests are checked if their group was
  entirely checked, removed tests are skipped, and the log notes what was restored.
- VCL GUI runner: **Select Failed** (Tests menu and toolbar), which checks only the shown tests that
  failed in the latest run, so they can be rerun with Run Selected.
- Version macros in `ASWUnitTests_Version.h`: `ASWUNITTESTS_VERSION_MAJOR`,
  `ASWUNITTESTS_VERSION_MINOR`, `ASWUNITTESTS_VERSION_PATCH`, `ASWUNITTESTS_VERSION_PRERELEASE`
  (empty on a release, e.g. `dev.1` between releases) and `ASWUNITTESTS_VERSION_STRING` (e.g.
  `1.1.0-dev.1`), usable in `#if`, plus a matching `ASWUnitTests::VersionPreRelease` constant. The
  existing `VersionMajor`, `VersionMinor`, `VersionPatch` and `Version` constants now come from the
  macros.
- `std::source_location` overloads of every `Check*`/`Assert*` method, `Skip()`, and
  `SetExceptionExpected()`, taking the caller's location by default instead of a method and line, so
  a call no longer needs `__func__, __LINE__`. Requires C++20, so they're unavailable with RAD
  Studio's 32-bit compilers; `ASWUNITTESTS_SOURCE_LOCATION_ENABLED` is defined when they're
  available. The method and line overloads are unchanged.
- `CheckEquals`/`AssertEquals`/`CheckNotEquals`/`AssertNotEquals` overloads for any two integer
  types other than `bool`, compared by value. A call mixing integer types (e.g. `int` and
  `int64_t`), or using one that matches none of the fixed-width overloads (e.g. `long` on Windows,
  `long long` on Linux), used to be an ambiguous-overload compile error. A negative value never
  equals an unsigned one, unlike with the built-in `==`.

### Changed

- A failed `CheckEquals`/`AssertEquals` of two `std::wstring` or `wchar_t const*` values shows both
  values, converted to UTF-8, like the `std::string` overloads, instead of only the message. A
  failed `CheckNotEquals`/`AssertNotEquals` of two strings, narrow or wide, shows the value they
  share, like the integer overloads. A null C string is shown as `(null)`, including when both are
  null.
- Development builds between releases are versioned with a SemVer pre-release (e.g. `1.1.0-dev.1`),
  which sorts before the release it leads up to.

### Fixed

- A failed `AssertTrue()` reporting "Expected false but was true". It now reports "Expected true but
  was false", by throwing `TExceptTrue` instead of `TExceptFalse`, so code that catches
  `TExceptFalse` to detect a failed `AssertTrue()` must catch `TExceptTrue` instead.
- `CheckEquals`/`AssertEquals`/`CheckNotEquals`/`AssertNotEquals` comparing two C strings (e.g. two
  string literals) as `bool` instead of by content, so an `Equals` check always passed and a
  `NotEquals` check always failed, whatever the text. New `char const*` and `wchar_t const*`
  overloads compare by content. A test that used to pass this way may now correctly fail.
- A failed `CheckEquals` of two `bool` values showing them as `1`/`0`. It now shows `true`/`false`,
  the same as `AssertEquals` and the `NotEquals` methods.
- CI test-results reporting failing for pull requests from forks, whose read-only token can't create
  check runs. Each CI job now shows its JUnit report on the run's summary page instead, which needs
  no write permission, and also uploads it as an artifact.

## [1.0.0] - 2026-09-27

### Added

- CONTRIBUTING.md, documenting the contribution workflow, branch and commit conventions, and bug
  report format.
- This CHANGELOG.md file.
- CI workflow (`.github/workflows/ci.yml`), building and running the self-test suite on Windows
  (MSVC, MinGW) and Linux (GCC, Clang), plus a JUnit-based native test-results summary and a
  partitioning smoke test.
- `--test-timeout-seconds`, aborting the run if a single test doesn't finish in time (e.g. an
  infinite loop), instead of hanging forever. The offending test is recorded as failed and the
  process exits with a dedicated exit code (`5`); if `--report-junit` was also given, the report
  still gets written, covering everything that completed before the timeout.
- `rad370/Build_Win32_Debug.bat` and `Build_Win32_Release.bat`, for building and testing the RAD
  Studio 32-bit target the same way the existing Win64x scripts already covered the 64-bit one.
- `--catch-crashes`, catching a native crash (e.g. an access violation or segmentation fault) in a
  test and recording it as failed instead of letting it take down the whole process. Most crash
  types let the run continue with the next test; a stack overflow on Windows, or any segmentation
  fault on POSIX (which can't be cheaply told apart from a stack overflow there), aborts the run
  afterward instead, with its own dedicated exit code (`6`). Never attempts to catch `SIGABRT`.
  Implemented with `AddVectoredExceptionHandler` on Windows, verified directly (including recovering
  from a genuine stack overflow) across MSVC, MinGW, and RAD Studio's `bcc32c`/`bcc64`, after
  `__try`/`__except` turned out to compile but not actually work on RAD Studio's compilers. On
  POSIX, catching a genuine stack overflow relies on an alternate signal stack
  (`sigaltstack()`/`SA_ONSTACK`), since the default handler would otherwise run on the same,
  already-exhausted stack that just overflowed and have nowhere to run; verified on Linux via
  CLion/SSH.
- Opt-in support for RAD Studio RTL exceptions (`System::Sysutils::Exception` and its subclasses,
  shared by VCL and FMX), enabled by defining `ASWUNITTESTS_RTL_EXCEPTIONS` in a C++Builder project
  built with `bcc32c` or `bcc64x` (any other compiler is a compile error). `SetExceptionExpected<T>`
  accepts RTL exception classes, matched polymorphically, with the optional message substring
  checked against `Message`. Failure details and unhandled RTL exceptions are reported as
  `ClassName: Message`, and an unhandled one exits with code `2` instead of `3` ("Unknown"). Under
  `--test-timeout-seconds`, an unexpected RTL exception crosses back from the worker thread as a
  `TExceptRTLException` (a `std::runtime_error` that keeps the original class in `RTLClass()`),
  since `std::exception_ptr` can't carry an RTL exception past its handler; verified on `bcc64x`,
  where it terminated the process.
- `vcl/console/rad370/`, a VCL console RAD Studio project with its own build scripts, defining
  `ASWUNITTESTS_RTL_EXCEPTIONS` and running the full self-test suite plus the RTL-specific tests in
  `vcl/tests/`.
- `ITestRunObserver`, set with `TTestHandler::SetRunObserver()`, for following a run as it happens
  (e.g. from a GUI runner): it receives the log output that would otherwise go to `std::cout`, a
  start and finish event for each test, and can stop the run between tests. Its events arrive on the
  thread that called `Run()`, even under `--test-timeout-seconds`. Without one, nothing changes.
- `TTestHandler::GetTests()`, listing every registered test as a `TTestId` (its group and test
  names, kept separate since a group name may contain `.`), e.g. for a runner to show the tests
  before running them.
- `ExitCodeForResults()` and `ToJUnitTestCases()` in `ASWUnitTests_CLI`, moved out of `main.cpp` so
  the console and GUI runners share them. A run stopped early through `ITestRunObserver` exits with
  `1`.
- `vcl/gui/rad370/`, a VCL GUI runner (with its own build scripts, sources in `vcl/gui/src/`, and a
  project group opening it with the VCL console project), running the same tests as the console
  runner. It shows every test in a check box tree with a colored status dot, a detail pane for the
  selected test or group, a Failures & Skips list, and the run's colored log. Its toolbar runs the
  checked tests (F9), reruns failures, stops a run after the current test, checks or unchecks every
  shown test, copies the details, and filters the tree with `--filter`-style wildcards; its main
  menu has the same commands, plus File > Export JUnit Report, saving the latest run's results as
  the report `--report-junit` writes, and the command line help and version. It remembers its window
  size, position, and panel sizes between sessions (per executable, in `%APPDATA%\ASWUnitTests`),
  with View > Reset Layout to restore the defaults, and GUI-only `--layout-ignore` and
  `--layout-reset` options to skip or delete the saved layout. It takes the console runner's command
  line options (`--filter` and partitions choose the initially checked tests, and `--filter` also
  fills in the filter box), plus GUI-only `--run`, running the checked tests on startup, and
  `--exit`, closing afterward with the console runner's exit code. Tests run on the main thread, so
  they can create VCL forms and controls.
- `TJUnitReportWriter::BuildXML()`, returning the report's XML as a string for a caller that writes
  the file itself (as the GUI runner does, since `Write()`'s `std::ofstream` can't open a path with
  characters outside the Windows code page from a `std::string`).

### Changed

- `TTestGroupBase::Run()` starts from empty `Results()`, so running the same group again (e.g.
  repeated runs in the VCL GUI runner) reports only that run's outcomes instead of adding them to
  the previous run's.
- MSVC warnings in the framework sources cleaned up, or suppressed where intentional, so a project
  building them at a high warning level stays clean.

### Fixed

- `CheckTrue()` and `CheckFalse()` failure messages had their expectations swapped (a failed
  `CheckTrue()` reported "Expected false but was true", and vice versa).
- A test that failed only through `Check*` calls recorded an empty failure detail, so its
  `--report-junit` entry was `<failure message="">` with no explanation. Its record now carries each
  of its `Check*` failures (followed by any `Assert*` failure after them). Console output is
  unchanged, since those failures are still logged as they happen.
- A `Check*` failure was ignored if the test then threw the exception it had set up with
  `SetExceptionExpected()`, so the test was reported as passed. It now fails, with the `Check*`
  failure in its detail.
- A failed `Assert*` counted as the expected exception while one was expected with
  `SetExceptionExpected()`, so the test was reported as passed. With the templated
  `SetExceptionExpected<T>()`, the requested type wasn't even checked. A failed `Assert*` now always
  fails the test.
- `IsStdoutTTY()` compile error on RAD Studio's 32-bit compiler (`bcc32c`), which declares the
  POSIX-style `isatty()` in `<io.h>` rather than the underscore-prefixed `_isatty()` MSVC, MinGW,
  and RAD Studio's own 64-bit compiler use. Windows64/MSVC/MinGW are unaffected.

## [0.26.5] - 2026-09-24

### Added

- Self-registration for test groups via `ASW_REGISTER_TEST_GROUP` /
  `ASW_REGISTER_TEST_GROUP_ORDERED`, so adding, removing, or renaming a test module never requires
  editing a framework source file.
- Parameterized/data-driven test support (`RegisterTestCases`), registering one independently named,
  filterable test case per row of data.
- `Skip()`, aborting a test and reporting it as skipped (separately from passed/failed, with no
  effect on the process exit code) without removing its registration.
- Tolerance-based floating-point comparison: `CheckNear`/`AssertNear` and
  `CheckNotNear`/`AssertNotNear` for `float`/`double`.
- Exception type and message matching for `SetExceptionExpected<TException>`, matched
  polymorphically against the thrown exception.
- `--filter`/`--filter-ignore-case` and `--list`, for selecting and previewing which registered
  tests will run.
- `--shuffle`/`--shuffle-seed`, to randomize test/group run order as a check against hidden
  inter-test coupling, with a logged seed for reproducing a shuffle-induced failure.
- `--partition-index`/`--partition-count`, splitting the suite across separate process invocations
  (e.g. a CI job matrix) for parallel execution with no in-process threading and no merge step.
- Colorized console output for pass/fail/skip status, with `--color`, `--no-color`, and
  `--color-pass`/`--color-fail`/`--color-skip` options, and automatic detection of terminal support
  and the `NO_COLOR` environment variable.
- `--report-junit`, writing a JUnit-style XML test report with no third-party dependency, recognized
  natively by most CI systems.
- `--pause`, prompting "press enter to continue" before exit, replacing the previous RAD
  Studio/C++Builder-only, debug-only pause behavior.
- Per-test timing in console output, and the test's full name included in its skip/fail message.
- `ASWUnitTests_StdOutRedirect`, a small utility for capturing console output written during a test.
- Unit tests covering the framework's own components (`ASWUnitTests_TestBase`, `ASWUnitTests_CLI`,
  `ASWUnitTests_Console`, `ASWUnitTests_Handler`, `ASWUnitTests_JUnitReport`,
  `ASWUnitTests_StdOutRedirect`), plus example tests for UTF-8 string handling.
- `ASWUnitTests_Sources.cmake`, `Deploy.bat`, `.gitattributes`, and submodule integration
  documentation, to support dropping this framework into another repository as a git submodule.
- ASCII-only source enforcement across framework and example files.

### Changed

- `main()` split into smaller functions, with command-line parsing extracted into its own
  `ASWUnitTests_CLI` unit.
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

Initial versioned release, switching the project to semantic versioning and adding
`ASWUnitTests_Version.h`/`--version`. Everything already present in the framework at this point
(test group registration, `Check`/`Assert` methods, RAD Studio and CMake build support, etc.) is
treated as the baseline and is not itemized commit-by-commit.

[Unreleased]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.2.1...v1.3.0
[1.2.1]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.2.0...v1.2.1
[1.2.0]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.1.0...v1.2.0
[1.1.0]: https://github.com/ASWSoftware/asw-unit-tests/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.5...v1.0.0
[0.26.5]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.3...v0.26.5
[0.26.3]: https://github.com/ASWSoftware/asw-unit-tests/compare/v0.26.1...v0.26.3
[0.26.1]: https://github.com/ASWSoftware/asw-unit-tests/releases/tag/v0.26.1
