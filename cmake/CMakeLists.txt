cmake_minimum_required(VERSION 3.20)

project(ASWUnitTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(${CMAKE_CURRENT_SOURCE_DIR}/../src/ASWUnitTests_Sources.cmake)

add_executable(ASWUnitTests
    ${ASWUNITTESTS_SOURCES}
    ../tests/Test_ASWTools_Random.cpp
    ../tests/Test_ASWTools_String.cpp
    ../tests/Test_ASWUnitTests_CLI.cpp
    ../tests/Test_ASWUnitTests_Console.cpp
    ../tests/Test_ASWUnitTests_CrashGuard.cpp
    ../tests/Test_ASWUnitTests_Handler.cpp
    ../tests/Test_ASWUnitTests_JUnitReport.cpp
    ../tests/Test_ASWUnitTests_StdOutRedirect.cpp
    ../tests/Test_ASWUnitTests_TestBase.cpp
    ../toTest/ASWTools_Common.cpp
    ../toTest/ASWTools_Random.cpp
    ../toTest/ASWTools_String.cpp
)

target_include_directories(ASWUnitTests PRIVATE
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
endif()

set_target_properties(ASWUnitTests PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
