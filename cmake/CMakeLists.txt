cmake_minimum_required(VERSION 3.20)

project(ASWUnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/ASWUnitTests_Sources.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/../gui/src/ASWUnitTests_GUI_Sources.cmake)

# The framework's self-tests, the shared GUI core's tests and the example code they test, which every runner
# compiles.
set(ASWUNITTESTS_SELF_TEST_SOURCES
    ../gui/tests/Test_ASWUnitTests_GUI_CommandLine.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Commands.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_INIDocument.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Layout.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_RunObserver.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Selection.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Shuffle.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_TestList.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Theme.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Tree.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_View.cpp
    ../tests/Test_ASWTools_Random.cpp
    ../tests/Test_ASWTools_String.cpp
    ../tests/Test_ASWUnitTests_AutoTest.cpp
    ../tests/Test_ASWUnitTests_CLI.cpp
    ../tests/Test_ASWUnitTests_Console.cpp
    ../tests/Test_ASWUnitTests_CrashGuard.cpp
    ../tests/Test_ASWUnitTests_Handler.cpp
    ../tests/Test_ASWUnitTests_JUnitReport.cpp
    ../tests/Test_ASWUnitTests_StdOutRedirect.cpp
    ../tests/Test_ASWUnitTests_Suite.cpp
    ../tests/Test_ASWUnitTests_TestBase.cpp
    ../tests/Test_ASWUnitTests_Utils.cpp
    ../tests/Test_ASWUnitTests_Version.cpp
    ../tests/UT_Helper_ExceptionTypes.cpp
    ../tests/UT_Helper_StringChecks.cpp
    ../toTest/ASWTools_Common.cpp
    ../toTest/ASWTools_Random.cpp
    ../toTest/ASWTools_String.cpp
)

add_executable(ASWUnitTests
    ${ASWUNITTESTS_SOURCES}
    ${ASWUNITTESTS_GUI_SOURCES}
    ${ASWUNITTESTS_SELF_TEST_SOURCES}
)

target_include_directories(ASWUnitTests PRIVATE
    ../gui/src
    ../gui/tests
    ../src
    ../tests
    ../toTest
)

# --test-timeout-seconds (ASWUnitTests_TestBase.cpp) uses std::thread/std::future, which needs an
# explicit link against a threading library on Linux and some MinGW-w64 distributions; a no-op on
# MSVC and toolchains where threading support needs no extra linker flag.
find_package(Threads REQUIRED)
target_link_libraries(ASWUnitTests PRIVATE Threads::Threads)

if(MSVC)
    # /MP compiles a target's files in parallel when MSBuild runs cl.exe, which otherwise compiles them one at a
    # time (MSBuild's own parallelism only builds projects side by side). Other generators, such as Ninja,
    # already run one compiler per file, and clang-cl ignores the option, so it's given to cl.exe only.
    target_compile_options(ASWUnitTests PRIVATE /W4 /Zc:__cplusplus $<$<CXX_COMPILER_ID:MSVC>:/MP>)
else()
    target_compile_options(ASWUnitTests PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(WIN32)
    # toTest/ASWTools_String.cpp uses the Win32 UUID/RPC string APIs. MSVC picks up
    # rpcrt4.lib via a #pragma comment, but MinGW/Clang need it linked explicitly.
    target_link_libraries(ASWUnitTests PRIVATE rpcrt4)

    # The Win32 GUI runner's units that need no window, and their tests, so they also run without the GUI.
    target_sources(ASWUnitTests PRIVATE
        ../win32/gui/src/ASWUnitTests_Win32GUI_DialogTemplate.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_DPI.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_MainLayout.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_System.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DialogTemplate.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DPI.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_MainLayout.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_System.cpp
    )
    target_include_directories(ASWUnitTests PRIVATE
        ../win32/gui/src
        ../win32/tests
    )
    # CommandLineToArgvW(), used by src/ASWUnitTests_Utils.cpp and the Win32 GUI's units. MSVC and MinGW link
    # shell32 by default; naming it doesn't depend on that.
    target_link_libraries(ASWUnitTests PRIVATE shell32)
endif()

if(MINGW)
    # Statically link the MinGW runtime (libgcc/libstdc++/libwinpthread) so the executable doesn't
    # depend on those DLLs being on PATH at runtime. Without this, running the built exe on a
    # machine (or CI runner) where MinGW's bin directory isn't on PATH fails to even launch: Windows'
    # loader can't resolve the missing DLL, and tools built on POSIX conventions - Git Bash included
    # - report that as a bare "command not found" (exit code 127) rather than anything mentioning a
    # missing DLL, which is exactly what GitHub Actions' Windows (MinGW) CI job hit.
    target_link_options(ASWUnitTests PRIVATE -static-libgcc -static-libstdc++ -static)
endif()

set_target_properties(ASWUnitTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

if(WIN32)
    # The Win32 GUI runner: the same tests as the console runner, in a window. Its WinMain takes the place of
    # the console runner's main.cpp. It's built from the same sources file a project using it would include,
    # so this build also checks that file.
    include(${CMAKE_CURRENT_SOURCE_DIR}/../win32/gui/src/ASWUnitTests_Win32GUI_Sources.cmake)

    add_executable(ASWUnitTests_Win32_GUI WIN32
        ${ASWUNITTESTS_WIN32_GUI_SOURCES}
        ${ASWUNITTESTS_SELF_TEST_SOURCES}
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DialogTemplate.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DPI.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_MainLayout.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_System.cpp
    )

    target_include_directories(ASWUnitTests_Win32_GUI PRIVATE
        ${ASWUNITTESTS_WIN32_GUI_INCLUDE_DIRS}
        ../gui/tests
        ../tests
        ../toTest
        ../win32/tests
    )

    target_link_libraries(ASWUnitTests_Win32_GUI PRIVATE
        ${ASWUNITTESTS_WIN32_GUI_LIBRARIES}
        Threads::Threads
        rpcrt4
    )

    target_link_options(ASWUnitTests_Win32_GUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS})

    if(MSVC)
        # /MP as for the console runner.
        target_compile_options(ASWUnitTests_Win32_GUI PRIVATE /W4 /Zc:__cplusplus $<$<CXX_COMPILER_ID:MSVC>:/MP>)
    else()
        target_compile_options(ASWUnitTests_Win32_GUI PRIVATE -Wall -Wextra -Wpedantic)
    endif()

    if(MINGW)
        # Statically linked, as the console runner is.
        target_link_options(ASWUnitTests_Win32_GUI PRIVATE -static-libgcc -static-libstdc++ -static)
    endif()

    set_target_properties(ASWUnitTests_Win32_GUI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
