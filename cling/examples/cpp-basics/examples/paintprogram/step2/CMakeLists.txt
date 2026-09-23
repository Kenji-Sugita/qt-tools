cmake_minimum_required(VERSION 3.26)

project(step2 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(step2)
else()
    add_executable(step2)
endif()

target_sources(step2 PRIVATE
    main.cpp
    paintwindow.cpp
    paintwindow.h
    scribblearea.cpp
    scribblearea.h
)

target_link_libraries(step2 PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(step2 PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
