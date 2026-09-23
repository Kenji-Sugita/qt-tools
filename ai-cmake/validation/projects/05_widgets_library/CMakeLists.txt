cmake_minimum_required(VERSION 3.16)

project(Pattern05WidgetsLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_library(Pattern05WidgetsLibrary
    customwidget.cpp
    customwidget.h
)

target_link_libraries(Pattern05WidgetsLibrary
    PUBLIC
        Qt6::Widgets
)
