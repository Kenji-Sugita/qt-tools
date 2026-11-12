cmake_minimum_required(VERSION 3.26)

project(sif LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui)

set(plugin_sources
    SimpleImageIOHandler.h
    SimpleImagePlugin.h
    SimpleImageIOHandler.cpp
    SimpleImagePlugin.cpp
    sif.json
)

if(QT_VERSION_MAJOR GREATER_EQUAL 6)
    qt_add_plugin(sif
        MODULE
        PLUGIN_TYPE imageformats
    )
else()
    add_library(sif MODULE)
endif()

target_sources(sif PRIVATE
    ${plugin_sources}
)

target_link_libraries(sif PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)
