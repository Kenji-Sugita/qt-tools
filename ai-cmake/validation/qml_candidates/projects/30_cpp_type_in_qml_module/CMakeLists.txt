cmake_minimum_required(VERSION 3.16)

project(Qml30CppTypeModule VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick)

qt_standard_project_setup()



qt_add_executable(Qml30CppTypeModule
    main.cpp
    counter.cpp
    counter.h
)

qt_add_qml_module(Qml30CppTypeModule
    URI Qml30CppTypeModule
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml30CppTypeModule
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml30CppTypeModule PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
