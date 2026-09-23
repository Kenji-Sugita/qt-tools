cmake_minimum_required(VERSION 3.24)

project(drag-and-drop VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(drag-and-drop)
else()
    add_executable(drag-and-drop)
endif()

target_sources(drag-and-drop PRIVATE
    main.cpp
    gizmo.cpp
    gizmo.h
)

target_link_libraries(drag-and-drop PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(drag-and-drop PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
