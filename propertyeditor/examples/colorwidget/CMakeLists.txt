cmake_minimum_required(VERSION 3.26)

project(colorwidgetplugin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets UiPlugin)

qt_standard_project_setup()

qt_add_library(colorwidgetplugin MODULE)

target_sources(colorwidgetplugin PRIVATE
    colorwidget.h
    colorwidgetplugin.h
    colorwidget.cpp
    colorwidgetplugin.cpp
    colorwidget.ui
    colorwidget.json
)

target_link_libraries(colorwidgetplugin PRIVATE
    Qt6::Widgets
    Qt6::UiPlugin
)
