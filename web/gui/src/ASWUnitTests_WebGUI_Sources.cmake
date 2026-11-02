# ASWUnitTests_WebGUI_Sources.cmake
#
# Exports the source files of the Web GUI runner's core, its units in this
# folder that need no sockets (JSON, HTTP, events, security checks, the
# page's API), as the ASWUNITTESTS_WEB_GUI_CORE_SOURCES list, and their
# directory as ASWUNITTESTS_WEB_GUI_SOURCE_DIR. This repository compiles
# them, and their tests, in every runner, so the tests run on every
# platform.

set(ASWUNITTESTS_WEB_GUI_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}")

set(ASWUNITTESTS_WEB_GUI_CORE_SOURCES
    "${ASWUNITTESTS_WEB_GUI_SOURCE_DIR}/ASWUnitTests_WebGUI_HTTP.cpp"
    "${ASWUNITTESTS_WEB_GUI_SOURCE_DIR}/ASWUnitTests_WebGUI_JSON.cpp"
    "${ASWUNITTESTS_WEB_GUI_SOURCE_DIR}/ASWUnitTests_WebGUI_Security.cpp"
    "${ASWUNITTESTS_WEB_GUI_SOURCE_DIR}/ASWUnitTests_WebGUI_SSE.cpp"
)
