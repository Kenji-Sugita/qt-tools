cmake_minimum_required(VERSION 3.26)

project(events LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_executable(events)
else()
    add_executable(events)
endif()

target_sources(events PRIVATE
    main.cpp
    fibevent.h
    fibthread.cpp
    fibthread.h
    fibwidget.cpp
    fibwidget.h
)

target_link_libraries(events PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

set_target_properties(events PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
