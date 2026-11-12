cmake_minimum_required(VERSION 3.16)

project(Pattern02QuickApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()

qt_add_executable(Pattern02QuickApp
    main.cpp
)

qt_add_qml_module(Pattern02QuickApp
    URI Pattern02QuickApp
    VERSION 1.0
    QML_FILES
        Main.qml
    RESOURCES
        images/logo.txt
)

target_link_libraries(Pattern02QuickApp
    PRIVATE
        Qt6::Quick
)

set_target_properties(Pattern02QuickApp PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
