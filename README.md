# ASWUnitTests - https://github.com/ASWSoftware/asw-unit-tests

ASWUnitTests is a speedy light-weight C++ unit test tool for Windows and Linux projects.

Requires C++17 or higher; the project itself is built and tested at C++20.

# Features

## Writing Tests

- **[Self-registering test groups](#registering-tests)** - `ASW_REGISTER_TEST_GROUP` adds a test
  module without editing any framework file.
- **[Self-registering test methods](#self-registering-test-methods)** - `ASW_TEST` defines a test
  method and registers it with its group, so a test can't be written but left out of the group's
  constructor.
- **[Test suites](#organizing-groups-into-suites)** - `TSuitePath` puts groups in nested suites, so
  `--filter "Framework.*"` selects a whole suite, and the GUI runners show suites in their tree.
- **Check and Assert methods** - `Check*` records a failure and lets the test continue; `Assert*`
  fails the test immediately. Covers `Equals`/`NotEquals`, `True`/`False`, `Null`/`NotNull`,
  `Same`/`NotSame`, `Empty`/`NotEmpty`, `IsType`/`IsNotType`, `Near`/`NotNear`,
  `Contains`/`NotContains`, `StartsWith`/`EndsWith` (and their `Not` forms), `Matches`/`NotMatches`
  (regular expression), `EqualsMem`/`NotEqualsMem` (bytes), and `GreaterThan`/`LessThan` (and their
  `OrEqual` forms), plus case-insensitive `IC` variants of the string `Equals`, `Contains`,
  `StartsWith` and `EndsWith` methods.
- **[Automatic call site (C++20)](#omitting-the-method-and-line-c20)** - Overloads taking a
  `std::source_location` report the caller's function and line, without passing
  `__func__, __LINE__`.
- **[Floating-point comparison](#comparing-floating-point-values)** - `CheckNear()`/`AssertNear()`
  compare two values within a tolerance: floating point, integers, or a mix.
- **[Expected exceptions](#expecting-a-specific-exception-type-or-message)** -
  `SetExceptionExpected()`, with an optional check of the exception's type and message, and
  [`CheckThrows()`/`CheckNoThrow()`](#checking-that-a-call-throws) for a single call.
- **[Parameterized tests](#parameterized-tests)** - `RegisterTestCases()` registers one named test
  case per row of data.
- **[Failing tests](#failing-a-test)** - `Fail()` fails a test unconditionally, e.g. on a code path
  it should never reach.
- **[Skipping tests](#skipping-a-test)** - `Skip()` reports a test as skipped, unconditionally or
  after a runtime check.
- **Setup and teardown hooks** - `SetUp_Group()`/`TearDown_Group()` run around each group, and
  `SetUp_Test()`/`TearDown_Test()` around each test.

## Running Tests

- **Filtering and listing** - `--filter` selects tests by wildcard pattern; `--list` previews the
  matches without running them.
- **Shuffled order** - `--shuffle` randomizes the run order to expose hidden coupling between tests;
  `--shuffle-seed` reproduces a given order.
- **Parallel partitions** - `--partition-index`/`--partition-count` split the suite across separate
  processes, e.g. a CI job matrix.
- **Hang protection** - `--test-timeout-seconds` aborts the run if a single test doesn't finish in
  time, instead of blocking forever.
- **Crash protection** - `--catch-crashes` records a native crash (e.g. an access violation) as a
  failed test instead of losing the whole process.
- **Unexpected exceptions** - An exception a test throws without expecting it fails only that test,
  with the exception's message, and the run carries on.
- **[GUI runners](#gui-runners)** - A Windows app that runs the same tests, with a checkbox tree to
  choose them, live pass/fail/skip status, and each failure's details and log: the
  [Win32 GUI runner](#win32-gui-runner), built with CMake or RAD Studio, or the
  [VCL GUI runner](#vcl-gui-runner), a RAD Studio VCL app.

## Reporting

- **JUnit XML report** - `--report-junit` writes a report that most CI systems read natively.
- **Colored console output** - Configurable pass/fail/skip colors, with automatic terminal detection
  and `NO_COLOR` support.
- **Per-test timing** - Every test logs its duration, making slow tests easy to spot.
- **[Meaningful exit codes](#command-line-options)** - Distinguish failed tests, unhandled
  exceptions, invalid arguments, timeouts, and crashes.

## Platforms and Integration

- **Portable** - Windows and Linux, with MSVC, MinGW, GCC, Clang, and RAD Studio's Clang-based
  compilers. Requires C++17 or later, with no third-party dependencies.
- **[Drop-in submodule](#submodule-integration)** - Add the repository as a git submodule and build
  its `src` folder alongside your own tests, with `ASWUnitTests_Sources.cmake` for CMake projects.
- **[RAD Studio RTL exceptions](#rad-studio-rtl-exceptions-vclfmx)** - Opt-in support for VCL/FMX
  `Exception` classes such as `EConvertError`, and for
  [`System::String`](#comparing-systemstring-vclfmx) in the string checks, via
  `ASWUNITTESTS_RTL_EXCEPTIONS`.

For the full list of `Check`/`Assert` methods, see `src/ASWUnitTests_TestBase.h`. For working
examples, see the `tests` folder, e.g. `tests/Test_ASWTools_String.cpp` for
`SetExceptionExpected()`.

# Donations:

If you find ASWUnitTests helpful, donations are always appreciated:

PayPal: donate@aswsoftware.com

Bitcoin: 15rKqL1numHJyE36ottMbhs5cmCjJkuowV

# How to Use

Important! Tests should be run outside of the debugger, unless you really want to see every
exception that occurs, due to the nature of unit testing.

For examples of how to use, see the `tests` and `toTest` folders and `src\ASWUnitTests_Handler.cpp`.

ASWUnitTests is intended to exist in a sub folder within your project, typically as a git submodule.
See [Submodule Integration](#submodule-integration) below for how to wire it. The `toTest` folder is
an example of source that is to be tested. While this example folder exists in the root of this
project, your source should be wherever you like.

## CMake

The `cmake` folder contains a portable CMake project for building with CMake, JetBrains CLion,
Visual Studio, Clang, or MinGW. From the repository root, configure and build it with:

```
cmake -S cmake -B build
cmake --build build --config Release
```

The executable is written to `build/bin/Release/ASWUnitTests.exe` with multi-configuration
generators. On Windows it also builds the [Win32 GUI runner](#win32-gui-runner),
`ASWUnitTests_Win32_GUI.exe`, in the same directory. The RAD Studio project writes its final
executable to the same `build/bin/<Config>` directory.

The CMake project does not need a separate version declaration (see [Versions](#versions)) because
it currently builds the test executable directly rather than packaging or installing it.

## Versions

ASWUnitTests follows [Semantic Versioning](https://semver.org), shown by `--version` and the VCL
GUI's caption. Releases are tagged on `main` (e.g. `v1.1.0`). Between releases, the `develop` branch
carries the next planned version with a pre-release, e.g. `1.2.0-dev.1`, which comes before `1.2.0`.
`src/ASWUnitTests_Version.h` has the version as macros, for code that supports several ASWUnitTests
versions, and as constants in the `ASWUnitTests` namespace:

```
#include "ASWUnitTests_Version.h"

#if ASWUNITTESTS_VERSION_MAJOR > 1 || (ASWUNITTESTS_VERSION_MAJOR == 1 && ASWUNITTESTS_VERSION_MINOR >= 1)
    // Uses something added in 1.1 (also present in 1.1.0-dev.N builds)
#endif
```

## Command Line Options

```
ASWUnitTests [options]

  --filter <pattern>   Run only tests whose full name matches <pattern>. A full name is
                       "GroupName.TestName", prefixed by "Suite.Path." for a group in a suite.
                       '*' matches any sequence of characters (including none); '?' matches
                       exactly one character. The whole name must match, e.g. "*String*" for
                       a substring search.
  --filter-ignore-case Match --filter's <pattern> case-insensitively. Has no effect without
                       --filter.
  --shuffle            Run groups, and each group's tests, in a randomized order instead of
                       the default deterministic order. The seed used is logged so a failure
                       caused by order can be reproduced via --shuffle-seed.
  --shuffle-seed <N>   Shuffle (implies --shuffle) using an explicit unsigned integer seed,
                       to reproduce a previous --shuffle run's order.
  --partition-index <N> 1-based index of this run's partition, from 1 to --partition-count.
                       Requires --partition-count.
  --partition-count <N> Splits the full test suite into <N> roughly-equal partitions by each
                       test's position in the canonical registration order (the same order
                       --list shows), so every test runs in exactly one partition regardless
                       of which group it's in. Run <N> separate invocations (e.g. one per CI
                       job), each with its own --partition-index, to run the suite in
                       parallel with no coordination between processes. Requires
                       --partition-index.
  --test-timeout-seconds <N>
                       Abort the run if any single test does not finish within <N> seconds. The
                       offending test is recorded as failed (with a message explaining why) and no
                       further tests or groups run afterward. There is no default; a hung test runs
                       indefinitely unless this is given.
  --catch-crashes      Catch a native crash (e.g. an access violation or segmentation fault) in a
                       test and record it as failed instead of letting it take down the whole
                       process. Most crash types let the run continue with the next test; a few (a
                       stack overflow on Windows, or any segmentation fault on POSIX, which can't
                       be cheaply told apart from a stack overflow there) abort the run afterward
                       instead, the same way --test-timeout-seconds does. Never attempts to catch
                       SIGABRT. Off by default; a crash terminates the process as usual unless this
                       is given.
  --color <mode>       One of "auto" (default; color only on an interactive terminal that
                       supports it, and only if the NO_COLOR environment variable isn't set),
                       "always", or "never".
  --no-color           Shorthand for --color never.
  --color-pass/--color-fail/--color-skip <color>
                       Set the color used for passed/failed/skipped status text. <color> is
                       one of: default, black, red, green, yellow, blue, magenta, cyan, white,
                       or bright-<name> for the bright variant (e.g. bright-red). Defaults:
                       pass=green, fail=red, skip=yellow.
  --report-junit <path> Write a JUnit-style XML test report to <path>, in addition to the
                       normal console output. Recognized by most CI systems (GitHub Actions,
                       GitLab CI, Jenkins, Azure DevOps, CircleCI) for native test result
                       reporting.
  --project-name <name> Set the name this run is identified by: shown in the console's
                       "Initializing..." line and, if --report-junit is also given, used as
                       the report's <testsuites name="..."> attribute (default: "ASWUnitTests").
                       Set this to your own project's name so console output and CI dashboards
                       both identify the run correctly.
  --list               List all registered tests by full name (see --filter) and exit, without
                       running anything. Combine with --filter to preview a pattern's matches
                       before running it.
  --pause              Prompt "press enter to continue" before exiting after a --list command
                       or test run. Useful when an IDE's Run command closes the console
                       immediately, so its output can't be read; set this as an argument in
                       that IDE's own run configuration.
  --version            Print the framework version and exit.
  --help               Show usage and exit.
```

The console output always states whether a filter is active (and its pattern) before running or
listing tests, and `--list` reports how many tests/groups matched out of the total registered, so
when output is redirected to a file, there's a record of why fewer tests ran or were listed than
expected. Likewise, it always states whether shuffle is enabled and, if so, the seed in use.

On Windows, the console runner reads its arguments from the UTF-16 command line rather than the C
runtime's `argv`, which holds them in the ANSI code page, so a `--filter` pattern, a
`--project-name` or a `--report-junit` path can hold any character. While it runs, it also sets the
console's output code page to UTF-8, and restores it afterwards, so non-ASCII test names and
messages show correctly. Output redirected to a file or a pipe is UTF-8 either way.

`--shuffle` is a sanity check against hidden inter-test/inter-group coupling. The deterministic
alphabetical default (see [Registering Tests](#registering-tests)) is for readable, reproducible
output day-to-day, while `--shuffle` deliberately breaks that to surface tests that secretly depend
on running in a particular order (e.g. via shared static/global state). If `--shuffle` causes a
failure, rerun with the logged seed via `--shuffle-seed` to reproduce it exactly while debugging. A
seed gives the same order with every compiler and platform, and adding `--filter` keeps the matching
tests in the order that seed gives the full suite, so you can narrow the rerun down to the tests
involved. A [suite](#organizing-groups-into-suites)'s groups are shuffled among themselves, so they
still run together.

`--partition-index`/`--partition-count` split the suite for parallel execution across separate OS
processes. Each invocation still runs single-threaded and writes to its own console/JUnit output.
Partitioning is by each test's position in the canonical registration order, not by group, so one
large group doesn't dominate a single partition. Partition membership is independent of `--shuffle`,
since it's computed from the canonical order rather than any shuffled one, so a given test's
partition never changes based on whether `--shuffle` is also passed. A typical setup is a CI job
matrix, e.g. 4 jobs each running:
`--partition-index <1..4> --partition-count 4 --report-junit results-<index>.xml`. ASWUnitTests
doesn't merge those reports itself, since most CI systems (GitHub Actions, GitLab CI, Jenkins, Azure
DevOps, CircleCI) already merge multiple JUnit XML files from parallel jobs natively. Combine
`--partition-index`/`--partition-count` with `--list` to preview which tests land in a given
partition, the same way `--list` can preview `--filter`.

`--test-timeout-seconds` guards against a single test hanging (e.g. an infinite loop) forever. There
is no default; without it, a hung test blocks the run indefinitely. When given, each test runs on
its own worker thread while the main thread waits with that timeout; a test that finishes normally
(whether it passes, fails, or throws) is completely unaffected. The worker thread starts with the
main thread's floating-point environment, such as its rounding mode. A test that doesn't finish in
time is recorded as failed, with a message naming it and the timeout that was exceeded, and the run
stops there: no further tests or groups run, and the process exits with a dedicated exit code (`5`)
distinct from an ordinary test failure (`1`). If `--report-junit` was also given, the report still
gets written, covering every test that completed before the timeout, plus the timed-out test's own
synthetic failure entry. The abandoned worker thread itself is never joined or forcibly stopped,
since there is no safe, portable way to interrupt a thread that may be stuck in an infinite loop. If
the test ends after all, moments later, its Check failures, its `Log()` lines and its outcome from
then on never reach the results, the log or a run observer. A test that has already begun recording
its outcome when the timeout comes counts as finished in time instead. Since that thread may still
be using its test group, the runner then ends without destroying the test groups, as it does after a
crash that aborts the run (see `--catch-crashes` below).

`--catch-crashes` guards against a single test crashing (e.g. dereferencing a null pointer) and
taking down the whole process with it. There is no default; without it, a crashing test still
crashes the process as usual. On Windows (MSVC, MinGW, and RAD Studio's Clang-based 32/64-bit
compilers alike) this is implemented with `AddVectoredExceptionHandler` rather than
`__try`/`__except`: the latter is the textbook approach and unwinds C++ objects properly when it
works, but it silently fails to catch anything at runtime under RAD Studio's compilers even though
it compiles, and GCC/MinGW does not implement the keywords at all, so this framework only relies on
the mechanism it directly verified actually works, across all four Windows compiler targets,
including recovering from a genuine stack overflow. On POSIX (Linux/Mac) it's a signal handler for
`SIGSEGV`/`SIGFPE`/`SIGILL`/`SIGBUS` that jumps back to a point just before the test started;
`SIGABRT` is never caught, since it usually means the C runtime itself already detected the
process's state is corrupt and is deliberately terminating rather than letting it continue.

A crash that's caught still gets recorded as a failed test, with a message describing the fault,
exactly like any other failure; if `--report-junit` was also given, it shows up in the report the
same way. Most crash types let the run continue with the next test afterward, since the whole point
of catching it (unlike a timeout, which can only abandon-and-abort) is that the harness can safely
keep going. A few specific crash types abort the run afterward instead, with a dedicated exit code
(`6`) distinct from an ordinary test failure (`1`), which is what a continued-past crash still shows
up as: a Windows stack overflow (`EXCEPTION_STACK_OVERFLOW` is unambiguous), or *any* `SIGSEGV` on
POSIX. That second one is a deliberate platform difference, not an oversight: unlike Windows, POSIX
delivers a stack overflow and an ordinary segfault as the exact same signal, and reliably telling
them apart needs inspecting the faulting address against the thread's stack bounds - real extra
complexity for what's already treated as an edge case on both platforms. The practical effect is
that the same ordinary null-pointer dereference continues the run on Windows but aborts it on
Linux/Mac. Catching a genuine stack overflow at all on POSIX (as opposed to distinguishing it from
an ordinary segfault) does rely on `sigaltstack()`/`SA_ONSTACK`: the default signal handler would
otherwise run on the same, already-exhausted stack that just overflowed, leaving it nowhere to run
and crashing the process for real instead.

Recovering from a crash at all is a raw jump back to before the test started (the same fundamental
technique `--test-timeout-seconds`' abandoned worker thread relies on, for a different reason): it
does not run destructors for any objects that were under construction on the crashing test's stack
at the moment of the fault. This is a real, accepted limitation of recovering from a hardware-level
fault, on any platform, not an oversight either.

Pass/fail/skip status text is colorized when writing to an interactive terminal that supports ANSI
escape codes. Output redirected to a file or pipe, or a non-interactive CI log, automatically gets
plain text with no escape codes, unless `--color=always` forces it (e.g. for a CI system that
supports ANSI in its own log viewer). The [NO_COLOR](https://no-color.org) environment variable is
also respected in the default `auto` mode.

Every test logs a `Finished test: "GroupName.TestName" - passed/failed/skipped (N.NNN ms)` line on
completion, timing from just before `SetUp_Test` to just after the test's outcome is determined.
This is useful for spotting slow tests without needing an external profiler.

`--report-junit` produces a standard `<testsuites>`/`<testsuite>`/`<testcase>` report. One
`<testsuite>` per test group, with `<failure>`/`<skipped>` elements carrying the same detail message
shown on the console. A group in a [suite](#organizing-groups-into-suites) is named by its qualified
name (e.g. `Framework.Console.TTest_TConsole`), in its `<testsuite>`'s `name` and each
`<testcase>`'s `classname`; suites don't get `<testsuite>` elements of their own. `--project-name`
sets both the console's `Initializing...` line and the report's `<testsuites name="...">` attribute.

Exit codes: `0` all run tests passed or were skipped (or `--version`/`--list`/`--help` completed),
`1` one or more tests failed, `2` a `std::exception` (or, with
[RTL exception support](#rad-studio-rtl-exceptions-vclfmx) enabled, an RTL `Exception`) from
`SetUp_Test()`, `TearDown_Test()` or `TearDown_Group()` ended the run, `3` an
exception of any other type from one of those ended the run, `4` invalid command line arguments, `5`
a test exceeded `--test-timeout-seconds` and the run was aborted, `6` a test crashed severely enough
(with `--catch-crashes` given) that the run was aborted. An exception a test throws without
expecting it (outside a `Check*`/`Assert*` call, with no `SetExceptionExpected()`) only fails that
test, with `Unexpected exception: ` and the exception's message as its detail, and the run carries
on, so it's an ordinary test failure (exit code `1`), not `2` or `3`. Likewise, a crash caught by
`--catch-crashes` that didn't force an abort is an ordinary test failure (exit code `1`), not `6`.
An exception from a group's `SetUp_Group()` doesn't end the run either: each of the group's tests
that would have run fails, with `SetUp_Group() exception: ` and the exception's message as its
detail, `TearDown_Group()` still runs, and the run carries on with the next group (exit code `1`).
Skipped tests never affect the exit code.

## GUI Runners

Two GUI runners run the same tests as the console runner, in a window for choosing tests and
reading their results: the [VCL GUI runner](#vcl-gui-runner), a RAD Studio VCL application, and the
[Win32 GUI runner](#win32-gui-runner), which uses only what ships with Windows and builds with CMake
(MSVC, MinGW, Clang) or RAD Studio. They look and work the same, so this section describes both, and
their own sections cover building them and running your own tests in them. Each of this
repository's builds of them runs its full self-test suite.

The window shows:

- **Test tree** - Every registered test under its group, and each group under its
  [suite](#organizing-groups-into-suites), if any, with a check box to choose what runs (all are
  checked at first) and a colored dot for its status: gray not run, blue running, green passed, red
  failed, amber skipped. A group's or suite's check box and dot summarize its tests, including those
  in the suites inside it. **View > Expand All** and **View > Collapse All** expand every suite and
  group, or collapse the tree to its top-level ones; changing the filter expands it all again.
- **Detail pane** - The selected test's result, duration, failure or skip detail, and its own log
  output, or how many of a selected group's or suite's tests have each status.
- **Failures & Skips** - Every failed or skipped test in the latest run. Double-click one to select
  it in the tree.
- **Log** - The run's full output, with failures in red and skips in amber.
- **Progress and status bars** - Progress (red once anything fails), counts, elapsed time, and the
  run's status.

The toolbar, whose commands are also in the Run and Tests menus:

- **Run Selected** (F9) runs the checked tests, and **Run Failed** reruns the latest run's failures.
- **Stop** ends the run after the current test. Closing the window during a run does the same, then
  closes.
- **Select All** and **Select None** check or uncheck every shown test, and **Select Failed** checks
  only the shown tests that failed in the latest run.
- **Copy Details** (Ctrl+Shift+C) copies the detail pane to the clipboard.
- **Filter** (Ctrl+F) shows only the tests whose `Group.Test` name contains its text, with the same
  `*` and `?` wildcards as `--filter`. It ignores the case of the ASCII letters, as
  [`CheckEqualsIC`](#comparing-strings-ignoring-case) does, so other letters, such as an accented
  capital and small E, still have to match exactly. While it hides tests, checking them and Run
  Selected only apply to the shown ones, and hidden tests keep their check marks.

**File > Export JUnit Report** saves the latest run's results as the same JUnit XML report
`--report-junit` writes, for when `--report-junit` wasn't given, or to keep a copy. The Help menu
shows the command line options (the same text as `--help`) and the framework version.

**Options > Run in Shuffled Order** runs the groups, and each group's tests, in a random order, like
`--shuffle`. **Options > Shuffle Seed...** chooses between a new random seed for each run and one
seed for every run, like `--shuffle-seed`, and clicking OK also turns shuffling on. The status bar
shows the seed: the one every run uses, or the latest run's. Shuffle Seed... offers that seed, so
choosing it reruns tests in the same order, Run Failed included, to track down a failure that
depends on the order. Both settings start from the command line and aren't saved.

When the window closes, it saves its size, position, maximized state, and panel sizes to
`%APPDATA%\ASWUnitTests\<exe name>.ini`, and restores them the next time it opens. A position on a
monitor that's no longer connected is ignored. An `--exit` run doesn't save the layout. **View >
Reset Layout** restores the default layout right away; `--layout-ignore` and `--layout-reset`
(below) control this from the command line. Since the file is named after the executable, the two
GUI runners, and each project's own build of them, keep separate layouts.

In both GUI runners, **View > Theme** chooses a light or dark look. **Use Windows Setting**, the
default, follows Windows' app mode setting (Settings > Personalization > Colors), and switches as
soon as it changes, but stays light while high contrast is on, so the window shows high contrast's
own colors. **Light** and **Dark** keep that look whatever Windows is set to. Light is the standard
Windows look. Dark is RAD Studio's Carbon VCL style in the VCL GUI runner, and the Win32 GUI runner
paints the same colors itself. A choice applies right away and is saved in the layout file at once,
so the next session starts with it; **View > Reset Layout** leaves it as it is. The Theme commands
are disabled during a run, and a change of the Windows setting during one applies when it ends. In
the Win32 GUI runner's dark look, the title bar is dark unless Windows shows the accent color on
title bars, which it then keeps while the window is active, and the Export JUnit Report dialog,
Windows' own, stays light.

**View > Theme > Dark Theme** chooses the dark look's colors: a built-in theme or a theme file of
your own, used whenever the look is dark, with Dark or with Use Windows Setting (high contrast keeps
its own colors). Both runners have five of RAD Studio's dark VCL styles built in: Carbon, Charcoal
Dark Slate, Cobalt XEMedia, Onyx Blue and Tablet Dark, which the Win32 GUI runner paints in their
colors. The Win32 GUI runner also has four tints of Carbon: Ember (warm brown), Forest (green),
Midnight (blue) and Plum (purple), which keep Carbon's colors for how tests did. Theme files go in
`%APPDATA%\ASWUnitTests\Themes`, which **Open Themes Folder** opens, and are shared by every test
app; the menu lists each one by its file name, below the built-in themes, and leaves out a file
named like one of them. The choice is saved in the layout file with the theme, and a problem with a
theme file, such as a misspelled role, a color that isn't one or a missing file, is reported in the
Log tab, with Carbon's colors used in its place.

The Win32 GUI runner's theme files are INI files, `<Name>.ini`, and **Save Theme Template...** saves
Carbon's colors as one to start from, every role listed under a comment on what its group colors:

```ini
[Theme]
Base=Dark

[Colors]
Window=#0F2238
Panel=#1B3350
```

`Base=Dark` is required. Each line under `[Colors]` sets a role's color as `#RRGGBB`, and a role
left out keeps Carbon's color. Choosing the theme again reads its file again, so a change shows at
once. The VCL GUI runner's theme files are VCL style files, `<Name>.vsf`, made with RAD Studio's
Bitmap Style Designer. Each needs a style name of its own, not that of a style the project already
has, such as Carbon. A style file is loaded once per session, so a change to it shows after a
restart.

The window also remembers which tests are checked, in `%APPDATA%\ASWUnitTests\<exe name>.selection`,
as long as the command line doesn't choose them itself (no `--run`, `--filter`, or partition
options). What it saves is what Run Selected would run, the checked tests that are shown; the filter
box itself starts empty. Since tests come and go between sessions, it adapts:

- A test that's been removed is skipped.
- A new test (or a renamed one, which looks the same) is checked if its group was entirely checked,
  and otherwise isn't. A test in a new group is checked only if every test was.
- If none of the checked tests still exist, every test is checked instead.

The log notes what was restored. So **Select Failed**, closing the window, fixing the code, and
reopening it leaves just those tests checked for Run Selected.

Tests run on the GUI's main thread, the one that runs its window, so a test can create windows (or,
in the VCL GUI runner, forms and controls). `--test-timeout-seconds` runs each test on a worker
thread instead, so it can't be combined with tests like that; the log says so when it's given.

A run that a test's timeout or crash abandoned leaves **Run Selected** and **Run Failed** disabled
until the GUI is restarted, since the timed-out test may still be running, and a crash may have left
the tests' state corrupted. Closing the window then ends the process without destroying the test
groups, as the console runner does.

The GUI takes the same [command line options](#command-line-options) as the console runner, with
these differences:

- `--filter` and `--partition-index`/`--partition-count` choose which tests start out checked,
  instead of the saved selection, and `--filter`'s pattern also fills in the filter box. The box
  shows every test the pattern matches (and maybe a few more, left unchecked, since the box ignores
  case and matches anywhere in the name).
- `--project-name` is also shown in the window's caption.
- `--shuffle` and `--shuffle-seed` set the Options menu's shuffle settings, which can then be
  changed.
- `--report-junit` writes the report after every run, including Run Failed.
- `--list`, `--pause`, and the color options are ignored, with a note in the log.
- `--help`, `--version`, and argument errors are shown in a dialog, and the GUI then exits without
  opening.
- `--run` (GUI-only) runs the checked tests as soon as the window opens.
- `--exit` (GUI-only, requires `--run`) closes the window when that run finishes, and exits with the
  same code the console runner would; a run ended with Stop exits with `1`. Without `--exit`, the
  GUI always exits with `0`.
- `--layout-ignore` (GUI-only) neither loads nor saves the window layout, or the theme, so the
  window opens with the defaults and the saved ones are left as they are. A theme chosen in the View
  menu then lasts only until the window closes.
- `--layout-reset` (GUI-only) deletes the saved window layout, and with it the theme, and opens with
  the defaults, which is a way to recover if a saved layout ever causes a problem. The layout is
  saved again when the window closes.

A GUI process doesn't make a Windows command prompt wait for it, so a script that needs `--exit`'s
exit code should start it with `start /wait` (or its own language's equivalent; Bash, including Git
Bash, waits for it anyway):

```
start /wait ASWUnitTests_VCL_GUI.exe --filter "*String*" --run --exit --report-junit results.xml
echo %ERRORLEVEL%
```

The GUI runners are built on `ITestRunObserver` (in `src\ASWUnitTests_TestBase.h`), set with
`TTestHandler::SetRunObserver()`. An observer receives the log output that would otherwise go to
`std::cout`, as plain text without the console's color codes, is told as each test starts and
finishes, and can stop a run between tests, so it can drive other kinds of runners too.

### VCL GUI Runner

`vcl\gui\rad370\ASWUnitTests_VCL_GUI.cbproj` builds the VCL GUI runner, for Win32 and Win64x, and
its `Build_*.bat` scripts write the executable to `vcl\gui\rad370\<Platform>\<Config>`.
`vcl\gui\rad370\ASWUnitTests_VCL_Group.groupproj` opens it together with the VCL console project.
It defines `ASWUNITTESTS_RTL_EXCEPTIONS`, so it has
[RTL exception support](#rad-studio-rtl-exceptions-vclfmx), and its tests can use the VCL.

To run your own tests in it, create a RAD Studio VCL application that defines
`ASWUNITTESTS_RTL_EXCEPTIONS`, and add to it: the framework's `src` files except `main.cpp`; the
files in `gui\src` (shared by the GUI runners) and `vcl\gui\src`, including
`ASWUnitTests_GUI_MainForm.dfm`, with both folders on the include path; your own test modules; and
a `WinMain()` modeled on `vcl\gui\rad370\ASWUnitTests_VCL_GUI.cpp`, which parses the command line,
applies the saved theme and starts the main form. For the dark theme, also add RAD Studio's Carbon
style to the application (Project > Options > Application > Appearance, which this repository's
project has as `Custom_Styles`); without it, Dark shows the light look. The other built-in dark
themes (Charcoal Dark Slate, Cobalt XEMedia, Onyx Blue and Tablet Dark) are listed only if their
styles are added too. For the ASWUnitTests icon,
set the application's icon to `resource\ASWUnitTests.ico` there too, as this repository's project
does.

### Win32 GUI Runner

The Win32 GUI runner uses only the Windows API, with the common controls and dialogs that ship with
Windows, and no VCL or third-party code, so it builds with the [CMake](#cmake) toolchains on Windows
(MSVC, MinGW, Clang) as well as with RAD Studio. It needs Windows 10 version 1703 or later. Win32 is
the name of the API: it builds as a 32-bit or a 64-bit program. Since it doesn't link the VCL, it
has no [RTL exception support](#rad-studio-rtl-exceptions-vclfmx); tests of VCL code belong in the
[VCL GUI runner](#vcl-gui-runner).

On Windows, this repository's CMake project builds it as the `ASWUnitTests_Win32_GUI` target, next
to the console runner (e.g. `build/bin/Release/ASWUnitTests_Win32_GUI.exe`).
`win32\gui\rad370\ASWUnitTests_Win32_GUI.cbproj` builds it with RAD Studio, for Win32 and Win64x,
and its `Build_*.bat` scripts write the executable to `win32\gui\rad370\<Platform>\<Config>`.

To run your own tests in it with CMake, include `win32\gui\src\ASWUnitTests_Win32GUI_Sources.cmake`
after `project()`. Its `ASWUNITTESTS_WIN32_GUI_SOURCES` lists the framework's `src` files except
`main.cpp`, the files in `gui\src` (shared by the GUI runners) and `win32\gui\src`, including the
runner's own `WinMain()`, its manifest, and `resource\ASWUnitTests_Icon.rc`, which gives the
executable the ASWUnitTests icon. It also exports the include path (which has the `resource` folder,
for the icon), libraries and linker options they need:

```cmake
include(third_party/asw-unit-tests/win32/gui/src/ASWUnitTests_Win32GUI_Sources.cmake)

add_executable(MyTestsGUI WIN32
    ${ASWUNITTESTS_WIN32_GUI_SOURCES}
    tests/Test_MyClass.cpp
)

target_include_directories(MyTestsGUI PRIVATE
    ${ASWUNITTESTS_WIN32_GUI_INCLUDE_DIRS}
    tests
)

target_link_libraries(MyTestsGUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_LIBRARIES})
target_link_options(MyTestsGUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS})
```

Link `Threads::Threads` too, as for the console runner (see [CMake](#cmake-1) under Submodule
Integration). With MinGW, also link its runtime statically
(`-static-libgcc -static-libstdc++ -static`, as this repository's `cmake\CMakeLists.txt` does) or
put its DLLs next to the executable, or the GUI may load another program's older `libstdc++-6.dll`
from `PATH` and fail to start.

For an icon of your own, remove the ASWUnitTests one from the list before `add_executable()`, and
add a resource script of your own whose icon is named `MAINICON`, which the window then shows:

```cmake
list(REMOVE_ITEM ASWUNITTESTS_WIN32_GUI_SOURCES "${ASWUNITTESTS_RESOURCE_DIR}/ASWUnitTests_Icon.rc")
```

With RAD Studio, the simplest start is a copy of `win32\gui\rad370\ASWUnitTests_Win32_GUI.cbproj`,
with its paths and test files changed. It's a C++ application with no framework
(`FrameworkType None`), with `TCHAR` mapped to `char`, since the runner's entry point is
`WinMain()`. It compiles the framework's `src` files except `main.cpp`, the files in `gui\src` and
`win32\gui\src`, with all three folders on the include path, and the test modules. Its manifest is
`win32\gui\src\ASWUnitTests_Win32GUI.manifest`, with DPI awareness set to None, so RAD Studio uses
the manifest as it is (the manifest declares per-monitor DPI awareness itself). Its icon is
`resource\ASWUnitTests.ico`, which RAD Studio embeds as `MAINICON`.

## Registering Tests

Test modules self-register with `TTestHandler` using the `ASW_REGISTER_TEST_GROUP` macro (declared
in `src\ASWUnitTests_Registry.h`). No file in `src` ever needs to be modified to add, remove, or
rename a test module. This makes it easy to drop ASWUnitTests into another repository (e.g. as a git
submodule). Place the macro at file scope, after the closing brace of the `ASWUnitTests` namespace,
in the test module's `.cpp` file:

```
// Whatever includes at the top of the file for your `TMyClassToTest` class, etc.
#include "ASWUnitTests_Registry.h"

namespace ASWUnitTests
{
    // ... TTest_TMyClassToTest class implementation ...

} // namespace ASWUnitTests

ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_TMyClassToTest)
```

Only the test module's own `.cpp`/`.h` files need to be added to your project's build (CMake, RAD
Studio, etc.). See `tests\Test_ASWTools_String.cpp` and `tests\Test_ASWTools_Random.cpp` for working
examples.

By default, groups run in alphabetical order by group name, deterministically across compilers and
linkers. To override that for a specific group, instead use
`ASW_REGISTER_TEST_GROUP_ORDERED(ClassName, order)`. Groups run in ascending order, with ties broken
alphabetically:

```
ASW_REGISTER_TEST_GROUP_ORDERED(ASWUnitTests::TTest_TMyClassToTest, -1) // runs before the alphabetical block
```

Run order is purely for readable, reproducible output. A group's `SetUp_Group`/`TearDown_Group`
should still make no assumption about which other groups have or haven't already run.

A group whose constructor throws doesn't stop the run. It's logged at startup as `!!ERROR!!`, and a
group named after its class as the registration macro gives it (e.g.
`ASWUnitTests::TTest_TMyClassToTest`), in the suite the registration names, stands in for it, with
one test, `(construction)`, which fails with `Test group class "..." could not be created: ` and the
exception's message. The rest of the run goes on as usual, and it ends with exit code `1`. The
exceptions to this are a duplicate test name and an invalid suite path, which still stop the run at
startup.

### Organizing Groups into Suites

To organize many groups, put each in a suite by passing a `TSuitePath` to its base class
constructor, ahead of the group's own name. A suite path names nested suites, outermost first,
separated by `.`:

```
TTest_TConsole::TTest_TConsole()
    : inherited(TSuitePath("Framework.Console"), "TTest_TConsole")
{
}
```

That puts group `TTest_TConsole` in suite `Console`, inside suite `Framework`. `TAutoTestGroup`
(see [Self-Registering Test Methods](#self-registering-test-methods)) has the same constructor. The
group's qualified name, `Framework.Console.TTest_TConsole`, starts all of its tests' full names, so:

- `--filter "Framework.*"` runs every test in suite `Framework`, including those in `Console`, and
  `--filter "Framework.Console.*"` runs just those in `Console`.
- `--list`, the console log and the `--report-junit` report show its tests as
  `Framework.Console.TTest_TConsole.TestName`, and the [GUI runners](#gui-runners)' test tree shows
  each suite as a node, whose check box checks or unchecks all of its tests.

The registration can name the suite instead, leaving the constructor as it is:

```
ASW_REGISTER_TEST_GROUP_IN_SUITE(TTest_TConsole, "Framework.Console")
```

`ASW_REGISTER_TEST_GROUP_IN_SUITE_ORDERED(ClassName, suitePath, order)` takes an order too. A group
whose constructor names a different suite stops the run at startup, and so does an `ITestGroup`
implemented without `TTestGroupBase` unless its own `GetSuitePath()` gives the registration's suite.

A suite's groups always run together. Within a suite, or at the top level, groups and suites run in
alphabetical order by their own names. `ASW_REGISTER_TEST_GROUP_ORDERED`'s order only places a group
among the groups and suites beside it, where a suite counts as order `0`, so it never moves a group
out of its suite.

Suites only organize groups: they have no `SetUp_Group`/`TearDown_Group` of their own. A suite name
can't be empty, so a path can't start or end with `.` or contain `..`; constructing a group with
such a path, or registering one in it, throws `TExceptInvalidSuitePath`, which stops the run at
startup. Moving a group into a suite changes its tests' full names, as renaming the group would, so
update any `--filter` pattern that names them.

### Expecting a Specific Exception Type or Message

`SetExceptionExpected(true, ...)` passes on any thrown exception, regardless of its type. To also
verify the exception's type (matched polymorphically, so a base class also matches its subclasses)
and, optionally, that its `what()` contains a given substring, use the templated overload instead:

```
SetExceptionExpected<std::invalid_argument>(__func__, __LINE__, "StrToInt32 invalid", "signed 32-bit int");
int32_t i = TStrTool::StrToInt32(invalid);
```

If the wrong exception type is thrown, or its message doesn't contain the given substring, the test
fails with a message showing what was actually caught. The type can be any type that can be thrown:
a `std::exception` class, RAD Studio's RTL `Exception` class or a subclass when
[RTL exception support](#rad-studio-rtl-exceptions-vclfmx) is enabled, a class of your own, or even
`int` or `char const*` (the type a string literal is thrown as):

```
SetExceptionExpected<int>(__func__, __LINE__, "the legacy parser throws its error code");
LegacyParse("x");
```

Only a `std::exception` (its `what()`) and an RTL exception (its `Message`) have a message, so
expecting a substring from any other type fails, naming the type that was caught (e.g.
`caught an exception without a message: an object of type "int"`). A class that doesn't derive from
`std::exception` also matches a `std::exception` subclass derived from it, whose `what()` is then
checked.

An expected exception never hides a failure elsewhere in the test. A failed `Assert*` still fails
the test rather than counting as the expected exception, and so does a failed `Check*` earlier in
the test, even if the expected exception then arrives.

### Checking That a Call Throws

`SetExceptionExpected()` covers a test that ends by throwing. To check a single call instead, so the
test carries on afterwards, pass it as a lambda to `CheckThrows`/`AssertThrows`, with the expected
exception type and, optionally, a substring its message must contain. `CheckNoThrow`/`AssertNoThrow`
check that a call doesn't throw, and a failure shows what it threw:

```
CheckThrows<std::out_of_range>([&] { list.At(5); }, __func__, __LINE__, "index past the end");
CheckThrows<std::invalid_argument>([&] { Parse("x"); }, __func__, __LINE__, "not a number", "invalid digit");
CheckEquals(3u, list.Count(), __func__, __LINE__, "a failed call leaves the list unchanged");
CheckNoThrow([&] { list.At(2); }, __func__, __LINE__, "last index");
```

The type is matched like `SetExceptionExpected<TException>()`'s: polymorphically, of any type (e.g.
`CheckThrows<int>`), and with [RTL exception support](#rad-studio-rtl-exceptions-vclfmx), RTL
exception types too. A failed `Assert*`, `Fail()` or `Skip()` inside the lambda isn't taken as the
thrown exception; it ends the test as usual, unless the requested type is that framework exception
(e.g. `CheckThrows<TExceptTrue>`).

### RAD Studio RTL Exceptions (VCL/FMX)

RAD Studio's RTL exceptions (`System::Sysutils::Exception` and its subclasses, such as
`EConvertError`) are shared by VCL and FMX, and don't derive from `std::exception`. By default, only
the generic `SetExceptionExpected(true, ...)` matches one, and one a test throws unexpectedly fails
it as `Unexpected exception: a non-std::exception object`.

A C++Builder project that links the RTL can opt in by defining `ASWUNITTESTS_RTL_EXCEPTIONS` in its
project options (Building > C++ Shared Options > Conditional defines). This requires RAD Studio's
Clang-based compilers (`bcc32c`/`bcc64x`). Defining it for any other compiler is a compile error, so
a misconfigured build fails loudly instead of silently dropping the feature. Nothing changes for
builds that don't define it.

With it enabled, the templated `SetExceptionExpected<TException>()` also accepts RTL exception
classes, matched polymorphically the same way (so `Exception` matches any RTL exception). The
optional message substring is checked against the exception's `Message`, converted to UTF-8:

```
SetExceptionExpected<EConvertError>(__func__, __LINE__, "StrToInt invalid", "not a valid integer");
StrToInt(L"abc");
```

A requested RTL type never matches a `std::exception`, and a requested `std::exception` type never
matches an RTL exception. Failure details, including a test's unexpected exception, and exceptions
that end the run show RTL exceptions as `ClassName: Message` (e.g.
`EConvertError: 'abc' is not a valid integer value`), and one that ends the run (e.g. from
`SetUp_Test()`) exits with code `2`, like a `std::exception`.

`--test-timeout-seconds` runs each test on a worker thread. Expected and unexpected exceptions from
the test itself work exactly the same there, because they're caught and checked on that thread. An
RTL exception from `SetUp_Test()` or `TearDown_Test()`, which ends the run, has to cross back to the
main thread, which `std::exception_ptr` can't do for RTL exceptions (rethrowing one after its
handler has exited terminates the process). It therefore arrives as a `TExceptRTLException`, a
`std::runtime_error` whose `what()` is the same `ClassName: Message` text, so the console output and
exit code are unchanged. Code that calls a group's `Run()` directly with a timeout can still check
the original type through `RTLClass()`:

```
catch (TExceptRTLException const& ex)
{
    if (ex.RTLClass()->InheritsFrom(__classid(EConvertError)))
        ...
}
```

`vcl\console\rad370\ASWUnitTests_VCL_Console.cbproj` is a working reference setup: a VCL console
project that defines `ASWUNITTESTS_RTL_EXCEPTIONS` and runs this repository's full self-test suite
plus the RTL-specific and GUI unit tests in `vcl\tests`. It has its own `Build_*.bat` scripts, and
writes its executable to `vcl\console\rad370\<Platform>\<Config>`. The
[VCL GUI runner](#vcl-gui-runner) is set up the same way.

### Comparing System::String (VCL/FMX)

The same `ASWUNITTESTS_RTL_EXCEPTIONS` define (see [above](#rad-studio-rtl-exceptions-vclfmx)) also
lets the string `Check`/`Assert` methods take a `System::String` (`UnicodeString`), an `AnsiString`,
a `UTF8String` or another `AnsiStringT`: `Equals`, `Contains`, `StartsWith`, `EndsWith`, `Matches`,
and their `Not` and `IC` forms. The other text may be any of those, or a `std::string`,
`std::wstring` or C string, including a literal:

```
CheckEquals("Ready", Label1->Caption, __func__, __LINE__, "status after loading");
CheckStartsWithIC(Edit1->Text, L"https://", __func__, __LINE__, "a secure URL");
```

Both texts are converted to UTF-8 and compared as two `std::string` values, so a failure shows them
the same way, as for [other strings of different types](#comparing-strings-of-different-types). An
`AnsiString` is converted from its code page, and narrow text is read as UTF-8, as elsewhere in the
framework. `CheckMatches` matches with `std::wregex`, so `.` matches one UTF-16 code unit. A null C
string isn't empty text: it follows the [C string rules](#comparing-c-strings), so it doesn't equal
an empty `System::String`. A `System::String` converts implicitly from a number or a character, but
these methods don't accept either, so `CheckEquals(Edit1->Text, 5, ...)` is a compile error rather
than a comparison with `"5"`. The message is still a `std::string`.

### Omitting the Method and Line (C++20)

Every `Check*`/`Assert*` method, `Skip()`, and both forms of `SetExceptionExpected()` also have an
overload without the method and line arguments. It takes an optional `std::source_location` as its
last argument instead, which defaults to the caller's location:

```
CheckEquals(5, total, "int and int64_t");
Skip("Windows-only feature");
SetExceptionExpected<std::invalid_argument>("StrToInt32 invalid", "signed 32-bit int");
```

Failure messages then show `std::source_location::function_name()` as the method. Unlike `__func__`,
its text is up to the compiler, and is typically the full signature, e.g.
`void TTest_TMyClassToTest::Test_Something()` on GCC and Clang, or
`void __cdecl TTest_TMyClassToTest::Test_Something(void)` on MSVC. The method and line overloads
remain, for a bare or custom name.

A helper that makes its own checks can take a location and pass it on, so its failures report the
helper's caller rather than the helper itself:

```
// In the class declaration
void CheckIsEven(int value, std::source_location loc = std::source_location::current());

// In the .cpp
void TTest_TMyClassToTest::CheckIsEven(int value, std::source_location loc)
{
    CheckTrue(value % 2 == 0, std::to_string(value) + " should be even", loc);
}
```

These overloads need C++20's `std::source_location`, so they're only declared when
`ASWUnitTests_TestBase.h` finds it supported, which it signals by defining
`ASWUNITTESTS_SOURCE_LOCATION_ENABLED`. RAD Studio's 32-bit compilers only support C++17, so they
don't have these overloads. Tests that also need to build there should keep the method and line
form, or check `ASWUNITTESTS_SOURCE_LOCATION_ENABLED`.

### Comparing C Strings

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` compare two C strings
(`char const*` or `wchar_t const*`, including string literals and character arrays) by content, the
same as `std::string` and `std::wstring`. A null pointer only matches another null pointer, never a
string, not even an empty one:

```
char const buffer[] = "abc";
CheckEquals("abc", buffer, __func__, __LINE__, "same text in a different buffer passes");
```

A failed `CheckEquals` shows both strings, and a failed `CheckNotEquals` the value they share, for C
strings, `std::string` and `std::wstring` alike. Wide text is converted to UTF-8, and a null pointer
is shown as `(null)`.

The [case-insensitive](#comparing-strings-ignoring-case), [substring](#checking-for-a-substring),
[prefix and suffix](#checking-the-start-or-end-of-a-string) and
[regular expression](#matching-a-regular-expression) checks take two C strings too. In
`CheckEqualsIC` and `CheckNotEqualsIC`, a null pointer only matches another null pointer, as in
`CheckEquals`. In `CheckContains`, `CheckStartsWith`, `CheckEndsWith` and `CheckMatches`, and their
`IC` forms, a null pointer, as either the text or the part, matches nothing, so they fail and their
`Not` forms pass:

```
char const* const name = FindName(); // null if not found
CheckStartsWith(name, "ASW", __func__, __LINE__, "fails, rather than crashing, when not found");
```

The same rules apply when a C string is compared with
[a string of another type](#comparing-strings-of-different-types).

### Comparing Strings of Different Types

The string checks (`Equals`, `Contains`, `StartsWith`, `EndsWith` and `Matches`, with their `Not`
and `IC` forms) also take two texts of different types: a `std::string` with a C string, narrow text
(`std::string`, `char const*`) with wide text (`std::wstring`, `wchar_t const*`), and, in
[RTL builds](#comparing-systemstring-vclfmx), the RTL string types. Narrow text is read as UTF-8, as
elsewhere in the framework. Two narrow texts, or two wide ones, are compared as they are. Any other
pair is converted to UTF-8 and compared as two `std::string` values, so a failure shows both texts
the same way:

```
std::wstring const title = window.Title();
CheckEquals("Settings", title, __func__, __LINE__, "the window's title");
```

`CheckMatches` and `CheckNotMatches` match two narrow texts with `std::regex`, where `.` matches one
byte of UTF-8, and any other pair with `std::wregex`, where `.` matches one wide character. On
Windows, that's one UTF-16 code unit, so a character beyond U+FFFF is two.

A null C string follows the [same rules](#comparing-c-strings) whatever it's compared with: it
equals only another null C string, narrow or wide, and matches nothing in the other checks.

### Comparing Pointers

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` compare any other two pointers
by address, and a failure shows both addresses:

```
CheckEquals(expectedWidget, list.Find("button"), __func__, __LINE__, "finds the widget that was added");
// Check failed for: "Test_Find" (42): Expected "0x1f2a40" but was "(null)". finds the widget that was added
```

Comparing two pointers to unrelated types (e.g. `int*` and `long*`) is a compile error.

`CheckSame`/`AssertSame` and `CheckNotSame`/`AssertNotSame` check that two values do or don't point
to the same object. They take raw pointers, arrays, and smart pointers (`std::unique_ptr`,
`std::shared_ptr`), in any mix, and compare C strings by address too, unlike `CheckEquals`:

```
CheckSame(buffer, Trim(buffer), __func__, __LINE__, "trims in place, returning the same buffer");
CheckSame(cache.Get("a"), cache.Get("a"), __func__, __LINE__, "both calls return the one shared_ptr object");
```

### Checking an Object's Type

`CheckIsType<TType>`/`AssertIsType<TType>` check that an object is a `TType`, and
`CheckIsNotType`/`AssertIsNotType` that it isn't. They take a raw pointer, a smart pointer
(`std::unique_ptr`, `std::shared_ptr`) or an object, of a polymorphic class (one with a virtual
function), including RTL classes. They use `dynamic_cast`, so a subclass of `TType` counts as one,
and a null pointer is never one. A failure shows the expected type and the object's actual type:

```
std::unique_ptr<TShape> const shape = factory.Create("circle");
CheckIsType<TCircle>(shape, __func__, __LINE__, "the factory makes a circle");
// Check failed for: "Test_Create" (42): Expected type "TCircle" but was "TSquare". the factory makes a circle
```

Type names come from `std::type_info::name()`, demangled with GCC, Clang and RAD Studio's
compilers, so their exact form (e.g. a namespace or `class ` prefix) depends on the compiler. With
[RTL exception support](#rad-studio-rtl-exceptions-vclfmx) enabled, a `TObject` descendant is named
by its `ClassName()` instead (e.g. `TStringList`), the same with every compiler.

### Comparing Strings, Ignoring Case

`CheckEqualsIC`/`AssertEqualsIC` and `CheckNotEqualsIC`/`AssertNotEqualsIC` compare two
`std::string` or `std::wstring` values like `CheckEquals`/`CheckNotEquals`, but ignoring case. A
failure shows both values, with wide text converted to UTF-8:

```
CheckEqualsIC("Content-Type", headerName, __func__, __LINE__, "header names ignore case");
```

Only the ASCII letters `A`-`Z` and `a`-`z` are matched regardless of case, so the result is the same
on every platform and in every locale, and the bytes of a multi-byte UTF-8 character are never
changed. Other letters, such as an accented capital and small E, still have to match exactly. The
[substring](#checking-for-a-substring) and
[prefix and suffix](#checking-the-start-or-end-of-a-string) checks ignore case the same way.

### Checking for a Substring

`CheckContains`/`AssertContains` pass when a `std::string` or `std::wstring` contains a given
substring, and `CheckNotContains`/`AssertNotContains` pass when it doesn't. The text comes first,
then the substring. Unlike `CheckTrue(text.find(substring) != std::string::npos, ...)`, a failure
shows both:

```
std::string const log = "Connected to server";
CheckContains(log, "timeout", __func__, __LINE__, "logs the timeout");
// Check failed for: "Test_Connect" (42): Expected "Connected to server" to contain "timeout". logs the timeout
```

The comparison is case-sensitive, and every string contains the empty string. A failure shows
`std::wstring` text converted to UTF-8.

`CheckContainsIC`/`AssertContainsIC` and `CheckNotContainsIC`/`AssertNotContainsIC` do the same,
ignoring the case of ASCII letters only, the same way as
[`CheckEqualsIC`](#comparing-strings-ignoring-case):

```
CheckContainsIC(log, "CONNECTED", __func__, __LINE__, "passes");
```

### Checking the Start or End of a String

`CheckStartsWith`/`AssertStartsWith` pass when a `std::string` or `std::wstring` starts with a given
prefix, and `CheckEndsWith`/`AssertEndsWith` when it ends with a given suffix.
`CheckNotStartsWith`/`AssertNotStartsWith` and `CheckNotEndsWith`/`AssertNotEndsWith` pass when it
doesn't. The text comes first, then the prefix or suffix, read as "text starts with prefix". A
failure shows both:

```
std::string const path = "logs/server.txt";
CheckEndsWith(path, ".log", __func__, __LINE__, "writes a log file");
// Check failed for: "Test_LogPath" (42): Expected "logs/server.txt" to end with ".log". writes a log file
```

As with the [substring checks](#checking-for-a-substring), the comparison is case-sensitive, every
string starts and ends with the empty string, and a failure shows `std::wstring` text converted to
UTF-8. The `IC` variants (`CheckStartsWithIC`, `CheckEndsWithIC`, and their `Assert` and `Not`
forms) ignore the case of ASCII letters only, the same way as
[`CheckEqualsIC`](#comparing-strings-ignoring-case):

```
CheckStartsWithIC(header, "content-type:", __func__, __LINE__, "passes for \"Content-Type: text/plain\"");
```

### Matching a Regular Expression

`CheckMatches`/`AssertMatches` pass when the *whole* of a `std::string` or `std::wstring` matches an
ECMAScript regular expression (as `std::regex_match` does), and `CheckNotMatches`/`AssertNotMatches`
when it doesn't. To match part of the text, start or end the pattern with `.*`. A failure shows the
text and the pattern:

```
CheckMatches(stamp, R"(\d{4}-\d{2}-\d{2})", __func__, __LINE__, "an ISO 8601 date");
// Check failed for: "Test_Stamp" (42): Expected "2026-1-7" to match "\d{4}-\d{2}-\d{2}". an ISO 8601 date
```

Wide text is matched with `std::wregex`, so `.` matches one wide character, and a failure shows it
as UTF-8. An invalid pattern fails the check, showing `std::regex_error`'s description, rather than
throwing. The text and the pattern may also be [C strings](#comparing-c-strings), or
[of different types](#comparing-strings-of-different-types).

### Comparing Integers of Different Types

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` accept any two integer types,
not just a matching pair of fixed-width ones, so there's no need for a suffix or cast like `0LL` to
pick an overload. This includes `long` and `unsigned long` (e.g. `DWORD`) on Windows, and
`long long` on Linux, which match none of the fixed-width types there. The values are compared as
numbers, so `-1` never equals an unsigned value, unlike with the built-in `==`:

```
int64_t total = 5;
CheckEquals(5, total, __func__, __LINE__, "int and int64_t");
CheckEquals(-1, 4294967295u, __func__, __LINE__, "fails, where -1 == 4294967295u is true");
```

`bool` is the exception: comparing a `bool` with an integer remains a compile error, since it's
usually a mistake.

### Comparing Enums

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` also compare two values of the
same scoped enum (`enum class`) type. Unlike `CheckTrue(mode == TColorMode::Always, ...)`, a failure
shows both values, as their underlying numbers:

```
CheckEquals(TColorMode::Always, options.ColorMode, __func__, __LINE__, "--color always");
// Check failed for: "Test_ColorOption" (42): Expected "1" but was "0". --color always
```

Comparing two different enum types, or an `enum class` with an integer, is a compile error. An
unscoped `enum` converts to an integer, so it's compared like one, as before.

### Comparing Memory

`CheckEqualsMem`/`AssertEqualsMem` compare a number of bytes at two addresses, and
`CheckNotEqualsMem`/ `AssertNotEqualsMem` check that they differ. A failure shows the offset of the
first differing byte, and up to 16 bytes from there in hex:

```
CheckEqualsMem(expected, packet.Data(), sizeof(expected), __func__, __LINE__, "encoded header");
// Check failed for: "Test_Encode" (42): Bytes differ at offset 2 of 4: expected "03 04" but was "FF 04". encoded header
```

Zero bytes, or the same pointer twice, are always equal, without reading either. Exactly one null
pointer is never equal to the other memory, and isn't read either.

### Comparing Floating-Point Values

There are no `float`/`double` overloads of `CheckEquals`/`AssertEquals`. Exact equality comparison
of floating-point values is unreliable (e.g. `0.1f + 0.2f != 0.3f`). Use `CheckNear`/`AssertNear`
instead, which pass when the absolute difference between the two values is within a given tolerance,
and `CheckNotNear`/`AssertNotNear` for the opposite (asserting two values are *not* within tolerance
of each other):

```
float sum = 0.1f + 0.2f;
CheckNear(0.3f, sum, 0.0001f, __func__, __LINE__, "sum should be close to 0.3");
```

Pick a tolerance appropriate to the computation being tested; there's no built-in default, since a
sensible tolerance depends heavily on the magnitude and accumulated error of the values involved.

A NaN is near nothing, not even another NaN, so `CheckNear` fails and `CheckNotNear` passes when
either value is a NaN, whatever the tolerance. Equal infinities are near each other.

A negative or NaN tolerance is a mistake in the test, so both `CheckNear` and `CheckNotNear` fail
with it, showing the tolerance (e.g. `Invalid tolerance -0.100000: it must be zero or more`).

The three values can be of any integer or floating-point types except `bool`, in any mix, such as
`CheckNear(2.0, aFloat, 0.01, ...)` or `CheckNear(10, count, 1, ...)`. When any of them is floating
point, all three are compared as their common type, as the built-in operators do, so an all-`float`
call keeps `float` arithmetic. Three integers are compared exactly, whatever their types, so
`-1` and `1u` are 2 apart, and a failure shows them as integers:

```
CheckNear(10, count, 1, __func__, __LINE__, "about ten");
// Check failed for: "Test_Count" (42): Expected "10 (tolerance 1)" but was "12 (diff 2)". about ten
```

### Comparing Greater Than and Less Than

`CheckGreaterThan`, `CheckGreaterThanOrEqual`, `CheckLessThan` and `CheckLessThanOrEqual`, and their
`Assert` versions, compare a value with a bound. The value comes first, then the bound, read as
"value >= bound". That's the other way round from `CheckEquals`, whose expected value comes first.
Unlike `CheckTrue(count >= 1, ...)`, a failure shows both:

```
CheckGreaterThanOrEqual(count, 1, __func__, __LINE__, "at least one item");
// Check failed for: "Test_Items" (42): Expected 0 to be >= 1. at least one item
```

They accept any two integer or floating-point types except `bool`. Two integers are compared by
value, like [`CheckEquals`](#comparing-integers-of-different-types) does, so `-1` is less than any
unsigned value. When either value is floating point, both are compared as their common type, as the
built-in operators do, and a NaN fails every check.

### Checking for Empty

`CheckEmpty`/`AssertEmpty` and `CheckNotEmpty`/`AssertNotEmpty` take anything with an `empty()`
member, such as a `std::string`, `std::wstring`, `std::vector` or `std::map`, a C string
(`char const*` or `wchar_t const*`, including character arrays), and with
[RTL support](#comparing-systemstring-vclfmx), a `System::String`, `AnsiString` or `UTF8String`.
Unlike `CheckTrue(list.empty(), ...)`, a failure shows what was there: the text, or the element
count of a container with a `size()`:

```
CheckEmpty(errors, __func__, __LINE__, "no errors reported");
// Check failed for: "Test_Load" (42): Expected empty but had 2 elements. no errors reported
CheckEmpty(GetWarning(), __func__, __LINE__, "no warning");
// Check failed for: "Test_Load" (43): Expected empty but was "low disk space". no warning
```

A character array is checked as the text it holds, whatever its size, so `char name[32] = ""` is
empty. A null C string is neither empty nor not empty: both checks fail, showing it as `(null)`:

```
char const* name = nullptr;
CheckNotEmpty(name, __func__, __LINE__, "has a name");
// Check failed for: "Test_Load" (44): Expected not empty but was "(null)". has a name
```

### Checking for Null

`CheckNull`/`AssertNull` and `CheckNotNull`/`AssertNotNull` compare a value with `nullptr`. They
accept anything that can be compared with `nullptr`, such as a raw pointer, `std::unique_ptr`,
`std::shared_ptr` or `std::function`:

```
std::unique_ptr<TWidget> widget = factory.Create("button");
AssertNotNull(widget, __func__, __LINE__, "factory should create a button");
CheckNull(factory.Find("missing"), __func__, __LINE__, "no widget has that name");
```

A C string is checked as a pointer, so an empty string such as `""` is not null.

### Failing a Test

Call `Fail(method, line, msg)` to fail the current test unconditionally, e.g. on a code path the
test should never reach. Like an `Assert*` method, it aborts the test, and it fails it even while an
exception is expected:

```
switch (shape.Kind())
{
    case TShapeKind::Circle: CheckNear(3.14159, shape.Area(), 0.0001, __func__, __LINE__, "unit circle"); break;
    case TShapeKind::Square: CheckNear(1.0, shape.Area(), 0.0001, __func__, __LINE__, "unit square"); break;
    default: Fail(__func__, __LINE__, "unexpected shape kind");
}
```

### Skipping a Test

Call `Skip(method, line, reason)` from within a test to abort it and have it reported as skipped. It
is separately counted from passed/failed, and does not affect the process exit code. No explicit
`return` is needed afterward, since `Skip()` throws to unwind the rest of the test body, the same
way `Assert*` methods do:

```
void TTest_TMyClassToTest::Test_WindowsOnlyFeature()
{
#if !defined(_WIN32)
    Skip(__func__, __LINE__, "Windows-only feature");
#endif
    // ... test body ...
}
```

This keeps a known-broken, environment-specific, or not-yet-implemented test's registration (and its
place in `--list` output) intact, instead of commenting out or deleting its `RegisterTest()` call
and losing the reminder that it exists. The reason is required, so future readers always know *why*
a test is being skipped.

Test modules themselves inherit from `TTestGroupBase`, and each test method is registered within
that module's constructor, as below, or registers itself where it's defined (see
[Self-Registering Test Methods](#self-registering-test-methods)). For example:

```
// Whatever include at the top of the file for your `TMyClassToTest` class, etc.

//---------------------------------------------------------------------------
TTest_TMyClassToTest::TTest_TMyClassToTest()
    : inherited("Name_of_My_Module_Unit_For_TMyClassToTest")
{
    // Can register test methods this way
    RegisterTest([this]()
        {
            Test_NameOfMethodBeingTested_WhatIsBeingTested();
        }, "NameOfMethodBeingTested_WhatIsBeingTested");

    // Or you can register test methods this way
    RegisterTest(&TTest_TMyClassToTest::Test_NameOfMethodBeingTested_WhatIsBeingTested,
        "NameOfMethodBeingTested_WhatIsBeingTested");
    RegisterTest(&TTest_TMyClassToTest::Test_NameOfMethod2BeingTested_WhatIsBeingTested,
        "NameOfMethod2BeingTested_WhatIsBeingTested");
}
//---------------------------------------------------------------------------
TTest_TMyClassToTest::~TTest_TMyClassToTest()
{
}
//---------------------------------------------------------------------------
void TTest_TMyClassToTest::SetUp_Group()
{
    // Any setup code to execute before running the group
}
//---------------------------------------------------------------------------
void TTest_TMyClassToTest::SetUp_Test(ITestCase& /*testCase*/)
{
//    Log("Setting up test: " + testCase.GetName());
    // Any setup code to execute before running a test
}
//---------------------------------------------------------------------------
void TTest_TMyClassToTest::TearDown_Group()
{
    // Any clean up code to run after the group test is done
}
//---------------------------------------------------------------------------
void TTest_TMyClassToTest::TearDown_Test(ITestCase& /*testCase*/)
{
//    Log("Tearing down test: " + testCase.GetName());
    // Any clean up code to run after a test
}
//---------------------------------------------------------------------------
```

A test name can be registered only once per group. Registering one again (with `RegisterTest()`,
`RegisterTestCases()` or `ASW_TEST`) throws `TExceptDuplicateTestName` from the group's constructor,
which stops the run at startup with a message naming the group and the test, since `--filter`,
`--list`, the JUnit report and the GUI runners couldn't tell the two tests apart.

### Self-Registering Test Methods

A test registered in the constructor is easy to forget: the method compiles and the group runs, just
without it. `ASW_TEST` (declared in `src\ASWUnitTests_Registry.h`) instead defines the test method
and registers it with its group in one step, so writing the test is registering it. To use it,
derive the group from `TAutoTestGroup<itself>` instead of `TTestGroupBase`, and declare its test
methods `public`:

```
// Test_TMyClassToTest.h
class TTest_TMyClassToTest : public TAutoTestGroup<TTest_TMyClassToTest>
{
private:
    typedef TAutoTestGroup<TTest_TMyClassToTest> inherited;

public: // Test methods, each defined with ASW_TEST
    void Test_NameOfMethodBeingTested_WhatIsBeingTested();
    void Test_NameOfMethod2BeingTested_WhatIsBeingTested();

public:
    TTest_TMyClassToTest();
    // ... SetUp_Group(), TearDown_Group(), etc., as before ...
};

// Test_TMyClassToTest.cpp
TTest_TMyClassToTest::TTest_TMyClassToTest()
    : inherited("Name_of_My_Module_Unit_For_TMyClassToTest")
{
    // No RegisterTest() calls needed for the ASW_TEST tests below.
}

ASW_TEST(TTest_TMyClassToTest, NameOfMethodBeingTested_WhatIsBeingTested)
{
    // ... test body, exactly as in a Test_ method; __func__ is still the method's name ...
}

ASW_TEST(TTest_TMyClassToTest, NameOfMethod2BeingTested_WhatIsBeingTested)
{
    // ... test body ...
}
```

`ASW_TEST(ClassName, TestName)` defines `ClassName::Test_TestName()` and registers it as the test
`TestName`. The method still needs its declaration in the class, since C++ can't add a member from
outside the class definition, and that declaration must be `public`, since `ASW_TEST` takes the
method's address from outside the class. Getting either wrong is a compile error, never a test that
silently doesn't run: a private method fails to compile, and so does `ASW_TEST` on a class that
doesn't derive from `TAutoTestGroup<ClassName>`.

The order is deterministic: a group runs its `ASW_TEST` tests first, in alphabetical order by name
(whatever order they're defined in), then any tests its constructor registers with `RegisterTest()`
or `RegisterTestCases()`, in the order it registers them, so the two styles can be mixed in one
group. `--shuffle`, `--filter`, `--list`, the JUnit report and the GUI runners treat `ASW_TEST`
tests exactly like any other. Parameterized tests are still registered in the constructor with
`RegisterTestCases()`, next to their data.

Keep a group's `ASW_TEST` tests in the same `.cpp` file as its `ASW_REGISTER_TEST_GROUP`. A test
registered in the constructor is linked in because the constructor refers to it, but nothing refers
to an `ASW_TEST` test, so a `.cpp` file holding only `ASW_TEST` tests could be left out by a linker
pulling the test code from a static library, just as a group's own file could. Compiling the test
files directly into the test executable, as the projects here do, avoids both.

A group that overrides `RegisterTest(TTestCase const&)` doesn't see its `ASW_TEST` tests pass
through that override, since they're registered from the `TAutoTestGroup` constructor, before the
derived class is constructed.

See `tests\Test_ASWTools_Random.cpp` for a working example.

### Parameterized Tests

`RegisterTestCases()` registers one test case per element of a `std::vector<TParam>`, calling the
given method with each element in turn, instead of hand-writing a loop inside a single test or
duplicating near-identical test methods for each input. Each row becomes an independent,
individually named, individually filterable test case, so `--filter`, `--shuffle`, colorized output,
and the JUnit report all work on it exactly like any other test:

```
struct THexCase
{
    char Input;
    int Expected;
};

// In the constructor, in place of a plain RegisterTest() call:
std::vector<THexCase> const hexCases =
{
    { 'A', 10 },
    { 'f', 15 },
    { '0', 0 },
};
RegisterTestCases(&TTest_TMyClassToTest::Test_HexSingleToByte, "HexSingleToByte", hexCases);

// Test_HexSingleToByte(THexCase const& testCase) then runs once per row, generating
// test names "HexSingleToByte[0]", "HexSingleToByte[1]", "HexSingleToByte[2]".
```

Pass an optional `std::function<std::string (TParam const&)>` name generator as a fourth argument to
label each row with something more meaningful than its index (e.g. `"HexSingleToByte[A]"`), which is
especially useful for reading `--filter` matches or a failing row's name in CI output.

## Submodule Integration

This repository's own `cmake\CMakeLists.txt`, `rad370\ASWUnitTests.cbproj`, and the RAD Studio
projects under `vcl` and `win32` only build *this* repo's own example tests and `toTest` code for
its own development and CI. Don't use them from a consuming project, and don't modify them. Doing
either means your changes live inside the submodule and get lost or conflict the next time you
update it. Instead, add ASWUnitTests as a git submodule (e.g. into `third_party\asw-unit-tests`) and
reference its `src` files from your own project's build file, alongside your own test modules:

```
my-project/
+-- third_party/asw-unit-tests/   <- git submodule, never modified, freely updated
|   +-- src/                      <- framework core (never touched)
|   +-- gui/src/                  <- shared by the GUI runners, if you use one (never touched)
|   +-- vcl/gui/src/              <- VCL GUI runner, if you use it (never touched)
|   +-- win32/gui/src/            <- Win32 GUI runner, if you use it (never touched)
|   +-- resource/                 <- the ASWUnitTests icon, for a runner's executable (never touched)
|   `-- cmake/, rad370/, tests/, toTest/, other vcl/ and win32/ folders   <- this framework's own example build, unused by you
+-- tests/                        <- your own test modules (Test_MyClass.cpp/.h), self-registered
`-- CMakeLists.txt / .cbproj      <- your own build file, in your own repo
```

Since `src\main.cpp` only calls into `TTestHandler` and never references a specific test class, you
compile it as-is from the submodule. There's no need to copy or duplicate it into your own tree.

### CMake

Include `src\ASWUnitTests_Sources.cmake` from your own `CMakeLists.txt` rather than listing the
framework's source filenames by hand; it exports `ASWUNITTESTS_SOURCES` (the compiled `.cpp` files)
and `ASWUNITTESTS_SOURCE_DIR` (for the include path). It stays in sync automatically if a future
version of this framework adds a core file:

```cmake
include(third_party/asw-unit-tests/src/ASWUnitTests_Sources.cmake)

add_executable(MyTests
    ${ASWUNITTESTS_SOURCES}
    tests/Test_MyClass.cpp
)

target_include_directories(MyTests PRIVATE
    ${ASWUNITTESTS_SOURCE_DIR}
    tests
)
```

`--test-timeout-seconds` uses `std::thread`/`std::future`, which needs an explicit link against a
threading library on Linux and some MinGW-w64 distributions (a no-op on MSVC and toolchains that
need no extra linker flag). Add it the same way this repository's own `cmake\CMakeLists.txt` does:

```cmake
find_package(Threads REQUIRED)
target_link_libraries(MyTests PRIVATE Threads::Threads)
```

For a GUI target, `win32\gui\src\ASWUnitTests_Win32GUI_Sources.cmake` does the same for the
[Win32 GUI runner](#win32-gui-runner); see that section.

### RAD Studio / Visual Studio / other IDE projects

Add the same files listed in `ASWUNITTESTS_SOURCES` from the submodule's `src` folder to your own
project, plus your own test modules. Check `src\ASWUnitTests_Sources.cmake` for added files after
updating the submodule.

On Windows, `main.cpp` defines `_tmain` (`main` elsewhere), so a project that maps `TCHAR` to
`wchar_t`, such as a RAD Studio project with "TCHAR maps to" set to `wchar_t`, links as well as one
that maps it to `char`. On Windows, the framework also uses `CommandLineToArgvW()`, from `shell32`,
which MSVC, MinGW and RAD Studio link by default.

For a C++Builder project that links the VCL or FMX, also define `ASWUNITTESTS_RTL_EXCEPTIONS` to
enable [RTL exception support](#rad-studio-rtl-exceptions-vclfmx) and
[`System::String` comparisons](#comparing-systemstring-vclfmx). To run your tests in the
[VCL GUI runner](#vcl-gui-runner) or the [Win32 GUI runner](#win32-gui-runner) instead of a console,
see its section for the files it needs.

# Coding Standards

To use uncrustify (for coding standards (pretty formatting) for this repo):

1. Get `uncrustify` and ensure that the path to `uncrustify.exe` is added to the Windows `Path`
   environment variable. https://github.com/uncrustify/uncrustify

2. Run the following command locally to set the git hooks directory (uncrustify will run upon
   commit):
```
git config --local core.hooksPath .githooks/
```
