cmake_minimum_required(VERSION 3.26)

project(completer LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(completer)
else()
    add_executable(completer)
endif()

target_sources(completer PRIVATE
    main.cpp
    countryselector.cpp
    countryselector.h
)

target_link_libraries(completer PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(completer PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
