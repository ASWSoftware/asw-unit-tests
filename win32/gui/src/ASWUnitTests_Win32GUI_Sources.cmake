# ASWUnitTests_Win32GUI_Sources.cmake
#
# Exports what a project needs to build the Win32 GUI runner (Windows only),
# so a project that adds this repository as a git submodule can run its tests
# in a window without tracking the GUI's filenames by hand across updates:
#
#     ASWUNITTESTS_WIN32_GUI_SOURCES       The framework's core sources (all
#                                          but main.cpp), the GUI runners'
#                                          shared core, this folder's units
#                                          including WinMain, the
#                                          manifest (the .manifest file
#                                          itself for MSVC, or the resource
#                                          script that embeds it for other
#                                          compilers), and
#                                          ASWUnitTests_Icon.rc, which
#                                          embeds the ASWUnitTests icon. A
#                                          project with an icon of its own
#                                          removes that script from the list
#                                          (see the usage below); the window
#                                          shows its icon if it's named
#                                          MAINICON.
#     ASWUNITTESTS_WIN32_GUI_INCLUDE_DIRS  Their directories, for the include
#                                          path (the resource compiler finds
#                                          the icon through it).
#     ASWUNITTESTS_WIN32_GUI_LIBRARIES     The Windows libraries it links.
#     ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS  Linker options it needs: with MSVC,
#                                          leaving the execution level to the
#                                          manifest; with MinGW-w64, marking
#                                          the executable for Windows Vista
#                                          or later, which gives its windows
#                                          the same frame as other programs.
#     ASWUNITTESTS_WIN32_GUI_SOURCE_DIR    This folder.
#     ASWUNITTESTS_RESOURCE_DIR            The repository's resource folder,
#                                          which has the icon.
#
# It includes src/ASWUnitTests_Sources.cmake and
# gui/src/ASWUnitTests_GUI_Sources.cmake, so their variables are also set.
# Include it after project(), which tells it the compiler.
#
# Usage, from the consuming project's own CMakeLists.txt (never edit this
# repository's own cmake/CMakeLists.txt to add your tests):
#
#     include(third_party/asw-unit-tests/win32/gui/src/ASWUnitTests_Win32GUI_Sources.cmake)
#     add_executable(MyTestsGUI WIN32
#         ${ASWUNITTESTS_WIN32_GUI_SOURCES}
#         tests/Test_MyClass.cpp
#     )
#     target_include_directories(MyTestsGUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_INCLUDE_DIRS} tests)
#     target_link_libraries(MyTestsGUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_LIBRARIES})
#     target_link_options(MyTestsGUI PRIVATE ${ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS})
#
# To use an icon of your own instead, before add_executable():
#
#     list(REMOVE_ITEM ASWUNITTESTS_WIN32_GUI_SOURCES "${ASWUNITTESTS_RESOURCE_DIR}/ASWUnitTests_Icon.rc")

include("${CMAKE_CURRENT_LIST_DIR}/../../../src/ASWUnitTests_Sources.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/../../../gui/src/ASWUnitTests_GUI_Sources.cmake")

set(ASWUNITTESTS_WIN32_GUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")
get_filename_component(ASWUNITTESTS_RESOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../resource" ABSOLUTE)

set(ASWUNITTESTS_WIN32_GUI_SOURCES
    ${ASWUNITTESTS_CORE_SOURCES}
    ${ASWUNITTESTS_GUI_SOURCES}
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_DialogTemplate.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_DPI.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_Glyphs.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_MainLayout.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_MainWindow.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_MessageDialog.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_Palette.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_ShuffleSeedDialog.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_Splitter.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_System.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_TextDialog.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_Theme.cpp"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_WinMain.cpp"
    "${ASWUNITTESTS_RESOURCE_DIR}/ASWUnitTests_Icon.rc"
)

set(ASWUNITTESTS_WIN32_GUI_INCLUDE_DIRS
    "${ASWUNITTESTS_SOURCE_DIR}"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}"
    "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}"
    "${ASWUNITTESTS_RESOURCE_DIR}"
)

set(ASWUNITTESTS_WIN32_GUI_LIBRARIES
    comctl32
    comdlg32
    gdi32
    shell32
    user32
    uxtheme
)

set(ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS)

if(MSVC)
    # The linker merges the manifest into the one it generates. /MANIFESTUAC:NO leaves the execution level to
    # the manifest's own trustInfo, rather than adding a second one.
    list(APPEND ASWUNITTESTS_WIN32_GUI_SOURCES
        "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI.manifest")
    list(APPEND ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS /MANIFESTUAC:NO)
else()
    # The manifest as a resource. GCC also links its distribution's default manifest, if it has one; GNU ld
    # drops a language-neutral default one silently, but WinLibs' has a language, so ld reports ".rsrc merge
    # failure: multiple non-default manifests". The link still succeeds and Windows uses this manifest, which
    # comes first.
    list(APPEND ASWUNITTESTS_WIN32_GUI_SOURCES
        "${ASWUNITTESTS_WIN32_GUI_SOURCE_DIR}/ASWUnitTests_Win32GUI_Manifest.rc")
endif()

if(MINGW)
    # GNU ld marks the executable for Windows subsystem 5.02 (XP) unless told otherwise, and Windows draws the
    # windows of an executable older than 6.0 (Vista) without the padded border, so their frames and title bars
    # would be thinner than in the other builds and the VCL GUI runner. lld already uses 6.0.
    list(APPEND ASWUNITTESTS_WIN32_GUI_LINK_OPTIONS
        -Wl,--major-subsystem-version,6
        -Wl,--minor-subsystem-version,0
    )
endif()
