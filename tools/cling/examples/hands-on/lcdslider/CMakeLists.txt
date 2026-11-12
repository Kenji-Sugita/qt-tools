cmake_minimum_required(VERSION 3.26)

project(lcdslider LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(lcdslider
    main.cpp
    lcdslider.cpp
    lcdslider.h
)

target_link_libraries(lcdslider PRIVATE Qt6::Widgets)

set_target_properties(lcdslider PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
