cmake_minimum_required(VERSION 3.26)

project(messagewidget LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(messagewidget
    main.cpp
    messagewidget.cpp
    messagewidget.h
)

target_link_libraries(messagewidget PRIVATE Qt6::Widgets)

set_target_properties(messagewidget PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
