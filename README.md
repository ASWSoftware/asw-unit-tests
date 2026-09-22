# ASWUnitTests - https://github.com/ASWSoftware/asw-unit-tests

ASWUnitTests is a speedy light-weight C++ unit test tool for Windows and Linux projects.

Requires C++17 or higher; the project itself is built and tested at C++20.

# Features

- `Check` prefix methods for `assert` sections of unit tests that aren't intended to throw (e.g. CheckTrue())
- `Assert` prefix methods for `assert` sections of unit tests that should throw right away (e.g. AssertTrue())
- `SetExceptionExpected()` - basic testing support for expected exceptions

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
