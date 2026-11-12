cmake_minimum_required(VERSION 3.26)

project(formcreationpractice LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(formcreationpractice
    main.cpp
    onebutton.ui
    onebuttonwithspacer.ui
    twobuttons.ui
    twobuttonswithgrid.ui
    twobuttonswithspacer.ui
)

target_link_libraries(formcreationpractice PRIVATE Qt6::Widgets)

set_target_properties(formcreationpractice PROPERTIES
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)
