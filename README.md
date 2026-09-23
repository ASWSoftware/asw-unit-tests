# ASWUnitTests - https://github.com/ASWSoftware/asw-unit-tests

ASWUnitTests is a speedy light-weight C++ unit test tool for Windows and Linux projects.

Requires C++17 or higher; the project itself is built and tested at C++20.

# Features

- `Check` prefix methods for `assert` sections of unit tests that aren't intended to throw (e.g. CheckTrue())
- `Assert` prefix methods for `assert` sections of unit tests that should throw right away (e.g. AssertTrue())
- `SetExceptionExpected()` - support for expected exceptions, with an optional exception-type and message check
- `CheckNear()`/`AssertNear()` - tolerance-based `float`/`double` comparison
- `Skip()` - aborts a test and reports it as skipped, unconditionally or after a runtime check, without removing
  its registration
- `RegisterTestCases()` - registers one test case per row of data for parameterized/data-driven tests, instead of
  hand-writing a loop or duplicating near-identical test methods
- `JUnit Report` - writes a JUnit-style XML test report, recognized natively by most CI systems, with no
  third-party dependency
- `--partition-index`/`--partition-count` - splits the suite across separate process invocations (e.g. a CI job
  matrix) for parallel execution, with no in-process threading and no merge step

See `ASWUnitTests_TestBase.h` for basic list of supported `Check/Assert` methods.
See the example unit test `Test_ASWTools_String.cpp` in `tests` folder for how to use `SetExceptionExpected()`.

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
project writes its final executable to the same `build/bin/<Config>` directory. Debug builds pause for input after the
tests; Release builds run the tests and exit.

The executable uses semantic versioning for its `--version` output. Update `src/ASWUnitTests_Version.h` when preparing a
release. The CMake project does not need a separate version declaration because it currently builds the test executable
directly rather than packaging or installing it.

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
  --version            Print the framework version and exit.
  --help               Show usage and exit.
```

The console output always states whether a filter is active (and its pattern) before running or listing tests, and
`--list` reports how many tests/groups matched out of the total registered — so if output is redirected to a file,
there's a record of why fewer tests ran or were listed than expected. Likewise, it always states whether shuffle is
enabled and, if so, the seed in use.

`--shuffle` is a sanity check against hidden inter-test/inter-group coupling — the deterministic alphabetical
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

Pass/fail/skip status text is colorized when writing to an interactive terminal that supports ANSI escape codes.
Output redirected to a file or pipe, or a non-interactive CI log, automatically gets plain text with no escape
codes, unless `--color=always` forces it (e.g. for a CI system that supports ANSI in its own log viewer).
The [NO_COLOR](https://no-color.org) environment variable is also respected in the default `auto` mode.

Every test logs a `Finished test: "GroupName.TestName" - passed/failed/skipped (N.NNN ms)` line on completion,
timing from just before `SetUp_Test` to just after the test's outcome is determined — useful for spotting slow
tests without needing an external profiler.

`--report-junit` produces a standard `<testsuites>`/`<testsuite>`/`<testcase>` report. One `<testsuite>` per test
group, with `<failure>`/`<skipped>` elements carrying the same detail message shown on the console. `--project-name`
sets both the console's `Initializing...` line and the report's `<testsuites name="...">` attribute.

Exit codes: `0` all run tests passed or were skipped (or `--version`/`--list`/`--help` completed), `1` one or more
tests failed, `2` an unhandled `std::exception` escaped a test, `3` an unhandled non-`std::exception` escaped a test,
`4` invalid command line arguments. Skipped tests never affect the exit code.

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

Only the test module's own `.cpp`/`.h` files need to be added to your project's build (CMake, RAD Studio, etc.) —
see `tests\Test_ASWTools_String.cpp` and `tests\Test_ASWTools_Random.cpp` for working examples.

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
message showing what was actually caught. This only works for exceptions deriving from `std::exception` — a thrown
object that doesn't (uncommon in practice) can't be inspected, so a type/message expectation against it fails with
an explanatory message rather than silently passing.

### Comparing Floating-Point Values

There are no `float`/`double` overloads of `CheckEquals`/`AssertEquals` — exact equality comparison of
floating-point values is unreliable (e.g. `0.1f + 0.2f != 0.3f`). Use `CheckNear`/`AssertNear` instead, which pass
when the absolute difference between the two values is within a given tolerance, and `CheckNotNear`/`AssertNotNear`
for the opposite (asserting two values are *not* within tolerance of each other):

```
float sum = 0.1f + 0.2f;
CheckNear(0.3f, sum, 0.0001f, __func__, __LINE__, "sum should be close to 0.3");
```

Pick a tolerance appropriate to the computation being tested; there's no built-in default, since a sensible
tolerance depends heavily on the magnitude and accumulated error of the values involved.

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

This repository's own `cmake\CMakeLists.txt` and `rad370\ASWUnitTests.cbproj` only build *this* repo's own example
tests and `toTest` code for its own development and CI. Don't use them from a consuming project, and don't modify
them — doing either means your changes live inside the submodule and get lost or conflict the next time you update
it. Instead, add ASWUnitTests as a git submodule (e.g. into `third_party\asw-unit-tests`) and reference its `src`
files from your own project's build file, alongside your own test modules:

```
my-project/
├── third_party/asw-unit-tests/   <- git submodule, never modified, freely updated
│   ├── src/                      <- framework core (never touched)
│   └── cmake/, rad370/, tests/, toTest/   <- this framework's own example build, unused by you
├── tests/                        <- your own test modules (Test_MyClass.cpp/.h), self-registered
└── CMakeLists.txt / .cbproj      <- your own build file, in your own repo
```

Since `src\main.cpp` only calls into `TTestHandler` and never references a specific test class, you compile it
as-is from the submodule — there's no need to copy or duplicate it into your own tree.

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

### RAD Studio / Visual Studio / other IDE projects

Add the same files listed in `ASWUNITTESTS_SOURCES` from the submodule's `src` folder to your own project, plus
your own test modules. Check `src\ASWUnitTests_Sources.cmake` for added files after updating the submodule.

# Coding Standards

To use uncrustify (for coding standards (pretty formatting) for this repo):

1. Get `uncrustify` and ensure that the path to `uncrustify.exe` is added to the Windows `Path` environment variable.
    https://github.com/uncrustify/uncrustify

2. Run the following command locally to set the git hooks directory (uncrustify will run upon commit):
```
git config --local core.hooksPath .githooks/
```
