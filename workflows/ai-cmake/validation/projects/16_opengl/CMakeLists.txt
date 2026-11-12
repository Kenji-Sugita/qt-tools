cmake_minimum_required(VERSION 3.16)

project(Pattern16OpenGLApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGLWidgets)

qt_standard_project_setup()

qt_add_executable(Pattern16OpenGLApp
    main.cpp
    glwidget.cpp
    glwidget.h
)

target_link_libraries(Pattern16OpenGLApp
    PRIVATE
        Qt6::Widgets
        Qt6::OpenGLWidgets
)

set_target_properties(Pattern16OpenGLApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
