cmake_minimum_required(VERSION 3.16)

project(Pattern07Tools VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(Pattern07GuiTool
    gui_main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern07GuiTool
    PRIVATE
        Qt6::Widgets
)

set_target_properties(Pattern07GuiTool PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)

qt_add_executable(Pattern07CliTool
    cli_main.cpp
)

target_link_libraries(Pattern07CliTool
    PRIVATE
        Qt6::Core
)
