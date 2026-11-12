cmake_minimum_required(VERSION 3.16)

project(Pattern06AppWithLibrary VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_library(Pattern06AppCore
    appmodel.cpp
    appmodel.h
)

target_link_libraries(Pattern06AppCore
    PUBLIC
        Qt6::Core
)

qt_add_executable(Pattern06App
    main.cpp
    mainwindow.cpp
    mainwindow.h
)

target_link_libraries(Pattern06App
    PRIVATE
        Pattern06AppCore
        Qt6::Widgets
)

set_target_properties(Pattern06App PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
