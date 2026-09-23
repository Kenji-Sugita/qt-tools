cmake_minimum_required(VERSION 3.16)

project(Qml25CppBackend VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Qml)

qt_standard_project_setup()



qt_add_executable(Qml25CppBackend
    main.cpp
    backend.cpp
    backend.h
)

qt_add_qml_module(Qml25CppBackend
    URI Qml25CppBackend
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml25CppBackend
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml25CppBackend PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
