cmake_minimum_required(VERSION 3.16)

project(Pattern12ResourceApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(Pattern12ResourceApp
    main.cpp mainwindow.cpp mainwindow.h
)


qt_add_resources(Pattern12ResourceApp "app_resources"
    PREFIX "/"
    FILES
        icons/app.txt
        images/splash.txt
)


target_link_libraries(Pattern12ResourceApp
    PRIVATE
        Qt6::Widgets
)

set_target_properties(Pattern12ResourceApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
