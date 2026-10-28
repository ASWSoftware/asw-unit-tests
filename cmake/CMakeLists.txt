cmake_minimum_required(VERSION 3.20)

project(ASWUnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/ASWUnitTests_Sources.cmake)

# The shared GUI core, the framework's self-tests and the example code they test, which every runner compiles.
set(ASWUNITTESTS_SELF_TEST_SOURCES
    ../gui/src/ASWUnitTests_GUI_CommandLine.cpp
    ../gui/src/ASWUnitTests_GUI_Commands.cpp
    ../gui/src/ASWUnitTests_GUI_INIDocument.cpp
    ../gui/src/ASWUnitTests_GUI_Layout.cpp
    ../gui/src/ASWUnitTests_GUI_RunObserver.cpp
    ../gui/src/ASWUnitTests_GUI_Selection.cpp
    ../gui/src/ASWUnitTests_GUI_Shuffle.cpp
    ../gui/src/ASWUnitTests_GUI_TestList.cpp
    ../gui/src/ASWUnitTests_GUI_Tree.cpp
    ../gui/src/ASWUnitTests_GUI_View.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_CommandLine.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Commands.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_INIDocument.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Layout.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_RunObserver.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Selection.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_Shuffle.cpp
    ../gui/tests/Test_ASWUnitTests_GUI_TestList.cpp
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
    ../toTest/ASWTools_Common.cpp
    ../toTest/ASWTools_Random.cpp
    ../toTest/ASWTools_String.cpp
)

add_executable(ASWUnitTests
    ${ASWUNITTESTS_SOURCES}
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
    target_compile_options(ASWUnitTests PRIVATE /W4 /Zc:__cplusplus)
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
    # the console runner's main.cpp.
    set(ASWUNITTESTS_WIN32_GUI_FRAMEWORK_SOURCES ${ASWUNITTESTS_SOURCES})
    list(REMOVE_ITEM ASWUNITTESTS_WIN32_GUI_FRAMEWORK_SOURCES "${ASWUNITTESTS_SOURCE_DIR}/main.cpp")

    add_executable(ASWUnitTests_Win32_GUI WIN32
        ${ASWUNITTESTS_WIN32_GUI_FRAMEWORK_SOURCES}
        ${ASWUNITTESTS_SELF_TEST_SOURCES}
        ../win32/gui/src/ASWUnitTests_Win32GUI_DialogTemplate.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_DPI.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_MainLayout.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_MainWindow.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_ShuffleSeedDialog.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_Splitter.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_System.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_TextDialog.cpp
        ../win32/gui/src/ASWUnitTests_Win32GUI_WinMain.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DialogTemplate.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_DPI.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_MainLayout.cpp
        ../win32/tests/Test_ASWUnitTests_Win32GUI_System.cpp
    )

    target_include_directories(ASWUnitTests_Win32_GUI PRIVATE
        ../gui/src
        ../gui/tests
        ../src
        ../tests
        ../toTest
        ../win32/gui/src
        ../win32/tests
    )

    target_link_libraries(ASWUnitTests_Win32_GUI PRIVATE
        Threads::Threads
        comctl32
        comdlg32
        ole32
        rpcrt4
        shell32
        uxtheme
    )

    if(MSVC)
        target_compile_options(ASWUnitTests_Win32_GUI PRIVATE /W4 /Zc:__cplusplus)
        # The linker merges the manifest into the one it generates. /MANIFESTUAC:NO leaves the execution level
        # to the manifest's own trustInfo, rather than adding a second one.
        target_sources(ASWUnitTests_Win32_GUI PRIVATE ../win32/gui/src/ASWUnitTests_Win32GUI.manifest)
        target_link_options(ASWUnitTests_Win32_GUI PRIVATE /MANIFESTUAC:NO)
    else()
        target_compile_options(ASWUnitTests_Win32_GUI PRIVATE -Wall -Wextra -Wpedantic)
        # The manifest as a resource. GCC also links its distribution's default manifest, if it has one; GNU ld
        # drops a language-neutral default one silently, but WinLibs' has a language, so ld reports ".rsrc merge
        # failure: multiple non-default manifests". The link still succeeds and Windows uses this manifest,
        # which comes first.
        target_sources(ASWUnitTests_Win32_GUI PRIVATE ../win32/gui/src/ASWUnitTests_Win32GUI_Manifest.rc)
    endif()

    if(MINGW)
        # Statically linked, as the console runner is.
        target_link_options(ASWUnitTests_Win32_GUI PRIVATE -static-libgcc -static-libstdc++ -static)
    endif()

    set_target_properties(ASWUnitTests_Win32_GUI PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    )
endif()
