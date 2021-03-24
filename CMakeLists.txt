cmake_minimum_required(VERSION 3.10)

project(QtOpenCV)

add_library(${PROJECT_NAME} cvmatandqimage.cpp)

find_package(
  Qt5
  COMPONENTS Core Gui
  REQUIRED)
find_package(OpenCV REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Core Qt5::Gui ${OpenCV_LIBS})

include(GNUInstallDirs)
target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
install(FILES ${CMAKE_CURRENT_LIST_DIR}/cvmatandqimage.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib/static)
install(EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
install(FILES ${CMAKE_CURRENT_LIST_DIR}/cmake/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
