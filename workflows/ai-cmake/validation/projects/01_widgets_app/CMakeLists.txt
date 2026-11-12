cmake_minimum_required(VERSION 3.16)

project(Pattern01WidgetsApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(Pattern01WidgetsApp
    main.cpp mainwindow.cpp mainwindow.h
)



target_link_libraries(Pattern01WidgetsApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(Pattern01WidgetsApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
