cmake_minimum_required(VERSION 3.26)

project(transparency LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(transparency)
else()
    add_executable(transparency)
endif()

target_sources(transparency PRIVATE
    main.cpp
    transparency.qrc
)

target_link_libraries(transparency PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(transparency PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
