# ASWUnitTests - https://github.com/ASWSoftware/asw-unit-tests

ASWUnitTests is a speedy light-weight C++ unit test tool for Windows and Linux projects.

Requires C++17 or higher; the project itself is built and tested at C++20.

# Features

## Writing Tests

- **[Self-registering test groups](#registering-tests)** - `ASW_REGISTER_TEST_GROUP` adds a test module without
  editing any framework file.
- **Check and Assert methods** - `Check*` records a failure and lets the test continue; `Assert*` fails the test
  immediately. Covers `Equals`/`NotEquals`, `True`/`False`, `Near`/`NotNear`, `Contains`/`NotContains`,
  `StartsWith`/`EndsWith` (and their `Not` forms), and `GreaterThan`/`LessThan` (and their `OrEqual` forms), plus
  case-insensitive `IC` variants of the string `Equals`, `Contains`, `StartsWith` and `EndsWith` methods.
- **[Automatic call site (C++20)](#omitting-the-method-and-line-c20)** - Overloads taking a `std::source_location`
  report the caller's function and line, without passing `__func__, __LINE__`.
- **[Floating-point comparison](#comparing-floating-point-values)** - `CheckNear()`/`AssertNear()` compare `float`
  and `double` values within a tolerance.
- **[Expected exceptions](#expecting-a-specific-exception-type-or-message)** - `SetExceptionExpected()`, with an
  optional check of the exception's type and message.
- **[Parameterized tests](#parameterized-tests)** - `RegisterTestCases()` registers one named test case per row of
  data.
- **[Skipping tests](#skipping-a-test)** - `Skip()` reports a test as skipped, unconditionally or after a runtime
  check.
- **Setup and teardown hooks** - `SetUp_Group()`/`TearDown_Group()` run around each group, and
  `SetUp_Test()`/`TearDown_Test()` around each test.

## Running Tests

- **Filtering and listing** - `--filter` selects tests by wildcard pattern; `--list` previews the matches without
  running them.
- **Shuffled order** - `--shuffle` randomizes the run order to expose hidden coupling between tests;
  `--shuffle-seed` reproduces a given order.
- **Parallel partitions** - `--partition-index`/`--partition-count` split the suite across separate processes,
  e.g. a CI job matrix.
- **Hang protection** - `--test-timeout-seconds` aborts the run if a single test doesn't finish in time, instead of
  blocking forever.
- **Crash protection** - `--catch-crashes` records a native crash (e.g. an access violation) as a failed test
  instead of losing the whole process.
- **[VCL GUI runner](#vcl-gui-runner)** - A RAD Studio VCL app that runs the same tests, with a checkbox tree to
  choose them, live pass/fail/skip status, and each failure's details and log.

## Reporting

- **JUnit XML report** - `--report-junit` writes a report that most CI systems read natively.
- **Colored console output** - Configurable pass/fail/skip colors, with automatic terminal detection and `NO_COLOR`
  support.
- **Per-test timing** - Every test logs its duration, making slow tests easy to spot.
- **[Meaningful exit codes](#command-line-options)** - Distinguish failed tests, unhandled exceptions, invalid
  arguments, timeouts, and crashes.

## Platforms and Integration

- **Portable** - Windows and Linux, with MSVC, MinGW, GCC, Clang, and RAD Studio's Clang-based compilers. Requires
  C++17 or later, with no third-party dependencies.
- **[Drop-in submodule](#submodule-integration)** - Add the repository as a git submodule and build its `src`
  folder alongside your own tests, with `ASWUnitTests_Sources.cmake` for CMake projects.
- **[RAD Studio RTL exceptions](#rad-studio-rtl-exceptions-vclfmx)** - Opt-in support for VCL/FMX `Exception`
  classes such as `EConvertError`, and for [`System::String`](#comparing-systemstring-vclfmx) in the string
  checks, via `ASWUNITTESTS_RTL_EXCEPTIONS`.

For the full list of `Check`/`Assert` methods, see `src/ASWUnitTests_TestBase.h`. For working examples, see the
`tests` folder, e.g. `tests/Test_ASWTools_String.cpp` for `SetExceptionExpected()`.

# Donations:

If you find ASWUnitTests helpful, donations are always appreciated:

PayPal:
donate@aswsoftware.com

Bitcoin:
15rKqL1numHJyE36ottMbhs5cmCjJkuowV

# How to Use

Important! Tests should be run outside of the debugger, unless you really want to see every exception that occurs, due
to the nature of unit testing.

For examples of how to use, see the `tests` and `toTest` folders and `src\ASWUnitTests_Handler.cpp`.

ASWUnitTests is intended to exist in a sub folder within your project, typically as a git submodule.
See [Submodule Integration](#submodule-integration) below for how to wire it.
The `toTest` folder is an example of source that is to be tested. While this example folder exists in the root of this
project, your source should be wherever you like.

## CMake

The `cmake` folder contains a portable CMake project for building with CMake, JetBrains CLion, Visual Studio, Clang,
or MinGW. From the repository root, configure and build it with:

```
cmake -S cmake -B build
cmake --build build --config Release
```

The executable is written to `build/bin/Release/ASWUnitTests.exe` with multi-configuration generators. The RAD Studio
project writes its final executable to the same `build/bin/<Config>` directory.

The CMake project does not need a separate version declaration (see [Versions](#versions)) because it currently builds
the test executable directly rather than packaging or installing it.

## Versions

ASWUnitTests follows [Semantic Versioning](https://semver.org), shown by `--version` and the VCL GUI's caption.
Releases are tagged on `main` (e.g. `v1.0.0`). Between releases, the `develop` branch carries the next planned version
with a pre-release, e.g. `1.1.0-dev.1`, which comes before `1.1.0`. `src/ASWUnitTests_Version.h` has the version as
macros, for code that supports several ASWUnitTests versions, and as constants in the `ASWUnitTests` namespace:

```
#include "ASWUnitTests_Version.h"

#if ASWUNITTESTS_VERSION_MAJOR > 1 || (ASWUNITTESTS_VERSION_MAJOR == 1 && ASWUNITTESTS_VERSION_MINOR >= 1)
    // Uses something added in 1.1 (also present in 1.1.0-dev.N builds)
#endif
```

## Command Line Options

```
ASWUnitTests [options]

  --filter <pattern>   Run only tests whose "GroupName.TestName" full name matches <pattern>.
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
  --list               List all registered tests as "GroupName.TestName" and exit, without
                       running anything. Combine with --filter to preview a pattern's matches
                       before running it.
  --pause              Prompt "press enter to continue" before exiting after a --list command
                       or test run. Useful when an IDE's Run command closes the console
                       immediately, so its output can't be read; set this as an argument in
                       that IDE's own run configuration.
  --version            Print the framework version and exit.
  --help               Show usage and exit.
```

The console output always states whether a filter is active (and its pattern) before running or listing tests, and
`--list` reports how many tests/groups matched out of the total registered, so when output is redirected to a file,
there's a record of why fewer tests ran or were listed than expected. Likewise, it always states whether shuffle is
enabled and, if so, the seed in use.

`--shuffle` is a sanity check against hidden inter-test/inter-group coupling. The deterministic alphabetical
default (see [Registering Tests](#registering-tests)) is for readable, reproducible output day-to-day, while
`--shuffle` deliberately breaks that to surface tests that secretly depend on running in a particular order (e.g.
via shared static/global state). If `--shuffle` causes a failure, rerun with the logged seed via `--shuffle-seed` to
reproduce it exactly while debugging.

`--partition-index`/`--partition-count` split the suite for parallel execution across separate OS processes.
Each invocation still runs single-threaded and writes to its own console/JUnit output. Partitioning is by each
test's position in the canonical registration order, not by group, so one large group doesn't dominate a single
partition. Partition membership is independent of `--shuffle`, since it's computed from the canonical order rather
than any shuffled one, so a given test's partition never changes based on whether `--shuffle` is also passed.
A typical setup is a CI job matrix, e.g. 4 jobs each running:
`--partition-index <1..4> --partition-count 4 --report-junit results-<index>.xml`.
ASWUnitTests doesn't merge those reports itself, since most CI systems (GitHub Actions, GitLab CI, Jenkins, Azure
DevOps, CircleCI) already merge multiple JUnit XML files from parallel jobs natively.
Combine `--partition-index`/`--partition-count` with `--list` to preview which tests land in a given partition, the
same way `--list` can preview `--filter`.

`--test-timeout-seconds` guards against a single test hanging (e.g. an infinite loop) forever. There is no
default; without it, a hung test blocks the run indefinitely. When given, each test runs on its own worker thread
while the main thread waits with that timeout; a test that finishes normally (whether it passes, fails, or throws)
is completely unaffected. A test that doesn't finish in time is recorded as failed, with a message naming it and
the timeout that was exceeded, and the run stops there: no further tests or groups run, and the process exits with
a dedicated exit code (`5`) distinct from an ordinary test failure (`1`). If `--report-junit` was also given, the
report still gets written, covering every test that completed before the timeout, plus the timed-out test's own
synthetic failure entry. The abandoned worker thread itself is never joined or forcibly stopped, since there is no
safe, portable way to interrupt a thread that may be stuck in an infinite loop; if a test finishes only moments
after being abandoned rather than truly hanging forever, its outcome may be recorded inconsistently, since nothing
synchronizes it with the timeout-handling thread at that point. In practice this only matters for a "barely"
timed-out test, not a genuinely hung one, and the process exits immediately afterward regardless.

`--catch-crashes` guards against a single test crashing (e.g. dereferencing a null pointer) and taking down the
whole process with it. There is no default; without it, a crashing test still crashes the process as usual. On
Windows (MSVC, MinGW, and RAD Studio's Clang-based 32/64-bit compilers alike) this is implemented with
`AddVectoredExceptionHandler` rather than `__try`/`__except`: the latter is the textbook approach and unwinds C++
objects properly when it works, but it silently fails to catch anything at runtime under RAD Studio's compilers
even though it compiles, and GCC/MinGW does not implement the keywords at all, so this framework only relies on
the mechanism it directly verified actually works, across all four Windows compiler targets, including recovering
from a genuine stack overflow. On POSIX (Linux/Mac) it's a signal handler for `SIGSEGV`/`SIGFPE`/`SIGILL`/`SIGBUS`
that jumps back to a point just before the test started; `SIGABRT` is never caught, since it usually means the C
runtime itself already detected the process's state is corrupt and is deliberately terminating rather than letting
it continue.

A crash that's caught still gets recorded as a failed test, with a message describing the fault, exactly like any
other failure; if `--report-junit` was also given, it shows up in the report the same way. Most crash types let
the run continue with the next test afterward, since the whole point of catching it (unlike a timeout, which can
only abandon-and-abort) is that the harness can safely keep going. A few specific crash types abort the run
afterward instead, with a dedicated exit code (`6`) distinct from an ordinary test failure (`1`), which is what a
continued-past crash still shows up as: a Windows stack overflow (`EXCEPTION_STACK_OVERFLOW` is unambiguous), or
*any* `SIGSEGV` on POSIX. That second one is a deliberate platform difference, not an
oversight: unlike Windows, POSIX delivers a stack overflow and an ordinary segfault as the exact same signal, and
reliably telling them apart needs inspecting the faulting address against the thread's stack bounds - real extra
complexity for what's already treated as an edge case on both platforms. The practical effect is that the same
ordinary null-pointer dereference continues the run on Windows but aborts it on Linux/Mac. Catching a genuine
stack overflow at all on POSIX (as opposed to distinguishing it from an ordinary segfault) does rely on
`sigaltstack()`/`SA_ONSTACK`: the default signal handler would otherwise run on the same, already-exhausted stack
that just overflowed, leaving it nowhere to run and crashing the process for real instead.

Recovering from a crash at all is a raw jump back to before the test started (the same fundamental technique
`--test-timeout-seconds`' abandoned worker thread relies on, for a different reason): it does not run destructors
for any objects that were under construction on the crashing test's stack at the moment of the fault. This is a
real, accepted limitation of recovering from a hardware-level fault, on any platform, not an oversight either.

Pass/fail/skip status text is colorized when writing to an interactive terminal that supports ANSI escape codes.
Output redirected to a file or pipe, or a non-interactive CI log, automatically gets plain text with no escape
codes, unless `--color=always` forces it (e.g. for a CI system that supports ANSI in its own log viewer).
The [NO_COLOR](https://no-color.org) environment variable is also respected in the default `auto` mode.

Every test logs a `Finished test: "GroupName.TestName" - passed/failed/skipped (N.NNN ms)` line on completion,
timing from just before `SetUp_Test` to just after the test's outcome is determined. This is useful for spotting
slow tests without needing an external profiler.

`--report-junit` produces a standard `<testsuites>`/`<testsuite>`/`<testcase>` report. One `<testsuite>` per test
group, with `<failure>`/`<skipped>` elements carrying the same detail message shown on the console. `--project-name`
sets both the console's `Initializing...` line and the report's `<testsuites name="...">` attribute.

Exit codes: `0` all run tests passed or were skipped (or `--version`/`--list`/`--help` completed), `1` one or more
tests failed, `2` an unhandled `std::exception` (or, with [RTL exception support](#rad-studio-rtl-exceptions-vclfmx)
enabled, an RTL `Exception`) escaped a test, `3` an unhandled exception of any other type escaped a test,
`4` invalid command line arguments, `5` a test exceeded `--test-timeout-seconds` and the run was aborted, `6` a test
crashed severely enough (with `--catch-crashes` given) that the run was aborted. A crash caught by
`--catch-crashes` that didn't force an abort is just an ordinary test failure (exit code `1`), not `6`. Skipped
tests never affect the exit code.

## VCL GUI Runner

`vcl\gui\rad370\ASWUnitTests_VCL_GUI.cbproj` builds a RAD Studio VCL application that runs the same tests as the
console runner, in a window for choosing tests and reading their results. It runs this repository's full self-test
suite, and its `Build_*.bat` scripts write the executable to `vcl\gui\rad370\<Platform>\<Config>`.
`vcl\gui\rad370\ASWUnitTests_VCL_Group.groupproj` opens it together with the VCL console project.

The window shows:

- **Test tree** - Every registered test under its group, with a check box to choose what runs (all are checked at
  first) and a colored dot for its status: gray not run, blue running, green passed, red failed, amber skipped. A
  group's check box and dot summarize its tests.
- **Detail pane** - The selected test's result, duration, failure or skip detail, and its own log output, or how many
  of a selected group's tests have each status.
- **Failures & Skips** - Every failed or skipped test in the latest run. Double-click one to select it in the tree.
- **Log** - The run's full output, with failures in red and skips in amber.
- **Progress and status bars** - Progress (red once anything fails), counts, elapsed time, and the run's status.

The toolbar, whose commands are also in the Run and Tests menus:

- **Run Selected** (F9) runs the checked tests, and **Run Failed** reruns the latest run's failures.
- **Stop** ends the run after the current test. Closing the window during a run does the same, then closes.
- **Select All** and **Select None** check or uncheck every shown test, and **Select Failed** checks only the shown
  tests that failed in the latest run.
- **Copy Details** (Ctrl+Shift+C) copies the detail pane to the clipboard.
- **Filter** (Ctrl+F) shows only the tests whose `Group.Test` name contains its text, ignoring case, with the same
  `*` and `?` wildcards as `--filter`. While it hides tests, checking them and Run Selected only apply to the shown
  ones, and hidden tests keep their check marks.

**File > Export JUnit Report** saves the latest run's results as the same JUnit XML report `--report-junit` writes,
for when `--report-junit` wasn't given, or to keep a copy. The Help menu shows the command line options (the same
text as `--help`) and the framework version.

When the window closes, it saves its size, position, maximized state, and panel sizes to
`%APPDATA%\ASWUnitTests\<exe name>.ini`, and restores them the next time it opens. A position on a monitor that's no
longer connected is ignored. An `--exit` run doesn't save the layout. **View > Reset Layout** restores the default
layout right away; `--layout-ignore` and `--layout-reset` (below) control this from the command line.

The window also remembers which tests are checked, in `%APPDATA%\ASWUnitTests\<exe name>.selection`, as long as the
command line doesn't choose them itself (no `--run`, `--filter`, or partition options). What it saves is what Run
Selected would run, the checked tests that are shown; the filter box itself starts empty. Since tests come and go
between sessions, it adapts:

- A test that's been removed is skipped.
- A new test (or a renamed one, which looks the same) is checked if its group was entirely checked, and otherwise
  isn't. A test in a new group is checked only if every test was.
- If none of the checked tests still exist, every test is checked instead.

The log notes what was restored. So **Select Failed**, closing the window, fixing the code, and reopening it leaves
just those tests checked for Run Selected.

Tests run on the main (VCL) thread, so a test can create forms and controls. `--test-timeout-seconds` runs each test
on a worker thread instead, so it can't be combined with tests like that; the log says so when it's given.

The GUI takes the same [command line options](#command-line-options) as the console runner, with these differences:

- `--filter` and `--partition-index`/`--partition-count` choose which tests start out checked, instead of the
  saved selection, and `--filter`'s pattern also fills in the filter box. The box shows every test the pattern matches (and maybe a few more, left
  unchecked, since the box ignores case and matches anywhere in the name).
- `--project-name` is also shown in the window's caption.
- `--report-junit` writes the report after every run, including Run Failed.
- `--list`, `--pause`, and the color options are ignored, with a note in the log.
- `--help`, `--version`, and argument errors are shown in a dialog, and the GUI then exits without opening.
- `--run` (GUI-only) runs the checked tests as soon as the window opens.
- `--exit` (GUI-only, requires `--run`) closes the window when that run finishes, and exits with the same code the
  console runner would; a run ended with Stop exits with `1`. Without `--exit`, the GUI always exits with `0`.
- `--layout-ignore` (GUI-only) neither loads nor saves the window layout, so the window opens with the defaults
  and the saved layout is left as it is.
- `--layout-reset` (GUI-only) deletes the saved window layout and opens with the defaults, which is a way to
  recover if a saved layout ever causes a problem. The layout is saved again when the window closes.

A GUI process doesn't make a Windows command prompt wait for it, so a script that needs `--exit`'s exit code should
start it with `start /wait` (or its own language's equivalent):

```
start /wait ASWUnitTests_VCL_GUI.exe --filter "*String*" --run --exit --report-junit results.xml
echo %ERRORLEVEL%
```

To run your own tests in the GUI, create a RAD Studio VCL application that defines `ASWUNITTESTS_RTL_EXCEPTIONS`, and
add to it: the framework's `src` files except `main.cpp`; the files in `vcl\gui\src`, including
`ASWUnitTests_GUI_MainForm.dfm`; your own test modules; and a `WinMain()` modeled on
`vcl\gui\rad370\ASWUnitTests_VCL_GUI.cpp`, which parses the command line and starts the main form.

The GUI is built on `ITestRunObserver` (in `src\ASWUnitTests_TestBase.h`), set with `TTestHandler::SetRunObserver()`.
An observer receives the log output that would otherwise go to `std::cout`, is told as each test starts and finishes,
and can stop a run between tests, so it can drive other kinds of runners too.

## Registering Tests

Test modules self-register with `TTestHandler` using the `ASW_REGISTER_TEST_GROUP` macro (declared in
`src\ASWUnitTests_Registry.h`). No file in `src` ever needs to be modified to add, remove, or rename a test
module. This makes it easy to drop ASWUnitTests into another repository (e.g. as a git submodule).
Place the macro at file scope, after the closing brace of the `ASWUnitTests` namespace, in the test module's `.cpp` file:

```
// Whatever includes at the top of the file for your `TMyClassToTest` class, etc.
#include "ASWUnitTests_Registry.h"

namespace ASWUnitTests
{
    // ... TTest_TMyClassToTest class implementation ...

} // namespace ASWUnitTests

ASW_REGISTER_TEST_GROUP(ASWUnitTests::TTest_TMyClassToTest)
```

Only the test module's own `.cpp`/`.h` files need to be added to your project's build (CMake, RAD Studio, etc.).
See `tests\Test_ASWTools_String.cpp` and `tests\Test_ASWTools_Random.cpp` for working examples.

By default, groups run in alphabetical order by group name, deterministically across compilers and linkers. To
override that for a specific group, instead use `ASW_REGISTER_TEST_GROUP_ORDERED(ClassName, order)`. Groups run in
ascending order, with ties broken alphabetically:

```
ASW_REGISTER_TEST_GROUP_ORDERED(ASWUnitTests::TTest_TMyClassToTest, -1) // runs before the alphabetical block
```

Run order is purely for readable, reproducible output. A group's `SetUp_Group`/`TearDown_Group` should still make
no assumption about which other groups have or haven't already run.

### Expecting a Specific Exception Type or Message

`SetExceptionExpected(true, ...)` passes on any thrown exception, regardless of its type. To also verify the
exception's type (matched polymorphically, so a base class also matches its subclasses) and, optionally, that its
`what()` contains a given substring, use the templated overload instead:

```
SetExceptionExpected<std::invalid_argument>(__func__, __LINE__, "StrToInt32 invalid", "signed 32-bit int");
int32_t i = TStrTool::StrToInt32(invalid);
```

If the wrong exception type is thrown, or its message doesn't contain the given substring, the test fails with a
message showing what was actually caught. This only works for exceptions deriving from `std::exception`, or from
RAD Studio's RTL `Exception` class when [RTL exception support](#rad-studio-rtl-exceptions-vclfmx) is enabled. A
thrown object that doesn't can't be inspected, so a type/message expectation against it fails with an explanatory
message rather than silently passing.

An expected exception never hides a failure elsewhere in the test. A failed `Assert*` still fails the test rather
than counting as the expected exception, and so does a failed `Check*` earlier in the test, even if the expected
exception then arrives.

### RAD Studio RTL Exceptions (VCL/FMX)

RAD Studio's RTL exceptions (`System::Sysutils::Exception` and its subclasses, such as `EConvertError`) are shared by
VCL and FMX, and don't derive from `std::exception`. By default, only the generic `SetExceptionExpected(true, ...)`
matches one, and one that escapes a test unexpectedly is reported as `Unhandled exception: Unknown`.

A C++Builder project that links the RTL can opt in by defining `ASWUNITTESTS_RTL_EXCEPTIONS` in its project options
(Building > C++ Shared Options > Conditional defines). This requires RAD Studio's Clang-based compilers
(`bcc32c`/`bcc64x`). Defining it for any other compiler is a compile error, so a misconfigured build fails loudly
instead of silently dropping the feature. Nothing changes for builds that don't define it.

With it enabled, the templated `SetExceptionExpected<TException>()` also accepts RTL exception classes, matched
polymorphically the same way (so `Exception` matches any RTL exception). The optional message substring is checked
against the exception's `Message`, converted to UTF-8:

```
SetExceptionExpected<EConvertError>(__func__, __LINE__, "StrToInt invalid", "not a valid integer");
StrToInt(L"abc");
```

A requested RTL type never matches a `std::exception`, and a requested `std::exception` type never matches an RTL
exception. Failure details and unhandled exceptions show RTL exceptions as `ClassName: Message` (e.g.
`EConvertError: 'abc' is not a valid integer value`), and an unhandled one exits with code `2`, like an unhandled
`std::exception`.

`--test-timeout-seconds` runs each test on a worker thread. Expected exceptions work exactly the same there, because
they're caught and checked on that thread. An *unexpected* RTL exception has to cross back to the main thread, which
`std::exception_ptr` can't do for RTL exceptions (rethrowing one after its handler has exited terminates the
process). It therefore arrives as a `TExceptRTLException`, a `std::runtime_error` whose `what()` is the same
`ClassName: Message` text, so the console output and exit code are unchanged. Code that calls a group's `Run()`
directly with a timeout can still check the original type through `RTLClass()`:

```
catch (TExceptRTLException const& ex)
{
    if (ex.RTLClass()->InheritsFrom(__classid(EConvertError)))
        ...
}
```

`vcl\console\rad370\ASWUnitTests_VCL_Console.cbproj` is a working reference setup: a VCL console project that defines
`ASWUNITTESTS_RTL_EXCEPTIONS` and runs this repository's full self-test suite plus the RTL-specific and GUI unit
tests in `vcl\tests`. It has its own `Build_*.bat` scripts, and writes its executable to
`vcl\console\rad370\<Platform>\<Config>`. The [VCL GUI runner](#vcl-gui-runner) is set up the same way.

### Comparing System::String (VCL/FMX)

The same `ASWUNITTESTS_RTL_EXCEPTIONS` define (see [above](#rad-studio-rtl-exceptions-vclfmx)) also lets the string
`Check`/`Assert` methods take a `System::String` (`UnicodeString`): `Equals`, `Contains`, `StartsWith`, `EndsWith`,
and their `Not` and `IC` forms. At least one of the two texts must be a `System::String`. The other may also be a
`std::string`, `std::wstring` or C string, including a literal:

```
CheckEquals("Ready", Label1->Caption, __func__, __LINE__, "status after loading");
CheckStartsWithIC(Edit1->Text, L"https://", __func__, __LINE__, "a secure URL");
```

Both texts are converted to UTF-8 and compared as two `std::string` values, so a failure shows them the same way.
Narrow text is read as UTF-8, as elsewhere in the framework, and a null C string is empty text, as it is for
`System::String`. A `System::String` converts implicitly from a number or a character, but these methods don't accept
either, so `CheckEquals(Edit1->Text, 5, ...)` is a compile error rather than a comparison with `"5"`. Other RTL string
types, such as `AnsiString`, need converting to `System::String` first, and the message is still a `std::string`.

### Omitting the Method and Line (C++20)

Every `Check*`/`Assert*` method, `Skip()`, and both forms of `SetExceptionExpected()` also have an overload without
the method and line arguments. It takes an optional `std::source_location` as its last argument instead, which
defaults to the caller's location:

```
CheckEquals(5, total, "int and int64_t");
Skip("Windows-only feature");
SetExceptionExpected<std::invalid_argument>("StrToInt32 invalid", "signed 32-bit int");
```

Failure messages then show `std::source_location::function_name()` as the method. Unlike `__func__`, its text is up
to the compiler, and is typically the full signature, e.g. `void TTest_TMyClassToTest::Test_Something()` on GCC and
Clang, or `void __cdecl TTest_TMyClassToTest::Test_Something(void)` on MSVC. The method and line overloads remain,
for a bare or custom name.

A helper that makes its own checks can take a location and pass it on, so its failures report the helper's caller
rather than the helper itself:

```
// In the class declaration
void CheckIsEven(int value, std::source_location loc = std::source_location::current());

// In the .cpp
void TTest_TMyClassToTest::CheckIsEven(int value, std::source_location loc)
{
    CheckTrue(value % 2 == 0, std::to_string(value) + " should be even", loc);
}
```

These overloads need C++20's `std::source_location`, so they're only declared when `ASWUnitTests_TestBase.h` finds it
supported, which it signals by defining `ASWUNITTESTS_SOURCE_LOCATION_ENABLED`. RAD Studio's 32-bit compilers only
support C++17, so they don't have these overloads. Tests that also need to build there should keep the method and
line form, or check `ASWUNITTESTS_SOURCE_LOCATION_ENABLED`.

### Comparing C Strings

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` compare two C strings (`char const*` or
`wchar_t const*`, including string literals and character arrays) by content, the same as `std::string` and
`std::wstring`. A null pointer only matches another null pointer, never a string, not even an empty one:

```
char const buffer[] = "abc";
CheckEquals("abc", buffer, __func__, __LINE__, "same text in a different buffer passes");
```

A failed `CheckEquals` shows both strings, and a failed `CheckNotEquals` the value they share, for C strings,
`std::string` and `std::wstring` alike. Wide text is converted to UTF-8, and a null pointer is shown as `(null)`.

### Comparing Strings, Ignoring Case

`CheckEqualsIC`/`AssertEqualsIC` and `CheckNotEqualsIC`/`AssertNotEqualsIC` compare two `std::string` or
`std::wstring` values like `CheckEquals`/`CheckNotEquals`, but ignoring case. A failure shows both values, with wide
text converted to UTF-8:

```
CheckEqualsIC("Content-Type", headerName, __func__, __LINE__, "header names ignore case");
```

Only the ASCII letters `A`-`Z` and `a`-`z` are matched regardless of case, so the result is the same on every platform
and in every locale, and the bytes of a multi-byte UTF-8 character are never changed. Other letters, such as an
accented capital and small E, still have to match exactly. The [substring](#checking-for-a-substring) and
[prefix and suffix](#checking-the-start-or-end-of-a-string) checks ignore case the same way.

### Checking for a Substring

`CheckContains`/`AssertContains` pass when a `std::string` or `std::wstring` contains a given substring, and
`CheckNotContains`/`AssertNotContains` pass when it doesn't. The text comes first, then the substring. Unlike
`CheckTrue(text.find(substring) != std::string::npos, ...)`, a failure shows both:

```
std::string const log = "Connected to server";
CheckContains(log, "timeout", __func__, __LINE__, "logs the timeout");
// Check failed for: "Test_Connect" (42): Expected "Connected to server" to contain "timeout". logs the timeout
```

The comparison is case-sensitive, and every string contains the empty string. A failure shows `std::wstring` text
converted to UTF-8.

`CheckContainsIC`/`AssertContainsIC` and `CheckNotContainsIC`/`AssertNotContainsIC` do the same, ignoring the case
of ASCII letters only, the same way as [`CheckEqualsIC`](#comparing-strings-ignoring-case):

```
CheckContainsIC(log, "CONNECTED", __func__, __LINE__, "passes");
```

### Checking the Start or End of a String

`CheckStartsWith`/`AssertStartsWith` pass when a `std::string` or `std::wstring` starts with a given prefix, and
`CheckEndsWith`/`AssertEndsWith` when it ends with a given suffix. `CheckNotStartsWith`/`AssertNotStartsWith` and
`CheckNotEndsWith`/`AssertNotEndsWith` pass when it doesn't. The text comes first, then the prefix or suffix, read as
"text starts with prefix". A failure shows both:

```
std::string const path = "logs/server.txt";
CheckEndsWith(path, ".log", __func__, __LINE__, "writes a log file");
// Check failed for: "Test_LogPath" (42): Expected "logs/server.txt" to end with ".log". writes a log file
```

As with the [substring checks](#checking-for-a-substring), the comparison is case-sensitive, every string starts and
ends with the empty string, and a failure shows `std::wstring` text converted to UTF-8. The `IC` variants
(`CheckStartsWithIC`, `CheckEndsWithIC`, and their `Assert` and `Not` forms) ignore the case of ASCII letters only, the
same way as [`CheckEqualsIC`](#comparing-strings-ignoring-case):

```
CheckStartsWithIC(header, "content-type:", __func__, __LINE__, "passes for \"Content-Type: text/plain\"");
```

### Comparing Integers of Different Types

`CheckEquals`/`AssertEquals` and `CheckNotEquals`/`AssertNotEquals` accept any two integer types, not just a matching
pair of fixed-width ones, so there's no need for a suffix or cast like `0LL` to pick an overload. This includes
`long` and `unsigned long` (e.g. `DWORD`) on Windows, and `long long` on Linux, which match none of the fixed-width
types there. The values are compared as numbers, so `-1` never equals an unsigned value, unlike with the built-in `==`:

```
int64_t total = 5;
CheckEquals(5, total, __func__, __LINE__, "int and int64_t");
CheckEquals(-1, 4294967295u, __func__, __LINE__, "fails, where -1 == 4294967295u is true");
```

`bool` is the exception: comparing a `bool` with an integer remains a compile error, since it's usually a mistake.

### Comparing Floating-Point Values

There are no `float`/`double` overloads of `CheckEquals`/`AssertEquals`. Exact equality comparison of
floating-point values is unreliable (e.g. `0.1f + 0.2f != 0.3f`). Use `CheckNear`/`AssertNear` instead, which pass
when the absolute difference between the two values is within a given tolerance, and `CheckNotNear`/`AssertNotNear`
for the opposite (asserting two values are *not* within tolerance of each other):

```
float sum = 0.1f + 0.2f;
CheckNear(0.3f, sum, 0.0001f, __func__, __LINE__, "sum should be close to 0.3");
```

Pick a tolerance appropriate to the computation being tested; there's no built-in default, since a sensible
tolerance depends heavily on the magnitude and accumulated error of the values involved.

### Comparing Greater Than and Less Than

`CheckGreaterThan`, `CheckGreaterThanOrEqual`, `CheckLessThan` and `CheckLessThanOrEqual`, and their `Assert`
versions, compare a value with a bound. The value comes first, then the bound, read as "value >= bound". That's the
other way round from `CheckEquals`, whose expected value comes first. Unlike `CheckTrue(count >= 1, ...)`, a failure
shows both:

```
CheckGreaterThanOrEqual(count, 1, __func__, __LINE__, "at least one item");
// Check failed for: "Test_Items" (42): Expected 0 to be >= 1. at least one item
```

They accept any two integer or floating-point types except `bool`. Two integers are compared by value, like
[`CheckEquals`](#comparing-integers-of-different-types) does, so `-1` is less than any unsigned value. When either
value is floating point, both are compared as their common type, as the built-in operators do, and a NaN fails every
check.

### Skipping a Test

Call `Skip(method, line, reason)` from within a test to abort it and have it reported as skipped. It is separately
counted from passed/failed, and does not affect the process exit code. No explicit `return` is needed afterward,
since `Skip()` throws to unwind the rest of the test body, the same way `Assert*` methods do:

```
void TTest_TMyClassToTest::Test_WindowsOnlyFeature()
{
#if !defined(_WIN32)
    Skip(__func__, __LINE__, "Windows-only feature");
#endif
    // ... test body ...
}
```

This keeps a known-broken, environment-specific, or not-yet-implemented test's registration (and its place in
`--list` output) intact, instead of commenting out or deleting its `RegisterTest()` call and losing the reminder
that it exists. The reason is required, so future readers always know *why* a test is being skipped.

Test modules themselves inherit from `TTestGroupBase` and each method that needs to be tested for a module must
be explicitely registered within that module's constructor. For example:

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

### Parameterized Tests

`RegisterTestCases()` registers one test case per element of a `std::vector<TParam>`, calling the given method with
each element in turn, instead of hand-writing a loop inside a single test or duplicating near-identical test
methods for each input. Each row becomes an independent, individually named, individually filterable test case,
so `--filter`, `--shuffle`, colorized output, and the JUnit report all work on it exactly like any other test:

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

Pass an optional `std::function<std::string (TParam const&)>` name generator as a fourth argument to label each row
with something more meaningful than its index (e.g. `"HexSingleToByte[A]"`), which is especially useful for reading
`--filter` matches or a failing row's name in CI output.

## Submodule Integration

This repository's own `cmake\CMakeLists.txt`, `rad370\ASWUnitTests.cbproj`, and the RAD Studio projects under `vcl`
only build *this* repo's own example tests and `toTest` code for its own development and CI. Don't use them from a
consuming project, and don't modify them. Doing either means your changes live inside the submodule and get lost or
conflict the next time you update it. Instead, add ASWUnitTests as a git submodule (e.g. into
`third_party\asw-unit-tests`) and reference its `src` files from your own project's build file, alongside your own
test modules:

```
my-project/
+-- third_party/asw-unit-tests/   <- git submodule, never modified, freely updated
|   +-- src/                      <- framework core (never touched)
|   +-- vcl/gui/src/              <- VCL GUI runner, if you use it (never touched)
|   `-- cmake/, rad370/, tests/, toTest/, other vcl/ folders   <- this framework's own example build, unused by you
+-- tests/                        <- your own test modules (Test_MyClass.cpp/.h), self-registered
`-- CMakeLists.txt / .cbproj      <- your own build file, in your own repo
```

Since `src\main.cpp` only calls into `TTestHandler` and never references a specific test class, you compile it
as-is from the submodule. There's no need to copy or duplicate it into your own tree.

### CMake

Include `src\ASWUnitTests_Sources.cmake` from your own `CMakeLists.txt` rather than listing the framework's source
filenames by hand; it exports `ASWUNITTESTS_SOURCES` (the compiled `.cpp` files) and `ASWUNITTESTS_SOURCE_DIR` (for
the include path). It stays in sync automatically if a future version of this framework adds a core file:

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

`--test-timeout-seconds` uses `std::thread`/`std::future`, which needs an explicit link against a threading
library on Linux and some MinGW-w64 distributions (a no-op on MSVC and toolchains that need no extra linker
flag). Add it the same way this repository's own `cmake\CMakeLists.txt` does:

```cmake
find_package(Threads REQUIRED)
target_link_libraries(MyTests PRIVATE Threads::Threads)
```

### RAD Studio / Visual Studio / other IDE projects

Add the same files listed in `ASWUNITTESTS_SOURCES` from the submodule's `src` folder to your own project, plus
your own test modules. Check `src\ASWUnitTests_Sources.cmake` for added files after updating the submodule.

For a C++Builder project that links the VCL or FMX, also define `ASWUNITTESTS_RTL_EXCEPTIONS` to enable
[RTL exception support](#rad-studio-rtl-exceptions-vclfmx) and
[`System::String` comparisons](#comparing-systemstring-vclfmx). To run your tests in the
[VCL GUI runner](#vcl-gui-runner) instead of a console, see that section for the files it needs.

# Coding Standards

To use uncrustify (for coding standards (pretty formatting) for this repo):

1. Get `uncrustify` and ensure that the path to `uncrustify.exe` is added to the Windows `Path` environment variable.
    https://github.com/uncrustify/uncrustify

2. Run the following command locally to set the git hooks directory (uncrustify will run upon commit):
```
git config --local core.hooksPath .githooks/
```
