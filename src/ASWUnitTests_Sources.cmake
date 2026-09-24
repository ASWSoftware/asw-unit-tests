# ASWUnitTests_Sources.cmake
#
# Exports the ASWUnitTests framework's core source files as the
# ASWUNITTESTS_SOURCES list, and their directory as ASWUNITTESTS_SOURCE_DIR,
# so a project that adds this repository as a git submodule can build the
# framework without tracking its filenames by hand across updates. Every
# core header has a matching .cpp, so listing ASWUNITTESTS_SOURCES is
# enough; CMake/IDEs associate each header with its .cpp automatically.
#
# Usage, from the consuming project's own CMakeLists.txt (never edit this
# repository's own cmake/CMakeLists.txt or rad370/ASWUnitTests.cbproj to add
# your tests):
#
#     include(third_party/asw-unit-tests/src/ASWUnitTests_Sources.cmake)
#     add_executable(MyTests
#         ${ASWUNITTESTS_SOURCES}
#         tests/Test_MyClass.cpp
#     )
#     target_include_directories(MyTests PRIVATE ${ASWUNITTESTS_SOURCE_DIR} tests)

set(ASWUNITTESTS_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(ASWUNITTESTS_SOURCES
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_CLI.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_Console.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_Exception.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_Handler.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_JUnitReport.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_Registry.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_StdOutRedirect.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_TestBase.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/ASWUnitTests_Version.cpp"
    "${ASWUNITTESTS_SOURCE_DIR}/main.cpp"
)
