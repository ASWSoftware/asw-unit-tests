# ASWUnitTests_GUI_Sources.cmake
#
# Exports the source files of the GUI runners' shared core, the units in this
# folder, which need neither Windows nor the VCL, as the
# ASWUNITTESTS_GUI_SOURCES list, and their directory as
# ASWUNITTESTS_GUI_SOURCE_DIR. A GUI runner compiles them together with the
# framework's core sources (src/ASWUnitTests_Sources.cmake) and its own
# units; win32/gui/src/ASWUnitTests_Win32GUI_Sources.cmake includes this
# file, so a project building the Win32 GUI runner needn't include it itself.

set(ASWUNITTESTS_GUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(ASWUNITTESTS_GUI_SOURCES
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_CommandLine.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Commands.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_INIDocument.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Layout.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_RunObserver.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Selection.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Shuffle.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_TestList.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Theme.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_Tree.cpp"
    "${ASWUNITTESTS_GUI_SOURCE_DIR}/ASWUnitTests_GUI_View.cpp"
)
