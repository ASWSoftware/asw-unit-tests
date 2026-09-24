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
    ../toTest/ASWTools_Common.cpp
    ../toTest/ASWTools_Random.cpp
    ../toTest/ASWTools_String.cpp
)

target_include_directories(ASWUnitTests PRIVATE
    ../src
    ../tests
    ../toTest
)

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
