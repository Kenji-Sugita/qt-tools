cmake_minimum_required(VERSION 3.26)

project(findTool LANGUAGES CXX)

# Windows ではビルドしない
if(WIN32)
    message(STATUS "findTool: skipped on Windows")
    return()
endif()

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(findTool)
else()
    add_executable(findTool)
endif()

target_sources(findTool PRIVATE
    main.cpp
    findDialog.cpp
    findDialog.h
)

target_link_libraries(findTool PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(findTool PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
