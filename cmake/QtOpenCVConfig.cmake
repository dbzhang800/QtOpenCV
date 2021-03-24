include(CMakeFindDependencyMacro)

find_dependency(Qt5 COMPONENTS Core Gui)
find_dependency(OpenCV)

include(${CMAKE_CURRENT_LIST_DIR}/QtOpenCVTargets.cmake)
