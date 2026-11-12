cmake_minimum_required(VERSION 3.16)

project(Qml22QuickControls VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2)

qt_standard_project_setup()



qt_add_executable(Qml22QuickControls
    main.cpp
)

qt_add_qml_module(Qml22QuickControls
    URI Qml22QuickControls
    VERSION 1.0
    QML_FILES
        Main.qml
)

target_link_libraries(Qml22QuickControls
    PRIVATE
        Qt6::Quick
)

set_target_properties(Qml22QuickControls PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
